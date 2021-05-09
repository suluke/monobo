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
    return validate(theJson, itsSchema);
  }

  template <typename JSON>
  constexpr ValidationResult validate(const JSON &theJson,
                                      const SchemaRef theSchema) const {
    using json_type = decltype(theJson.getType());
    SchemaObjectAccessor aSchema{itsContext, theSchema};
    if (aSchema.isTrueSchema())
      return std::nullopt;
    if (aSchema.isFalseSchema())
      return makeError("Schema to validate against is `false`");
    const auto &aApplicator = aSchema.template getSection<SchemaApplicator>();
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
    if (theJson.getType() == json_type::NUMBER) {
      // maximum
      if (const auto &aMaximum = aValidation.getMaximum()) {
        if (aMaximum < theJson.toNumber()) {
          return makeError("Value above maximum");
        }
      }

      // minimum
      if (const auto &aMinimum = aValidation.getMinimum()) {
        if (aMinimum > theJson.toNumber()) {
          return makeError("Value below minimum exceeded");
        }
      }
    }
    // const
    if (const auto &aConst = aValidation.getConst()) {
      if (*aConst != theJson) {
        return makeError(
            "Element does not match the expected constant (const)");
      }
    }

    if (theJson.getType() == json_type::ARRAY) {
      if (aValidation.getMaxContains() < aValidation.getMinContains()) {
        return makeError(
            "Impossible for array to satisfy maxContains<minContains "
            "expectation (probably schema error)");
      }
      // contains
      if (const auto &aContains = aApplicator.getContains()) {
        size_t aNumMatching{0};
        for (const auto &aElm : theJson.toArray()) {
          ValidationResult aSubVal =
              validate(aElm, aContains->getRefInternal());
          if (!aSubVal)
            ++aNumMatching;
        }
        if (aNumMatching == 0u && aValidation.getMinContains() > 0) {
          return makeError("Expected element not found in array (contains)");
        }
        if (aNumMatching < aValidation.getMinContains()) {
          return makeError("Fewer array elements than expected match");
        }
        if (aNumMatching > aValidation.getMaxContains()) {
          return makeError("More array elements than expected match");
        }
      }
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
