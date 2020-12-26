#ifndef CLI_ARGS_CLI_ARGS_BASE_H
#define CLI_ARGS_CLI_ARGS_BASE_H

#include "gsl/span"
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <variant>
#include <vector>

namespace cli_args {

namespace detail {

/// The name of a command line flag (i.e. CliOpt), i.e. the unique
/// identifier after '-' or '--'.
/// If a CliOpt is not given a name or the name is an empty string (or
/// even nullptr) the option is automatically expected to handle all
/// positional arguments. Use CliMetaName to still provide a meaningful
/// short description for what these values are being used.
struct CliName {
  // TODO compile-time check that "name" does not begin with '-'
  constexpr explicit CliName(std::string_view name) : name(name) {}

  const std::string_view &get() const { return name; }

private:
  std::string_view name;
};

/// A name for the value that is assigned to a CliOpt.
/// E.g. FILENAME in `-o FILENAME`.
struct CliMetaName {
  constexpr explicit CliMetaName(std::string_view name) : name(name) {}

  const std::string_view &get() const { return name; }

private:
  std::string_view name;
};

/// A textual description of what a CliOpt does. Will be printed as part
/// of the standard help output.
struct CliDesc {
  constexpr explicit CliDesc(std::string_view desc) : desc(desc) {}

  const std::string_view &get() const { return desc; }

private:
  std::string_view desc;
};

/// A value to assign to an CliOpt in case no value was provided on the
/// command line.
template <typename ValTy> struct CliInit {
  constexpr explicit CliInit(ValTy val) : val(val) {}

  const ValTy &get() const { return val; }

private:
  ValTy val;
};

/// Associates a CliOpt with an external variable that is used for storing
/// its value. Useful when making existing global variables configurable
/// over the command line.
template <typename ValTy> struct CliStorage {
  constexpr explicit CliStorage(ValTy &val) : val(val) {}

  ValTy &get() { return val; }

private:
  ValTy &val;
};

/// Indicates the "app" an option belongs to. The default AppTag is void.
template <typename AppTag> struct CliAppTag {};

/// Indicates that a value for an option is required.
struct CliRequired {};

/// Indicates that after an option with this attribute was encountered
/// the library is supposed to stop processing any following arguments
/// Use ParseArgs::getNumArgsRead() to find out where exactly this
/// library stopped parsing arguments.
struct CliTerminal {};
} // namespace detail

/// Convenience type aliases
using name = detail::CliName;
using meta = detail::CliMetaName;
using desc = detail::CliDesc;
using required = detail::CliRequired;
using terminal = detail::CliTerminal;
template <typename T> detail::CliInit<T> init(T &&val) {
  return detail::CliInit<T>(std::forward<T>(val));
}
template <typename T>
detail::CliInit<std::vector<T>> init(std::initializer_list<T> &&val) {
  return detail::CliInit<std::vector<T>>(
      std::forward<std::initializer_list<T>>(val));
}
template <typename T> detail::CliStorage<T> storage(T &storage) {
  return CliStorage(storage);
}
template <typename T> detail::CliAppTag<T> app() {
  return detail::CliAppTag<T>();
}

namespace detail {

struct CliOptConcept;

/// TODO Maybe we want a non-template registry of registries
template <typename AppTag> struct CliOptRegistry final {
  using optionmap_t = std::map<std::string_view, CliOptConcept *>;
  using optionset_t = std::set<CliOptConcept *>;

  static CliOptRegistry &get() {
    static CliOptRegistry instance;
    return instance;
  }
  ~CliOptRegistry();

  bool hasOption(const std::string_view &name) const {
    return optionsByName.count(name);
  }
  bool hasUnnamed() const { return hasOption(""); }
  CliOptConcept &getOption(const std::string_view &name) const {
    assert(hasOption(name) &&
           "Cannot retrieve unregistered option from registry");
    return *optionsByName.find(name)->second;
  }
  CliOptConcept &getUnnamed() const { return getOption(""); }
  void addOption(CliOptConcept &opt);
  void removeOption(CliOptConcept &opt) {
    std::vector<std::string_view> removedKeys;
    for (const auto &KeyValuePair : optionsByName)
      if (KeyValuePair.second == &opt)
        removedKeys.push_back(KeyValuePair.first);
    for (const std::string_view &key : removedKeys)
      optionsByName.erase(key);
    options.erase(&opt);
  }

  auto begin() const { return options.begin(); }
  auto end() const { return options.end(); }

private:
  optionmap_t optionsByName;
  optionset_t options;
  CliOptRegistry() = default;
};

/// Interface for all types of options.
/// Although this class does implement some common functionality we want to make
/// sure that it does not commit on any types or algorithms that a user might
/// want to be able to change. E.g. "isRequired" will always be of type bool
/// whereas the container for an option's names could be a std::vector or any
/// other container. Furthermore, it should be obvious that this class cannot be
/// a template because of it being an _interface_
struct CliOptConcept {
  virtual ~CliOptConcept() {
    // automatic unregistration
    unregister();
  }

