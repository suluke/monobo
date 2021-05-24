#ifndef JSON_SCHEMA_SCHEMA_VALIDATOR_H
#define JSON_SCHEMA_SCHEMA_VALIDATOR_H

#include <cmath>
#include <regex>

#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/utf-8.h"
#include "json_schema/2019-09/model/applicator.h"
#include "json_schema/2019-09/model/content.h"
#include "json_schema/2019-09/model/core.h"
#include "json_schema/2019-09/model/format.h"
#include "json_schema/2019-09/model/metadata.h"
#include "json_schema/2019-09/model/validation.h"
#include "json_schema/2019-09/validate/error_codes.h"

namespace json_schema {

template <typename ContextTy,
          typename ErrorHandling = cjson::ErrorWillReturnNone,
          typename Encoding = cjson::Utf8>
class SchemaValidator {
public:
  using SchemaRef = typename ContextTy::SchemaRef;
  using SchemaAccessor = SchemaObjectAccessor<ContextTy>;
  using ErrorDetail = typename ErrorHandling::ErrorDetail;
  using ValidationResult = typename std::optional<ErrorDetail>;

  constexpr SchemaValidator(const SchemaRef &theSchema,
                            const ContextTy &theContext)
      : itsContext{theContext}, itsSchema{theSchema} {}
  SchemaValidator(const SchemaAccessor &theSchema)
      : itsContext{theSchema.getContext()}, itsSchema{
                                                theSchema.getRefInternal()} {}

  template <typename JSON>
  constexpr ValidationResult validate(const JSON &theJson) const {
    return validate(theJson, itsSchema);
  }

  template <typename JSON>
  constexpr ValidationResult validate(const JSON &theJson,
                                      const SchemaRef theSchema) const {
    return validate(theJson, SchemaAccessor{itsContext, theSchema});
  }
  template <typename JSON>
  constexpr ValidationResult validate(const JSON &theJson,
                                      const SchemaAccessor theSchema) const {
    using json_type = decltype(theJson.getType());
    SchemaAccessor aSchema{theSchema};
    if (aSchema.isTrueSchema())
      return std::nullopt;
    if (aSchema.isFalseSchema())
      return makeError(ErrorCode::UNKNOWN,
                       "Schema to validate against is `false`");
    const auto &aApplicator = aSchema.template getSection<SchemaApplicator>();
    const auto &aValidation = aSchema.template getSection<SchemaValidation>();
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
        return makeError(ErrorCode::UNKNOWN, "Type is not allowed");
    }
    // number
    if (theJson.getType() == json_type::NUMBER) {
      // minimum
      if (const auto &aMinimum = aValidation.getMinimum();
          aMinimum.has_value()) {
        if (aMinimum > theJson.toNumber())
          return makeError(ErrorCode::UNKNOWN, "Value below minimum");
      }

      // maximum
      if (const auto &aMaximum = aValidation.getMaximum();
          aMaximum.has_value()) {
        if (aMaximum < theJson.toNumber())
          return makeError(ErrorCode::UNKNOWN, "Value above maximum");
      }

      // exclusiveMinimum
      if (const auto &aExcMinimum = aValidation.getExclusiveMinimum();
          aExcMinimum.has_value()) {
        if (aExcMinimum >= theJson.toNumber())
          return makeError(ErrorCode::UNKNOWN, "Value below exclusive minimum");
      }

      // exclusiveMaximum
      if (const auto &aExcMaximum = aValidation.getExclusiveMaximum();
          aExcMaximum.has_value()) {
        if (aExcMaximum <= theJson.toNumber()) {
          return makeError(ErrorCode::UNKNOWN, "Value above exclusive maximum");
        }
      }

      // multipleOf
      if (const auto &aMulOf = aValidation.getMultipleOf();
          aMulOf.has_value()) {
        const double aQuot = theJson.toNumber() / aMulOf;
        if (aQuot != std::trunc(aQuot))
          return makeError(ErrorCode::UNKNOWN,
                           "Value is not a multiple of expected (multipleOf)");
      }
    }
    // string
    if (theJson.getType() == json_type::STRING) {
      const auto &aStr = theJson.toString();
      if (const auto aMinLen = aValidation.getMinLength();
          aMinLen.has_value()) {
        const auto aDecodedLength = decodeLength(aStr);
        if (ErrorHandling::isError(aDecodedLength))
          return ErrorHandling::getError(aDecodedLength);
        if (aMinLen > ErrorHandling::unwrap(aDecodedLength)) {
          return makeError(
              ErrorCode::UNKNOWN,
              "String has fewer characters than required (minLength)");
        }
      }
      if (const auto aMaxLen = aValidation.getMaxLength();
          aMaxLen.has_value()) {
        if (aMaxLen < aStr.size()) {
          const auto aDecodedLength = decodeLength(aStr);
          if (ErrorHandling::isError(aDecodedLength))
            return ErrorHandling::getError(aDecodedLength);
          if (aMaxLen < ErrorHandling::unwrap(aDecodedLength))
            return makeError(
                ErrorCode::UNKNOWN,
                "String has more characters than allowed (maxLength)");
        }
      }
    }
    // const
    if (const auto &aConst = aValidation.getConst()) {
      if (*aConst != theJson)
        return makeError(
            ErrorCode::UNKNOWN,
            "Element does not match the expected constant (const)");
    }
    // enum
    if (const auto &aEnum = aValidation.getEnum()) {
      bool aFoundMatching = false;
      for (const auto &aElm : *aEnum)
        if (aElm == theJson) {
          aFoundMatching = true;
          break;
        }
      if (!aFoundMatching)
        return makeError(
            ErrorCode::UNKNOWN,
            "Element does not match any of the expected constants (enum)");
    }

