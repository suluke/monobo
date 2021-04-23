#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "json_schema/2019-09/schema_standard.h"
#include "json_schema/2019-09/schema_validator.h"
#include "json_schema/dynamic_schema.h"

#include "constexpr_json/ext/error_is_detail.h"
#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/stream_parser.h"

#include "cli_args/cli_args.h"
#include "cli_args/parsers/path.h"

namespace cl = ::cli_args;
namespace fs = ::std::filesystem;

const char *const TOOLNAME = "json_schema_testsuite_driver";
const char *const TOOLDESC = "Verify JSON documents using JSON-Schema";

enum ERROR {
  OK = 0,
  ERROR_INVALID_JSON = 1,
  ERROR_MALFORMED_TEST = 2,
  ERROR_TEST_PARSING_FAILED = 3,
  ERROR_OPEN_FAILED = 11,
  ERROR_READ_FAILED = 12,
};

static cl::list<fs::path> gInputs(cl::meta("files"),
                                  cl::desc("File(s) to be validated"),
                                  cl::init({"-"}));

using Standard = json_schema::Standard_2019_09</*Lenient=*/false>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader = Standard::template SchemaReader<Context, cjson::ErrorWillThrow>;
using ReadResult = typename Reader::template ReadResult<1>;
using Schema = ReadResult::SchemaObject;
using Validator = json_schema::SchemaValidator<Context>;

class TestsuiteFixture : public testing::Test {};
class TestsuiteTest : public TestsuiteFixture {
public:
  explicit TestsuiteTest(
      const std::shared_ptr<cjson::DynamicDocument> &theJsonDoc,
      const std::shared_ptr<ReadResult> &theSchemaContext,
      const cjson::DynamicDocument::EntityRef theData,
      const bool theShouldBeValid)
      : itsJsonDoc{theJsonDoc}, itsSchemaContext{theSchemaContext},
        itsData{theData}, itsShouldBeValid{theShouldBeValid} {}
  void TestBody() override {
    Validator aValidator((*itsSchemaContext)[0]);
    const auto aErrorMaybe = aValidator.validate(itsData);
    if (itsShouldBeValid) {
      EXPECT_FALSE(aErrorMaybe);
    } else {
      EXPECT_TRUE(aErrorMaybe);
    }
  }

private:
  std::shared_ptr<cjson::DynamicDocument> itsJsonDoc;
  std::shared_ptr<ReadResult> itsSchemaContext;
  cjson::DynamicDocument::EntityRef itsData;
  bool itsShouldBeValid{true};
};

struct TestReport {
  bool hasError() const noexcept { return itsHasError; }
  void addError(ERROR theEC, const char *const theMsg) { itsHasError = true; }

private:
  bool itsHasError{false};
};

struct TestGroupReport {
  bool hasError() const noexcept { return itsHasError; }
  void addError(ERROR theEC, const char *const theMsg) { itsHasError = true; }
  void addReport(const TestReport &theReport) {
    itsSingleTestReports.emplace_back(theReport);
    itsHasError |= theReport.hasError();
  }

private:
  bool itsHasError{false};
  std::vector<TestReport> itsSingleTestReports;
};

static TestReport
processTest(const cjson::DynamicDocument::EntityRef &theTest,
            const char *const theGroupName,
            const std::shared_ptr<ReadResult> &theSchema,
            const std::shared_ptr<cjson::DynamicDocument> &theJson) {
  TestReport aResult;
  if (theTest.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST,
                     "Expected test to be of type object");
    return aResult;
  }
  const auto &aTestObject = theTest.toObject();
  std::string aDesc = "<description missing>";
  if (auto aDescription = aTestObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING)
      aDesc = aDescription->toString();
  }
  const auto &aValid = aTestObject["valid"];
  if (!aValid || aValid->getType() != cjson::Entity::BOOL) {
    aResult.addError(ERROR_MALFORMED_TEST,
                     "`valid` key in test either not existing or not bool");
    return aResult;
  }
  const bool aShouldBeValid = aValid->toBool();
  const auto &aData = aTestObject["data"];
  if (!aData) {
    aResult.addError(ERROR_MALFORMED_TEST, "`data` not found in test");
    return aResult;
  }

  testing::RegisterTest(
      theGroupName, aDesc.c_str(), nullptr, nullptr, __FILE__, __LINE__,
      // Important to use the fixture type as the return type here.
      [=]() -> TestsuiteFixture * {
        return new TestsuiteTest(theJson, theSchema, *aData, aShouldBeValid);
      });

  return aResult;
}

