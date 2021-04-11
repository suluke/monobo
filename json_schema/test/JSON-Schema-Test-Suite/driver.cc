#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
  ERROR_TEST_FAILED = 3,
  ERROR_OPEN_FAILED = 11,
  ERROR_READ_FAILED = 12,
};

static cl::list<fs::path> gInputs(cl::meta("files"),
                                  cl::desc("File(s) to be validated"),
                                  cl::init({"-"}));

using Standard = json_schema::Standard_2019_09</*Lenient=*/false>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader = Standard::template SchemaReader<Context, cjson::ErrorWillThrow>;
using Schema = typename Reader::template ReadResult<1>::SchemaObject;
using Validator = json_schema::SchemaValidator<Context>;

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

static TestReport processTest(const cjson::DynamicDocument::EntityRef &theTest,
                              const Schema &theSchema) {
  TestReport aResult;
  if (theTest.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST,
                     "Expected test to be of type object");
    return aResult;
  }
  const auto &aTestObject = theTest.toObject();
  std::cout << "  EXEC TEST:";
  if (auto aDescription = aTestObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING)
      std::cout << "     \"" << aDescription->toString() << "\"";
  }
  std::cout << "\n";
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

  Validator aValidator(theSchema);
  const auto aErrorMaybe = aValidator.validate(*aData);
  if (aShouldBeValid && aErrorMaybe) {
    aResult.addError(ERROR_TEST_FAILED, "Unexpected validation failure");
    std::cerr << "  [FAIL] Unexpected validation failure\n";
  } else if (!aShouldBeValid && !aErrorMaybe) {
    aResult.addError(ERROR_TEST_FAILED, "Unexpected validation success");
    std::cerr << "  [FAIL] Unexpected validation success\n";
  } else {
    std::cout << "  [SUCCESS]\n";
  }

  return aResult;
}

static TestGroupReport
processTestGroup(const cjson::DynamicDocument::EntityRef &theGroup) {
  TestGroupReport aResult;
  if (theGroup.getType() != cjson::Entity::OBJECT) {
    aResult.addError(ERROR_MALFORMED_TEST, "Expected test to be object\n");
    return aResult;
  }
  const auto &aGroupObject = theGroup.toObject();
  if (auto aDescription = aGroupObject["description"]) {
    if (aDescription->getType() == cjson::Entity::STRING) {
      std::cout << "EXEC TEST GROUP: \"" << aDescription->toString() << "\"\n";
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
  const auto &aSchemaReadRes = ErrorHandling::unwrap(aSchemaOrError);
  const auto &aTests = aGroupObject["tests"];
  if (aTests && aTests->getType() == cjson::Entity::ARRAY) {
    const auto &aTestsArray = aTests->toArray();
    for (const auto &aTestElm : aTestsArray)
      aResult.addReport(processTest(aTestElm, aSchemaReadRes[0]));
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
  const auto aJsonDoc = std::move(ErrorHandling::unwrap(*aJsonInput));
  const auto aJsonRoot = aJsonDoc->getRoot();
  if (aJsonRoot.getType() != cjson::Entity::ARRAY) {
    std::cerr << "Expected root element of test json " << thePath
              << " to be a list\n";
    return ERROR_MALFORMED_TEST;
  }
  for (const auto &aGroup : aJsonRoot.toArray())
    theReports.emplace_back(processTestGroup(aGroup));
  return OK;
}

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }

  std::error_code aEc;
  std::vector<TestGroupReport> aReports;
  for (const fs::path &aPath : gInputs) {
    if (fs::is_directory(aPath)) {
      for (const auto &aDirEntry : fs::directory_iterator(aPath)) {
        if (!aDirEntry.is_directory() && aDirEntry.path().extension() == ".json") {
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
      return ERROR_TEST_FAILED;
  }
  return OK;
}
