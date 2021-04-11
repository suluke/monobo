#ifndef JSON_SCHEMA_UTIL_VARIANT_H
#define JSON_SCHEMA_UTIL_VARIANT_H

namespace json_schema {
namespace impl {
template <typename T> struct type_result { using type = T; };

/// Probably totally over-engineered, but if we only have 3 types in a variant
/// we don't want to waste more than a byte on storing a variant's current index
template <size_t Sz> constexpr auto select_index_type() {
  if constexpr (std::numeric_limits<unsigned char>::max() >= Sz + 1) {
    return type_result<unsigned char>{};
  } else if constexpr (std::numeric_limits<unsigned short>::max() >= Sz + 1) {
    return type_result<unsigned short>{};
  } else if constexpr (std::numeric_limits<unsigned int>::max() >= Sz + 1) {
    return type_result<unsigned int>{};
  } else if constexpr (std::numeric_limits<unsigned long>::max() >= Sz + 1) {
    return type_result<unsigned long>{};
  } else {
    return type_result<size_t>{};
  }
}
template <size_t Sz>
using index_type = typename decltype(impl::select_index_type<Sz>())::type;

/// variant_storage template
/// The typeless specialization is used to support the valueless case (e.g.
/// valueless_by_exception but also )
template <typename... Ts> struct variant_storage;
template <> struct variant_storage<> {
  constexpr variant_storage() = default;
  constexpr variant_storage(variant_storage, type_result<variant_storage>) {}
};

/// These cute little helpers help implementing the "if we get some X&&, use it
/// for alternative T that would be selected in overload resolution"
/// requirement.
template <typename T, typename I1, typename I2> struct InitSelect {
  constexpr type_result<void> operator()(...) { return {}; }
  constexpr type_result<I1> operator()(I1) { return {}; }
  constexpr type_result<I2> operator()(I2) { return {}; }
};
template <typename T, typename I1> struct InitSelect<T, I1, void> {
  constexpr type_result<void> operator()(...) { return {}; }
  constexpr type_result<I1> operator()(I1) { return {}; }
};
template <typename T, typename I2> struct InitSelect<T, void, I2> {
  constexpr type_result<void> operator()(...) { return {}; }
  constexpr type_result<I2> operator()(I2) { return {}; }
};

template <typename T, typename Best> constexpr auto select_initialized_impl() {
  return type_result<Best>{};
}
template <typename T, typename Best, typename H, typename... Ts>
constexpr auto select_initialized_impl() {
  using Recursion =
      typename decltype(select_initialized_impl<T, Best, Ts...>())::type;
  using Result = std::invoke_result_t<InitSelect<T, H, Recursion>, T>;
  return Result{};
}
template <typename T, typename... Ts> constexpr auto select_initialized_type() {
  if constexpr (std::is_same_v<T, variant_storage<>>) {
    return type_result<variant_storage<>>{};
  } else {
    return select_initialized_impl<T, void, Ts...>();
  }
}
template <typename T, typename... Ts>
using type_initialized_by =
    typename decltype(select_initialized_type<T, Ts...>())::type;

static_assert(std::is_same_v<type_initialized_by<bool, char, char *>, char>);

/// Now comes the meaty implementation of variant_storage
template <typename T, typename... Ts> struct variant_storage<T, Ts...> {
  constexpr variant_storage() = default;
  template <typename U, typename V>
  constexpr variant_storage(U &&theInit, type_result<V> theTag)
      : itsData{theInit, theTag} {}

  // constexpr variant_storage &operator=(const variant_storage&) = default;
  constexpr variant_storage &operator=(const variant_storage<> &theEmpty) {
    itsData.itsTail = theEmpty;
    return *this;
  }

  template <bool IsActive, typename U, typename V>
  constexpr void assign(U &&theInit, [[maybe_unused]] type_result<V> theTag) {
    if constexpr (std::is_same_v<T, V>) {
      if constexpr (IsActive) {
        itsData.itsHead = theInit;
      } else {
        // new (&itsData.itsHead) V{theInit};
        itsData = Data{std::forward<U>(theInit), theTag};
      }
    } else {
      if constexpr (IsActive) {
        if constexpr (std::is_same_v<decltype(itsData.itsTail),
                                     variant_storage<>>)
          itsData.itsTail = variant_storage<>{};
        else
          itsData.itsTail.template assign<IsActive>(std::forward<U>(theInit),
                                                    theTag);
      } else {
        // new (&itsData.itsTail)
        // variant_storage<Ts...>{std::forward<U>(theInit), theTag};
        itsData = Data{std::forward<U>(theInit), theTag};
      }
    }
  }

  template <bool IsActive>
  constexpr void assign_pos(const size_t thePos,
                            const variant_storage &theData) {
    if (thePos == 0) {
      assign<IsActive>(theData.itsData.itsHead, type_result<T>{});
    } else {
      if constexpr (!IsActive) {
        itsData = Data{variant_storage<>{}, type_result<variant_storage<>>{}};
      }
      if constexpr (!std::is_same_v<decltype(itsData.itsTail),
                                    variant_storage<>>) {
        itsData.itsTail.template assign_pos<IsActive>(thePos - 1,
                                                      theData.itsData.itsTail);
      }
    }
  }

  template <typename U> const U &get() const {
    if constexpr (std::is_same_v<T, U>) {
      return itsData.itsHead;
    } else {
      return itsData.itsTail.template get<U>();
    }
  }

  static_assert(std::is_trivially_destructible_v<T>,
                "Constexpr variant only supports trivially destructible types");
  union Data {
    constexpr Data() : itsHead{} {}
    template <typename U>
    constexpr Data(U &&theInit, type_result<T>)
        : itsHead{std::forward<U>(theInit)} {}
    template <typename U, typename V>
    constexpr Data(U &&theInit, type_result<V> theTag)
        : itsTail{std::forward<U>(theInit), theTag} {}
    constexpr Data &operator=(const Data &) = default;

    T itsHead;
    variant_storage<Ts...> itsTail;
  } itsData;
};

// FIXME is this too complicated? Also, this seems highly reusable
template <typename T, size_t Offset, typename... Ts> struct position_of_impl;

template <typename T, size_t Offset> struct position_of_impl<T, Offset> {
  static constexpr size_t value = static_cast<size_t>(-1);
};
template <typename T, size_t Offset, typename H, typename... Ts>
struct position_of_impl<T, Offset, H, Ts...> {
private:
  static constexpr auto compute() {
    if constexpr (std::is_same_v<T, H>)
      return std::integral_constant<size_t, Offset>{};
    else if constexpr (std::is_same_v<T, variant_storage<>>)
      return std::integral_constant<size_t, static_cast<size_t>(-1)>{};
    else
      return std::integral_constant<
          size_t, position_of_impl<T, Offset + 1, Ts...>::value>{};
  }

public:
  static constexpr size_t value = decltype(compute())::value;
};
template <typename T, typename... Ts>
inline constexpr size_t position_of = position_of_impl<T, 0, Ts...>::value;

static_assert(position_of<char, char, const char *> == 0);
static_assert(position_of<const char *, char, const char *> == 1);

template <size_t Pos, typename T, typename... Ts>
constexpr auto type_at_impl() {
  if constexpr (Pos == 0) {
    return type_result<T>{};
  } else {
    return type_at_impl<Pos - 1, Ts...>;
  }
}

template <size_t Pos, typename... Ts>
using type_at = typename decltype(type_at_impl<Pos, Ts...>())::type;

} // namespace impl

/// Shitty std::variant reimplementation
template <typename... Ts> class Variant {
private:
  static_assert(sizeof...(Ts) > 0, "Variant cannot be empty");
  using index_t = impl::index_type<sizeof...(Ts)>;
  template <typename T>
  static inline constexpr size_t tpos =
      impl::position_of<impl::type_initialized_by<T, Ts...>, Ts...>;

public:
  constexpr Variant() = default;
  // constexpr Variant(const Variant &theOther) = default;
  constexpr Variant(const Variant &theOther)
      : Variant{impl::variant_storage<>{}} {
    *this = theOther;
  }
  constexpr Variant &operator=(const Variant &theOther) {
    if (index() == theOther.index()) {
      itsStorage.template assign_pos<true>(theOther.itsActiveIdx,
                                           theOther.itsStorage);
    } else {
      itsStorage.template assign_pos<false>(theOther.itsActiveIdx,
                                            theOther.itsStorage);
      itsActiveIdx = theOther.itsActiveIdx;
    }
    return *this;
  }

  template <typename T, typename = std::enable_if_t<
                            !std::is_same_v<std::decay_t<T>, Variant>>>
  constexpr Variant(T &&theInit)
      : itsActiveIdx{tpos<T> == static_cast<size_t>(-1)
                         ? static_cast<index_t>(-1)
                         : static_cast<index_t>(tpos<T>)},
        itsStorage(std::forward<T>(theInit),
                   impl::select_initialized_type<T, Ts...>()) {}

  template <typename T> constexpr Variant &operator=(T &&theInit) {
    auto aTag = impl::select_initialized_type<T, Ts...>();
    using NewTy = typename decltype(aTag)::type;
    constexpr size_t aNewIdx = impl::position_of<NewTy, Ts...>;
    if (aNewIdx == index()) {
      itsStorage.template assign<true>(std::forward<T>(theInit), aTag);
    } else {
      itsStorage.template assign<false>(std::forward<T>(theInit), aTag);
    }
    itsActiveIdx = aNewIdx;
    return *this;
  }

  constexpr size_t index() const noexcept { return itsActiveIdx; }

  template <typename T> const T &get() const {
    if (index() != tpos<T>)
      throw "Illegal variant access";
    return itsStorage.template get<T>();
  }

private:
  index_t itsActiveIdx{0};
  impl::variant_storage<Ts...> itsStorage;
};
template <typename T, typename... Ts>
constexpr bool holds_alternative(const Variant<Ts...> &theVariant) {
  return theVariant.index() == impl::position_of<T, Ts...>;
}
template <typename T, typename... Ts>
inline const T &get(const Variant<Ts...> &theVariant) {
  return theVariant.template get<T>();
}

// static_assert(Variant<char, const char *>{'a'}.index() == 0u);
// static_assert(Variant<char, const char *>{"a"}.index() == 1u);

} // namespace json_schema
#endif // JSON_SCHEMA_UTIL_VARIANT_H
