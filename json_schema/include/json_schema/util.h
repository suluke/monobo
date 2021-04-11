#ifndef JSON_SCHEMA_UTIL_H
#define JSON_SCHEMA_UTIL_H

#include "json_schema/util/optional.h"
#include "json_schema/util/type_tag.h"
#include "json_schema/util/variant.h"

namespace json_schema {

template <typename Container> class SubscriptIterator {
public:
  constexpr SubscriptIterator() = default;
  constexpr SubscriptIterator(const SubscriptIterator &) = default;
  constexpr SubscriptIterator &operator=(const SubscriptIterator &) = default;

  constexpr SubscriptIterator(const Container &theContainer,
                              const ptrdiff_t thePos)
      : itsPos{thePos}, itsContainer{theContainer} {}

  constexpr SubscriptIterator &operator++() {
    ++itsPos;
    return *this;
  }
  constexpr SubscriptIterator operator++(int) {
    SubscriptIterator aCopy{*this};
    ++(*this);
    return aCopy;
  }
  constexpr auto operator*() const { return itsContainer[itsPos]; }
  constexpr bool operator==(const SubscriptIterator &theOther) const {
    return itsPos == theOther.itsPos && itsContainer == theOther.itsContainer;
  }
  constexpr bool operator!=(const SubscriptIterator &theOther) const {
    return !(*this == theOther);
  }

private:
  ptrdiff_t itsPos;
  Container itsContainer;
};

template <typename Iter, typename Decorator> class DecoratingIterator {
public:
  constexpr DecoratingIterator() = default;
  constexpr DecoratingIterator(const DecoratingIterator &) = default;
  constexpr DecoratingIterator &operator=(const DecoratingIterator &) = default;

  constexpr DecoratingIterator(Iter theIterator, Decorator theDecorator)
      : itsIter{theIterator}, itsDecorator{theDecorator} {}

  constexpr DecoratingIterator &operator++() {
    ++itsIter;
    return *this;
  }
  constexpr DecoratingIterator operator++(int) {
    DecoratingIterator aCopy{*this};
    ++(*this);
    return aCopy;
  }
  constexpr auto operator*() const { return itsDecorator(*itsIter); }
  constexpr bool operator==(const DecoratingIterator &theOther) const {
    return itsIter == theOther.itsIter && itsDecorator == theOther.itsDecorator;
  }
  constexpr bool operator!=(const DecoratingIterator &theOther) const {
    return !(*this == theOther);
  }

private:
  Iter itsIter;
  Decorator itsDecorator;
};

template <typename Ptr> struct pointer_traits {
  using pointer = Ptr;

private:
  template <typename T>
  static typename T::element_type select_element_type(...) {
    return std::declval<typename T::element_type>();
  }
  template <template <typename, typename...> typename P, typename T,
            typename... Args>
  static T select_element_type(P<T, Args...>) {
    return std::declval<T>();
  }

  static std::ptrdiff_t select_ptrdiff(...) { return {}; }
  template <typename T> static typename T::ptrdiff_t select_ptrdiff(T) {
    return std::declval<typename T::ptrdiff_t>();
  }

public:
  using element_type = decltype(select_element_type(std::declval<Ptr>()));
  using difference_type = decltype(select_ptrdiff(std::declval<Ptr>()));
  static constexpr pointer pointer_to(element_type &theElm) {
    return pointer::pointer_to(theElm);
  }
};

template <typename T> struct pointer_traits<T *> {
  using pointer = T *;
  using element_type = T;
  using difference_type = std::ptrdiff_t;
  static constexpr pointer pointer_to(element_type &theElm) { return &theElm; }
};

} // namespace json_schema
#endif // JSON_SCHEMA_UTIL_H
