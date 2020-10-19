#include "base/CountDownLatch.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>

using namespace tmuduo;

int main(void){
    std::vector<std::thread> pool;
    CountDownLatch latch(10);
    for(int i = 0;i < 10;++i){
        pool.emplace_back([&latch](){
            latch.countDown();
            latch.wait();
            std::cout << "this thread is " << std::this_thread::get_id() << '\n';
        });
        if(i & 1){
            std::cout << "i = " << i << ", sleep 2 s" << '\n';
            sleep(2);
        }
    }
    for(std::thread& t:pool){
        if(t.joinable())
            t.join();
    }
    return 0;
}