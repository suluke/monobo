#ifndef JSON_SCHEMA_2019_09_READER_VALIDATION_H
#define JSON_SCHEMA_2019_09_READER_VALIDATION_H

#include "json_schema/2019-09/model/validation.h"
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
          aResult.NUM_CHARS += aElm.toString().size();
          aResult.NUM_STRING_LIST_ITEMS += 1;
        }
      } else if (theKey == "dependentRequired") {
        if (aType != TypeEnum::OBJECT)
          return makeError("dependentRequired MUST be of type object "
                           "(2019-09/Validation:6.5.4.)");
        for (const auto &aKVPair : theValue.toObject()) {
          aResult.NUM_CHARS += aKVPair.first.size();
          const auto &aDeps = aKVPair.second;
          if (aDeps.getType() != TypeEnum::ARRAY)
            return makeError(
                "dependentRequired properties MUST be of type array "
                "(2019-09/Validation:6.5.4.)");
          aResult.NUM_STRINGLIST_DICT_ENTRIES += 1;
          for (const auto &aDepProp : aDeps.toArray()) {
            if (aDepProp.getType() != TypeEnum::STRING)
              return makeError(
                  "dependentRequired properties' array entries MUST "
                  "be of type string "
                  "(2019-09/Validation:6.5.4.)");
            aResult.NUM_CHARS += aDepProp.toString().size();
            aResult.NUM_STRING_LIST_ITEMS += 1;
          }
        }
      } else if (theKey == "const") {
        aResult.JSON_INFO += cjson::DocumentInfo::read(theValue);
        aResult.NUM_JSON_REFS += 1;
      } else if (theKey == "enum") {
        if (aType != TypeEnum::ARRAY)
          return makeError(
              "enum MUST be of type array (2019-09/Validation:6.1.3.");
        for (const auto &aElm : theValue.toArray()) {
          aResult.JSON_INFO += cjson::DocumentInfo::read(aElm);
          aResult.NUM_JSON_LIST_ITEMS += 1;
          aResult.NUM_JSON_REFS += 1;
        }
      } else if (theKey == "type") {
        if (aType == TypeEnum::STRING) {
          aResult.NUM_TYPES_LIST_ITEMS += 1;
        } else if (aType == TypeEnum::ARRAY) {
          for (const auto &aElm : theValue.toArray()) {
            if (aElm.getType() != TypeEnum::STRING)
              return makeError("type array entry MUST be of type string "
                               "(2019-09/Validation:6.1.1.");
            aResult.NUM_TYPES_LIST_ITEMS += 1;
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
      auto &aValidation = theSchema.template getSection<SchemaValidation>();
      if (theKey == "multipleOf") {
        aValidation.itsMultipleOf = theValue.toNumber();
      } else if (theKey == "maximum") {
        aValidation.itsMaximum = theValue.toNumber();
      } else if (theKey == "exclusiveMaximum") {
        aValidation.itsExclusiveMinimum = theValue.toNumber();
      } else if (theKey == "minimum") {
        aValidation.itsMinimum = theValue.toNumber();
      } else if (theKey == "exclusiveMinimum") {
        aValidation.itsExclusiveMinimum = theValue.toNumber();
      } else if (theKey == "maxLength") {
        aValidation.itsMaxLength = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "minLength") {
        aValidation.itsMinLength = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "pattern") {
        aValidation.itsPattern = Reader::toPtr(theReader.allocateString(theValue.toString()));
      } else if (theKey == "maxItems") {
        aValidation.itsMaxItems = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "minItems") {
        aValidation.itsMinItems = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "uniqueItems") {
        aValidation.itsUniqueItems = theValue.toBool();
      } else if (theKey == "maxContains") {
        aValidation.itsMaxContains = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "minContains") {
        aValidation.itsMinContains = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "maxProperties") {
        aValidation.itsMaxProperties = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "minProperties") {
        aValidation.itsMinProperties = static_cast<size_t>(theValue.toNumber());
      } else if (theKey == "required") {
        auto aStringBuf = theReader.readStringList(theValue);
        if (Reader::ErrorHandling::isError(aStringBuf))
          return Reader::ErrorHandling::template convertError<bool>(aStringBuf);
        aValidation.itsRequired = Reader::toPtr(Reader::ErrorHandling::unwrap(aStringBuf));
      } else if (theKey == "dependentRequired") {
        using StringRef = typename Reader::Storage::StringRef;
        using StringList = typename Reader::template List<StringRef>;
        auto aDict = theReader.template allocateMap<StringRef, StringList>(
            theValue.toObject().size());
        ptrdiff_t aIdx{0};
        for (const auto &aKVPair : theValue.toObject()) {
          const auto aKey = theReader.allocateString(aKVPair.first);
          const auto aStringsOrError = theReader.readStringList(aKVPair.second);
          if (Reader::ErrorHandling::isError(aStringsOrError))
            return Reader::ErrorHandling::template convertError<bool>(
                aStringsOrError);
          theReader.setMapEntry(aDict, aIdx++, aKey,
                                Reader::ErrorHandling::unwrap(aStringsOrError));
        }
        aValidation.itsDependentRequired = Reader::toPtr(aDict);
      } else if (theKey == "const") {
        aValidation.itsConst = Reader::toPtr(theReader.allocateJson(theValue));
      } else if (theKey == "enum") {
        auto aJsons =
            theReader.template allocateList<typename Reader::Storage::JsonRef>(
                theValue.toArray().size());
        ptrdiff_t aIdx{0};
        for (const auto aElm : theValue.toArray()) {
          theReader.setListItem(aJsons, aIdx++, theReader.allocateJson(aElm));
        }
        aValidation.itsEnum = Reader::toPtr(aJsons);
      } else if (theKey == "type") {
        using TypeEnum = decltype(std::declval<JSON>().getType());
        const auto aStringToType =
            [](const std::string_view theString) -> std::optional<Types> {
          if (theString == "array")
            return Types::ARRAY;
          else if (theString == "boolean")
            return Types::BOOLEAN;
          else if (theString == "integer")
            return Types::INTEGER;
          else if (theString == "null")
            return Types::NUL;
          else if (theString == "number")
            return Types::NUMBER;
          else if (theString == "object")
            return Types::OBJECT;
          else if (theString == "string")
            return Types::STRING;
          else
            return std::nullopt;
        };
        if (theValue.getType() == TypeEnum::STRING) {
          auto aTypesList = theReader.template allocateList<Types>(1u);
          const auto aTypeMaybe = aStringToType(theValue.toString());
          if (!aTypeMaybe)
            return Reader::ErrorHandling::template makeError<bool>(
                "Encountered unknown type");
          theReader.setListItem(aTypesList, 0, *aTypeMaybe);
          aValidation.itsType = Reader::toPtr(aTypesList);
        } else if (theValue.getType() == TypeEnum::ARRAY) {
          auto aTypesList =
              theReader.template allocateList<Types>(theValue.toArray().size());
          ptrdiff_t aIdx{0};
          for (const auto &aJsonItem : theValue.toArray()) {
            const auto aTypeMaybe = aStringToType(aJsonItem.toString());
            if (!aTypeMaybe)
              return Reader::ErrorHandling::template makeError<bool>(
                  "Encountered unknown type");
            theReader.setListItem(aTypesList, aIdx++, *aTypeMaybe);
          }
          aValidation.itsType = Reader::toPtr(aTypesList);
        } else {
          return Reader::ErrorHandling::template makeError<bool>(
              "`type` property is neither string nor list");
        }
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_READER_VALIDATION_H
