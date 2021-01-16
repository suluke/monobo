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
  size_t NUM_DEFS_ENTRIES{0};
  size_t NUM_EXAMPLES_ENTRIES{0};
  size_t NUM_ENUM_ENTRIES{0};
  size_t NUM_TYPE_ENTRIES{0};
  size_t NUM_REQUIRED_ENTRIES{0};
  size_t NUM_DEP_REQUIRED_LISTS{0};
  size_t NUM_DEP_REQUIRED_LIST_ENTRIES{0};
  size_t NUM_PROPERTIES_ENTRIES{0};
  size_t NUM_PATTERN_PROPERTIES_ENTRIES{0};
  size_t NUM_DEPENTENT_SCHEMA_ENTRIES{0};
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
    NUM_DEFS_ENTRIES += theOther.NUM_DEFS_ENTRIES;
    NUM_EXAMPLES_ENTRIES += theOther.NUM_EXAMPLES_ENTRIES;
    NUM_ENUM_ENTRIES += theOther.NUM_ENUM_ENTRIES;
    NUM_TYPE_ENTRIES += theOther.NUM_TYPE_ENTRIES;
    NUM_REQUIRED_ENTRIES += theOther.NUM_REQUIRED_ENTRIES;
    NUM_DEP_REQUIRED_LISTS += theOther.NUM_DEP_REQUIRED_LISTS;
    NUM_DEP_REQUIRED_LIST_ENTRIES += theOther.NUM_DEP_REQUIRED_LIST_ENTRIES;
    NUM_PROPERTIES_ENTRIES += theOther.NUM_PROPERTIES_ENTRIES;
    NUM_PATTERN_PROPERTIES_ENTRIES += theOther.NUM_PATTERN_PROPERTIES_ENTRIES;
    NUM_DEPENTENT_SCHEMA_ENTRIES += theOther.NUM_DEPENTENT_SCHEMA_ENTRIES;
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
