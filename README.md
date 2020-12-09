# Tmuduo
Tmuduo 是一个用于学习的项目，其存在的目的有二：
* 深入理解 [muduo](https://github.com/chenshuo/muduo) 和 [libevent](https://libevent.org/) 等优秀网络库的设计，提升自身对网络编程的理解
* 学习如何去做出一个开源软件
Tmuduo 采用 cmake 进行构建， conan 进行包管理。在编译器上优先选择了 clang 进行编译(也支持使用 gcc 编译)，采用 clang-format 格式化，遵守 google 开源项目代码风格

# 存在意义
Tmuduo 本质上是一个学习型项目，其存在的意义如下：
1. 通过对 muduo 的模仿与重新实现，能够对网络库的运作原理有一个深刻的理解
2. 通过修改代码，编译后再与 muduo 进行比较，能够使得一些不那么容易被注意到的设计亮点亮相于台前
3. 通过重新编写 muduo 的网络示例，能够常见的网络服务有更加深刻的了解
4. 增加了一些自己的工程实践，muduo 网络库中有不少 non-trivial 的示例和方法，例如如何使用 test harness 对多线程程序进行自动化测试，如何实现基于 REST 风格的进程监控，如何实现基于 Observer Pattern 的进程监控等等。关于这方面具体可参考我的博客，以及相关的代码实现。

# 环境要求
* Ubuntu 16.04.2 LTS 及以上
* clang version 3.8.0-2ubuntu4 或 gcc version 5.4.0 20160609
* conan
* cmake 3.13 及以上

# 文件目录
base: 存放基础库文件
net: 存放网络库文件
tests: 存放测试文件，包含了单元测试和功能测试
examples: 存放示例文件


# 编译运行方法
```bash
# 安装相应的库文件
$ sudo apt-get install libcurl4-openssl-dev libc-ares-dev
$ sudo apt-get install protobuf-compiler libprotobuf-dev
# 编译 Debug 版本
$ ./build.sh -j4
# 编译 Release 版本
$ BUILD_TYPE=release ./build.sh -j4
```

# 和 muduo 相比有何区别？
主要的改动集中在两方面：
1. 源代码方面
    * 在线程库的选择上，使用 C++ 的标准线程库来代替 POSIX 线程库，在保证性能的同时尽可能地保留了 muduo 原有的设计理念。
    * 在测试框架上，我采用了 Google 的 gtest 测试框架，并基于此重新编写了 tmuduo 的测试用例。
    * 作为 《Linux多线程服务端编程 ——　使用　muduo C++ 网络库》一书中练习的实践，我对书中的一些猜想进行了验证，这些修改零零散散分布在代码之中。对于尚未完成或仍可改进的部分练习，我放在 TODO 列表当中，留待以后解决
2. 工程构建方面
    * tmuduo 采用 cmake 构建，利用 conan 进行第三方包的管理。在 cmake 上与 muduo 原有的 cmake 区别甚小，主要是 conan 的部分。之所以使用 conan 进行包管理，是因为某些特殊原因，使得 gtest 的下载异常缓慢。

# 相关文章：
在实现 tmuduo 的过程当中，我验证了一些猜想，学习的过程当中不断经历了先将一个东西从 tmuduo 中移除，然后随着对 muduo 理解的加深，最后又将其添加进来的过程，我将这一过程当中的心得体会编撰成文章，发表在我的博客上，如下：
[muduo 网络库源码剖析系列文章](https://www.hacker-cube.com/categories/muduo%E6%BA%90%E7%A0%81%E5%89%96%E6%9E%90//)

# TODO
- [ ] 使用智能指针来管理 TimerQueue 中的 timer 节点  
- [ ] 使用 Go 语言来为 multiplexer 实现 test harness  
- [ ] 将 multiplexer、demux 以及 socks4a 级联起来，编写一个新的 example  
- [ ] 完善 cdns 用例中关于 fd 可写情况的处理  
