//
// Created by kuiper on 2021/2/12.
//

#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <sys/time.h>

#include "leveldb/env.h"
#include "port/port.h"
#include "util/mutexlock.h"
#include "util/no_destructor.h"

namespace leveldb {

    namespace {

        int g_open_read_only_file_limit = -1;

        constexpr const int kDefaultMmapLimit = (sizeof(void *) >= 8) ? 1000 : 0;

        int g_mmap_limit = kDefaultMmapLimit;

#if defined(HAVE_O_CLOEXEC)
        constexpr const int kOpenBaseFlags = O_CLOEXEC;
#else
        constexpr const int kOpenBaseFlags = 0;
#endif

        constexpr const size_t kWritableFileBufferSize = 65536;

        Status PosixError(const std::string &context, int errno_number) {
            if (errno_number == ENOENT) {
                return Status::NotFound(context, std::strerror(errno_number));
            } else {
                return Status::IOError(context, std::strerror(errno_number));
            }
        }


        class Limiter {
        public:
            explicit Limiter(int max_acquires) :
                    acquires_allowed_(max_acquires) {}

            Limiter(const Limiter &) = delete;

            Limiter &operator=(const Limiter &) = delete;

            bool Acquire() {
                auto old = acquires_allowed_.fetch_sub(1, std::memory_order_relaxed);
                if (old > 0) {
                    return true;
                }
                acquires_allowed_.fetch_add(1, std::memory_order_relaxed);
                return false;
            }

            void Release() {
                acquires_allowed_.fetch_add(1, std::memory_order_relaxed);
            }

        private:
            std::atomic<int> acquires_allowed_;
        };

        class PosixSequentialFile final : public SequentialFile {
        public:
            PosixSequentialFile(std::string filename, int fd)
                    : fd_(fd), filename_(std::move(filename)) {}

            ~PosixSequentialFile() override {
                close(fd_);
            }

            Status Read(size_t n, Slice *result, char *scratch) override {
                Status status;
                while (true) {
                    ::ssize_t nRead = ::read(fd_, scratch, n);
                    if (nRead < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        status = PosixError(filename_, errno);
                        break;
                    }
                    *result = Slice(scratch, nRead);
                    break;
                }
                return status;
            }

            Status Skip(uint64_t n) override {
                if (::lseek(fd_, n, SEEK_CUR) == static_cast<off_t>(-1)) {
                    return PosixError(filename_, errno);
                }
                return Status::OK();
            }

        private:
            const int fd_;
            const std::string filename_;
        };


        class PosixRandomAccessFile final : public RandomAccessFile {
        public:

            PosixRandomAccessFile(std::string filename, int fd, Limiter *fd_limiter)
                    : has_permanent_fd_(fd_limiter->Acquire()),
                      fd_(has_permanent_fd_ ? fd : -1),
                      fd_limiter_(fd_limiter),
                      filename_(std::move(filename)) {
                if (!has_permanent_fd_) {
                    assert(fd_ == -1);
                    ::close(fd); // 每次读的时候open.
                }
            }

            ~PosixRandomAccessFile() override {
                if (has_permanent_fd_) {
                    assert(fd_ != -1);
                    ::close(fd_);
                    fd_limiter_->Release();
                }
            }

            Status Read(uint64_t offset, size_t n, Slice *result, char *scratch) const override {
                int fd = fd_;
                if (!has_permanent_fd_) {
                    fd = ::open(filename_.c_str(), O_RDONLY | kOpenBaseFlags);
                    if (fd < 0) {
                        if (fd < 0) {
                            return PosixError(filename_, errno);
                        }
                    }
                }
                assert(fd != -1);

                Status status;
                ssize_t nRead = ::pread(fd, scratch, n, static_cast<off_t>(offset));
                *result = Slice(scratch, (nRead < 0) ? 0 : nRead);
                if (nRead < 0) {
                    status = PosixError(filename_, errno);
                }

                if (!has_permanent_fd_) {
                    assert(fd != fd_);
                    ::close(fd);
                }
                return status;
            }

