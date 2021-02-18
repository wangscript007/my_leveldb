//
// Created by kuiper on 2021/2/12.
//

#include "leveldb/env.h"
#include <cstdarg>

#if defined(_WIN32) && defined(LEVELDB_DELETEFILE_UNDEFINED)
#undef DeleteFile
#endif


namespace leveldb {

    Env::Env() = default;

    Env::~Env() = default;

    Status Env::NewAppendableFile(const std::string &fname, WritableFile **result) {
        return Status::NotSupported("NewAppendableFile", fname);
    }

    Status Env::RemoveDir(const std::string &dirname) {
        return DeleteDir(dirname);
    }

    Status Env::DeleteDir(const std::string &dirname) {
        return RemoveDir(dirname);
    }

    Status Env::RemoveFile(const std::string &fname) {
        return DeleteFile(fname);
    }

    Status Env::DeleteFile(const std::string &fname) {
        return RemoveFile(fname);
    }

    static Status DoWriteStringToFile(Env *env, const Slice &data, const std::string &fname, bool should_sync) {
        WritableFile *wf = nullptr;
        Status s = env->NewWritableFile(fname, &wf);
        if (!s.IsOK()) {
            return s;
        }
        s = wf->Append(data);
        if (s.IsOK() && should_sync) {
            s = wf->Sync();
        }
        if (s.IsOK()) {
            s = wf->Close();
        }
        delete wf;
        if (!s.IsOK()) {
            env->RemoveFile(fname);
        }
        return s;
    }

    Status WriteStringToFile(Env *env, const Slice &data, const std::string &fname) {
        return DoWriteStringToFile(env, data, fname, false);
    }

    Status WriteStringToFileSync(Env *env, const Slice &data, const std::string &fname) {
        return DoWriteStringToFile(env, data, fname, true);
    }

    // FIXME ReadFileToString
}