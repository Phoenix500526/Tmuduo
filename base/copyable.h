##ifndef TMUDUO_BASE_COPYABLE_H_
#define TMUDUO_BASE_COPYABLE_H_
    namespace tmuduo {
  // copyable 仅作为一个标签类，用来强调一个类是值语义。
  //由于 copyable 是一个空基类，因此在编译时会被优化，不必担心为此负上性能负担
  class copyable {
   protected:
    copyable() = default;
    ~copyable() = default;
  };
}
#endif  // TMUDUO_BASE_COPYABLE_H_