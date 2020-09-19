#ifndef CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
#define CONSTEXPR_JSON_ERROR_IS_EXCEPT_H

#include "constexpr_json/ext/error_detail.h"

namespace cjson {
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

  template <typename T>
  [[noreturn]] static constexpr ErrorOr<T> makeError(const ErrorCode theEC,
                                                     const intptr_t thePos) {
    throw std::invalid_argument(ErrorDetail::getErrorCodeDesc(theEC));
  }

  template <typename T, typename U>
  static constexpr ErrorOr<T>
  convertError(const ErrorOr<U> &theError,
               const intptr_t thePositionOffset = 0) {
    throw std::logic_error("Thrown errors do not need type conversion");
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_EXCEPT_H
