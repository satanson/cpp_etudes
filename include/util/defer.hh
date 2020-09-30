//
// Created by grakra on 2020/9/30.
//

#ifndef CPP_ETUDES_DEFER_HH
#define CPP_ETUDES_DEFER_HH
template<typename F>
struct Defer {
  F f;
  Defer(F &&f) : f(f) {
  }
  ~Defer() {
    f();
  }
};
#define _DEFER(line, f) auto defer_##line=Defer(f);
#define DEFER(f) _DEFER(__LINE__,(f))

#endif //CPP_ETUDES_DEFER_HH
