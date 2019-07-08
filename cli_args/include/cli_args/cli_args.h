#ifndef CLI_ARGS_CLI_ARGS_H
#define CLI_ARGS_CLI_ARGS_H
/// A C++17 command line argument library
///
/// Goals:
/// * Declarative command line definition
/// * Type safety
/// * Automatic documentation (help dialog) generation
/// * Header-only implementation
/// * Easy customization and extension
///
/// Non-Goals:
/// * Windows-style option parsing ('/X /Y' instead of '-X -Y')
/// * Short vs long option differentiation (especially collapsing of
///   multiple short options into one, e.g. '-abc' = '-a -b -c')
/// * Provide defaults for what can trivially be implemented by the user.
///   E.g. default options like --help or --version.
///
/// Features:
/// * Declarative configuration of the option parser using option variables.
/// * Support for scalar and aggregate options.
/// * Support for '-opt=val' syntax
/// * Support for global or local option definitions.
///   Local options will unregister automatically when going out of scope.
/// * Support for application namespaces ("AppTag") to avoid
///   option pollution from global options in different compilation units.
/// * Easy extendability for custom types by specializing CliParseValue.
///   Alternatively, implement your own option types, only relying on
///   the CliOptConcept interface and ParseOption public APIs
/// * Automatic help text generation (printHelp).
/// * Support for 'stacked' option parsing, i.e. stopping parsing at some
///   point to continue with a different parsing configuration.
///
/// Brief overview:
/// * cl::opt for single-valued options
/// * cl::list for multi-valued options
/// * cl::name to specify names for an option
///   - No name or "": Handle positional arguments
///   - Multiple option names are possible
/// * cl::meta for describing value roles (used in printHelp)
/// * cl::desc for option description (used in printHelp)
/// * cl::init for initial option values
/// * cl::required to force users to provide a value
/// * cl::storage to re-use existing variables for option storage
/// * cl::terminal to stop the parser after the corresponding option
///   has been parsed.
/// * cl::app to specify the application the option is intended for

#include "cli_args_parser.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

namespace cli_args {
namespace detail {
struct CliLibCfgStd {
  static std::ostream &outs() { return std::cout; }
  static std::ostream &errs() { return std::cerr; }

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
    const auto &names = err.option.getNames();
    errs() << "Missing required value for option \""
           << (names.empty() ? err.option.getMeta() : names.at(0)) << "\"\n";
  }
};
} // namespace detail
} // namespace cli_args

#include "parsers/path.h"
#include "parsers/string.h"
#include "parsers/unsigned.h"

namespace cli_args {
template <typename T> using opt = api<detail::CliLibCfgStd>::opt<T>;
template <typename T> using list = api<detail::CliLibCfgStd>::list<T>;
template <typename AppTag = void>
struct ParseArgs : public api<detail::CliLibCfgStd>::ParseArgs<AppTag> {
  using base_t = api<detail::CliLibCfgStd>::ParseArgs<AppTag>;
  ParseArgs(int argc, const char **argv, int offset = 1)
      : base_t(argc, argv, offset) {}
};

/// TODO Allow influencing display order
template <typename AppTag = void>
void PrintHelp(const char *tool, const char *desc, std::ostream &os) {
  using namespace ::cli_args::detail;
  const CliOptRegistry<AppTag> &registry = CliOptRegistry<AppTag>::get();
  const auto display = [](const detail::CliOptConcept &opt, std::ostream &os) {
    if (!opt.getNames().empty() && opt.getNames().at(0) != "") {
      const detail::CliOptConcept::string_span names = opt.getNames();
      const auto nameBegin = names.begin(), nameEnd = names.end();
      unsigned firstColWidth = 0,
               maxFirstColWidth = 18; // FIXME this is a pretty arbitrary value
      for (auto nameIt = nameBegin; nameIt != nameEnd; ++nameIt) {
        if (nameIt != nameBegin) {
          os << ", ";
          firstColWidth += 2;
        }
        if (nameIt->size() == 1)
          os << "-";
        else
          os << "--";
        os << *nameIt;
        firstColWidth += 1 + nameIt->size() + (nameIt->size() > 1);
      }
      std::ios_base::fmtflags defaultFmtFlags(os.flags());
      os << std::setw(maxFirstColWidth - firstColWidth) << "" << std::setw(0)
         << opt.getDescription();
      os.flags(defaultFmtFlags);
    } else
      os << opt.getMeta();
  };
  // FIXME this is still in MVP state
  os << "Usage: " << tool << " [OPTION]...";
  const CliOptConcept *eatAll = nullptr;
  if (registry.hasUnnamed()) {
    eatAll = &registry.getUnnamed();
    os << " ";
    if (!eatAll->isRequired())
      os << "[";
    display(*eatAll, os);
    if (!eatAll->isRequired())
      os << "]";
    os << "...";
  }
  os << "\n"
     << "Options:\n";
  std::vector<CliOptConcept *> optsSorted(registry.begin(), registry.end());
  std::sort(optsSorted.begin(), optsSorted.end(),
            [](CliOptConcept *A, CliOptConcept *B) {
              return A->getShortestName().compare(B->getShortestName()) < 0;
            });
  for (const CliOptConcept *opt : optsSorted) {
    if (opt == eatAll)
      continue;
    os << "  ";
    display(*opt, os);
    os << '\n';
  }
}
} // namespace cli_args

#endif // CLI_ARGS_CLI_ARGS_H
