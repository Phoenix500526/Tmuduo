#include "base/SyncQueue.h"
#include "base/Logging.h"
#include "base/Thread.h"

using namespace tmuduo;
using TaskQueue = SyncQueue<Thread::ThreadFunc>;

void takeTest(TaskQueue& queue_) {
  Thread::ThreadFunc task;
  queue_.take(task);
  if (task) {
    task();
  }
  CurrentThread::sleepUsec(1000 * 1000);
}

void run(int id) { LOG_INFO << "task " << id << " is running"; }

void putTest_Lvalue(TaskQueue& queue_, int id) {
  Thread::ThreadFunc task = std::bind(&run, id);
  queue_.put(task);
}

void putTest_Rvalue(TaskQueue& queue_, int id) {
  queue_.put(std::bind(&run, id));
}

void RunInProducerL(TaskQueue& queue_, int beg, int end) {
  for (int i = beg; i < end; ++i) {
    LOG_INFO << "Adding task " << i;
    putTest_Lvalue(queue_, i);
    CurrentThread::sleepUsec(400 * 1000);
  }
}

void RunInProducerR(TaskQueue& queue_, int beg, int end) {
  for (int i = beg; i < end; ++i) {
    LOG_INFO << "Adding task " << i;
    putTest_Rvalue(queue_, i);
    CurrentThread::sleepUsec(500 * 1000);
  }
}

void RunInConsumer(TaskQueue& queue_, int lim) {
  for (int i = 0; i < lim; ++i) {
    takeTest(queue_);
  }
}

int main(void) {
  //多生产者，单消费者模式
  TaskQueue queue_(10);
  Thread producer_L(std::bind(&RunInProducerL, std::ref(queue_), 0, 10),
                    "producer_L");
  Thread producer_R(std::bind(&RunInProducerR, std::ref(queue_), 11, 20),
                    "producer_R");
  CurrentThread::sleepUsec(1000 * 1000);
  Thread consumer(std::bind(&RunInConsumer, std::ref(queue_), 15), "Consumer");
  producer_L.join();
  producer_R.join();
  consumer.join();
  //单生产者，多消费者模式
  Thread producer1(std::bind(&RunInProducerR, std::ref(queue_), 21, 100),
                   "Producer1");
  Thread consumer1(std::bind(&RunInConsumer, std::ref(queue_), 20),
                   "Consumer1");
  Thread consumer2(std::bind(&RunInConsumer, std::ref(queue_), 10),
                   "Consumer2");
  Thread consumer3(std::bind(&RunInConsumer, std::ref(queue_), 50),
                   "Consumer2");

  producer1.join();
  consumer1.join();
  consumer2.join();
  consumer3.join();
  return 0;
}