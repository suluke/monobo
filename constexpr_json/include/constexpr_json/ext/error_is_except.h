#ifndef CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
#define CONSTEXPR_JSON_ERROR_IS_EXCEPT_H

#include "constexpr_json/ext/json_error_detail.h"

namespace cjson {
template<typename ErrorDetail=JsonErrorDetail>
struct ErrorWillThrow {
  template <typename T> using ErrorOr = T;

  template <typename T>
  static constexpr bool isError(const ErrorOr<T> &) noexcept {
    return false;
  }

  template <typename T>
  static constexpr T &unwrap(ErrorOr<T> &theValue) noexcept {
    return theValue;
  }

  template <typename T, typename... Args>
  [[noreturn]] static constexpr ErrorOr<T> makeError(Args &&... theArgs) {
    ErrorDetail::raiseError(std::forward<Args>(theArgs)...)
  }

  template <typename T, typename U, typename... Args>
  static constexpr ErrorOr<T> convertError(const ErrorOr<U> &theError,
                                           Args &&... theArgs) {
    throw std::logic_error("Thrown errors do not need type conversion");
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
