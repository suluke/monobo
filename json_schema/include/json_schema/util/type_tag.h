#ifndef JSON_SCHEMA_UTIL_TYPE_TAG_H
#define JSON_SCHEMA_UTIL_TYPE_TAG_H

namespace json_schema {
template <typename... Ts> struct type_tag {
  explicit type_tag() noexcept = default;
};
} // namespace json_schema
#endif // JSON_SCHEMA_UTIL_TYPE_TAG_H