        private:
            const int fd_;
            const std::string filename_;
            // 是否是永久的fd, 相反的则为临时fd 临时的fd在read时才open/close
            // 是永久or临时取决于Limiter
            const bool has_permanent_fd_;
            Limiter *const fd_limiter_;
        };


        class PosixMmapReadableFile final : public RandomAccessFile {
        public:
            PosixMmapReadableFile(std::string filename, char *mmap_base, size_t length, Limiter *mmap_limiter)
                    : mmap_base_(mmap_base),
                      length_(length),
                      mmap_limiter_(mmap_limiter),
                      filename_(std::move(filename)) {
            }

            ~PosixMmapReadableFile() override {
                ::munmap(static_cast<void *>(mmap_base_), length_);
                mmap_limiter_->Release();
            }

            Status Read(uint64_t offset, size_t n, Slice *result, char *scratch) const override {
                if (offset + n > length_) {
                    *result = Slice();
                    return PosixError(filename_, EINVAL);
                }

                *result = Slice(mmap_base_ + offset, n);
                return Status::OK();
            }

        private:
            char *const mmap_base_;
            const size_t length_;
            Limiter *const mmap_limiter_;
            const std::string filename_;
        };

        //
        // PosixWritableFile
        //
        class PosixWritableFile final : public WritableFile {
        public:

            PosixWritableFile(std::string filename, int fd)
                    : pos_(0),
                      fd_(fd),
                      is_manifest_(IsManifest(filename)),
                      dirname_(Dirname(filename)),
                      filename_(std::move(filename)) {
            }

            ~PosixWritableFile() override {
                if (fd_ >= 0) {
                    Close();
                }
            }

            Status Append(const Slice &data) override {
                size_t write_size = data.size();
                const char *write_data = data.data();

                // 先写入缓冲区
                auto copy_size = std::min(write_size, kWritableFileBufferSize - pos_);
                std::memcpy(buf_ + pos_, write_data, copy_size);
                write_data += copy_size;
                write_size -= copy_size;
                pos_ += copy_size;

                // data全部写进缓冲区了则直接返回成功
                if (write_size == 0) {
                    return Status::OK();
                }

                Status status = FlushBuffer();
                if (!status.IsOK()) {
                    return status;
                }

                // 缓冲区清空后, 如果待写入的数据小于缓冲区，直接写入缓冲区
                if (write_size < kWritableFileBufferSize) {
                    std::memcpy(buf_, write_data, write_size);
                    pos_ = write_size;
                    return Status::OK();
                }

                // 如果比缓冲区大，直接刷入磁盘
                return WriteUnbuffered(write_data, write_size);
            }

            Status Close() override {
                Status status = FlushBuffer();
                const int close_result = ::close(fd_);
                if (close_result < 0 && status.IsOK()) {
                    status = PosixError(filename_, errno);
                }
                fd_ = -1;
                return status;
            }

            Status Flush() override {
                return FlushBuffer();
            }

            Status Sync() override {
                Status status = SyncDirIfManifest();
                if (!status.IsOK()) {
                    return status;
                }

                status = FlushBuffer();
                if (!status.IsOK()) {
                    return status;
                }

                return SyncFd(fd_, filename_);
            }


        private:

            // @brief 将缓冲区的数据进行write。
            // @brief 很可能只写入了文件系统的缓冲区, 并没刷入磁盘.
            Status FlushBuffer() {
                Status status = WriteUnbuffered(buf_, pos_);
                if (status.IsOK()) {
                    pos_ = 0;
                }
                return status;
            }

            // @brief
            // @param
            // @param
            // @return
            Status WriteUnbuffered(const char *data, size_t size) {
                while (size > 0) {
                    ssize_t write_result = ::write(fd_, data, size);
                    if (write_result < 0) {
                        if (errno == EINTR) {
                            continue;
                        }
                        return PosixError(filename_, errno);
                    }
                    data += write_result;
                    size -= write_result;
                }
                return Status::OK();
            }

