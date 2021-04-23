#ifndef JSON_SCHEMA_SCHEMA_VALIDATOR_H
#define JSON_SCHEMA_SCHEMA_VALIDATOR_H

#include "constexpr_json/ext/error_is_nullopt.h"
#include "json_schema/2019-09/model/applicator.h"
#include "json_schema/2019-09/model/content.h"
#include "json_schema/2019-09/model/core.h"
#include "json_schema/2019-09/model/format.h"
#include "json_schema/2019-09/model/metadata.h"
#include "json_schema/2019-09/model/validation.h"

namespace json_schema {
namespace v8n {
template <typename ErrorHandling> class Items {
  template <typename JSON>
  static constexpr std::optional<typename ErrorHandling::ErrorDetail>
  validate() {}
};
} // namespace v8n

template <typename ContextTy,
          typename ErrorHandling = cjson::ErrorWillReturnNone>
class SchemaValidator {
public:
  using SchemaRef = typename ContextTy::SchemaRef;
  using ErrorDetail = typename ErrorHandling::ErrorDetail;

  constexpr SchemaValidator(const SchemaRef &theSchema,
                            const ContextTy &theContext)
      : itsContext{theContext}, itsSchema{theSchema} {}
  SchemaValidator(const SchemaObjectAccessor<ContextTy> &theSchema)
      : itsContext{theSchema.getContext()}, itsSchema{
                                                theSchema.getRefInternal()} {}

  template <typename JSON>
  constexpr typename std::optional<ErrorDetail>
  validate(const JSON &theJson) const {
    using json_type = decltype(theJson.getType());
    SchemaObjectAccessor aSchema{itsContext, itsSchema};
    if (aSchema.isTrueSchema())
      return std::nullopt;
    const auto &aValidation = aSchema.template getSection<SchemaValidation>();
    if (const auto &aMinProps = aValidation.getMinProperties();
        aMinProps.has_value()) {
      if (theJson.getType() == json_type::OBJECT) {
        const auto &aObject = theJson.toObject();
        if (aObject.size() < aMinProps) {
          return makeError("Object has not enough properties (minProperties)");
        }
      }
    }
    return std::nullopt;
  }

private:
  ErrorDetail makeError(const char *const theMsg) const {
    return ErrorHandling::unwrap(
        ErrorHandling::template makeError<bool>(theMsg));
  }

  ContextTy itsContext;
  SchemaRef itsSchema;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_VALIDATOR_H
