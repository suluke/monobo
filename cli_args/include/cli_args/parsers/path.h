#ifndef CLI_ARGS_DEFAULTS_PATH_H
#define CLI_ARGS_DEFAULTS_PATH_H

#if __has_include(<filesystem>)
#include <filesystem>
#define CLI_ARGS_FS_NS std::filesystem
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
#define CLI_ARGS_FS_NS std::experimental::filesystem
#else
#error Failed to find C++ filesystem API support
#endif

namespace cli_args {
namespace detail {
namespace fs = CLI_ARGS_FS_NS;

template <>
inline std::optional<fs::path>
CliLibCfgStd::parse(const std::string_view &value) {
  return fs::path(value);
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_PATH_H
