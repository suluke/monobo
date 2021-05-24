#ifndef JSON_SCHEMA_MODELLING_H
#define JSON_SCHEMA_MODELLING_H

namespace json_schema {
enum class Types {
  ARRAY,
  BOOLEAN,
  INTEGER,
  NUL,
  NUMBER,
  OBJECT,
  STRING,
};

/// This "accidentally" implements for constexpr_json types. But be assured, you
/// only need to provide it for your favorite json lib's type enum to make
/// things work.
template <class U>
constexpr bool operator==(const json_schema::Types &theType1,
                          const U &theType2) {
  using T = json_schema::Types;
  switch (theType2) {
  case U::NUL:
    return theType1 == T::NUL;
  case U::ARRAY:
    return theType1 == T::ARRAY;
  case U::BOOL:
    return theType1 == T::BOOLEAN;
  case U::NUMBER:
    return theType1 == T::NUMBER;
  case U::OBJECT:
    return theType1 == T::OBJECT;
  case U::STRING:
    return theType1 == T::STRING;
  }
  return false;
}
template <class U>
constexpr bool operator==(const U &theType2,
                          const json_schema::Types &theType1) {
  return theType1 == theType2;
}

class tristate {
  enum value : char { Undefined = -1, False = 0, True = 1 };
  value itsValue{Undefined};

public:
  constexpr tristate() noexcept = default;
  constexpr tristate(const tristate &) noexcept = default;
  constexpr tristate(tristate &&) noexcept = default;
  constexpr tristate &operator=(const tristate &) noexcept = default;
  constexpr tristate &operator=(tristate &&) noexcept = default;
  constexpr tristate &operator=(bool theBool) noexcept {
    itsValue = theBool ? True : False;
    return *this;
  }

  constexpr bool operator==(bool theBool) const noexcept {
    return itsValue == static_cast<char>(theBool);
  }
  constexpr bool operator!=(bool theBool) const noexcept {
    return itsValue != static_cast<char>(theBool);
  }
  constexpr bool has_value() const noexcept { return itsValue != Undefined; }
  constexpr bool value() const {
    if (itsValue == Undefined)
      throw "std::bad_optional_access";
    return static_cast<bool>(itsValue);
  }
};

template <typename T, T Fn()> class nullary_function {
public:
  constexpr T operator()() const noexcept(noexcept(Fn())) { return Fn(); }
};

template <typename DefaultGen> class with_default {
public:
  using value_type = std::decay_t<std::invoke_result_t<DefaultGen>>;

private:
  value_type itsValue{DefaultGen{}()};

public:
  constexpr with_default() noexcept(noexcept(value_type{
      DefaultGen{}()})) = default;
  constexpr with_default(const with_default &) noexcept(
      std::is_nothrow_copy_constructible_v<value_type>) = default;
  constexpr with_default(with_default &&) noexcept(
      std::is_nothrow_move_constructible_v<value_type>) = default;
  constexpr with_default &operator=(const with_default &) noexcept(
      std::is_nothrow_copy_assignable_v<value_type>) = default;
  constexpr with_default &operator=(with_default &&) noexcept(
      std::is_nothrow_move_assignable_v<value_type>) = default;
  constexpr with_default &operator=(const value_type &theValue) noexcept(
      std::is_nothrow_copy_assignable_v<value_type>) {
    itsValue = theValue;
    return *this;
  }
  constexpr with_default &operator=(value_type &&theValue) noexcept(
      std::is_nothrow_move_assignable_v<value_type>) {
    itsValue = theValue;
    return *this;
  }
  constexpr const value_type &value() const noexcept { return itsValue; }
  constexpr operator const value_type &() const noexcept { return itsValue; }
  constexpr explicit operator bool() const {
    return has_value();
  }
  constexpr bool has_value() const {
    const auto aDefault = DefaultGen{}();
    return (aDefault == aDefault && itsValue != aDefault) ||
           // for NaN case:
           (aDefault != aDefault && itsValue == itsValue);
  }
};
} // namespace json_schema

#endif // JSON_SCHEMA_MODELLING_H
