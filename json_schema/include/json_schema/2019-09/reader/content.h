#ifndef JSON_SCHEMA_2019_09_READER_CONTENT_H
#define JSON_SCHEMA_2019_09_READER_CONTENT_H

#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderContent {
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

      if (theKey == "contentMediaType") {
        if (aType != TypeEnum::STRING)
          return makeError("contentMediaType must be of type string "
                           "(2019-09/Validation:8.4.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "contentEncoding") {
        if (aType != TypeEnum::STRING)
          return makeError("contentEncoding must be of type string "
                           "(2019-09/Validation:8.3.)");
        aResult.NUM_CHARS += theValue.toString().size();
      } else if (theKey == "contentSchema") {
        const auto aSubInfo = Reader::read(theValue);
        if (ErrorHandling::isError(aSubInfo))
          return ErrorHandling::template convertError<InfoMaybe>(aSubInfo);
        aResult += ErrorHandling::unwrap(aSubInfo);
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
      auto &aContent = theSchema.template getSection<SchemaContent>();
      if (theKey == "contentMediaType") {
        aContent.itsContentMediaType = Reader::toPtr(theReader.allocateString(theValue.toString()));
      } else if (theKey == "contentEncoding") {
        aContent.itsContentEncoding = Reader::toPtr(theReader.allocateString(theValue.toString()));
      } else if (theKey == "contentSchema") {
        auto aSchema = theReader.readSchema(theValue);
        if (Reader::ErrorHandling::isError(aSchema))
          return Reader::ErrorHandling::template convertError<bool>(aSchema);
        aContent.itsContentSchema = Reader::toPtr(Reader::ErrorHandling::unwrap(aSchema));
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_READER_CONTENT_H
