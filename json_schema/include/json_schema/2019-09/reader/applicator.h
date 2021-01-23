#ifndef JSON_SCHEMA_2019_09_READER_APPLICATOR_H
#define JSON_SCHEMA_2019_09_READER_APPLICATOR_H

#include "json_schema/2019-09/model/applicator.h"
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
        return ErrorHandling::template makeError<InfoMaybe>(theMsg);
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
        aSubSchemaInfos.NUM_SCHEMA_LIST_ITEMS = theJson.toArray().size();
        for (const auto &aSubJson : theJson.toArray()) {
          const auto aInfoOrError = Reader::read(aSubJson);
          if (ErrorHandling::isError(aInfoOrError))
            return ErrorHandling::template convertError<InfoMaybe>(
                aInfoOrError);
          aSubSchemaInfos += ErrorHandling::unwrap(aInfoOrError);
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
        else {
          const auto aInfoOrError = Reader::read(theValue);
          if (ErrorHandling::isError(aInfoOrError))
            return ErrorHandling::template convertError<InfoMaybe>(
                aInfoOrError);
          SchemaInfo aResult = ErrorHandling::unwrap(aInfoOrError);
          aResult.NUM_SCHEMA_LIST_ITEMS += 1;
          return aResult;
        }
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
        aResult.NUM_SCHEMA_DICT_ENTRIES += theValue.toObject().size();
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
      auto &aApplicator = theSchema.template getSection<SchemaApplicator>();
      if (theKey == "additionalItems") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsAdditionalItems = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "unevaluatedItems") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsUnevaluatedItems =
            Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "contains") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsContains = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "additionalProperties") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsAdditionalProperties =
            Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "unevaluatedProperties") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsUnevaluatedProperties =
            Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "propertyNames") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsPropertyNames = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "if") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsIf = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "then") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsThen = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "else") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsElse = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "not") {
        const auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aApplicator.itsNot = Reader::ErrorHandling::unwrap(aSchema);
      } else if (theKey == "properties") {
        auto aSchemaDict = theReader.readSchemaDict(theValue);
        if (Reader::ErrorHandling::isError(aSchemaDict))
          return Reader::ErrorHandling::template convertError<bool>(
              aSchemaDict);
        aApplicator.itsProperties = Reader::ErrorHandling::unwrap(aSchemaDict);
      } else if (theKey == "patternProperties") {
        auto aSchemaDict = theReader.readSchemaDict(theValue);
        if (Reader::ErrorHandling::isError(aSchemaDict))
          return Reader::ErrorHandling::template convertError<bool>(
              aSchemaDict);
        aApplicator.itsPatternProperties =
            Reader::ErrorHandling::unwrap(aSchemaDict);
      } else if (theKey == "dependentSchemas") {
        auto aSchemaDict = theReader.readSchemaDict(theValue);
        if (Reader::ErrorHandling::isError(aSchemaDict))
          return Reader::ErrorHandling::template convertError<bool>(
              aSchemaDict);
        aApplicator.itsDependentSchemas =
            Reader::ErrorHandling::unwrap(aSchemaDict);
      } else if (theKey == "items") {
        using TypeEnum = decltype(std::declval<JSON>().getType());
        if (theValue.getType() != TypeEnum::ARRAY) {
          auto aSchemaBuf =
              theReader.template allocateList<typename Reader::SchemaRef>(1);
          const auto aSchema = theReader.readSchema(theValue);
          if (Reader::ErrorHandling::isError(aSchema))
            return Reader::ErrorHandling::template convertError<bool>(aSchema);
          theReader.setListItem(aSchemaBuf, 0,
                                Reader::ErrorHandling::unwrap(aSchema));
          aApplicator.itsItems = aSchemaBuf;
        } else {
          auto aSchemaBuf = theReader.readSchemaList(theValue);
          if (Reader::ErrorHandling::isError(aSchemaBuf))
            return Reader::ErrorHandling::template convertError<bool>(
                aSchemaBuf);
          aApplicator.itsItems = Reader::ErrorHandling::unwrap(aSchemaBuf);
        }
      } else if (theKey == "allOf") {
        auto aSchemaBuf = theReader.readSchemaList(theValue);
        if (Reader::ErrorHandling::isError(aSchemaBuf))
          return Reader::ErrorHandling::template convertError<bool>(aSchemaBuf);
        aApplicator.itsAllOf = Reader::ErrorHandling::unwrap(aSchemaBuf);
      } else if (theKey == "anyOf") {
        auto aSchemaBuf = theReader.readSchemaList(theValue);
        if (Reader::ErrorHandling::isError(aSchemaBuf))
          return Reader::ErrorHandling::template convertError<bool>(aSchemaBuf);
        aApplicator.itsAnyOf = Reader::ErrorHandling::unwrap(aSchemaBuf);
      } else if (theKey == "oneOf") {
        auto aSchemaBuf = theReader.readSchemaList(theValue);
        if (Reader::ErrorHandling::isError(aSchemaBuf))
          return Reader::ErrorHandling::template convertError<bool>(aSchemaBuf);
        aApplicator.itsOneOf = Reader::ErrorHandling::unwrap(aSchemaBuf);
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_READER_APPLICATOR_H
