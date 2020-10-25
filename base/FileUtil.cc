#include "base/FileUtil.h"
#include "base/Logging.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace tmuduo;

FileUtil::AppendFile::AppendFile(StringArg filename)
    : fp_(::fopen(filename.c_str(), "ae")), writtenBytes_(0) {
  assert(fp_);
  ::setbuffer(fp_, buffer_, sizeof buffer_);
}

FileUtil::AppendFile::~AppendFile() { ::fclose(fp_); }

void FileUtil::AppendFile::append(const char* logline, const size_t len) {
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    if (0 == x) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
      }
      break;
    }
    n += x;
    remain = len - n;
  }
  writtenBytes_ += len;
}

void FileUtil::AppendFile::flush() { ::fflush(fp_); }

size_t FileUtil::AppendFile::write(const char* logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

//注意这里有一个小 tips：fopen 和 open 之间的区别？
//当需要顺序访问文件的情况下，fopen 由于可以支持缓冲，其性能表现会比 open
//要更加良好
//可是如果需要随机访问文件，或者使用 seek
//等功能，使用缓冲的性能就会大大下降，此时使用 open 就会更加合理。
//友情提示：使用 fopen 配合缓冲时，不要忘记在必要的时候使用 fflush 刷新缓冲
FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
    : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)), err_(0) {
  buf_[0] = '\0';
  if (fd_ < 0) {
    err_ = errno;
  }
}

FileUtil::ReadSmallFile::~ReadSmallFile() {
  if (fd_ >= 0) {
    ::close(fd_);
  }
}

template <typename String>
int FileUtil::ReadSmallFile::readToString(int maxSize, String* content,
                                          int64_t* fileSize,
                                          int64_t* modifyTime,
                                          int64_t* createTime) {
  static_assert(8 == sizeof(off_t), "_FILE_OFFSET_BITS = 64");
  assert(content != nullptr);
  int err = err_;
  if (fd_ >= 0) {
    content->clear();
    if (fileSize) {
      struct stat statbuf;
      if (0 == ::fstat(fd_, &statbuf)) {
        if (S_ISREG(statbuf.st_mode)) {
          *fileSize = statbuf.st_size;
          content->reserve(static_cast<int>(
              std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
        } else if (S_ISDIR(statbuf.st_mode)) {
          err = EISDIR;
        }
        if (modifyTime) {
          *modifyTime = statbuf.st_mtime;
        }
        if (createTime) {
          *createTime = statbuf.st_ctime;
        }
      } else {
        err = errno;
      }
    }
  }
  while (content->size() < implicit_cast<size_t>(maxSize)) {
    size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(),
                             sizeof(buf_));
    ssize_t n = ::read(fd_, buf_, toRead);
    if (n > 0) {
      content->append(buf_, n);
    } else {
      if (n < 0) {
        err = errno;
      }
      break;
    }
  }
  return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int* size) {
  int err = err_;
  if (fd_ >= 0) {
    ssize_t n = ::pread(fd_, buf_, sizeof(buf_) - 1, 0);
    if (n >= 0) {
      if (size) {
        *size = static_cast<int>(n);
      }
      buf_[n] = '\0';
    } else {
      err = errno;
    }
  }
  return err;
}

//模板特例化，省略了 <> 符号
template int FileUtil::readFile(StringArg filename, int maxSize,
                                std::string* content, int64_t*, int64_t*,
                                int64_t*);

template int FileUtil::ReadSmallFile::readToString(int maxSize,
                                                   std::string* content,
                                                   int64_t*, int64_t*,
                                                   int64_t*);