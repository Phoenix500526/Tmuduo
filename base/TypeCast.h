#ifndef TMUDUO_BASE_TYPESCAST_H_
#define TMUDUO_BASE_TYPESCAST_H_

#ifndef NDEBUG
#include <assert.h>
#endif

namespace tmuduo {
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
template <typename TO, typename FROM>
inline TO implicit_cast(FROM const& f) {
  return f;
}

// down_cast 只接受指针类型的向下转型,
template <typename TO, typename FROM>
inline To down_cast(FROM* f) {
  // 利用 implicit_cast 进行反向检查以确保 TO 是 FROM 的子类
  // 条件if(false)结合模板可以实现编译期类型检查的功能，同时由于编译器会进行优化，因此不会负上任何性能上的开销
  if (false) {
    implicit_cast<FROM*, TO>(0);
  }
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<TO>(f);
}

}  // namespace tmuduo
#endif  // TMUDUO_BASE_TYPESCAST_H_