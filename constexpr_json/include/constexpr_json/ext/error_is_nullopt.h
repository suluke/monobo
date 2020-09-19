#ifndef CONSTEXPR_JSON_ERROR_IS_NULLOPT_H
#define CONSTEXPR_JSON_ERROR_IS_NULLOPT_H

#include "constexpr_json/error_codes.h"

#include <optional>

namespace cjson {
/// Basic error handling strategy
///
/// Will return optional<Result> or nullopt on error
struct ErrorWillReturnNone {
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
  static constexpr ErrorOr<T> makeError(const ErrorCode theEC,
                                        const intptr_t thePos) {
    return std::nullopt;
  }

  template <typename T, typename U>
  static constexpr ErrorOr<T>
  convertError(const ErrorOr<U> &theError,
               const intptr_t thePositionOffset = 0) {
    return std::nullopt;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_IS_NULLOPT_H
