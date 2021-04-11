#ifndef JSON_SCHEMA_UTIL_OPTIONAL_H
#define JSON_SCHEMA_UTIL_OPTIONAL_H

namespace json_schema {
template <typename T> struct Optional {
  using value_type = T;

  constexpr Optional() = default;
  template <
      typename U = value_type,
      typename std::enable_if_t<std::is_constructible_v<T, U &&> &&
                                    !std::is_same_v<std::decay_t<U>, Optional>,
                                bool> = true>
  constexpr Optional(U &&theValue)
      : itsStorage(std::forward<U>(theValue)), itsHasValue(true) {}
  constexpr Optional(const Optional &theOther) { *this = theOther; }
  constexpr Optional(Optional &&theOther) { *this = std::move(theOther); }

  static_assert(std::is_trivially_destructible_v<T>,
                "Non-trivially destructible types not supported");

  constexpr Optional &operator=(const Optional &theOther) = default;
  // {
  //   itsHasValue = theOther.itsHasValue;
  //   if (itsHasValue) {
  //     itsStorage = Storage{theOther.itsStorage.itsValue};
  //   } else {
  //     itsStorage = Storage{};
  //   }
  //   return *this;
  // }
  constexpr Optional &operator=(Optional &&theOther) = default;
  // {
  //   itsHasValue = theOther.itsHasValue;
  //   if (itsHasValue) {
  //     itsStorage = Storage{std::move(theOther.itsStorage.itsValue)};
  //   } else {
  //     itsStorage = Storage{};
  //   }
  //   return *this;
  // }
  constexpr Optional &operator=(const T &theValue) {
    itsHasValue = true;
    itsStorage.itsValue = theValue;
    return *this;
  }

  constexpr const T &operator*() const { return itsStorage.itsValue; }
  constexpr T &operator*() { return itsStorage.itsValue; }
  constexpr const T *operator->() const { return &itsStorage.itsValue; }
  constexpr T *operator->() { return &itsStorage.itsValue; }

  constexpr operator bool() const { return itsHasValue; }

private:
  union Storage {
    constexpr Storage() : itsVoid{} {}
    template <typename... Args>
    constexpr Storage(Args &&...theArgs)
        : itsValue{std::forward<Args>(theArgs)...} {}

    struct {
    } itsVoid;
    T itsValue;
  } itsStorage;
  bool itsHasValue{false};
};
} // namespace json_schema
#endif // JSON_SCHEMA_UTIL_OPTIONAL_H
