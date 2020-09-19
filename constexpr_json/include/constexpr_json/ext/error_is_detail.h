#ifndef CONSTEXPR_JSON_ERROR_IS_DETAIL_H
#define CONSTEXPR_JSON_ERROR_IS_DETAIL_H

#include "constexpr_json/ext/error_detail.h"

#include <variant>

namespace cjson {
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

  template <typename T>
  static constexpr ErrorOr<T> makeError(const ErrorCode theEC,
                                        const intptr_t thePos) {
    return ErrorDetail{theEC, thePos};
  }

  template <typename T, typename U>
  static constexpr ErrorOr<T>
  convertError(const ErrorOr<U> &theError,
               const intptr_t thePositionOffset = 0) {
    ErrorDetail aDetail = std::get<ErrorDetail>(theError);
    aDetail.itsPosition += thePositionOffset;
    return aDetail;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_DETAIL_H
