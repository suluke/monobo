#ifndef JSON_SCHEMA_SCHEMA_VALIDATOR_H
#define JSON_SCHEMA_SCHEMA_VALIDATOR_H

#include <cmath>

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
  using ValidationResult = typename std::optional<ErrorDetail>;

  constexpr SchemaValidator(const SchemaRef &theSchema,
                            const ContextTy &theContext)
      : itsContext{theContext}, itsSchema{theSchema} {}
  SchemaValidator(const SchemaObjectAccessor<ContextTy> &theSchema)
      : itsContext{theSchema.getContext()}, itsSchema{
                                                theSchema.getRefInternal()} {}

  template <typename JSON>
  constexpr ValidationResult validate(const JSON &theJson) const {
    using json_type = decltype(theJson.getType());
    SchemaObjectAccessor aSchema{itsContext, itsSchema};
    if (aSchema.isTrueSchema())
      return std::nullopt;
    if (aSchema.isFalseSchema())
      return makeError("Schema to validate against is `false`");
    const auto &aValidation = aSchema.template getSection<SchemaValidation>();
    // minProperties
    if (const auto &aMinProps = aValidation.getMinProperties();
        aMinProps.has_value()) {
      if (theJson.getType() == json_type::OBJECT) {
        const auto &aObject = theJson.toObject();
        if (aObject.size() < aMinProps) {
          return makeError("Object has not enough properties (minProperties)");
        }
      }
    }
    // maxProperties
    if (const auto &aMaxProps = aValidation.getMaxProperties();
        aMaxProps.has_value()) {
      if (theJson.getType() == json_type::OBJECT) {
        const auto &aObject = theJson.toObject();
        if (aObject.size() > aMaxProps) {
          return makeError("Object has too many properties (maxProperties)");
        }
      }
    }
    // type
    if (const auto &aTypes = aValidation.getType()) {
      const auto aElmType = theJson.getType();
      const bool isNumber = Types::NUMBER == aElmType;
      bool aIsAllowed = false;
      for (const auto &aType : *aTypes) {
        if (aType == aElmType ||
            (aType == Types::INTEGER && isNumber &&
             theJson.toNumber() == std::trunc(theJson.toNumber()))) {
          aIsAllowed = true;
          break;
        }
      }
      if (!aIsAllowed)
        return makeError("Type is not allowed");
    }
    return std::nullopt;
  }

private:
  constexpr ErrorDetail makeError(const char *const theMsg) const {
    return ErrorHandling::getError(
        ErrorHandling::template makeError<bool>(theMsg));
  }

  ContextTy itsContext;
  SchemaRef itsSchema;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_VALIDATOR_H
