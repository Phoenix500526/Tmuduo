#ifndef TMUDUO_BASE_TYPESCAST_H_
#define TMUDUO_BASE_TYPESCAST_H_

#include <assert.h>
#include <string.h>  // memset
#include <memory>

namespace tmuduo {

inline void memZero(void* p, size_t n) { memset(p, 0, n); }
// implicit_cast 在将类类型向上转型时会进行类型检查，更加安全
// 例如：
// class Top{};
// class MiddleA : public Top{};
// class MiddleB : public Top{};
// class Bottom{}:public MiddleA, public MiddleB{};
// void func(Middle const& A){
//     cout << "A" << endl;
// }
// void func(Middle const& B){
//     cout << "B" << endl;
// }
// int main(){
//     Bottom bot;
//     func(static_cast<MiddleA const&>(bot));  //输出 A
//     func(static_cast<MiddleB const&>(bot)); //输出 B
// }
// 当不小心将 bot 的类型改成了 Top
// 类型则也能通过编译，但会引发未定义行为。这是因为 static_cast
// 在对自定义类型进行转型时不会进行类型检查
template <typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

// down_cast 只接受指针类型的向下转型,
template <typename To, typename From>
inline To down_cast(From* f) {
  // 利用 implicit_cast 进行反向检查以确保 To 是 From 的子类
  // 条件if(false)结合模板可以实现编译期类型检查的功能，同时由于编译器会进行优化，因此不会负上任何性能上的开销
  if (false) {
    implicit_cast<From*, To>(0);
  }
#if !defined(NDEBUG) && !defined(GOOGLE_PROToBUF_NO_RTTI)
  assert(f == nullptr ||
         dynamic_cast<To>(f) != nullptr);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

//对于以下两个函数模板的使用存疑：究竟是出于什么目的才要使用模板封装已有的 get
//函数
template <typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr) {
  return ptr.get();
}

template <typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr) {
  return ptr.get();
}

// down_pointer_cast 用于对将指向基类的智能指针向下转型为子类指针
template <typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(
    const ::std::shared_ptr<From>& f) {
  if (false) {
    implicit_cast<From*, To*>(0);
  }
#ifndef NDEBUG
  assert(f == nullptr || dynamic_cast<To*>(get_pointer(f)) != nullptr);
#endif
  return ::std::static_pointer_cast<To>(f);
}

}  // namespace tmuduo
#endif  // TMUDUO_BASE_TYPESCAST_H_