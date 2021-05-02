#ifndef CLI_ARGS_CLI_ARGS_CONFIG_H
#define CLI_ARGS_CLI_ARGS_CONFIG_H

#include <iostream>
#include <optional>

#include "cli_args/cli_args_base.h"
#include "cli_args/cli_args_errors.h"

namespace cli_args {
namespace detail {

/// Bare-bones do-nothing implementation of the CliLibCfg concept
struct CliLibCfgBase {
  CliLibCfgBase() = default;
  CliLibCfgBase(int offset) : offset(offset) {}

  void report(const cli_args::error::MaxPositionalExceededError &err) const {}
  void report(const cli_args::error::UnknownOptionError &err) const {}
  void report(const cli_args::error::ParseError &err) const {}
  void report(const cli_args::error::ValidationError &err) const {}

  using string_span = detail::CliOptConcept::string_span;

  int handle_unrecognized(const std::string_view &, const string_span &) const {
    return -1;
  }

  int offset{1};
};

/// Basic implementation of the CliLibCfg concept sufficient for most use cases
struct CliLibCfgStd : public CliLibCfgBase {
  template <typename T>
  static std::optional<T> parse(const std::string_view &value);

  static void report(const cli_args::error::MaxPositionalExceededError &err) {
    errs() << "Too many positional arguments given:\n";
    for (const std::string_view &arg : err.exceeding)
      errs() << arg << '\n';
  }
  static void report(const cli_args::error::UnknownOptionError &err) {
    errs() << "Encountered unknown option " << err.option << '\n';
  }
  static void report(const cli_args::error::ParseError &err) {}
  static void report(const cli_args::error::ValidationError &err) {
    const auto &names = err.option->getNames();
    errs() << "Missing required value for option \""
           << (names.empty() ? err.option->getMeta() : names.at(0)) << "\"\n";
  }

  int handle_unrecognized(const std::string_view &opt,
                          const string_span &vals) const {
    return unrecognized_handler(opt, vals);
  }

  using callback_unrecognized =
      std::function<int(const std::string_view &, const string_span &)>;

  static callback_unrecognized &onunrecognized() {
    static callback_unrecognized callback =
        [](const std::string_view &, const string_span &) { return -1; };
    return callback;
  }

  // Can also be set on the config instance
  callback_unrecognized &unrecognized_handler{onunrecognized()};

  static std::ostream &errs() { return std::cerr; }
};
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_CLI_ARGS_CONFIG_H
