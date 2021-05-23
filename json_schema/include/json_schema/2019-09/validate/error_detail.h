#ifndef JSON_SCHEMA_201909_VALIDATE_ERROR_DETAIL_H
#define JSON_SCHEMA_201909_VALIDATE_ERROR_DETAIL_H

#include "json_schema/2019-09/validate/error_codes.h"

#include <string>

namespace json_schema {

struct ValidationErrorDetail {
  ValidationErrorDetail(const ErrorCode theEC, const char *const theDesc)
      : itsEC{theEC}, itsDesc{theDesc} {}

  ErrorCode getCode() const noexcept { return itsEC; }
  std::string what() const {
    std::string aResult;
    aResult = getErrorCodeDescription(itsEC);
    aResult += ": ";
    aResult += itsDesc;
    return aResult;
  }

  static const char *getErrorCodeDescription(const ErrorCode theEC) {
    switch (theEC) {
    case ErrorCode::UNKNOWN:
      return "Unknown";
    case ErrorCode::ENCODING_ERROR:
      return "Encoding error";
    }
    return "";
  }

private:
  ErrorCode itsEC;
  const char *itsDesc;
};
} // namespace json_schema
namespace std {
std::string to_string(const json_schema::ErrorCode theEC) {
  return json_schema::ValidationErrorDetail::getErrorCodeDescription(theEC);
}
} // namespace std
#endif // JSON_SCHEMA_201909_VALIDATE_ERROR_DETAIL_H
