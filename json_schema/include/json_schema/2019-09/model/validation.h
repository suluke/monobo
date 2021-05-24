#ifndef JSON_SCHEMA_2019_09_MODEL_VALIDATION_H
#define JSON_SCHEMA_2019_09_MODEL_VALIDATION_H

#include "json_schema/modelling.h"
#include <limits>

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

    constexpr auto getMultipleOf() const {
      return itsValidation->itsMultipleOf;
    }
    constexpr auto getMaximum() const { return itsValidation->itsMaximum; }
    constexpr auto getExclusiveMaximum() const {
      return itsValidation->itsExclusiveMaximum;
    }
    constexpr auto getMinimum() const { return itsValidation->itsMinimum; }
    constexpr auto getExclusiveMinimum() const {
      return itsValidation->itsExclusiveMinimum;
    }
    constexpr auto getMaxLength() const { return itsValidation->itsMaxLength; }
    constexpr auto getMinLength() const { return itsValidation->itsMinLength; }
    constexpr auto getMaxItems() const { return itsValidation->itsMaxItems; }
    constexpr auto getMinItems() const { return itsValidation->itsMinItems; }
    constexpr auto getMaxContains() const {
      return itsValidation->itsMaxContains;
    }
    constexpr auto getMinContains() const {
      return itsValidation->itsMinContains;
    }
    constexpr auto getMaxProperties() const {
      return itsValidation->itsMaxProperties;
    }
    constexpr auto getMinProperties() const {
      return itsValidation->itsMinProperties;
    }
    constexpr auto getUniqueItems() const {
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

private:
  template <typename T> struct negative {
    constexpr double operator()() const noexcept(noexcept(-T{}())) {
      return -T{}();
    }
  };

public:
  using with_default_nan = with_default<
      nullary_function<double, std::numeric_limits<double>::quiet_NaN>>;
  using with_default_inf = with_default<
      nullary_function<double, std::numeric_limits<double>::infinity>>;
  using with_default_negative_inf = with_default<negative<
      nullary_function<double, std::numeric_limits<double>::infinity>>>;
  using with_default_max =
      with_default<nullary_function<size_t, std::numeric_limits<size_t>::max>>;
  using with_default_zero = with_default<std::integral_constant<size_t, 0>>;

  with_default_nan itsMultipleOf{};

  with_default_inf itsMaximum{};
  with_default_inf itsExclusiveMaximum{};
  with_default_negative_inf itsMinimum{};
  with_default_negative_inf itsExclusiveMinimum{};

  with_default_max itsMaxLength{};
  with_default_zero itsMinLength{};
  with_default_max itsMaxItems{};
  with_default_zero itsMinItems{};
  with_default_max itsMaxContains{};
  with_default<std::integral_constant<size_t, 1>> itsMinContains{};
  with_default_max itsMaxProperties{};
  with_default_zero itsMinProperties{};
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
