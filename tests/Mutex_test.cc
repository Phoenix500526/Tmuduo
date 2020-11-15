#include "base/Mutex.h"
#include <chrono>
#include <thread>
#include <vector>

using namespace tmuduo;
using namespace std;

Mutex g_mutex;
vector<int> g_vec;
const int kCount = 10 * 1000 * 1000;
int g_count = 0;

void threadFunc() {
  for (int i = 0; i < kCount; ++i) {
    UniqueLock lock(g_mutex);
    g_vec.push_back(i);
  }
}

//显式声明 foo 为非内联函数,因为一旦对 foo 进行内联，就会影响 UniqueLock
//的作用域
int foo() __attribute__((noinline));
int foo() {
  UniqueLock lock(g_mutex);
  if (!g_mutex.isLockedByThisThread()) {
    printf("FAIL\n");
    return -1;
  }
  ++g_count;
  return 0;
}

int main(void) {
  using millis = chrono::milliseconds;
  using std_time = chrono::steady_clock::time_point;
  foo();
  if (g_count != 1) {
    printf("foo calls twice\n");
    abort();
  }
  const int kMaxThreads = 8;
  g_vec.reserve(kMaxThreads * kCount);
  std_time start = chrono::steady_clock::now();
  for (int i = 0; i < kCount; ++i) {
    g_vec.push_back(i);
  }
  std_time end = chrono::steady_clock::now();
  millis end_sub_start = chrono::duration_cast<millis>(end - start);
  printf("single thread without lock %ld ms\n", end_sub_start.count());
  start = chrono::steady_clock::now();
  threadFunc();
  end = chrono::steady_clock::now();
  end_sub_start = chrono::duration_cast<millis>(end - start);
  printf("single thread with lock %ld ms\n", end_sub_start.count());
  for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads) {
    vector<thread> pools;
    g_vec.clear();
    start = chrono::steady_clock::now();
    for (int i = 0; i < nthreads; ++i) {
      pools.emplace_back(threadFunc);
    }
    for (int i = 0; i < nthreads; ++i) {
      pools[i].join();
    }
    end = chrono::steady_clock::now();
    end_sub_start = chrono::duration_cast<millis>(end - start);
    printf("%d thread(s) with lock %ld ms\n", nthreads, end_sub_start.count());
  }
  return 0;
}