#ifndef JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H
#define JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H

#include "json_schema/model/validation/types.h"

namespace json_schema {
template <typename Storage> class SchemaValidation {
public:
  constexpr SchemaValidation() = default;
  constexpr SchemaValidation(const SchemaValidation &) = default;
  constexpr SchemaValidation(SchemaValidation &&) = default;
  constexpr SchemaValidation &operator=(const SchemaValidation &) = default;
  constexpr SchemaValidation &operator=(SchemaValidation &&) = default;

  using StringList =
      typename Storage::template Buffer<typename Storage::String>;
  using TypesList =
      typename Storage::template Buffer<Types>;
  using JsonList = typename Storage::template Buffer<typename Storage::Json>;
  using StringListDict = typename Storage::template Map<typename Storage::String, StringList>;

  // private:
  double itsMultipleOf{};
  double itsMaximum{};
  double itsExclusiveMaximum{};
  double itsMinimum{};
  double itsExclusiveMinimum{};
  size_t itsMaxLength{};
  size_t itsMinLength{};
  size_t itsMaxItems{};
  size_t itsMinItems{};
  size_t itsMaxContains{};
  size_t itsMinContains{1};
  size_t itsMaxProperties{};
  size_t itsMinProperties{};
  bool itsUniqueItems{false};
  typename Storage::String itsPattern{};
  StringList itsRequired{};
  StringListDict itsDependentRequired{};
  TypesList itsType{};
  typename Storage::Json itsConst{};
  JsonList itsEnum{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_VALIDATION_VALIDATION_H
