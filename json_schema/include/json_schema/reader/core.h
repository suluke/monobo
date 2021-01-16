#ifndef JSON_SCHEMA_READER_CORE_H
#define JSON_SCHEMA_READER_CORE_H

#include "json_schema/model/format/uri_reference.h"
#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderCore {
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
      SchemaInfo aResult;
      using TypeEnum = decltype(std::declval<JSON>().getType());
      const auto aType = theValue.getType();
      if (theKey == "$id") {
        if (aType != TypeEnum::STRING)
          return makeError("$id MUST be of type string (2019-09/Core:8.2.2.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$schema") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "$schema MUST be of type string (2019-09/Core:8.1.1.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$anchor") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "$anchor MUST be of type string (2019-09/Core:8.2.3.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$ref") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "$ref MUST be of type string (2019-09/Core:8.2.4.1.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$recursiveRef") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "$recursiveRef MUST be of type string (2019-09/Core:8.2.4.2.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$recursiveAnchor") {
        if (aType != TypeEnum::BOOL)
          return makeError("$recursiveAnchor MUST be of type "
                           "boolean (2019-09/Core:8.2.4.2.2.)");
      } else if (theKey == "$vocabulary") {
        const auto aVocabInfo = computeVocabulary<Reader>(theValue);
        if (ErrorHandling::isError(aVocabInfo))
          return ErrorHandling::template convertError<InfoMaybe>(aVocabInfo);
        aResult += ErrorHandling::unwrap(aVocabInfo);
      } else if (theKey == "$comment") {
        if (aType != TypeEnum::STRING)
          return makeError(
              "$comment MUST be of type string (2019-09/Core:8.3.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "$defs") {
        const auto aDefsInfo = computeDefs<Reader>(theValue);
        if (ErrorHandling::isError(aDefsInfo))
          return ErrorHandling::template convertError<InfoMaybe>(aDefsInfo);
        aResult += ErrorHandling::unwrap(aDefsInfo);
      } else {
        return InfoMaybe{};
      }
      return aResult;
    }
    template <typename Reader>
    static constexpr
        typename Reader::ErrorHandling::template ErrorOr<SchemaInfo>
        computeVocabulary(const typename Reader::JSON &theJson) {
      const auto makeError = [](const char *theMsg) {
        return Reader::ErrorHandling::template makeError<SchemaInfo>();
      };
      SchemaInfo aResult;
      using TypeEnum = decltype(theJson.getType());
      if (theJson.getType() != TypeEnum::OBJECT)
        return makeError("$vocabulary MUST be an object (2019-09/Core:8.1.2.)");
      const auto aVocabObj = theJson.toObject();
      for (const auto &aVocabKVPair : aVocabObj) {
        if (aVocabKVPair.second.getType() != TypeEnum::BOOL)
          return makeError(
              "$vocabulary entry must have type bool (2019-09/Core:8.1.2)");
        aResult.NUM_VOCAB_ENTRIES += 1;
        aResult.NUM_CHARS += aVocabKVPair.first.size();
      }
      return aResult;
    }
    template <typename Reader>
    static constexpr
        typename Reader::ErrorHandling::template ErrorOr<SchemaInfo>
        computeDefs(const typename Reader::JSON &theJson) {
      const auto makeError = [](const char *theMsg) {
        return Reader::ErrorHandling::template makeError<SchemaInfo>();
      };
      SchemaInfo aResult;
      using TypeEnum = decltype(theJson.getType());
      if (theJson.getType() != TypeEnum::OBJECT)
        return makeError("$defs MUST be an object (2019-09/Core:8.2.5.)");
      const auto aDefsObj = theJson.toObject();
      for (const auto &aDefKVPair : aDefsObj) {
        const auto aSubInfo = Reader::read(aDefKVPair.second);
        if (Reader::ErrorHandling::isError(aSubInfo))
          return aSubInfo;
        aResult.NUM_CHARS += aDefKVPair.first.size();
        aResult.NUM_DEFS_ENTRIES += 1;
        aResult += Reader::ErrorHandling::unwrap(aSubInfo);
      }
      return aResult;
    }
  };
  struct SchemaReader {
    template <typename Reader, typename JSON>
    static constexpr typename Reader::ErrorOrConsumed
    readSchema(Reader &theReader, typename Reader::SchemaObject &theSchema,
               const std::string_view &theKey, const JSON &theValue) {
      auto &aCore = theSchema.getCore();
      if (theKey == "$id") {
        aCore.itsId = theReader.allocateString(theValue.toString());
      } else if (theKey == "$schema") {
        aCore.itsSchema = theReader.allocateString(theValue.toString());
      } else if (theKey == "$anchor") {
        aCore.itsAnchor = theReader.allocateString(theValue.toString());
      } else if (theKey == "$ref") {
        aCore.itsRef = theReader.allocateString(theValue.toString());
      } else if (theKey == "$recursiveRef") {
        aCore.itsRecursiveRef = theReader.allocateString(theValue.toString());
      } else if (theKey == "$recursiveAnchor") {
        aCore.itsRecursiveAnchor = theValue.toBool();
      } else if (theKey == "$vocabulary") {
        const size_t aVocabSize = theValue.toObject().size();
        auto aVocabObj =
            theReader
                .template allocateMap<typename Reader::Storage::String, bool>(
                    aVocabSize);
        ptrdiff_t aIdx{0};
        for (const auto &aKVPair : theValue.toObject()) {
          const auto aStr = theReader.allocateString(aKVPair.first);
          const bool aBool = aKVPair.second.toBool();
          theReader.setMapEntry(aVocabObj, aIdx++, aStr, aBool);
        }
        aCore.itsVocabulary = std::move(aVocabObj);
      } else if (theKey == "$comment") {
        aCore.itsComment = theReader.allocateString(theValue.toString());
      } else if (theKey == "$defs") {
        const size_t aDefsSize = theValue.toObject().size();
        auto aDefsObj =
            theReader.template allocateMap<typename Reader::Storage::String,
                                           typename Reader::Storage::Schema>(
                aDefsSize);
        ptrdiff_t aIdx{0};
        for (const auto &aKVPair : theValue.toObject()) {
          const auto aStr = theReader.allocateString(aKVPair.first);
          const auto aSchema = theReader.readSchema(theValue);
          if (Reader::ErrorHandling::isError(aSchema))
            return Reader::ErrorHandling::template convertError<bool>(aSchema);
          theReader.setMapEntry(aDefsObj, aIdx++, aStr,
                                Reader::ErrorHandling::unwrap(aSchema));
        }
        aCore.itsDefs = std::move(aDefsObj);
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_READER_CORE_H
