#include "cli_args/cli_args.h"

#include <gtest/gtest.h>

TEST(cli_args, regular_use_opt) {
  std::array<const char *, 3> aArgs{"exe", "--opt", "val"};
  cli_args::opt<std::string_view> aOpt{cli_args::name("opt")};
  EXPECT_TRUE(cli_args::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data()));
  EXPECT_EQ(*aOpt, "val");
}

TEST(cli_args, regular_use_bool_flag) {
  std::array<const char *, 2> aArgs{"exe", "--flag"};
  cli_args::opt<bool> aOpt{cli_args::name("flag"), cli_args::init(false)};
  EXPECT_FALSE(aOpt);
  EXPECT_TRUE(cli_args::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data()));
  EXPECT_TRUE(aOpt);
}

TEST(cli_args, unrecognized_is_error) {
  std::array<const char *, 2> aArgs{"exe", "--unrecognized"};
  EXPECT_FALSE(
      cli_args::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data()));
}

TEST(cli_args, unrecognized_callback) {
  {
    cli_args::cfg::onunrecognized() =
        [](const std::string_view &theOpt,
           const gsl::span<const std::string_view> &theVals) {
          return theVals.size();
        };
    std::array<const char *, 2> aArgs{"exe", "--unrecognized"};
    EXPECT_TRUE(
        cli_args::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data()));
  }
  {
    cli_args::cfg::onunrecognized() =
        [](const std::string_view &theOpt,
           const gsl::span<const std::string_view> &theVals) { return -1; };
    std::array<const char *, 2> aArgs{"exe", "--unrecognized"};
    EXPECT_FALSE(
        cli_args::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data()));
  }
}

namespace cl_test {
using namespace cli_args;
using namespace cli_args::error;
/// This config for testing does the following:
/// - all option types != std::string_view fail to parse
/// - all errors are recorded into a corresponding optionals
struct CliLibCfg_CollectReports : public cli_args::detail::CliLibCfgBase {

  void report(const MaxPositionalExceededError &err) const { psnlExceed = err; }
  void report(const UnknownOptionError &err) const { unknownOption = err; }
  void report(const ParseError &err) const { parseError = err; }
  void report(const ValidationError &err) const { validationError = err; }

  template <typename T>
  static std::optional<T> parse(const std::string_view &value) {
    if constexpr (std::is_same_v<T, std::string_view>)
      return value;
    else
      return std::nullopt;
  }

  mutable std::optional<MaxPositionalExceededError> psnlExceed;
  mutable std::optional<UnknownOptionError> unknownOption;
  mutable std::optional<ParseError> parseError;
  mutable std::optional<ValidationError> validationError;
};

using cfg = CliLibCfg_CollectReports;
template <typename T> using opt = api<cfg>::opt<T>;
template <typename T> using list = api<cfg>::list<T>;
template <typename AppTag = void>
struct ParseArgs : public api<cfg>::ParseArgs<AppTag> {
  using base_t = api<cfg>::ParseArgs<AppTag>;
  ParseArgs(int argc, const char **argv, const cfg &config = {})
      : base_t(argc, argv, config) {}
};
} // namespace cl_test

TEST(cli_args, report_unrecognized) {
  std::array<const char *, 2> aArgs{"exe", "--unrecognized"};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.unknownOption);
}

TEST(cli_args, report_positional_exceeded) {
  std::array<const char *, 2> aArgs{"exe", "positional"};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.psnlExceed);
}

TEST(cli_args, report_validation_err) {
  std::array<const char *, 1> aArgs{"exe"};
  cl_test::opt<std::string_view> aRequired{cl_test::name("required"),
                                           cl_test::required()};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.validationError);
}

// There are 3 situations causing a parse error:
// 1. --opt val => val cannot be parsed
// 2. --opt=val => val cannot be parsed
// 3. positional => positional cannot be parsed

TEST(cli_args, report_parse_err_regular) { // 1.
  std::array<const char *, 3> aArgs{"exe", "--opt", "val"};
  cl_test::opt<int> aInt{cl_test::name("opt")};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.parseError);
}
TEST(cli_args, report_parse_err_assigned) { // 2.
  std::array<const char *, 2> aArgs{"exe", "--opt=val"};
  cl_test::opt<int> aInt{cl_test::name("opt")};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.parseError);
}
TEST(cli_args, report_parse_err_positional) { // 3.
  std::array<const char *, 2> aArgs{"exe", "positional"};
  cl_test::opt<int> aPositionalInt{cl_test::meta("positional")};
  cl_test::cfg aCfg;
  cl_test::ParseArgs(static_cast<int>(aArgs.size()), aArgs.data(), aCfg);
  EXPECT_TRUE(aCfg.parseError);
}
