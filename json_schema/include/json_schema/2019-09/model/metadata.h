#ifndef JSON_SCHEMA_2019_09_MODEL_METADATA_H
#define JSON_SCHEMA_2019_09_MODEL_METADATA_H

namespace json_schema {
template <typename Storage> class SchemaMetadata {
public:
  constexpr SchemaMetadata() = default;
  constexpr SchemaMetadata(const SchemaMetadata &) = default;
  constexpr SchemaMetadata(SchemaMetadata &&) = default;
  constexpr SchemaMetadata& operator=(const SchemaMetadata &) = default;
  constexpr SchemaMetadata& operator=(SchemaMetadata &&) = default;
private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_METADATA_H
