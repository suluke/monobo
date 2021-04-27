#include <fstream>
#include <iostream>
#include <regex>

#include <gtest/gtest.h>

#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/stream_parser.h"

#include "cli_args/cli_args.h"

#include "jsontestsuite_driver_defs.h"

namespace cl = ::cli_args;
namespace fs = ::std::filesystem;

const char *const TOOLNAME = "jsontestsuite_driver";
const char *const TOOLDESC = "Driver integrating the JSONSTestSuite project "
                             "into the constexpr_json test suite";

static cl::list<fs::path> gInputs(cl::meta("files"),
                                  cl::desc("File(s) to be validated"),
                                  cl::init({JSONTESTSUITE_BUILTIN_TESTDIRS}));

enum ERROR {
  OK = 0,
  INVALID_FILE_NAME = 10,
  INTERNAL_ERROR = 11,
};

enum class Expectation { ACCEPT, REJECT, WURST };

class TestsuiteFixture : public testing::Test {};
class TestsuiteTest : public TestsuiteFixture {
public:
  explicit TestsuiteTest(const fs::path &theTestFile,
                         const Expectation theExpectation)
      : itsTestFile{theTestFile}, itsExpectation{theExpectation} {}
  void TestBody() override {
    using ErrorHandling = cjson::ErrorWillReturnNone;
    using Parser = cjson::StreamParser<ErrorHandling>;
    Parser::Result aResult;
    std::ifstream aFileIn(itsTestFile, std::ios::binary);
    ASSERT_TRUE(aFileIn);
    aResult = Parser::parse(aFileIn);
    ASSERT_TRUE(aResult);
    if (itsExpectation == Expectation::ACCEPT) {
      EXPECT_FALSE(ErrorHandling::isError(*aResult));
    } else if (itsExpectation == Expectation::REJECT) {
      EXPECT_TRUE(ErrorHandling::isError(*aResult));
    }
  }

private:
  fs::path itsTestFile;
  Expectation itsExpectation;
};

static const std::regex gTestFileRegex("([iny])_(.+)\\.json");
static ERROR processFilePath(const fs::path &thePath) {
  std::cmatch aMatchResults;
  fs::path aFilenameOnly = thePath.filename();
  if (!std::regex_match(aFilenameOnly.c_str(), aMatchResults, gTestFileRegex)) {
    std::cerr << "Test file " << thePath.c_str() << " incorrectly named\n";
    return INVALID_FILE_NAME;
  }
  if (aMatchResults.size() != 3)
    return INTERNAL_ERROR;
  Expectation aExpected{Expectation::WURST};
  const char *aGroupDesc = "???";
  std::string aExpectStr{aMatchResults[1].str()};
  switch (aExpectStr[0]) {
  case 'i':
    aExpected = Expectation::WURST;
    aGroupDesc = "indifferent";
    break;
  case 'n':
    aGroupDesc = "reject";
    aExpected = Expectation::REJECT;
    break;
  case 'y':
    aGroupDesc = "accept";
    aExpected = Expectation::ACCEPT;
    break;
  default: {
    return INTERNAL_ERROR;
  }
  }
  testing::RegisterTest(
      aGroupDesc, aMatchResults[2].str().c_str(), nullptr, nullptr, __FILE__,
      __LINE__,
      // Important to use the fixture type as the return type here.
      [=]() -> TestsuiteFixture * {
        return new TestsuiteTest(thePath, aExpected);
      });
  return OK;
}

int main(int argc, const char **argv) {
  cl::cfg::onunrecognized() =
      [&](const std::string_view &theName,
          const cl::cfg::string_span &theValues) -> int {
    if (theName.substr(0, 6) == "gtest_")
      return static_cast<int>(theValues.size());
    return -1;
  };
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }

  for (const fs::path &aPath : gInputs) {
    if (fs::is_directory(aPath)) {
      for (const auto &aDirEntry : fs::directory_iterator(aPath)) {
        if (!aDirEntry.is_directory() &&
            aDirEntry.path().extension() == ".json") {
          if (ERROR aErrc = processFilePath(aDirEntry))
            return aErrc;
        }
      }
    } else {
      if (ERROR aErrc = processFilePath(aPath))
        return aErrc;
    }
  }

  testing::InitGoogleTest(&argc, const_cast<char **>(argv));
  return RUN_ALL_TESTS();
}
