#ifndef CLI_ARGS_DEFAULTS_UNSIGNED_H
#define CLI_ARGS_DEFAULTS_UNSIGNED_H

#include <charconv>

namespace cli_args {
namespace detail {
template <>
inline std::optional<unsigned>
CliLibCfgStd::parse(const std::string_view &value) {
  unsigned u;
  auto result = std::from_chars(value.data(), value.data() + value.size(), u);
  if (result.ptr != value.data() + value.size())
    return std::nullopt;
  return u;
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_UNSIGNED_H
