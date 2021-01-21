#pragma once
template <typename...>
class visitor_t {};

template <typename TYPE>
class visitor_t<TYPE> {
 public:
  virtual void visit(TYPE *t) = 0;
};

template <typename T, typename... TYPES>
class visitor_t<T, TYPES...> : public visitor_t<TYPES...> {
 public:
  using visitor_t<TYPES...>::visit;
  virtual void visit(T *t) = 0;
  virtual ~visitor_t() = 0;
};

template <typename T>
class visitable_t {
 public:
  virtual void accept(T *) = 0;
};