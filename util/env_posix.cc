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

#include "leveldb/env.h"

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

                return Status::OK();
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
                // MacOsX上使用fcntl
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

            }

            static Slice Basename(const std::string &filename) {

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

    } // endof namespace {

}