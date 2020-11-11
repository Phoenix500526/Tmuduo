#ifndef TMUDUO_EXAMPLES_HUB_CODEC_H_
#define TMUDUO_EXAMPLES_HUB_CODEC_H_

#include "base/TypeCast.h"
#include "net/Buffer.h"

namespace pubsub {

enum class ParseResult {
  kError,
  kSuccess,
  kContinue,
};

ParseResult parseMessage(tmuduo::net::Buffer* buf, std::string* cmd,
                         std::string* topic, std::string* content);
}  // namespace pubsub

#endif  // TMUDUO_EXAMPLES_HUB_CODEC_H_