            Status SyncDirIfManifest() {
                Status status;
                if (!is_manifest_) {
                    return status;
                }
                int fd = ::open(dirname_.c_str(), O_RDONLY | kOpenBaseFlags);
                if (fd < 0) {
                    status = PosixError(dirname_, errno);
                } else {
                    status = SyncFd(fd, dirname_);
                    ::close(fd);
                }
                return status;
            }

            static Status SyncFd(int fd, const std::string &fd_path) {
                // MacOsX和ios上使用fcntl
#if  defined(F_FULLFSYNC)
                if (::fcntl(fd, F_FULLFSYNC) == 0) {
                    return Status::OK();
                }
#endif
#if HAVE_FDATASYNC
                bool sync_success = ::fdatasync(fd) == 0;
#else
                bool sync_success = ::fsync(fd);
#endif
                if (sync_success) {
                    return Status::OK();
                }
                return PosixError(fd_path, errno);
            }


            static std::string Dirname(const std::string &filename) {
                std::string::size_type separator_pos = filename.rfind('/');
                if (separator_pos == std::string::npos) {
                    return std::string(".");
                }
                assert(filename.find('/', separator_pos + 1) == std::string::npos);
                return filename.substr(0, separator_pos);
            }

            static Slice Basename(const std::string &filename) {
                std::string::size_type separator_pos = filename.rfind('/');
                if (separator_pos == std::string::npos) {
                    return Slice(filename);
                }
                assert(filename.find('/', separator_pos + 1) == std::string::npos);
                return Slice(filename.data() + separator_pos + 1, filename.length() - separator_pos - 1);
            }

            static bool IsManifest(const std::string &filename) {
                return Basename(filename).starts_with("MANIFEST");
            }

        private:
            char buf_[kWritableFileBufferSize]{};
            size_t pos_;
            int fd_;

            const bool is_manifest_;
            const std::string filename_;
            const std::string dirname_;
        };

        int LockOrUnlock(int fd, bool lock) {
            errno = 0;
            struct ::flock file_lock_info{};
            std::memset(&file_lock_info, 0, sizeof(file_lock_info));
            file_lock_info.l_type = lock ? F_WRLCK : F_UNLCK;
            file_lock_info.l_whence = SEEK_SET;
            file_lock_info.l_start = 0;
            file_lock_info.l_len = 0; // until end of file
            return ::fcntl(fd, F_SETLK, &file_lock_info);
        }

        class PosixFileLock : public FileLock {
        public:
            PosixFileLock(int fd, std::string filename)
                    : fd_(fd),
                      filename_(std::move(filename)) {}

            NO_DISCARD
            int fd() const { return fd_; }

            NO_DISCARD
            const std::string &filename() const { return filename_ ;};
        private:
            const int fd_;
            const std::string filename_;
        };

        class PosixLockTable {
        public:
            bool Insert(const std::string &fname) {
                MutexLock guard(&mu_);
                return locked_files_.insert(fname).second;
            }

            void Remove(const std::string &fname) {
                MutexLock guard(&mu_);
                locked_files_.erase(fname);
            }

        private:
            port::Mutex mu_;
            std::set<std::string> locked_files_;
        };

        // !!!! POSIX ENV implementation !!!!
        class PosixEnv : public Env {
        public:
            PosixEnv();

            ~PosixEnv() override {
                static const char msg[] =
                        "PosixEnv singleton destroyed. Unsupported behavior!\n";
                std::fwrite(msg, 1, sizeof(msg), stderr);
                std::abort();
            }

            Status NewSequentialFile(const std::string &fname, SequentialFile **result) override {
                int fd = ::open(fname.c_str(), O_RDONLY | kOpenBaseFlags);
                if (fd < 0) {
                    *result = nullptr;
                    return PosixError(fname, errno);
                }
                *result = new PosixSequentialFile(fname, fd);
                return Status::OK();
            }

