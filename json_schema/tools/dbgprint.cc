#include <filesystem>
#include <fstream>
#include <iostream>

#include "json_schema/2019-09/schema_printer.h"
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

const char *const TOOLNAME = "json_schema_reprint";
const char *const TOOLDESC = "Interactive driver for json_schema debug printer";

static cl::opt<fs::path> gSchema(cl::meta("schema"),
                                 cl::desc("Schema to be printed"),
                                 cl::init("-"));

using ErrorHandling = cjson::ErrorWillReturnNone;
using Standard = json_schema::Standard_2019_09</*Lenient=*/false>;
using Context = json_schema::DynamicSchemaContext<Standard>;
using Reader = Standard::template SchemaReader<Context, ErrorHandling>;
using Schema = typename Reader::template ReadResult<1>::SchemaObject;

enum ERROR {
  OK = 0,
  ERROR_INVALID_JSON = 1,
  ERROR_MALFORMED_SCHEMA = 2,
  ERROR_TEST_FAILED = 3,
  ERROR_OPEN_FAILED = 11,
  ERROR_READ_FAILED = 12,
};

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }

  using Parser = cjson::StreamParser<ErrorHandling>;
  std::string aReadString;
  Parser::Result aJsonReadRes;

  if (gSchema == "-") {
    aJsonReadRes = Parser::parse(std::cin, &aReadString);
  } else {
    std::ifstream aFileIn(gSchema->c_str(), std::ios::binary);
    if (!aFileIn)
      return ERROR_OPEN_FAILED;
    aJsonReadRes = Parser::parse(aFileIn, &aReadString);
  }
  if (!aJsonReadRes)
    return ERROR_READ_FAILED;
  const auto &aJsonParseRes = *aJsonReadRes;

  if (ErrorHandling::isError(aJsonParseRes))
    return ERROR_INVALID_JSON;

  const auto &aJsonPtr = ErrorHandling::unwrap(aJsonParseRes);

  const auto aSchemaOrError = Reader::read(aJsonPtr->getRoot());
  if (ErrorHandling::isError(aSchemaOrError))
    return ERROR_MALFORMED_SCHEMA;
  const auto &aSchemaReadRes = ErrorHandling::unwrap(aSchemaOrError);

  json_schema::SchemaPrinter aPrinter(aSchemaReadRes[0]);
  std::cout << aPrinter << "\n";

  return 0;
}
