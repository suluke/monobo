#include "cli_args/cli_args.h"
#include "constexpr_json/ext/ascii.h"
#include "constexpr_json/ext/error_is_detail.h"
#include "constexpr_json/ext/multi_encoding.h"
#include "constexpr_json/ext/printing.h"
#include "constexpr_json/ext/stream_parser.h"
#include "constexpr_json/ext/utf-8.h"

#include <fstream>

namespace cl = ::cli_args;

const char *const TOOLNAME = "json_format";
const char *const TOOLDESC = "Format JSON documents";

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_OPEN_FAILED = 11;
static constexpr int ERROR_READ_FAILED = 12;
static constexpr int ERROR_INVALID_OPTION = 13;

static cl::opt<std::string> gInput(cl::name("f"), cl::name("file"),
                                   cl::desc("File to be validated"),
                                   cl::init("-"));

static cl::opt<std::string>
    gEncoding(cl::name("e"), cl::name("encoding"),
              cl::desc("The encoding of the file to be validated"),
              cl::init("utf8"));

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }
  using Encoding = cjson::MultiEncoding<cjson::Ascii, cjson::Utf8>;
  Encoding aEnc{cjson::Utf8{}};
  if (gEncoding == "utf8") {
    // NOOP
  } else if (gEncoding == "ascii") {
    aEnc = Encoding{cjson::Ascii{}};
  } else {
    std::cerr << "Unknown encoding specified: " << *gEncoding << "\n";
    return ERROR_INVALID_OPTION;
  }
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using Parser = cjson::StreamParser<ErrorHandling>;
  std::string aReadString;
  Parser::Result aResult;

  if (gInput == "-") {
    aResult = Parser::parse(std::cin, &aReadString);
  } else {
    std::ifstream aFileIn(gInput, std::ios::binary);
    if (!aFileIn)
      return ERROR_OPEN_FAILED;
    aResult = Parser::parse(aFileIn, &aReadString);
  }
  if (!aResult)
    return ERROR_READ_FAILED;
  if (ErrorHandling::isError(*aResult))
    return ERROR_INVALID_JSON;
  cjson::Printer aPrinter;
  aPrinter.print(std::cout, ErrorHandling::unwrap(*aResult)->getRoot());
  return 0;
}