    // applicator
    {
      // not
      if (const auto &aNot = aApplicator.getNot()) {
        if (!validate(theJson, *aNot))
          return makeError(ErrorCode::UNKNOWN,
                           "Expected schema not to match (not)");
      }
      // allOf
      if (const auto &aAllOf = aApplicator.getAllOf()) {
        for (const auto &aSchema : *aAllOf)
          if (const auto aRes = validate(theJson, aSchema))
            return makeError(
                ErrorCode::UNKNOWN,
                "Element does not match (at least) one of the given "
                "schemas (allOf)",
                *aRes);
      }
      // anyOf
      if (const auto &aAnyOf = aApplicator.getAnyOf()) {
        bool aHasMatching = false;
        for (const auto &aSchema : *aAnyOf) {
          if (!validate(theJson, aSchema)) {
            aHasMatching = true;
            break;
          }
        }
        if (!aHasMatching)
          return makeError(
              ErrorCode::UNKNOWN,
              "Element does not match any of the given schemas (anyOf)");
      }
      // oneOf
      if (const auto &aOneOf = aApplicator.getOneOf()) {
        int aNumMatching{0};
        for (const auto &aSchema : *aOneOf)
          if (!validate(theJson, aSchema))
            ++aNumMatching;
        if (aNumMatching == 0)
          return makeError(
              ErrorCode::UNKNOWN,
              "Expected one of the given schemas to match, but none "
              "matched (oneOf)");
        if (aNumMatching != 1)
          return makeError(
              ErrorCode::UNKNOWN,
              "Expected exactly one of the given schemas to match, "
              "but more than one matched (oneOf)");
      }
    }

    // object
    if (theJson.getType() == json_type::OBJECT) {
      const auto &aJsonObject = theJson.toObject();
      const auto &aPatternProps = aApplicator.getPatternProperties();

      // properties
      if (const auto &aProps = aApplicator.getProperties()) {
        for (const auto &aNameSchemaPair : *aProps) {
          if (const auto &aProp = aJsonObject[aNameSchemaPair.first]) {
            ValidationResult aSubVal = validate(*aProp, aNameSchemaPair.second);
            if (aSubVal)
              return makeError(
                  ErrorCode::UNKNOWN,
                  "Schema verification of property failed (properties)");
          }
        }
        if (const auto &aAdditionalProps =
                aApplicator.getAdditionalProperties()) {
          // checks if a property is matched by patternProperties
          const auto isMatched =
              [&aPatternProps](const std::string_view &aKey) -> bool {
            const auto matchesKey = [&aKey](const auto &aKVPair) -> bool {
              std::regex aRegex{aKVPair.first.begin(), aKVPair.first.end()};
              return std::regex_search(aKey.begin(), aKey.end(), aRegex);
            };
            return aPatternProps &&
                   std::any_of(aPatternProps->begin(), aPatternProps->end(),
                               matchesKey);
          };
          for (const auto &aKVPair : aJsonObject) {
            if (aProps->contains(aKVPair.first))
              continue;
            if (isMatched(aKVPair.first))
              continue;
            if (auto aErrorMaybe = validate(aKVPair.second, *aAdditionalProps)) {
              return makeError(
                  ErrorCode::UNKNOWN,
                  "Additional property does not match (additionalProperties)");
            }
          }
        }
      }
      // minProperties
      if (const auto &aMinProps = aValidation.getMinProperties();
          aMinProps.has_value()) {
        if (aJsonObject.size() < aMinProps) {
          return makeError(ErrorCode::UNKNOWN,
                           "Object has not enough properties (minProperties)");
        }
      }
      // maxProperties
      if (const auto &aMaxProps = aValidation.getMaxProperties();
          aMaxProps.has_value()) {
        if (aJsonObject.size() > aMaxProps) {
          return makeError(ErrorCode::UNKNOWN,
                           "Object has too many properties (maxProperties)");
        }
      }

      // required
      if (const auto &aRequired = aValidation.getRequired()) {
        for (const auto &aKey : *aRequired) {
          if (!aJsonObject[aKey])
            return makeError(ErrorCode::UNKNOWN,
                             "Required property not found (required)");
        }
      }
    }

