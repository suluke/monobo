#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json_schema/2019-09/schema_standard.h"
#include "json_schema/dynamic_schema.h"

#include "constexpr_json/ext/error_is_detail.h"
#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/ext/error_is_nullopt.h"
#include "constexpr_json/ext/stream_parser.h"

#include "cli_args/cli_args.h"
#include "cli_args/parsers/path.h"

namespace cl = ::cli_args;
namespace fs = ::std::filesystem;
using namespace std::string_literals;

const char *const TOOLNAME = "json_schema_testsuite_driver";
const char *const TOOLDESC = "Verify JSON documents using JSON-Schema";

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_MALFORMED_TEST = 2;
static constexpr int ERROR_OPEN_FAILED = 11;
static constexpr int ERROR_READ_FAILED = 12;

static cl::opt<fs::path> gSchema(cl::name("s"), cl::name("schema"),
                                 cl::desc("Schema to be validated against"),
                                 cl::required());
static cl::list<fs::path> gInputs(cl::meta("files"),
                                  cl::desc("File(s) to be validated"),
                                  cl::init({"-"s}));

using Standard = json_schema::Standard_2019_09</*Lenient=*/false>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader =
    Standard::template SchemaReader<Context, cjson::ErrorWillThrow>;

static int processTest(const cjson::DynamicDocument::EntityRef &aTest) {
  if (aTest.getType() != cjson::Entity::OBJECT) {
    std::cerr << "Expected test to be object\n";
    return ERROR_MALFORMED_TEST;
  }
  const auto &aTestObject = aTest.toObject();
  if (auto aDescription = aTestObject["description"]) {
    if (aDescription->getType() != cjson::Entity::STRING)
      return ERROR_MALFORMED_TEST;
    std::cout << aDescription->toString() << "\n";
  }
  if (!aTestObject["schema"]) {
    std::cerr << "No schema in test\n";
    return ERROR_MALFORMED_TEST;
  }
  using ErrorHandling = Reader::ErrorHandling;
  const auto aSchemaOrError = Reader::read(*aTestObject["schema"]);
  if (ErrorHandling::isError(aSchemaOrError)) {
    std::cerr << "Failed to build test schema\n";
    return ERROR_MALFORMED_TEST;
  }
  return 0;
}

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }

  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using Parser = cjson::StreamParser<ErrorHandling>;
  Parser::Result aResult;

  std::error_code aEc;
  for (auto &aPath : gInputs) {
    if (aPath == "-") {
      aResult = Parser::parse(std::cin);
    } else {
      std::ifstream aFileIn(aPath, std::ios::binary);
      if (!aFileIn)
        return ERROR_OPEN_FAILED;
      aResult = Parser::parse(aFileIn);
    }
    if (!aResult) {
      std::cerr << "Failed to read file " << aPath << "\n";
      return ERROR_READ_FAILED;
    }
    if (ErrorHandling::isError(*aResult)) {
      std::cerr << "Failed to parse JSON " << aPath << "\n";
      return ERROR_INVALID_JSON;
    }
    const auto aJsonDoc = std::move(ErrorHandling::unwrap(*aResult));
    const auto aJsonRoot = aJsonDoc->getRoot();
    if (aJsonRoot.getType() != cjson::Entity::ARRAY) {
      std::cerr << "Expected root element of test json " << aPath
                << " to be a list\n";
      return ERROR_MALFORMED_TEST;
    }
    for (const auto &aTest : aJsonRoot.toArray()) {
      if (auto aErrc = processTest(aTest))
        return aErrc;
    }
  }
  return 0;
}
