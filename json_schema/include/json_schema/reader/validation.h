#ifndef JSON_SCHEMA_READER_VALIDATION_H
#define JSON_SCHEMA_READER_VALIDATION_H

#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderValidation {
public:
  struct InfoReader {
    using InfoMaybe = std::optional<SchemaInfo>;

    template <typename Reader>
    static constexpr auto readItem(const std::string_view &theKey,
                                   const typename Reader::JSON &theValue) ->
        typename Reader::ErrorOrInfoMaybe {
      using JSON = typename Reader::JSON;
      using ErrorHandling = typename Reader::ErrorHandling;
      const auto makeError = [](const char *theMsg) {
        return ErrorHandling::template makeError<InfoMaybe>(theMsg);
      };
      SchemaInfo aResult;
      using TypeEnum = decltype(std::declval<JSON>().getType());
      const TypeEnum aType = theValue.getType();

      if (theKey == "multipleOf") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "multipleOf MUST be of type number (2019-09/Validation:6.2.1.");
      } else if (theKey == "maximum") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "maximum MUST be of type number (2019-09/Validation:6.2.2.");
      } else if (theKey == "exclusiveMaximum") {
        if (aType != TypeEnum::NUMBER)
          return makeError("exclusiveMaximum MUST be of type number "
                           "(2019-09/Validation:6.2.3.");
      } else if (theKey == "minimum") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "minimum MUST be of type number (2019-09/Validation:6.2.4.");
      } else if (theKey == "exclusiveMinimum") {
        if (aType != TypeEnum::NUMBER)
          return makeError("exclusiveMinimum MUST be of type number "
                           "(2019-09/Validation:6.2.5.");
      } else if (theKey == "maxLength") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "maxLength MUST be of type number (2019-09/Validation:6.3.1.");
      } else if (theKey == "minLength") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "minLength MUST be of type number (2019-09/Validation:6.3.2.");
      } else if (theKey == "pattern") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "pattern MUST be of type string (2019-09/Validation:6.3.3.");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "maxItems") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "maxItems MUST be of type number (2019-09/Validation:6.4.1.");
      } else if (theKey == "minItems") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "minItems MUST be of type number (2019-09/Validation:6.4.2.");
      } else if (theKey == "uniqueItems") {
        if (aType != TypeEnum::BOOL)
          return makeError(
              "uniqueItems MUST be of type boolean (2019-09/Validation:6.4.3.");
      } else if (theKey == "maxContains") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "maxContains MUST be of type number (2019-09/Validation:6.4.4.");
      } else if (theKey == "minContains") {
        if (aType != TypeEnum::NUMBER)
          return makeError(
              "minContains MUST be of type number (2019-09/Validation:6.4.5.");
      } else if (theKey == "maxProperties") {
        if (aType != TypeEnum::NUMBER)
          return makeError("maxProperties MUST be of type number "
                           "(2019-09/Validation:6.5.1.");
      } else if (theKey == "minProperties") {
        if (aType != TypeEnum::NUMBER)
          return makeError("minProperties MUST be of type number "
                           "(2019-09/Validation:6.5.2.");
      } else if (theKey == "required") {
        if (aType != TypeEnum::ARRAY)
          return makeError(
              "required MUST be of type array (2019-09/Validation:6.5.3.");
        for (const auto &aElm : theValue.toArray()) {
          if (aElm.getType() != TypeEnum::STRING)
            return makeError("required entries MUST be of type string "
                             "(2019-09/Validation:6.5.3.");
          // aResult.NUM_REQUIRED_ENTRIES += 1;
          aResult.NUM_CHARS += aElm.toString().size();
        }
      } else if (theKey == "dependentRequired") {
        if (aType != TypeEnum::OBJECT)
          return makeError("dependentRequired MUST be of type object "
                           "(2019-09/Validation:6.5.4.)");
        // aResult.NUM_DEP_REQUIRED_LISTS += theValue.toObject().size();
        for (const auto &aKVPair : theValue.toObject()) {
          aResult.NUM_CHARS += aKVPair.first.size();
          const auto &aDeps = aKVPair.second;
          if (aDeps.getType() != TypeEnum::ARRAY)
            return makeError(
                "dependentRequired properties MUST be of type array "
                "(2019-09/Validation:6.5.4.)");
          // aResult.NUM_DEP_REQUIRED_LIST_ENTRIES += aDeps.toArray().size();
          for (const auto &aDepProp : aDeps.toArray()) {
            if (aDepProp.getType() != TypeEnum::STRING)
              return makeError(
                  "dependentRequired properties' array entries MUST "
                  "be of type string "
                  "(2019-09/Validation:6.5.4.)");
            aResult.NUM_CHARS += aDepProp.toString().size();
          }
        }
      } else if (theKey == "const") {
        aResult.JSON_INFO += cjson::DocumentInfo::read(theValue);
      } else if (theKey == "enum") {
        if (aType != TypeEnum::ARRAY)
          return makeError(
              "enum MUST be of type array (2019-09/Validation:6.1.3.");
        for (const auto &aElm : theValue.toArray()) {
          aResult.JSON_INFO += cjson::DocumentInfo::read(aElm);
          // aResult.NUM_ENUM_ENTRIES += 1;
        }
      } else if (theKey == "type") {
        if (aType == TypeEnum::STRING) {
          // aResult.NUM_TYPE_ENTRIES += 1;
        } else if (aType == TypeEnum::ARRAY) {
          for (const auto &aElm : theValue.toArray()) {
            if (aElm.getType() != TypeEnum::STRING)
              return makeError("type array entry MUST be of type string "
                               "(2019-09/Validation:6.1.1.");
            // aResult.NUM_TYPE_ENTRIES += 1;
          }
        } else {
          return makeError("type MUST be of type array or string "
                           "(2019-09/Validation:6.1.1.");
        }
      } else {
        return InfoMaybe{};
      }

      return aResult;
    }
  };
  struct SchemaReader {
    template <typename Reader, typename JSON>
    static constexpr typename Reader::ErrorOrConsumed
    readSchema(Reader &theReader, typename Reader::SchemaObject &theSchema,
               const std::string_view &theKey, const JSON &theValue) {
      if (theKey == "multipleOf") {
      } else if (theKey == "maximum") {
      } else if (theKey == "exclusiveMaximum") {
      } else if (theKey == "minimum") {
      } else if (theKey == "exclusiveMinimum") {
      } else if (theKey == "maxLength") {
      } else if (theKey == "minLength") {
      } else if (theKey == "pattern") {
      } else if (theKey == "maxItems") {
      } else if (theKey == "minItems") {
      } else if (theKey == "uniqueItems") {
      } else if (theKey == "maxContains") {
      } else if (theKey == "minContains") {
      } else if (theKey == "maxProperties") {
      } else if (theKey == "minProperties") {
      } else if (theKey == "required") {
      } else if (theKey == "dependentRequired") {
      } else if (theKey == "const") {
      } else if (theKey == "enum") {
      } else if (theKey == "type") {
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_READER_VALIDATION_H