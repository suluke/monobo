#ifndef CLI_ARGS_CLI_ARGS_PARSER_H
#define CLI_ARGS_CLI_ARGS_PARSER_H

#include "cli_args_base.h"
#include "cli_args_errors.h"

namespace cli_args {
template <typename LibCfg> struct api {
  template <typename T> using opt = detail::CliOpt<T, LibCfg>;
  template <typename T> using list = detail::CliList<T, LibCfg>;

  /// Entry point to the cli_args library.
  // TODO allow specifying multiple AppTags (=> Categories)
  template <typename AppTag = void> struct ParseArgs {
    /// Parse the command line arguments specified by @p argc and @p argv.
    /// The parameter @p tool is the application's name and @p desc should
    /// contain a brief description of what the application does. Both
    /// these values are used by the help message.
    ParseArgs(int argc, const char **argv, int offset = 1) {
      using namespace ::cli_args::detail;
      using namespace ::cli_args::error;
      bool verbatim = false;
      std::vector<std::string_view> values;
      std::vector<std::string_view> positional;
      auto &registry = options();
      for (int argNum = offset; argNum < argc; ++argNum) {
        ++numArgsRead;
        std::string_view arg = argv[argNum];
        if (!verbatim && arg == "--") {
          // Treat all remaining arguments as verbatim
          verbatim = true;
        } else if (!verbatim && arg.front() == '-' && arg.size() > 1) {
          // We have an option. Do we know it?
          std::string_view name = parseOptName(arg);
          if (!registry.hasOption(name)) {
            report(UnknownOptionError{arg});
            return;
          }
          CliOptConcept &opt = registry.getOption(name);
          // Is the argument just the name or also an '=' assignment?
          if (auto prefixLen = &name.front() - &arg.front() + name.size();
              prefixLen < arg.size()) {
            std::string_view inlineVal =
                arg.substr(prefixLen + 1 /* equals sign */);
            values.push_back(inlineVal);
            auto res = opt.parse(values, true);
            if (!res) {
              report(ParseError{name, values, opt});
              return;
            }
            if (0 > *res || *res > values.size())
              std::abort(); // Illegal number of values read by option
            if (*res == 0)
              std::abort(); // Failing to parse inline argument MUST result in
            // std::nullopt
            // Don't forget to clear!!!
            values.clear();
            // If this is the final option, only break to give positional
            // arguments collected so far the chance to be consumed by
            // a corresponding option
            if (opt.isTerminal())
              break;
            // The option ends after the inline value
            continue;
          }
          // Collect all arguments that could be values to the current option
          while (argNum + 1 < argc) {
            arg = argv[argNum + 1];
            if (!verbatim) {
              if (arg == "--") {
                verbatim = true;
                ++argNum;
                continue;
              } else if (arg.front() == '-' && arg.size() > 1) {
                // '-'-prefixed values are still treated as flags
                break;
              }
            }
            ++argNum;
            values.push_back(arg);
          }
          auto res = opt.parse(values, false);
          if (!res) {
            report(ParseError{name, values, opt});
            return;
          }
          if (0 > *res || *res > values.size())
            std::abort(); // Illegal number of values read by option
          values.erase(values.begin(), values.begin() + *res);
          numArgsRead += *res;
          positional.insert(positional.end(), values.begin(), values.end());
          values.clear();
          if (opt.isTerminal())
            break;
        } else
          positional.push_back(arg);
      }
      // Parse positional arguments
      if (!positional.empty()) {
        if (!registry.hasUnnamed()) {
          report(MaxPositionalExceededError{positional});
          return;
        }
        CliOptConcept &eatAll = registry.getUnnamed();
        auto res = eatAll.parse(positional, false);
        if (!res) {
          report(ParseError{"", positional, eatAll});
          return;
        }
        if (0 > *res || *res > positional.size())
          std::abort(); // Illegal number of values read by option
        numArgsRead += *res;
        if (eatAll.isTerminal())
          return;
        if (*res != positional.size()) {
          const gsl::span<std::string_view> exceeding(positional.data() + *res,
                                                      positional.size() - *res);
          report(MaxPositionalExceededError{exceeding});
          return;
        }
      }
      // Check that all options are in a valid state
      bool allValid = true;
      for (const CliOptConcept *opt : registry)
        if (!validate(*opt)) {
          report(ValidationError{*opt});
          allValid = false;
        }
      if (!allValid) {
        return;
      }
      parseSuccess = true;
    }

    operator bool() const { return parseSuccess; }
    int getNumArgsRead() const { return numArgsRead; }

  private:
    bool parseSuccess = false;
    int numArgsRead = 0;

    /// After all command line arguments have been processed, this function
    /// is used for checking that this option is in a valid state.
    /// E.g., `require`d options need to have been specified by the user.
    /// FIXME this should be part of CliOptConcept or even LibCfg
    bool validate(const detail::CliOptConcept &opt) const {
      if (opt.isRequired()) {
        return opt.hasValueSpecified();
      }
      return true;
    }

    /// Extract the name part of an option from a string, i.e.
    /// everything between '-'/'--' and '='/the end of the string
    static std::string_view parseOptName(std::string_view opt) {
      if (opt.front() == '-')
        opt = opt.substr(1);
      if (opt.front() == '-')
        opt = opt.substr(1);
      if (auto eqPos = opt.find("="); eqPos != std::string_view::npos) {
        opt = opt.substr(0, eqPos);
      }
      return opt;
    }
    detail::CliOptRegistry<AppTag> &options() const {
      return detail::CliOptRegistry<AppTag>::get();
    }
    template <typename ErrorTy> void report(const ErrorTy &err) {
      LibCfg::report(err);
    }
  };
};
} // namespace cli_args
#endif // CLI_ARGS_CLI_ARGS_PARSER_H
