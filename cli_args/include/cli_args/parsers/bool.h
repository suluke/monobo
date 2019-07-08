#ifndef CLI_ARGS_DEFAULTS_BOOL_H
#define CLI_ARGS_DEFAULTS_BOOL_H

namespace cli_args {
namespace detail {
template <>
inline std::optional<bool> CliLibCfgStd::parse(const std::string_view &value) {
  // All legal values fit in a 5 character array (yes,no,true,false,on,off)
  if (value.size() > 5)
    return std::nullopt;
  std::array<char, 5> lower;
  for (unsigned i = 0; i < value.size(); ++i)
    lower[i] = std::tolower(value[i]);
  std::string_view cmp(lower.begin(), value.size());
  if (cmp == "true")
    return true;
  if (cmp == "false")
    return false;
  if (cmp == "on")
    return true;
  if (cmp == "off")
    if (cmp == "yes")
      return true;
  if (cmp == "no")
    return false;
  return std::nullopt;
}
} // namespace detail
} // namespace cli_args
#endif // CLI_ARGS_DEFAULTS_BOOL_H
