#ifndef JSON_SCHEMA_2019_09_MODEL_VALIDATION_H
#define JSON_SCHEMA_2019_09_MODEL_VALIDATION_H

#include "json_schema/2019-09/model/types.h"

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
  using TypesList = typename Storage::template Buffer<Types>;
  using JsonList = typename Storage::template Buffer<typename Storage::Json>;
  using StringListDict =
      typename Storage::template Map<typename Storage::String, StringList>;

  template <typename Context> class Accessor {
  public:
    using StringListAccessor =
        typename Context::template BufferAccessor<typename Storage::String>;
    using TypesListAccessor = typename Context::template BufferAccessor<Types>;
    using StringListDictAccessor =
        typename Context::template MapAccessor<typename Storage::String,
                                               StringList>;
    using JsonListAccessor =
        typename Context::template BufferAccessor<typename Storage::Json>;

    constexpr Accessor(const Context &theContext,
                       const SchemaValidation &theValidation)
        : itsContext(&theContext), itsValidation(&theValidation) {}

    constexpr double getMultipleOf() const {
      return itsValidation->itsMultipleOf;
    }
    constexpr double getMaximum() const { return itsValidation->itsMaximum; }
    constexpr double getExclusiveMaximum() const {
      return itsValidation->itsExclusiveMaximum;
    }
    constexpr double getMinimum() const { return itsValidation->itsMinimum; }
    constexpr double getExclusiveMinimum() const {
      return itsValidation->itsExclusiveMinimum;
    }
    constexpr size_t getMaxLength() const {
      return itsValidation->itsMaxLength;
    }
    constexpr size_t getMinLength() const {
      return itsValidation->itsMinLength;
    }
    constexpr size_t getMaxItems() const { return itsValidation->itsMaxItems; }
    constexpr size_t getMinItems() const { return itsValidation->itsMinItems; }
    constexpr size_t getMaxContains() const {
      return itsValidation->itsMaxContains;
    }
    constexpr size_t getMinContains() const {
      return itsValidation->itsMinContains;
    }
    constexpr size_t getMaxProperties() const {
      return itsValidation->itsMaxProperties;
    }
    constexpr size_t getMinProperties() const {
      return itsValidation->itsMinProperties;
    }
    constexpr bool getUniqueItems() const {
      return itsValidation->itsUniqueItems;
    }
    constexpr std::string_view getPattern() const {
      return itsContext->getString(itsValidation->itsPattern);
    }
    constexpr StringListAccessor getRequired() const {
      return StringListAccessor{*itsContext, itsValidation->itsRequired};
    }
    constexpr StringListDictAccessor getDependentRequired() const {
      return StringListDictAccessor{*itsContext,
                                    itsValidation->itsDependentRequired};
    }
    constexpr TypesListAccessor getType() const {
      return TypesListAccessor{*itsContext, itsValidation->itsType};
    }
    constexpr std::optional<typename Context::JsonAccessor> getConst() const {
      if (itsValidation->itsConst.isValid())
        return itsContext->getJson(itsValidation->itsConst);
      return std::nullopt;
    }
    constexpr JsonListAccessor getEnum() const {
      return JsonListAccessor{*itsContext, itsValidation->itsEnum};
    }

  private:
    const Context *itsContext;
    const SchemaValidation *itsValidation;
  };

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
#endif // JSON_SCHEMA_2019_09_MODEL_VALIDATION_H
