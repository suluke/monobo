#ifndef JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H
#define JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H

#include "json_schema/schema_object.h"

namespace json_schema {
template <typename Storage> class SchemaApplicator {
public:
  using SchemaRef = typename Storage::Schema;
  using SchemaList = typename Storage::template Buffer<SchemaRef>;
  using SchemaDict =
      typename Storage::template Map<typename Storage::String, SchemaRef>;

  constexpr SchemaApplicator() = default;
  constexpr SchemaApplicator(const SchemaApplicator &) = default;
  constexpr SchemaApplicator(SchemaApplicator &&) = default;
  constexpr SchemaApplicator &operator=(const SchemaApplicator &) = default;
  constexpr SchemaApplicator &operator=(SchemaApplicator &&) = default;

template <typename Context> class Accessor {
  public:
    using SchemaAccessor = std::variant<bool, SchemaObjectAccessor<Context>>;
    using SchemaListAccessor = typename Context::template BufferAccessor<SchemaRef>;

    constexpr Accessor(const Context &theContext, const SchemaApplicator &theApplicator)
        : itsContext(&theContext), itsApplicator(&theApplicator) {}

    constexpr SchemaAccessor getAdditionalItems() const {
      if (itsApplicator->itsAdditionalItems == itsContext->getTrueSchemaRef())
        return true;
      return SchemaObjectAccessor<Context>{*itsContext, itsApplicator->itsAdditionalItems};
    }
    constexpr SchemaAccessor getUnevaluatedItems() const {
      if (itsApplicator->itsUnevaluatedItems == itsContext->getTrueSchemaRef())
        return true;
      return SchemaObjectAccessor<Context>{*itsContext, itsApplicator->itsUnevaluatedItems};
    }
    constexpr SchemaListAccessor getItems() const {
      return SchemaListAccessor{*itsContext, itsApplicator->itsItems};
    }

  private:
    const Context *itsContext;
    const SchemaApplicator *itsApplicator;
  };

  // private:
  SchemaRef itsAdditionalItems{};
  SchemaRef itsUnevaluatedItems{};
  SchemaList itsItems{};
  SchemaRef itsContains{};
  SchemaRef itsAdditionalProperties{};
  SchemaRef itsUnevaluatedProperties{};
  SchemaDict itsProperties{};
  SchemaDict itsPatternProperties{};
  SchemaDict itsDependentSchemas{};
  SchemaRef itsPropertyNames{};
  SchemaRef itsIf{};
  SchemaRef itsThen{};
  SchemaRef itsElse{};
  SchemaList itsAllOf{};
  SchemaList itsAnyOf{};
  SchemaList itsOneOf{};
  SchemaRef itsNot{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H
