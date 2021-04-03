#ifndef JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H
#define JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H

#include "json_schema/schema_object.h"
#include <optional>

namespace json_schema {
template <typename Storage> class SchemaApplicator {
public:
  template <typename T> using Ptr = typename Storage::template Ptr<T>;
  using SchemaRef = typename Storage::SchemaRef;
  using SchemaPtr = typename Storage::SchemaPtr;
  using SchemaList = typename Storage::template BufferPtr<SchemaRef>;
  using SchemaDict =
      typename Storage::template MapPtr<typename Storage::StringRef, SchemaRef>;

  constexpr SchemaApplicator() = default;
  constexpr SchemaApplicator(const SchemaApplicator &) = default;
  constexpr SchemaApplicator(SchemaApplicator &&) = default;
  constexpr SchemaApplicator &operator=(const SchemaApplicator &) = default;
  constexpr SchemaApplicator &operator=(SchemaApplicator &&) = default;

  template <typename Context> class Accessor {
  public:
    using SchemaAccessor = SchemaObjectAccessor<Context>;
    using MaybeSchemaAccessor = std::optional<SchemaAccessor>;
    using SchemaListAccessor =
        typename Context::template BufferAccessor<SchemaRef>;
    using MaybeSchemaListAccessor = std::optional<SchemaListAccessor>;
    using SchemaDictAccessor =
        typename Context::template MapAccessor<typename Storage::StringRef,
                                               SchemaRef>;
    using MaybeSchemaDictAccessor = std::optional<SchemaDictAccessor>;

    constexpr Accessor(const Context &theContext,
                       const SchemaApplicator &theApplicator)
        : itsContext(&theContext), itsApplicator(&theApplicator) {}

    constexpr MaybeSchemaAccessor getAdditionalItems() const {
      if (const auto aAdditionalItems = itsApplicator->itsAdditionalItems)
        return SchemaAccessor{*itsContext, *aAdditionalItems};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getUnevaluatedItems() const {
      if (const auto aUnevaluatedItems = itsApplicator->itsUnevaluatedItems)
        return SchemaAccessor{*itsContext, *aUnevaluatedItems};
      return std::nullopt;
    }
    constexpr MaybeSchemaListAccessor getItems() const {
      if (itsApplicator->itsItems)
        return SchemaListAccessor{*itsContext, *itsApplicator->itsItems};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getContains() const {
      if (const auto aContains = itsApplicator->itsContains)
        return SchemaAccessor{*itsContext, *aContains};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getAdditionalProperties() const {
      if (const auto aAdditionalProperties =
              itsApplicator->itsAdditionalProperties)
        return SchemaAccessor{*itsContext, *aAdditionalProperties};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getUnevaluatedProperties() const {
      if (const auto aUnevaluatedProperties =
              itsApplicator->itsUnevaluatedProperties)
        return SchemaAccessor{*itsContext, *aUnevaluatedProperties};
      return std::nullopt;
    }
    constexpr MaybeSchemaDictAccessor getProperties() const {
      if (const auto &aProperties = itsApplicator->itsProperties)
        return SchemaDictAccessor{*itsContext, *aProperties};
      return std::nullopt;
    }
    constexpr MaybeSchemaDictAccessor getPatternProperties() const {
      if (const auto &aPatternProperties = itsApplicator->itsPatternProperties)
        return SchemaDictAccessor{*itsContext, *aPatternProperties};
      return std::nullopt;
    }
    constexpr MaybeSchemaDictAccessor getDependentSchemas() const {
      if (const auto &aDependentSchemas = itsApplicator->itsDependentSchemas)
        return SchemaDictAccessor{*itsContext, *aDependentSchemas};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getPropertyNames() const {
      if (const auto aPropertyNames = itsApplicator->itsPropertyNames)
        return SchemaAccessor{*itsContext, *aPropertyNames};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getIf() const {
      if (const auto aIf = itsApplicator->itsIf)
        return SchemaAccessor{*itsContext, *aIf};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getThen() const {
      if (const auto aThen = itsApplicator->itsThen)
        return SchemaAccessor{*itsContext, *aThen};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getElse() const {
      if (const auto aElse = itsApplicator->itsElse)
        return SchemaAccessor{*itsContext, *aElse};
      return std::nullopt;
    }
    constexpr MaybeSchemaListAccessor getAllOf() const {
      if (itsApplicator->itsAllOf)
        return SchemaListAccessor{*itsContext, *itsApplicator->itsAllOf};
      return std::nullopt;
    }
    constexpr MaybeSchemaListAccessor getAnyOf() const {
      if (itsApplicator->itsAnyOf)
        return SchemaListAccessor{*itsContext, *itsApplicator->itsAnyOf};
      return std::nullopt;
    }
    constexpr MaybeSchemaListAccessor getOneOf() const {
      if (itsApplicator->itsOneOf)
        return SchemaListAccessor{*itsContext, *itsApplicator->itsOneOf};
      return std::nullopt;
    }
    constexpr MaybeSchemaAccessor getNot() const {
      if (const auto aNot = itsApplicator->itsNot)
        return SchemaAccessor{*itsContext, *aNot};
      return std::nullopt;
    }

  private:
    const Context *itsContext;
    const SchemaApplicator *itsApplicator;
  };

  // private:
  SchemaPtr itsAdditionalItems{};
  SchemaPtr itsUnevaluatedItems{};
  SchemaList itsItems{};
  SchemaPtr itsContains{};
  SchemaPtr itsAdditionalProperties{};
  SchemaPtr itsUnevaluatedProperties{};
  SchemaDict itsProperties{};
  SchemaDict itsPatternProperties{};
  SchemaDict itsDependentSchemas{};
  SchemaPtr itsPropertyNames{};
  SchemaPtr itsIf{};
  SchemaPtr itsThen{};
  SchemaPtr itsElse{};
  SchemaList itsAllOf{};
  SchemaList itsAnyOf{};
  SchemaList itsOneOf{};
  SchemaPtr itsNot{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_APPLICATOR_H
