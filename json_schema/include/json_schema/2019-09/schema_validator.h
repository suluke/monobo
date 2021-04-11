#ifndef JSON_SCHEMA_SCHEMA_VALIDATOR_H
#define JSON_SCHEMA_SCHEMA_VALIDATOR_H

#include "constexpr_json/ext/error_is_nullopt.h"

namespace json_schema {
namespace v8n {
template <typename ErrorHandling> class Items {
  template <typename JSON>
  static constexpr std::optional<typename ErrorHandling::ErrorDetail> validate() {}
};
} // namespace v8n

template <typename ContextTy,
          typename ErrorHandling = cjson::ErrorWillReturnNone>
class SchemaValidator {
public:
  using SchemaRef = typename ContextTy::SchemaRef;

  constexpr SchemaValidator(const SchemaRef &theSchema,
                            const ContextTy &theContext)
      : itsContext{theContext}, itsSchema{theSchema} {}
  SchemaValidator(const SchemaObjectAccessor<ContextTy> &theSchema)
      : itsContext{theSchema.getContext()}, itsSchema{
                                                theSchema.getRefInternal()} {}

  template <typename JSON>
  constexpr typename std::optional<typename ErrorHandling::ErrorDetail>
  validate(const JSON &theJson) const {
    return std::nullopt;
  }

private:
  ContextTy itsContext;
  SchemaRef itsSchema;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_VALIDATOR_H
