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
  constexpr bool has_value() const
      noexcept(noexcept(itsValue != DefaultGen{}())) {
    return itsValue != DefaultGen{}();
  }
};

using with_default_false = with_default<std::integral_constant<bool, false>>;

} // namespace json_schema

#endif // JSON_SCHEMA_MODELLING_H
