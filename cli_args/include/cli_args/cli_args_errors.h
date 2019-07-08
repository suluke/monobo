#ifndef CLI_ARGS_CLI_ARGS_ERRORS_H
#define CLI_ARGS_CLI_ARGS_ERRORS_H

#include "gsl/span"
#include <string_view>

namespace cli_args {
namespace error {
struct MaxPositionalExceededError {
  const gsl::span<std::string_view> exceeding;
};
struct UnknownOptionError {
  const std::string_view option;
};
struct ParseError {
  const std::string_view name;
  const gsl::span<std::string_view> values;
  const cli_args::detail::CliOptConcept &option;
};
struct ValidationError {
  const cli_args::detail::CliOptConcept &option;
};
} // namespace error
} // namespace cli_args

#endif // CLI_ARGS_CLI_ARGS_ERRORS_H
