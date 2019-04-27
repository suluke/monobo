#ifndef CLI_ARGS_DEFAULTS_PATH_H
#define CLI_ARGS_DEFAULTS_PATH_H

#include <filesystem>

namespace cli_args {
namespace detail {
template <>
inline std::optional<std::filesystem::path>
CliLibCfgStd::parse(const std::string_view &value) {
  return std::filesystem::path(value);
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_PATH_H
