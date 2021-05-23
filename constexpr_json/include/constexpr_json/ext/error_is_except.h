#ifndef CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
#define CONSTEXPR_JSON_ERROR_IS_EXCEPT_H

#include "constexpr_json/ext/json_error_detail.h"

namespace cjson {
namespace detail {
struct LogicErrorBuilder {
  static std::logic_error create() { return std::logic_error{"An error occurred"}; }
  template <typename T, typename... Args>
  static std::logic_error create(T &&theDesc, Args &&...) {
    if constexpr (std::is_constructible_v<std::string, T>)
      return std::logic_error{std::string(std::forward<T>(theDesc))};
    else
      return std::logic_error{std::to_string(std::forward<T>(theDesc))};
  }
};
} // namespace detail
template <typename ExceptionBuilder = detail::LogicErrorBuilder>
struct ErrorWillThrow {
  using ErrorDetail = void;
  template <typename T> using ErrorOr = T;

  template <typename T>
  static constexpr bool isError(const ErrorOr<T> &) noexcept {
    return false;
  }

  template <typename T>
  static constexpr T &unwrap(ErrorOr<T> &theValue) noexcept {
    return theValue;
  }
  template <typename T>
  static constexpr const T &unwrap(const ErrorOr<T> &theValue) noexcept {
    return theValue;
  }

  template <typename T, typename... Args>
  [[noreturn]] static constexpr ErrorOr<T> makeError(Args &&...theArgs) {
    throw ExceptionBuilder::create(std::forward<Args>(theArgs)...);
  }

  template <typename T, typename U, typename... Args>
  static constexpr ErrorOr<T> convertError(const ErrorOr<U> &theError,
                                           Args &&...theArgs) {
    throw std::logic_error("Thrown errors do not need type conversion");
  }

  template <typename T>
  static constexpr ErrorDetail getError(const ErrorOr<T> &theError) {
    static_assert(!std::is_same_v<ErrorOr<T>, T>,
                  "Exception-based error handling does not permit "
                  "return-value-based error introspection");
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
