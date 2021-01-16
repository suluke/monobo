#ifndef CONSTEXPR_JSON_ERROR_IS_NULLOPT_H
#define CONSTEXPR_JSON_ERROR_IS_NULLOPT_H

#include "constexpr_json/error_codes.h"

#include <optional>

namespace cjson {
/// Basic error handling strategy
///
/// Will return optional<Result> or nullopt on error
struct ErrorWillReturnNone {
  using ErrorDetail = bool;
  template <typename T> using ErrorOr = std::optional<T>;

  template <typename T>
  static constexpr bool isError(const ErrorOr<T> &theValue) noexcept {
    return !static_cast<bool>(theValue);
  }

  template <typename T>
  static constexpr const T &unwrap(const ErrorOr<T> &theValue) noexcept {
    return *theValue;
  }
  template <typename T>
  static constexpr T &unwrap(ErrorOr<T> &theValue) noexcept {
    return *theValue;
  }

  template <typename T, typename... Args>
  static constexpr ErrorOr<T> makeError(Args &&... theArgs) {
    return std::nullopt;
  }

  template <typename T, typename U, typename... Args>
  static constexpr ErrorOr<T> convertError(const ErrorOr<U> &theError,
                                           Args &&... theArgs) {
    return std::nullopt;
  }

  template <typename T>
  static constexpr ErrorDetail getError(const ErrorOr<T> &theError) {
    return static_cast<bool>(theError);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_NULLOPT_H
