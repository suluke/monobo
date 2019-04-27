#ifndef CLI_ARGS_DEFAULTS_STRING_H
#define CLI_ARGS_DEFAULTS_STRING_H

#include <string>

namespace cli_args {
namespace detail {
template <>
inline std::optional<std::string>
CliLibCfgStd::parse(const std::string_view &value) {
  return std::string(value);
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_STRING_H