  using string_span = gsl::span<const std::string_view>;

  virtual string_span getNames() const = 0;

  [[nodiscard]] virtual std::optional<string_span::size_type>
  parse(const string_span &values, bool isInline) = 0;

  /// The description for this option
  [[nodiscard]] const std::string_view &getDescription() const { return desc; }
  /// The meta name for this option's value(s)
  [[nodiscard]] const std::string_view &getMeta() const { return meta; }
  /// Returns whether this is a required option or not.
  [[nodiscard]] bool isRequired() const { return required; }
  /// Whether the parser should stop parsing after encountering this option
  [[nodiscard]] bool isTerminal() const { return terminal; };
  /// Whether a value was specified on the command line, or in other words if a
  /// value has been parsed for this option.
  [[nodiscard]] bool hasValueSpecified() const { return hasGivenValue; };

  [[nodiscard]] bool isUnnamed() const {
    if (getNames().empty())
      return true;
    for (const std::string_view &name : getNames())
      if (name.empty())
        return true;
    return false;
  }
  [[nodiscard]] std::string_view getShortestName() const {
    if (getNames().empty())
      return "";
    const std::string_view *shortest = nullptr;
    for (const std::string_view &name : getNames())
      if (!shortest || shortest->size() > name.size())
        shortest = &name;
    return *shortest;
  }

  void unregister() {
    if (registrator)
      registrator.unregister(*this);
  }

  CliOptConcept(const CliOptConcept &) = delete;
  CliOptConcept &operator=(const CliOptConcept &) = delete;
  CliOptConcept &operator=(CliOptConcept &&) = delete;

protected:
  CliOptConcept() = default;

private:
  struct Registrator {
    template <typename AppTag> void set() {
      reg = &registerFun<AppTag>;
      unreg = &unregisterFun<AppTag>;
    }
    void unset() {
      reg = nullptr;
      unreg = nullptr;
    }
    operator bool() const { return reg && unreg; }
    void registerWithApp(CliOptConcept &opt) { reg(opt); }
    void unregister(CliOptConcept &opt) { unreg(opt, *this); }

  private:
    using RegistrationCB = void (*)(CliOptConcept &);
    RegistrationCB reg = nullptr;
    using UnregistrationCB = void (*)(CliOptConcept &, Registrator &);
    UnregistrationCB unreg = nullptr;

    /// Static function to be used for registering a CliOpt with a
    /// CliOptRegistry with a given AppTag. Definition follows further below
    /// after ParseArg has been defined.
    template <typename AppTag> static void registerFun(CliOptConcept &opt) {
      CliOptRegistry<AppTag>::get().addOption(opt);
    }
    template <typename AppTag>
    static void unregisterFun(CliOptConcept &opt, Registrator &self) {
      // CAREFUL! This function might be called from opt's destructor AFTER all
      // derived destructors have been called. So don't use virtual functions on
      // opt!
      CliOptRegistry<AppTag>::get().removeOption(opt);
      // return to unregistered state to prevent further deregistrations
      // using this registrator function
      self.unset();
    }
  };
  /// Registration is performed dynamically after all configuration flags
  /// passed to the constructor have been evaluated. It is possible to
  /// register with a different AppTag, so the registration function
  /// needs to be changeable.
  Registrator registrator;

protected:
  template <typename AppTag> void setAppForRegistration() {
    assert(!registrator && "Must not register with more than one app tag");
    registrator.set<AppTag>();
  }
  void registerWithApp() {
    if (!registrator)
      registrator.set<void>();
    registrator.registerWithApp(*this);
  }
  void setRequired() { required = true; }
  void setTerminal() { terminal = true; }
  void setMeta(const std::string_view &theMeta) { meta = theMeta; }
  void setDesc(const std::string_view &theDesc) { desc = theDesc; }
  void setValueSpecified() { hasGivenValue = true; }

private:
  bool required = false;
  bool terminal = false;
  bool hasGivenValue = false;
  std::string_view meta = "";
  std::string_view desc = "";
};

template <typename AppTag> CliOptRegistry<AppTag>::~CliOptRegistry() {
  while (!options.empty())
    (*options.begin())->unregister();
}
template <typename AppTag>
void CliOptRegistry<AppTag>::addOption(CliOptConcept &opt) {
  for (const std::string_view &name : opt.getNames()) {
    assert(!optionsByName.count(name) &&
           "Option already registered with that name");
    optionsByName[name] = &opt;
  }
  if (opt.getNames().empty()) {
    assert(!hasUnnamed() && "An unnamed option has already been registered");
    optionsByName[""] = &opt;
  }
  options.emplace(&opt);
}

/// Common functionality for command line options
/// As opposed to CliOptConcept, this class is templated.
template <typename DerivedTy, typename DecayTy, typename LibCfg>
struct CliOptBase : public CliOptConcept {
  /// Automatic conversion to the option's underlying value.
  constexpr operator DecayTy &() { return **this; }
  constexpr operator DecayTy &() const { return **this; }

