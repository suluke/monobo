#ifndef CLI_ARGS_DEFAULTS_UNSIGNED_H
#define CLI_ARGS_DEFAULTS_UNSIGNED_H

#if __has_include(<charconv>)
#include <charconv>
#define CLI_ARGS_HAS_CHARCONV true
#else
#define CLI_ARGS_HAS_CHARCONV false
#endif

namespace cli_args {
namespace detail {
template <>
inline std::optional<unsigned>
CliLibCfgStd::parse(const std::string_view &value) {
  unsigned u{};
#if CLI_ARGS_HAS_CHARCONV
  auto result = std::from_chars(value.data(), value.data() + value.size(), u);
  if (result.ptr != value.data() + value.size())
    return std::nullopt;
#else
  size_t result{};
  u = std::stoi(std::string{value.data(), value.size()}, &result);
  if (result != value.size())
    return std::nullopt;
#endif
  return u;
}
} // namespace detail
} // namespace cli_args

#undef CLI_ARGS_HAS_CHARCONV
#endif // CLI_ARGS_DEFAULTS_UNSIGNED_H
