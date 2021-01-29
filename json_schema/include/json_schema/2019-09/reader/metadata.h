#ifndef JSON_SCHEMA_2019_09_READER_METADATA_H
#define JSON_SCHEMA_2019_09_READER_METADATA_H

#include "json_schema/2019-09/model/metadata.h"
#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderMetadata {
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

      if (theKey == "title") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "title must be of type string (2019-09/Validation:9.1.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "description") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "description must be of type string (2019-09/Validation:9.1.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "default") {
        aResult.JSON_INFO += cjson::DocumentInfo::read(theValue);
        aResult.NUM_JSON_REFS += 1;
      } else if (theKey == "deprecated") {
        if (aType != TypeEnum::BOOL)
          return makeError(
              "deprecated must be of type boolean (2019-09/Validation:9.3.)");
      } else if (theKey == "readOnly") {
        if (aType != TypeEnum::BOOL)
          return makeError(
              "readOnly must be of type boolean (2019-09/Validation:9.4.)");
      } else if (theKey == "writeOnly") {
        if (aType != TypeEnum::BOOL)
          return makeError(
              "writeOnly must be of type boolean (2019-09/Validation:9.4.)");
      } else if (theKey == "examples") {
        if (aType != TypeEnum::ARRAY)
          return makeError(
              "examples must be of type array (2019-09/Validation:9.5.)");
        aResult.NUM_JSON_LIST_ITEMS += theValue.toArray().size();
        aResult.NUM_JSON_REFS += theValue.toArray().size();
        for (const auto &aExample : theValue.toArray())
          aResult.JSON_INFO += cjson::DocumentInfo::read(aExample);
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
      auto &aMetadata = theSchema.template getSection<SchemaMetadata>();
      if (theKey == "title") {
        aMetadata.itsTitle = theReader.allocateString(theValue.toString());
      } else if (theKey == "description") {
        aMetadata.itsDescription = theReader.allocateString(theValue.toString());
      } else if (theKey == "default") {
        aMetadata.itsDefault = theReader.allocateJson(theValue);
      } else if (theKey == "deprecated") {
        aMetadata.itsDeprecated = theValue.toBool();
      } else if (theKey == "readOnly") {
        aMetadata.itsReadOnly = theValue.toBool();
      } else if (theKey == "writeOnly") {
        aMetadata.itsWriteOnly = theValue.toBool();
      } else if (theKey == "examples") {
        auto aJsons =
            theReader.template allocateList<typename Reader::Storage::Json>(
                theValue.toArray().size());
        ptrdiff_t aIdx{0};
        for (const auto aElm : theValue.toArray()) {
          theReader.setListItem(aJsons, aIdx++, theReader.allocateJson(aElm));
        }
        aMetadata.itsExamples = aJsons;
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_READER_METADATA_H
