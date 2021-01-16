#ifndef JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H
#define JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H

namespace json_schema {
template <typename Storage> class SchemaValidation {
public:
  constexpr SchemaValidation() = default;
  constexpr SchemaValidation(const SchemaValidation &) = default;
  constexpr SchemaValidation(SchemaValidation &&) = default;
  constexpr SchemaValidation &operator=(const SchemaValidation &) = default;
  constexpr SchemaValidation &operator=(SchemaValidation &&) = default;

private:
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H
