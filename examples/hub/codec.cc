#include "examples/hub/codec.h"

using namespace tmuduo;
using namespace tmuduo::net;
using namespace pubsub;
using std::string;

ParseResult pubsub::parseMessage(Buffer* buf, string* cmd, string* topic,
                                 string* content) {
  ParseResult result = ParseResult::kError;
  const char* crlf = buf->findCRLF();
  if (crlf) {
    const char* space = std::find(buf->peek(), crlf, ' ');
    if (space != crlf) {
      cmd->assign(buf->peek(), space);
      topic->assign(space + 1, crlf);
      if (*cmd == "pub") {
        const char* start = crlf + 2;
        crlf = buf->findCRLF(start);
        if (crlf) {
          content->assign(start, crlf);
          buf->retrieveUntil(crlf + 2);
          result = ParseResult::kSuccess;
        } else {
          result = ParseResult::kContinue;
        }
      } else {
        buf->retrieveUntil(crlf + 2);
        result = ParseResult::kSuccess;
      }
    } else {
      result = ParseResult::kError;
    }
  } else {
    result = ParseResult::kContinue;
  }
  return result;
}