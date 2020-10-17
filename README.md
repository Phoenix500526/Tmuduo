# Tmuduo
Tmuduo 是一个用于学习的项目，其存在的目的有二：
* 深入理解 [muduo](https://github.com/chenshuo/muduo) 和 [libevent](https://libevent.org/) 等优秀网络库的设计，提升自身对网络编程的理解
* 学习如何去做出一个开源软件
Tmuduo 采用 cmake 进行构建， conan 进行包管理。在编译器上优先选择了 clang 进行编译(也支持使用 gcc 编译)，遵守 google 开源项目代码风格

# 环境要求
* Ubuntu 16.04.2 LTS 或以上
* clang version 3.8.0-2ubuntu4 或 gcc version 5.4.0 20160609
* conan

# 文件目录
3-rd: 第三方库文件
test: 存放测试文件
net: 存放网络文件
base: 存放底层文件