static TestGroupReport
processTestGroup(const cjson::DynamicDocument::EntityRef &theGroup,
                 const std::shared_ptr<cjson::DynamicDocument> &theJsonDoc) {
  TestGroupReport aResult;
  if (theGroup.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST, "Expected test to be object\n");
    return aResult;
  }
  const auto &aGroupObject = theGroup.toObject();
  std::string aGroupDesc = "<group description missing>";
  if (auto aDescription = aGroupObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING) {
      aGroupDesc = aDescription->toString();
    }
  }
  if (!aGroupObject["schema"]) {
    aResult.addError(ERROR_MALFORMED_TEST, "No schema in test\n");
    return aResult;
  }
  using ErrorHandling = Reader::ErrorHandling;
  const auto aSchemaOrError = Reader::read(*aGroupObject["schema"]);
  if (ErrorHandling::isError(aSchemaOrError)) {
    aResult.addError(ERROR_MALFORMED_TEST, "Failed to build test schema\n");
    return aResult;
  }
  const auto aSchemaReadRes =
      std::make_shared<ReadResult>(ErrorHandling::unwrap(aSchemaOrError));
  const auto &aTests = aGroupObject["tests"];
  if (aTests && aTests->getType() == cjson::Entity::ARRAY) {
    const auto &aTestsArray = aTests->toArray();
    for (const auto &aTestElm : aTestsArray)
      aResult.addReport(processTest(aTestElm, aGroupDesc.c_str(),
                                    aSchemaReadRes, theJsonDoc));
  }
  return aResult;
}

ERROR processFilePath(const fs::path &thePath,
                      std::vector<TestGroupReport> &theReports) {
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using Parser = cjson::StreamParser<ErrorHandling>;
  Parser::Result aJsonInput;
  if (thePath == "-") {
    aJsonInput = Parser::parse(std::cin);
  } else {
    std::ifstream aFileIn(thePath, std::ios::binary);
    if (!aFileIn)
      return ERROR_OPEN_FAILED;
    aJsonInput = Parser::parse(aFileIn);
  }
  if (!aJsonInput) {
    std::cerr << "Failed to read file " << thePath << "\n";
    return ERROR_READ_FAILED;
  }
  if (ErrorHandling::isError(*aJsonInput)) {
    std::cerr << "Failed to parse JSON " << thePath << "\n";
    return ERROR_INVALID_JSON;
  }
  // This converts from unique to shared_ptr to be shared between tests
  std::shared_ptr<cjson::DynamicDocument> aJsonDoc =
      std::move(ErrorHandling::unwrap(*aJsonInput));
  const auto aJsonRoot = aJsonDoc->getRoot();
  if (aJsonRoot.getType() != cjson::Entity::ARRAY) {
    std::cerr << "Expected root element of test json " << thePath
              << " to be a list\n";
    return ERROR_MALFORMED_TEST;
  }
  for (const auto &aGroup : aJsonRoot.toArray())
    theReports.emplace_back(processTestGroup(aGroup, aJsonDoc));
  return OK;
}

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }
  int aDummyArgc{1};
  testing::InitGoogleTest(&aDummyArgc, const_cast<char **>(argv));

  std::error_code aEc;
  std::vector<TestGroupReport> aReports;
  for (const fs::path &aPath : gInputs) {
    if (fs::is_directory(aPath)) {
      for (const auto &aDirEntry : fs::directory_iterator(aPath)) {
        if (!aDirEntry.is_directory() &&
            aDirEntry.path().extension() == ".json") {
          if (int aErrc = processFilePath(aDirEntry, aReports))
            return aErrc;
        }
      }
    } else {
      if (int aErrc = processFilePath(aPath, aReports))
        return aErrc;
    }
  }
  for (const auto &aReport : aReports) {
    if (aReport.hasError())
      return ERROR_TEST_PARSING_FAILED;
  }
  return RUN_ALL_TESTS();
}
