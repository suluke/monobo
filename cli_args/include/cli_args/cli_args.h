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

#include "cli_args_config.h"
#include "cli_args_help.h"
#include "cli_args_parser.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

#include "parsers/bool.h"
#include "parsers/path.h"
#include "parsers/string.h"
#include "parsers/string_view.h"
#include "parsers/unsigned.h"

namespace cli_args {
using cfg = detail::CliLibCfgStd;
template <typename T> using opt = api<cfg>::opt<T>;
template <typename T> using list = api<cfg>::list<T>;
template <typename AppTag = void>
struct ParseArgs : public api<cfg>::ParseArgs<AppTag> {
  using base_t = api<cfg>::ParseArgs<AppTag>;
  ParseArgs(int argc, const char **argv, const cfg &config = {})
      : base_t(argc, argv, config) {}
};
} // namespace cli_args

#endif // CLI_ARGS_CLI_ARGS_H
