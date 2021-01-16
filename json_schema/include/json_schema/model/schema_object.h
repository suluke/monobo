#ifndef JSON_SCHEMA_MODEL_SCHEMA_H
#define JSON_SCHEMA_MODEL_SCHEMA_H

#include "json_schema/model/applicator/applicator.h"
#include "json_schema/model/core/core.h"
#include "json_schema/model/format/format.h"
#include "json_schema/model/metadata/metadata.h"
#include "json_schema/model/validation/validation.h"

#include "gsl/span"

namespace json_schema {

template <typename Storage>
class SchemaObject : private SchemaCore<Storage>,
                     private SchemaApplicator<Storage>,
                     private SchemaMetadata<Storage>,
                     private SchemaValidation<Storage>,
                     private SchemaFormat<Storage> {
public:
  using SchemaCore = json_schema::SchemaCore<Storage>;

  constexpr SchemaObject() = default;
  constexpr SchemaObject(const SchemaObject &) = default;
  constexpr SchemaObject(SchemaObject &&) = default;
  constexpr SchemaObject &operator=(const SchemaObject &) = default;
  constexpr SchemaObject &operator=(SchemaObject &&) = default;

  constexpr SchemaCore &getCore() noexcept {
    return static_cast<SchemaCore &>(*this);
  }
  constexpr const SchemaCore &getCore() const noexcept {
    return static_cast<const SchemaCore &>(*this);
  }
};
} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_SCHEMA_H
