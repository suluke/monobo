#ifndef JSON_SCHEMA_READER_APPLICATOR_H
#define JSON_SCHEMA_READER_APPLICATOR_H

#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderApplicator {
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
        return ErrorHandling::template makeError<InfoMaybe>();
      };
      using ErrorOrInfoMaybe =
          typename ErrorHandling::template ErrorOr<InfoMaybe>;
      using TypeEnum = decltype(std::declval<JSON>().getType());
      const TypeEnum aType = theValue.getType();

      // Helper functions
      using ResultTy = ErrorOrInfoMaybe;
      const auto aComputeSubSchema = [](const JSON &theJson) -> ResultTy {
        const auto aInfoOrError = Reader::read(theJson);
        if (ErrorHandling::isError(aInfoOrError))
          return ErrorHandling::template convertError<InfoMaybe>(aInfoOrError);
        return ErrorHandling::unwrap(aInfoOrError);
      };
      const auto aSumSchemaArray = [](const JSON &theJson) -> ResultTy {
        SchemaInfo aSubSchemaInfos{};
        for (const auto &aSubJson : theJson.toArray()) {
          const auto aInfoOrError = Reader::read(aSubJson);
          if (ErrorHandling::isError(aInfoOrError))
            return ErrorHandling::template convertError<InfoMaybe>(
                aInfoOrError);
          aSubSchemaInfos =
              aSubSchemaInfos + ErrorHandling::unwrap(aInfoOrError);
        }
        return aSubSchemaInfos;
      };

      if (theKey == "additionalItems" || theKey == "unevaluatedItems" ||
          theKey == "contains" || theKey == "additionalProperties" ||
          theKey == "unevaluatedProperties" || theKey == "propertyNames" ||
          theKey == "if" || theKey == "then" || theKey == "else" ||
          theKey == "not") {
        // key => schema
        return aComputeSubSchema(theValue);
      } else if (theKey == "items") {
        // key => schema | array[schema]
        if (aType == TypeEnum::ARRAY)
          return aSumSchemaArray(theValue);
        return aComputeSubSchema(theValue);
      } else if (theKey == "properties" || theKey == "patternProperties" ||
                 theKey == "dependentSchemas") {
        // key => object[schema]
        if (aType != TypeEnum::OBJECT)
          return makeError("properties/patternProperties/dependentSchemas MUST "
                           "be of type object");
        SchemaInfo aResult{};
        for (const auto &aKVPair : theValue.toObject()) {
          const auto aInfoOrError = Reader::read(aKVPair.second);
          if (ErrorHandling::isError(aInfoOrError))
            return aInfoOrError;
          aResult += ErrorHandling::unwrap(aInfoOrError);
          aResult.NUM_CHARS += aKVPair.first.size();
        }
        if (theKey == "properties")
          aResult.NUM_PROPERTIES_ENTRIES += theValue.toObject().size();
        else if (theKey == "patternProperties")
          aResult.NUM_PATTERN_PROPERTIES_ENTRIES += theValue.toObject().size();
        else if (theKey == "dependentSchemas")
          aResult.NUM_DEPENTENT_SCHEMA_ENTRIES += theValue.toObject().size();
        else
          throw "Implementation error: Unhandled case";
        return aResult;
      } else if (theKey == "allOf" || theKey == "anyOf" || theKey == "oneOf") {
        // key => array[schema]
        if (aType != TypeEnum::ARRAY)
          return makeError("allOf/anyOf/oneOf MUST be of type array");
        return aSumSchemaArray(theValue);
      }

      return InfoMaybe{};
    }
  };
  struct SchemaReader {
    template <typename Reader, typename JSON>
    static constexpr typename Reader::ErrorOrConsumed
    readSchema(Reader &theReader, typename Reader::SchemaObject &theSchema,
               const std::string_view &theKey, const JSON &theValue) {
      if (theKey == "additionalItems") {
      } else if (theKey == "unevaluatedItems") {
      } else if (theKey == "contains") {
      } else if (theKey == "additionalProperties") {
      } else if (theKey == "unevaluatedProperties") {
      } else if (theKey == "propertyNames") {
      } else if (theKey == "if") {
      } else if (theKey == "then") {
      } else if (theKey == "else") {
      } else if (theKey == "not") {
      } else if (theKey == "items") {
      } else if (theKey == "properties") {
      } else if (theKey == "patternProperties") {
      } else if (theKey == "dependentSchemas") {
      } else if (theKey == "allOf") {
      } else if (theKey == "anyOf") {
      } else if (theKey == "oneOf") {
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_READER_APPLICATOR_H
