#ifndef JSON_SCHEMA_MODEL_FORMAT_FORMAT_H
#define JSON_SCHEMA_MODEL_FORMAT_FORMAT_H

namespace json_schema {
template <typename Storage> class SchemaFormat {
public:
  constexpr SchemaFormat() = default;
  constexpr SchemaFormat(const SchemaFormat &) = default;
  constexpr SchemaFormat(SchemaFormat &&) = default;
  constexpr SchemaFormat& operator=(const SchemaFormat &) = default;
  constexpr SchemaFormat& operator=(SchemaFormat &&) = default;
private:
  typename Storage::String itsValue{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_FORMAT_FORMAT_H
