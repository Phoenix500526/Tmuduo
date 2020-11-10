#include <stdio.h>
#include "net/EventLoop.h"
using namespace tmuduo;
using namespace tmuduo::net;

void print() { printf("hello world\n"); }

int main() {
  EventLoop loop;
  loop.runAfter(5, print);
  loop.loop();
}