            Status NewRandomAccessFile(const std::string &fname, RandomAccessFile **result) override {
                *result = nullptr;
                int fd = ::open(fname.c_str(), O_RDONLY | kOpenBaseFlags);
                if (fd < 0) {
                    return PosixError(fname, errno);
                }

                // 优先使用mmap
                if (!mmap_limiter_.Acquire()) {
                    *result = new PosixRandomAccessFile(fname, fd, &fd_limiter_);
                    return Status::OK();
                }

                // FIXME file_size=0时会导致mmap failed.
                uint64_t file_size;
                Status status = GetFileSize(fname, &file_size);
                if (status.IsOK()) {
                    void *mmap_base = ::mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
                    if (mmap_base != MAP_FAILED) {
                        *result = new PosixMmapReadableFile(fname, reinterpret_cast<char *>(mmap_base), file_size,
                                                            &mmap_limiter_);
                    } else {
                        status = PosixError(fname, errno);
                    }
                }

                ::close(fd);
                if (!status.IsOK()) {
                    mmap_limiter_.Release();
                }
                return status;
            }

            Status NewWritableFile(const std::string &fname, WritableFile **result) override {
                int fd = ::open(fname.c_str(), O_TRUNC | O_WRONLY | O_CREAT | kOpenBaseFlags, 0644);
                if (fd < 0) {
                    *result = nullptr;
                    return PosixError(fname, errno);
                }
                *result = new PosixWritableFile(fname, fd);
                return Status::OK();
            }

            Status NewAppendableFile(const std::string &fname, WritableFile **result) override {
                int fd = ::open(fname.c_str(),
                                O_APPEND | O_WRONLY | O_CREAT | kOpenBaseFlags, 0644);
                if (fd < 0) {
                    *result = nullptr;
                    return PosixError(fname, errno);
                }

                *result = new PosixWritableFile(fname, fd);
                return Status::OK();
            }

            bool FileExists(const std::string &fname) override {
                return ::access(fname.c_str(), F_OK) == 0;
            }

            Status GetChildren(const std::string &dir_path, std::vector<std::string> *result) override {
                result->clear();
                ::DIR *dir = ::opendir(dir_path.c_str());
                if (dir == nullptr) {
                    return PosixError(dir_path, errno);
                }

                struct ::dirent *entry;
                while ((entry = ::readdir(dir)) != nullptr) {
                    result->emplace_back(entry->d_name);
                }

                ::closedir(dir);
                return Status::OK();
            }

            Status RemoveFile(const std::string &filename) override {
                if (::unlink(filename.c_str()) != 0) {
                    return PosixError(filename, errno);
                }
                return Status::OK();
            }

            Status CreateDir(const std::string &dirname) override {
                if (::mkdir(dirname.c_str(), 0755) != 0) {
                    return PosixError(dirname, errno);
                }
                return Status::OK();
            }

            Status RemoveDir(const std::string &dirname) override {
                if (::rmdir(dirname.c_str()) != 0) {
                    return PosixError(dirname, errno);
                }
                return Status::OK();
            }

            Status GetFileSize(const std::string &fname, uint64_t *file_size) override {
                struct ::stat file_stat{};
                if (::stat(fname.c_str(), &file_stat) != 0) {
                    *file_size = 0;
                    return PosixError(fname, errno);
                }
                *file_size = file_stat.st_size;
                return Status::OK();
            }

            Status RenameFile(const std::string &src, const std::string &target) override {
                if (std::rename(src.c_str(), target.c_str()) != 0) {
                    return PosixError(src, errno);
                }
                return Status::OK();
            }

            Status LockFile(const std::string &fname, FileLock **lock) override {
                *lock = nullptr;

                int fd = ::open(fname.c_str(), O_RDWR | O_CREAT | kOpenBaseFlags, 0644);
                if (fd < 0) {
                    return PosixError(fname, errno);
                }

                if (!locks_.Insert(fname)) {
                    ::close(fd);
                    return Status::IOError("lock" + fname, "already held by process");
                }

                if (LockOrUnlock(fd, true) == -1) {
                    int lock_errno = errno;
                    ::close(fd);
                    locks_.Remove(fname);
                    return PosixError("lock" + fname, lock_errno);
                }

                *lock = new PosixFileLock(fd, fname);
                return Status::OK();
            }

