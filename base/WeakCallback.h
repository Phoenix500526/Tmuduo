#ifndef TMUDUO_BASE_WEAKCALLBACK_H_
#define TMUDUO_BASE_WEAKCALLBACK_H_

#include <functional>
#include <memory>

namespace tmuduo {

//在事件驱动模型下，为异步回调函数实现一个弱回调很有必要。
//因为在异步回调过程中，回调的时机是不确定的，因此可能会在弱引用被释放后才调用回调函数，这会导致程序崩溃
template <typename CLASS, typename... ARGS>
class WeakCallback {
 public:
  WeakCallback(const std::weak_ptr<CLASS>& obj,
               const std::function<void(CLASS*, ARGS...)>& func)
      : object_(obj), function_(func) {}
  ~WeakCallback() = default;
  void operator()(ARGS&&... args) const {
    std::shared_ptr<CLASS> s_ptr(object_.lock());
    if (s_ptr) {
      function_(s_ptr.get(), std::forward<ARGS>(args)...);
    }
  }

 private:
  std::weak_ptr<CLASS> object_;
  std::function<void(CLASS*, ARGS...)> function_;
};

//将 shared_ptr 转化成为 weak_ptr，将成员函数指针转化成为首个参数为 this
//指针的函数对象
template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& obj,
                                              void (CLASS::*func)(ARGS...)) {
  return WeakCallback<CLASS, ARGS...>(obj, func);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& obj,
                                              void (CLASS::*func)(ARGS...)
                                                  const) {
  return WeakCallback<CLASS, ARGS...>(obj, func);
}

}  // namespace tmuduo
#endif  // TMUDUO_BASE_WEAKCALLBACK_H_