  /// Use the dereference (->) operator in cases where implicit conversion
  /// to ValContainer fails, i.e. in member lookups.
  constexpr DecayTy *operator->() {
    DecayTy *ptr = static_cast<DerivedTy *>(this)->getPtr();
    assert(ptr && "Casting option to underlying type without initialization");
    return ptr;
  }
  constexpr DecayTy *operator->() const {
    DecayTy *ptr = static_cast<const DerivedTy *>(this)->getPtr();
    assert(ptr && "Casting option to underlying type without initialization");
    return ptr;
  }

  /// Use the dereference (*) operator in cases where implicit conversion
  /// to ValContainer fails, i.e. in member lookups.
  constexpr DecayTy &operator*() { return *this->operator->(); }
  constexpr DecayTy &operator*() const { return *this->operator->(); }

  template <typename T> constexpr bool operator==(const T &theT) {
    return (**this) == theT;
  }
  template <typename T> constexpr bool operator!=(const T &theT) {
    return (**this) != theT;
  }
  template <typename T> constexpr bool operator<(const T &theT) {
    return (**this) < theT;
  }
  template <typename T> constexpr bool operator<=(const T &theT) {
    return (**this) <= theT;
  }
  template <typename T> constexpr bool operator>(const T &theT) {
    return (**this) > theT;
  }
  template <typename T> constexpr bool operator>=(const T &theT) {
    return (**this) >= theT;
  }

  CliOptConcept::string_span getNames() const override { return names; }

protected:
  std::vector<std::string_view> names;

  /// Overload to be called after all flags passed to the constructor
  /// have been `consume`d.
  constexpr void consume() {}

  template <typename... args_t>
  constexpr void consume(const CliName &name, args_t &&...args) {
    names.emplace_back(name.get());
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }

  template <typename... args_t>
  constexpr void consume(const CliMetaName &name, args_t &&...args) {
    setMeta(name.get());
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }

  template <typename... args_t>
  constexpr void consume(const CliDesc &desc, args_t &&...args) {
    setDesc(desc.get());
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }

  template <typename AppTag, typename... args_t>
  constexpr void consume(const CliAppTag<AppTag> &tag, args_t &&...args) {
    setAppForRegistration<AppTag>();
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }

  template <typename... args_t>
  constexpr void consume(const CliRequired &, args_t &&...args) {
    setRequired();
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }

  template <typename... args_t>
  constexpr void consume(const CliTerminal &, args_t &&...args) {
    setTerminal();
    static_cast<DerivedTy *>(this)->consume(std::forward<args_t>(args)...);
  }
};

/// Implementation of a single-value command line option.
template <typename ValTy, typename LibCfg>
struct CliOpt : public CliOptBase<CliOpt<ValTy, LibCfg>, ValTy, LibCfg> {
  using DecayTy = ValTy;
  using base_t = CliOptBase<CliOpt<ValTy, LibCfg>, DecayTy, LibCfg>;
  friend base_t;
  using string_span = CliOptConcept::string_span;

  template <typename... args_t> constexpr explicit CliOpt(args_t &&...args) {
    consume(std::forward<args_t>(args)...);
    if (!getPtr())
      value = std::make_unique<ValTy>();
    CliOptConcept::registerWithApp();
  }

  /// Import conversion operators from base_t
  using base_t::operator DecayTy &;
  using base_t::operator->;
  using base_t::operator*;

  /// Try to parse the given string_views to assign values to this option.
  /// Requires a matching specialization of the CliParseValue template
  /// function.
  std::optional<string_span::size_type> parse(const string_span &values,
                                              bool isInline) override {
    /// We specialize option parsing for booleans since they should only be
    /// set implicitly (just the flag) or using an inline value
    if constexpr (std::is_same_v<bool, ValTy>) {
      if (values.empty() || (!values.empty() && !isInline)) {
        assign(true);
        return 0;
      }
    } else {
      // gcc7 thinks the param is unused if the branch above is not entered
      std::ignore = isInline;
    }
    auto parsed = LibCfg::template parse<ValTy>(values.at(0));
    if (!parsed)
      return std::nullopt;
    assign(std::move(*parsed));
    return 1;
  }

private:
  /// The rationale behind using a unique_ptr to a heap-allocated ValTy
  /// in case no storage is specified is that we want CliOpts to take as
  /// little memory as possible in case storage *IS* specified. In that
  /// case, making OwnedVal = ValTy, the `value` member would be the
  /// same size as ValTy, i.e. taking twice as much storage as ValTy
  using OwnedVal = std::unique_ptr<ValTy>;
  std::variant<ValTy *, OwnedVal> value = nullptr;