            Status UnlockFile(FileLock *lock) override {
                auto *posixFileLock = dynamic_cast<PosixFileLock *>(lock);
                if (LockOrUnlock(posixFileLock->fd(), false) == -1) {
                    return PosixError("unlock " + posixFileLock->filename(), errno);
                }
                locks_.Remove(posixFileLock->filename());
                ::close(posixFileLock->fd());
                delete posixFileLock;
                return Status::OK();
            }

            void Schedule(void (*func)(void *), void *arg) override;

            void StartThread(void (*func)(void *), void *arg) override {
                std::thread thr(func, arg);
                thr.detach();
            }

            Status GetTestDirectory(std::string *path) override {
                return Status::OK();
            }

            Status NewLogger(const std::string &fname, Logger **result) override {
                return Status::OK();
            }

            uint64_t NowMicros() override {
                static constexpr uint64_t kUsecondsPerSecond = 1000000;
                struct ::timeval tv{};
                ::gettimeofday(&tv, nullptr);
                return static_cast<uint64_t>(tv.tv_sec) * kUsecondsPerSecond + tv.tv_usec;
            }

            void SleepForMicroseconds(int micros) override {
                std::this_thread::sleep_for(std::chrono::microseconds(micros));
            }

        private:
            void BackgroundThreadMain();

            static void BackgroundThreadEntryPoint(PosixEnv *env) {
                env->BackgroundThreadMain();
            }

            struct BackgroundWorkItem {
                explicit BackgroundWorkItem(void (*func)(void *arg), void *a)
                        : function(func), arg(a) {}

                void (*const function)(void *);

                void *const arg;
            };

            port::Mutex background_work_mutex_;
            port::CondVar background_work_cv_;                          // guarded by background_work_mutex_
            bool started_background_thread_;                            // guarded by background_work_mutex_
            std::queue<BackgroundWorkItem> background_work_queue_;      // guarded by background_work_mutex_

            PosixLockTable locks_;   // Thread-safe
            Limiter mmap_limiter_;  // Thread-safe
            Limiter fd_limiter_;    // Thread-safe
        };


        int MaxMmaps() { return g_mmap_limit; }

        int MaxOpenFiles() {
            if (g_open_read_only_file_limit >= 0) {
                return g_open_read_only_file_limit;
            }
            struct ::rlimit rlim{};
            if (::getrlimit(RLIMIT_NOFILE, &rlim)) {
                // getrlimit failed, fallback to hard-coded default.
                g_open_read_only_file_limit = 50;
            } else if (rlim.rlim_cur == RLIM_INFINITY) {
                g_open_read_only_file_limit = std::numeric_limits<int>::max();
            } else {
                // Allow use of 20% of available file descriptors for read-only files.
                g_open_read_only_file_limit = rlim.rlim_cur / 5;
            }
            return g_open_read_only_file_limit;
        }

        PosixEnv::PosixEnv()
                : background_work_cv_(&background_work_mutex_),
                  started_background_thread_(false),
                  mmap_limiter_(MaxMmaps()),
                  fd_limiter_(MaxOpenFiles()) {}

        void PosixEnv::Schedule(void (*func)(void *), void *arg) {
            background_work_mutex_.Lock();

            if (!started_background_thread_) {
                started_background_thread_ = true;
                std::thread background_thread(&BackgroundThreadEntryPoint, this);
                background_thread.detach();
            }

            background_work_queue_.emplace(func, arg);
            background_work_mutex_.Unlock();
            background_work_cv_.Signal();

        }

        void PosixEnv::BackgroundThreadMain() {
            for (;;) {
                background_work_mutex_.Lock();
                while (background_work_queue_.empty()) {
                    background_work_cv_.Wait();
                }
                assert(!background_work_queue_.empty());
                auto func = background_work_queue_.front().function;
                auto arg = background_work_queue_.front().arg;
                background_work_queue_.pop();

                background_work_mutex_.Unlock();
                // start routine
                func(arg);
            }
        }


    } // end of namespace {

    Env *Env::Default() {
        static NoDestructor<PosixEnv> envContainer;
        return envContainer.get();
    }

}