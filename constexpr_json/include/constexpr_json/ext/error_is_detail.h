#ifndef CONSTEXPR_JSON_ERROR_IS_DETAIL_H
#define CONSTEXPR_JSON_ERROR_IS_DETAIL_H

#include "constexpr_json/ext/json_error_detail.h"

#include <variant>

namespace cjson {
template<typename ErrorDetail=JsonErrorDetail>
struct ErrorWillReturnDetail {
  template <typename T> using ErrorOr = std::variant<ErrorDetail, T>;

  template <typename T>
  static constexpr bool isError(const ErrorOr<T> &theValue) noexcept {
    return !std::holds_alternative<T>(theValue);
  }

  template <typename T>
  static constexpr const T &unwrap(const ErrorOr<T> &theValue) noexcept {
    return std::get<T>(theValue);
  }

  template <typename T, typename... Args>
  static constexpr ErrorOr<T> makeError(Args &&... theArgs) {
    return ErrorDetail{std::forward<Args>(theArgs)...};
  }

  template <typename T, typename U, typename... Args>
  static constexpr ErrorOr<T> convertError(const ErrorOr<U> &theError,
                                           Args &&... theArgs) {
    return std::get<ErrorDetail>(theError).convert(std::forward<Args>(theArgs)...);
  }

  template <typename T>
  static constexpr ErrorDetail getError(const ErrorOr<T> &theError) {
    return std::get<ErrorDetail>(theError);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_DETAIL_H
