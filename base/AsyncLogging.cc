#include "base/AsyncLogging.h"
#include "base/LogFile.h"
#include "base/Timestamp.h"

#include <stdio.h>

using namespace tmuduo;

AsyncLogging::AsyncLogging(const std::string& basename, off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval),
      running_(true),
      basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_() {
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len) {
  UniqueLock lock(mutex_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(currentBuffer_));
    if (nextBuffer_) {
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      currentBuffer_.reset(new Buffer);
    }
    currentBuffer_->append(logline, len);
    cond_.notify_one();
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  LogFile output(basename_, rollSize_, false);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_) {
    assert((newBuffer1 && (0 == newBuffer1->length())));
    assert((newBuffer2 && (0 == newBuffer2->length())));
    assert(buffersToWrite.empty());
    {
      UniqueLock lock(mutex_);
      if (buffers_.empty()) {
        cond_.wait_for(lock, flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      //采用 swap 的方式，将数据移至局部对象 buffersToWrite
      //中，这样在处理的时候不会产生竞态
      //问题，而且缩短了临界区。这同样也是为了避免日志前端的等待
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }
    assert(!buffersToWrite.empty());
    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf,
               "Dropped log messages at %s, %zd large buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite) {
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}