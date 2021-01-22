#ifndef JSON_SCHEMA_MODEL_SCHEMA_H
#define JSON_SCHEMA_MODEL_SCHEMA_H

#include "json_schema/model/applicator/applicator.h"
#include "json_schema/model/core/core.h"
#include "json_schema/model/format/format.h"
#include "json_schema/model/metadata/metadata.h"
#include "json_schema/model/validation/validation.h"

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

  template <template <typename> typename Section>
  constexpr Section<Storage> &getSection() noexcept {
    return static_cast<Section<Storage> &>(*this);
  }
  template <template <typename> typename Section>
  constexpr const Section<Storage> &getSection() const noexcept {
    return static_cast<const Section<Storage> &>(*this);
  }
};

template <typename Context> class SchemaObjectAccessor {
public:
  using SchemaRef = typename Context::SchemaRef;
  using Storage = typename Context::Storage;
  template <template <typename> typename Section>
  using Accessor = typename Section<Storage>::template Accessor<Context>;

  constexpr SchemaObjectAccessor(const Context &theContext,
                                 const SchemaRef theSchemaObject)
      : itsContext(&theContext), itsSchemaObject(theSchemaObject) {}

  template <template <typename> typename Section>
  constexpr const Accessor<Section> getSection() const noexcept {
    return Accessor<Section>{
        *itsContext,
        itsContext->getSchemaObject(itsSchemaObject).template getSection<Section>()};
  }

private:
  const Context *itsContext{};
  SchemaRef itsSchemaObject;
};

} // namespace json_schema
#endif // JSON_SCHEMA_MODEL_SCHEMA_H
