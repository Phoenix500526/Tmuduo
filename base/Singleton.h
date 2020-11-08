#ifndef TMUDUO_BASE_SINGLETON_H_
#define TMUDUO_BASE_SINGLETON_H_

#include <assert.h>
#include <stdint.h>
#include "base/noncopyable.h"

namespace tmuduo {
// has_no_destroy 可以检测一个类中是否声明了 no_destroy 函数
//对于继承而来的成员函数同样生效，具体可见我的博客：
// https://www.hacker-cube.com/
//的文章《对-muduo-网络库单例模式的思考与实践-下》
template <typename T>
class has_no_destroy {
  using yes = char;
  using no = int32_t;
  struct Base {
    void no_destroy();
  };
  struct Derive : public T, public Base {};
  template <typename C, C>
  class Helper {};
  template <typename U>
  static no test(
      U*, Helper<decltype(&Base::no_destroy), &U::no_destroy>* = nullptr);
  static yes test(...);

 public:
  static const bool value =
      sizeof(yes) == sizeof(test(static_cast<Derive*>(nullptr)));
};

template <typename T>
class Singleton : noncopyable {
 public:
  Singleton() = delete;
  ~Singleton() = delete;
  static T& instance() {
    static T instance;
    return instance;
  }
};

}  // namespace tmuduo
#endif  // TMUDUO_BASE_SINGLETON_H_