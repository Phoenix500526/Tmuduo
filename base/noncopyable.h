#ifndef TMUDUO_BASE_NONCPOYABLE_H_
#define TMUDUO_BASE_NONCPOYABLE_H_
namespace tmuduo {
class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  //将构造函数和析构函数都声明为 protected
  //可将实例化的过程限制在子类当中，从而避免外部实例化 noncopyable 对象
  noncopyable() = default;
  ~noncopyable() = default;
};
}  // namespace tmuduo
#endif  // TMUDUO_BASE_NONCPOYABLE_H_