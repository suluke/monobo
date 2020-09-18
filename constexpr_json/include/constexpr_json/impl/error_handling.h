#ifndef CONSTEXPR_JSON_ERROR_HANDLING_H
#define CONSTEXPR_JSON_ERROR_HANDLING_H

#include "constexpr_json/error_codes.h"

#include <optional>
#include <string_view>
#include <variant>

namespace cjson {

struct ErrorDetail {
  const ErrorCode itsCode;
  /// (Rough) character location where the error was encountered
  ///
  /// We will NOT store this explicitly as line+offset. Errors are not expected
  /// to occur and thus we can defer the complexity of tracking/computing the
  /// line being currently processed to when the error is being reported.
  intptr_t itsPosition;

  std::pair<intptr_t, intptr_t>
  computeLineLocation(const std::string_view theJson) {
    // TODO
    return {-1, -1};
  }

  static constexpr const char *
  getErrorCodeDesc(const ErrorCode theEC) noexcept {
    switch (theEC) {
    case ErrorCode::UNKNOWN:
      return "Unknown error occurred";
    }
    return nullptr;
  }
};

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
};

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
};

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
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_HANDLING_H
