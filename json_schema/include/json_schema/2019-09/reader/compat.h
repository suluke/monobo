#ifndef JSON_SCHEMA_2019_09_READER_COMPAT_H
#define JSON_SCHEMA_2019_09_READER_COMPAT_H

#include "json_schema/schema_info.h"
#include <string_view>

namespace json_schema {
class ReaderCompat {
public:
  struct InfoReader {
    using InfoMaybe = std::optional<SchemaInfo>;

    template <typename Reader>
    static constexpr auto readItem(const std::string_view &theKey,
                                   const typename Reader::JSON &theValue) ->
        typename Reader::ErrorOrInfoMaybe {
      if (theKey == "definitions" || theKey == "dependencies")
        return SchemaInfo{};
      return InfoMaybe{};
    }
  };
  struct SchemaReader {
    template <typename Reader, typename JSON>
    static constexpr typename Reader::ErrorOrConsumed
    readSchema(Reader &theReader, typename Reader::SchemaObject &theSchema,
               const std::string_view &theKey, const JSON &theValue) {
      if (theKey == "definitions") {
      } else if (theKey == "dependencies") {
      } else {
        return false;
      }
      return true;
    }
  };

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_READER_COMPAT_H
