#ifndef CLI_ARGS_DEFAULTS_STRING_VIEW_H
#define CLI_ARGS_DEFAULTS_STRING_VIEW_H

#include <string_view>

namespace cli_args {
namespace detail {
template <>
inline std::optional<std::string_view>
CliLibCfgStd::parse(const std::string_view &value) {
  return value;
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_STRING_VIEW_H
