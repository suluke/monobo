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

  using StringListPtr =
      typename Storage::template BufferPtr<typename Storage::StringRef>;
  using StringListRef =
      typename Storage::template BufferRef<typename Storage::StringRef>;
  using TypesList = typename Storage::template BufferPtr<Types>;
  using JsonList =
      typename Storage::template BufferPtr<typename Storage::JsonRef>;
  using StringListDict =
      typename Storage::template MapPtr<typename Storage::StringRef,
                                        StringListRef>;

  template <typename Context> class Accessor {
  public:
    using StringListAccessor =
        typename Context::template BufferAccessor<typename Storage::StringRef>;
    using StringListAccessorMaybe = std::optional<StringListAccessor>;

    using TypesListAccessor = typename Context::template BufferAccessor<Types>;
    using TypesListAccessorMaybe = std::optional<TypesListAccessor>;

    using StringListDictAccessor =
        typename Context::template MapAccessor<typename Storage::StringRef,
                                               StringListRef>;
    using StringListDictAccessorMaybe = std::optional<StringListDictAccessor>;

    using JsonListAccessor =
        typename Context::template BufferAccessor<typename Storage::JsonRef>;
    using JsonListAccessorMaybe = std::optional<JsonListAccessor>;

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
    constexpr std::optional<std::string_view> getPattern() const {
      if (!itsValidation->itsPattern)
        return std::nullopt;
      return itsContext->getString(*itsValidation->itsPattern);
    }
    constexpr StringListAccessorMaybe getRequired() const {
      if (!itsValidation->itsRequired)
        return std::nullopt;
      return StringListAccessor{*itsContext, *itsValidation->itsRequired};
    }
    constexpr StringListDictAccessorMaybe getDependentRequired() const {
      if (!itsValidation->itsDependentRequired)
        return std::nullopt;
      return StringListDictAccessor{*itsContext,
                                    *itsValidation->itsDependentRequired};
    }
    constexpr TypesListAccessorMaybe getType() const {
      if (!itsValidation->itsType)
        return std::nullopt;
      return TypesListAccessor{*itsContext, *itsValidation->itsType};
    }
    constexpr std::optional<typename Context::JsonAccessor> getConst() const {
      if (!itsValidation->itsConst)
        return std::nullopt;
      return itsContext->getJson(*itsValidation->itsConst);
    }
    constexpr JsonListAccessorMaybe getEnum() const {
      if (!itsValidation->itsEnum)
        return std::nullopt;
      return JsonListAccessor{*itsContext, *itsValidation->itsEnum};
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
  typename Storage::StringPtr itsPattern{};
  StringListPtr itsRequired{};
  StringListDict itsDependentRequired{};
  TypesList itsType{};
  typename Storage::JsonPtr itsConst{};
  JsonList itsEnum{};
};
} // namespace json_schema
#endif // JSON_SCHEMA_2019_09_MODEL_VALIDATION_H