    // array
    if (theJson.getType() == json_type::ARRAY) {
      const auto &aJsonArray = theJson.toArray();
      // maxItems
      if (const auto aMaxItems = aValidation.getMaxItems()) {
        if (aMaxItems < aJsonArray.size()) {
          return makeError(
              ErrorCode::UNKNOWN,
              "Array has more items than expected (maxProperties)");
        }
      }
      // minItems
      if (const auto aMinItems = aValidation.getMinItems()) {
        if (aMinItems > aJsonArray.size()) {
          return makeError(
              ErrorCode::UNKNOWN,
              "Array has fewer items than required (minProperties)");
        }
      }

      // items
      if (const auto &aItems = aApplicator.getItems()) {
        if (aItems->index() == 0) {
          const auto &aItemSchemaList = json_schema::get<size_t{0}>(*aItems);
          for (size_t aIdx = 0;
               aIdx < aItemSchemaList.size() && aIdx < aJsonArray.size();
               ++aIdx) {
            if (validate(aJsonArray[aIdx], aItemSchemaList[aIdx]))
              return makeError(
                  ErrorCode::UNKNOWN,
                  "Item does not match expected schema at position (items)");
          }
          // additionalItems
          if (aJsonArray.size() > aItemSchemaList.size()) {
            if (const auto &aAdditionalItems =
                    aApplicator.getAdditionalItems()) {
              for (size_t aIdx = aItemSchemaList.size();
                   aIdx < aJsonArray.size(); ++aIdx) {
                if (auto aErrMaybe =
                        validate(aJsonArray[aIdx], *aAdditionalItems))
                  return makeError(ErrorCode::UNKNOWN,
                                   "Item does not match additional items "
                                   "schema (additionalItems)",
                                   *aErrMaybe);
              }
            }
          }
        } else if (aItems->index() == 1) {
          const auto &aItemSchema = json_schema::get<size_t{1}>(*aItems);
          for (const auto &aItem : aJsonArray) {
            if (auto aErrMaybe = validate(aItem, aItemSchema))
              return makeError(ErrorCode::UNKNOWN,
                               "Item does not match expected schema (items)",
                               *aErrMaybe);
          }
        } else {
          throw "Implementation error";
        }
      }

      // uniqueItems
      if (aValidation.getUniqueItems()) {
        size_t aPos = 0;
        for (const auto &aElm1 : aJsonArray) {
          for (size_t aIdx = ++aPos; aIdx < aJsonArray.size(); ++aIdx)
            if (aElm1 == aJsonArray[aIdx])
              return makeError(ErrorCode::UNKNOWN,
                               "Found duplicate item (uniqueItems)");
        }
      }

      if (aValidation.getMaxContains() < aValidation.getMinContains()) {
        return makeError(
            ErrorCode::UNKNOWN,
            "Impossible for array to satisfy maxContains<minContains "
            "expectation (probably schema error)");
      }
      // contains
      if (const auto &aContains = aApplicator.getContains()) {
        size_t aNumMatching{0};
        for (const auto &aElm : aJsonArray) {
          ValidationResult aSubVal =
              validate(aElm, aContains->getRefInternal());
          if (!aSubVal)
            ++aNumMatching;
        }
        // minContains
        if (aNumMatching == 0u && aValidation.getMinContains() > 0) {
          return makeError(ErrorCode::UNKNOWN,
                           "Expected element not found in array (contains)");
        }
        if (aNumMatching < aValidation.getMinContains()) {
          return makeError(ErrorCode::UNKNOWN,
                           "Fewer array elements than expected match");
        }
        // maxContains
        if (aNumMatching > aValidation.getMaxContains()) {
          return makeError(ErrorCode::UNKNOWN,
                           "More array elements than expected match");
        }
      }
    }
    return std::nullopt;
  }

private:
  constexpr ErrorDetail makeError(const ErrorCode theEC,
                                  const char *const theMsg) const {
    return ErrorHandling::getError(
        ErrorHandling::template makeError<bool>(theEC, theMsg));
  }
  constexpr ErrorDetail makeError(const ErrorCode theEC,
                                  const char *const theMsg,
                                  const ErrorDetail &theSubError) const {
    return theSubError;
  }
  using DecodeLengthResult = typename ErrorHandling::template ErrorOr<size_t>;
  constexpr DecodeLengthResult
  decodeLength(const std::string_view theStr) const {
    size_t aResult{0};
    std::string_view aRem = theStr;
    while (!aRem.empty()) {
      const size_t aDecodeLen = itsEncoding.decodeFirst(aRem).second;
      if (aDecodeLen == 0)
        return ErrorHandling::template makeError<size_t>(
            ErrorCode::ENCODING_ERROR, "Failed to decode string");
      aRem.remove_prefix(aDecodeLen);
      ++aResult;
    }
    return aResult;
  }

  ContextTy itsContext;
  SchemaRef itsSchema;
  Encoding itsEncoding;
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_VALIDATOR_H