  /// Returns the storage location of this option. During option
  /// initialization this function can return nullptr. However, after
  /// the constructor has been executed this function should never
  /// return nullptr.
  constexpr ValTy *getPtr() const {
    ValTy *val = nullptr;
    std::visit(
        [&val](auto &&value) {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, ValTy *>)
            val = value;
          else
            val = value.get();
        },
        value);
    return val;
  }

  constexpr void assign(ValTy &&val) {
    CliOptConcept::setValueSpecified();
    *getPtr() = std::move(val);
  }

  template <typename... args_t>
  constexpr void consume(const CliStorage<ValTy> &storage, args_t &&...args) {
    if (getPtr() && !std::holds_alternative<OwnedVal>(value))
      std::abort(); // Multiple storage specifiers given
    OwnedVal prev(nullptr);
    if (std::holds_alternative<OwnedVal>(value))
      prev = std::move(std::get<OwnedVal>(value));
    value = &storage.val;
    if (prev)
      *storage.val = *prev;
    consume(std::forward<args_t>(args)...);
  }

  template <typename T, typename... args_t>
  constexpr void consume(const CliInit<T> &init, args_t &&...args) {
    if (!getPtr())
      value = std::make_unique<ValTy>(static_cast<const ValTy &>(init.get()));
    else
      *getPtr() = init.get();
    consume(std::forward<args_t>(args)...);
  }
  /// Import `consume` implementations from base_t
  using base_t::consume;
};

/// Implementation of a multi-value command line option
template <typename ValTy, typename LibCfg>
struct CliList
    : public CliOptBase<CliList<ValTy, LibCfg>, std::vector<ValTy>, LibCfg> {
  using container_t = std::vector<ValTy>;
  using DecayTy = container_t;
  using base_t = CliOptBase<CliList<ValTy, LibCfg>, DecayTy, LibCfg>;
  friend base_t;
  using string_span = CliOptConcept::string_span;
  using index_type = typename container_t::size_type;

  template <typename... args_t> constexpr explicit CliList(args_t &&...args) {
    consume(std::forward<args_t>(args)...);
    if (!getPtr())
      list = std::make_unique<container_t>();
    CliOptConcept::registerWithApp();
  }

  /// Import conversion operators from base_t
  using base_t::operator DecayTy &;
  using base_t::operator->;
  using base_t::operator*;

  /// Try to parse the given string_views to assign values to this option.
  /// Requires a matching specialization of the CliParseValue template
  /// function.
  std::optional<string_span::size_type> parse(const string_span &values,
                                              bool isInline = false) {
    for (const auto &val : values) {
      auto parsed = LibCfg::template parse<ValTy>(val);
      if (!parsed) {
        clear();
        return std::nullopt;
      }
      insert(std::move(*parsed));
    }
    return values.size();
  }

  auto begin() const { return (*this)->begin(); }

  auto end() const { return (*this)->end(); }

  auto &operator[](index_type i) { return (**this)[i]; }

private:
  using OwnedContainer = std::unique_ptr<container_t>;
  std::variant<container_t *, OwnedContainer> list = nullptr;

  void insert(ValTy &&val) {
    // When getting a value for the first time, make sure the list is empty
    if (!CliOptConcept::hasValueSpecified())
      getPtr()->clear();
    CliOptConcept::setValueSpecified();
    getPtr()->emplace_back(std::move(val));
  }

  void clear() { getPtr()->clear(); }

  constexpr container_t *getPtr() const {
    container_t *res = nullptr;
    std::visit(
        [&res](auto &&value) {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, container_t *>)
            res = value;
          else
            res = value.get();
        },
        list);
    return res;
  }

  /// Import `consume` implementations from base_t
  using base_t::consume;

  template <typename T, typename... args_t>
  void consume(const CliInit<T> &init, args_t &&...args) {
    if (!getPtr())
      list = std::make_unique<container_t>();
    container_t &list = *this;
    if constexpr (std::is_assignable_v<T, ValTy>)
      list.emplace_back(init.get());
    else
      list.insert(list.end(), init.get().begin(), init.get().end());
    consume(std::forward<args_t>(args)...);
  }
};
} // namespace detail
} // namespace cli_args

#endif // CLI_ARGS_CLI_ARGS_BASE_H
