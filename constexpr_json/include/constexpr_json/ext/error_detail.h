#ifndef CONSTEXPR_JSON_ERROR_DETAIL_H
#define CONSTEXPR_JSON_ERROR_DETAIL_H

#include "constexpr_json/error_codes.h"

#include <string_view>

namespace cjson {
struct ErrorDetail {
  ErrorCode itsCode;
  /// (Rough) character location where the error was encountered
  ///
  /// We will NOT store this explicitly as line+offset. Errors are not expected
  /// to occur and thus we can defer the complexity of tracking/computing the
  /// line being currently processed to when the error is being reported.
  intptr_t itsPosition;

  template<typename EncodingTy>
  constexpr std::pair<intptr_t, intptr_t>
  computeLocation(const std::string_view theJson) const noexcept {
    if (itsPosition < 0)
      return {-1, -1};
    intptr_t aLine = 0, aCol = 0;
    std::string_view aRemaining{theJson};
    while(theJson.size() - aRemaining.size() < static_cast<size_t>(itsPosition)) {
      const auto [aChar, aCharWidth] = EncodingTy::decodeFirst(aRemaining);
      aRemaining.remove_prefix(aCharWidth);
      ++aCol;
      if (aChar == '\n') {
        aCol = 0;
        ++aLine;
      }
    }
    return {aLine, aCol};
  }

  constexpr const char *what() const noexcept {
    return getErrorCodeDesc(itsCode);
  }

  static constexpr const char *
  getErrorCodeDesc(const ErrorCode theEC) noexcept {
    switch (theEC) {
    case ErrorCode::UNKNOWN:
      return "Unknown error occurred";
    case ErrorCode::UNREACHABLE:
      return "Unreachable code executed";
    case ErrorCode::NULL_READ_FAILED:
      return "Failed to read element: Expected null";
    case ErrorCode::BOOL_READ_FAILED:
      return "Failed to read element: Expected true or false";
    case ErrorCode::NUMBER_READ_FAILED:
      return "Failed to read element: Expected valid number";
    case ErrorCode::STRING_READ_FAILED:
      return "Failed to read element: Expected valid string";
    case ErrorCode::ARRAY_EXPECTED_COMMA:
      return "Failed to read array: Expected comma";
    case ErrorCode::ARRAY_UNEXPECTED_TOKEN:
      return "Failed to read array: Illegal first character or unexpected EOF";
    case ErrorCode::OBJECT_EXPECTED_COLON:
      return "Failed to read object: Expected colon";
    case ErrorCode::OBJECT_EXPECTED_COMMA:
      return "Failed to read object: Expected comma";
    case ErrorCode::OBJECT_UNEXPECTED_TOKEN:
      return "Failed to read object: Illegal first character or unexpected EOF";
    case ErrorCode::OBJECT_KEY_READ_FAILED:
      return "Failed to read object: Could not read property key";
    case ErrorCode::TYPE_DEDUCTION_FAILED:
      return "Failed to determine element type";
    case ErrorCode::TRAILING_CONTENT:
      return "Encountered additional characters when EOF was expected";
    case ErrorCode::MAX_DEPTH_EXCEEDED:
      return "Exceeded maximum document nesting level";
    }
    return nullptr;
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_ERROR_DETAIL_H
