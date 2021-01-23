#ifndef JSON_SCHEMA_SCHEMA_VALIDATOR_H
#define JSON_SCHEMA_SCHEMA_VALIDATOR_H

#include "constexpr_json/ext/error_is_nullopt.h"

namespace json_schema {
template <typename SchemaTy, typename ContextTy,
          typename ErrorHandling = cjson::ErrorWillReturnNone>
class SchemaValidator {
public:
  constexpr SchemaValidator(const SchemaTy &theSchema,
                            const ContextTy &theContext)
      : itsSchema{theSchema}, itsContext{theContext} {}

  template <typename JSON>
  constexpr typename std::optional<typename ErrorHandling::ErrorDetail>
  validate(const JSON &theJson) const {
    return std::nullopt;
  }

private:
  SchemaTy itsSchema;
  ContextTy itsContext;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_VALIDATOR_H
