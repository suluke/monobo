#ifndef JSON_SCHEMA_READER_FORMAT_H
#define JSON_SCHEMA_READER_FORMAT_H

#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderFormat {
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
      const TypeEnum aType = theValue.getType();

      if (theKey == "format") {
        if (aType != TypeEnum::STRING)
          return makeError("format property must be of type string "
                           "(2019-09/Validation:7.1.)");
        aResult.NUM_CHARS += theValue.toString().size();
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
      if (theKey == "format") {

      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_READER_FORMAT_H
