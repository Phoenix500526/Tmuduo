#include <stdio.h>
#include "net/EventLoop.h"
using namespace tmuduo;
using namespace tmuduo::net;

void print(EventLoop* loop, int* count) {
  if (*count < 5) {
    printf("count = %d\n", ++(*count));
  } else {
    loop->quit();
  }
}

int main() {
  EventLoop loop;
  int count = 0;
  loop.runEvery(1, std::bind(print, &loop, &count));
  loop.loop();
}