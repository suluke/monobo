#ifndef JSON_SCHEMA_SCHEMA_INFO_H
#define JSON_SCHEMA_SCHEMA_INFO_H

#include "constexpr_json/document_info.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include <cstddef>

namespace json_schema {
class SchemaInfo {
public:
  size_t NUM_SCHEMAS{0};
  size_t NUM_CHARS{0};
  size_t NUM_VOCAB_ENTRIES{0};
  size_t NUM_SCHEMA_BUFFER_ITEMS{0};
  size_t NUM_SCHEMA_DICT_ENTRIES{0};
  cjson::DocumentInfo JSON_INFO{};

  constexpr SchemaInfo() noexcept = default;
  constexpr SchemaInfo(const SchemaInfo &) noexcept = default;
  constexpr SchemaInfo(SchemaInfo &&) noexcept = default;
  constexpr SchemaInfo &operator=(const SchemaInfo &) noexcept = default;
  constexpr SchemaInfo &operator=(SchemaInfo &&) noexcept = default;

  constexpr SchemaInfo &operator+=(const SchemaInfo &theOther) {
    NUM_SCHEMAS += theOther.NUM_SCHEMAS;
    NUM_CHARS += theOther.NUM_CHARS;
    NUM_VOCAB_ENTRIES += theOther.NUM_VOCAB_ENTRIES;
    NUM_SCHEMA_BUFFER_ITEMS += theOther.NUM_SCHEMA_BUFFER_ITEMS;
    NUM_SCHEMA_DICT_ENTRIES += theOther.NUM_SCHEMA_DICT_ENTRIES;
    JSON_INFO += theOther.JSON_INFO;
    return *this;
  }

  constexpr SchemaInfo operator+(const SchemaInfo &theOther) const noexcept {
    SchemaInfo aCopy{*this};
    aCopy += theOther;
    return aCopy;
  }
};
} // namespace json_schema
#endif // JSON_SCHEMA_SCHEMA_INFO_H
