#include "cli_args/cli_args.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/error_is_detail.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>

namespace cl = ::cli_args;

const char *const TOOLNAME = "json_validate";
const char *const TOOLDESC = "Validate JSON documents";

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_OPEN_FAILED = 11;
static constexpr int ERROR_READ_FAILED = 12;

static cl::opt<std::string> gInput(cl::name("f"), cl::name("file"),
                                   cl::desc("File to be validated"),
                                   cl::init("-"));

static std::optional<std::string> readStream(std::istream &theStream) {
  std::stringstream aSS;
  size_t aBufSize{1024u};
  std::string aBuf(aBufSize, '\0');
  while (!theStream.eof()) {
    theStream.read(aBuf.data(), aBufSize - 1);
    if (theStream.bad())
      return std::nullopt;
    const auto aReadLen = static_cast<size_t>(theStream.gcount());
    aBuf[aReadLen] = '\0';
    aSS << std::string_view{aBuf.data(), aReadLen};
  }
  return aSS.str();
}

static std::ostream &printError(std::ostream &theOS,
                                const cjson::JsonErrorDetail &theError,
                                const std::string_view theJson) {
  theOS << "ERROR: " << theError.what();

  if (theError.itsPosition >= 0) {
    const auto aLineNo = theError.computeLocation<cjson::Utf8>(theJson).first;
    theOS << " - in line " << (aLineNo + 1) << " near "
          << "\"" << theJson.substr(theError.itsPosition, 10) << "\"";
  }
  return theOS;
}

int main(int argc, const char **argv) {
  if (!cl::ParseArgs(argc, argv)) {
    cl::PrintHelp(TOOLNAME, TOOLDESC, std::cout);
    return 1;
  }

  std::optional<std::string> aJsonStr;
  if (gInput == "-") {
    aJsonStr = readStream(std::cin);
  } else {
    std::ifstream aFileIn(gInput, std::ios::binary);
    if (!aFileIn)
      return ERROR_OPEN_FAILED;
    aJsonStr = readStream(aFileIn);
  }
  if (!aJsonStr)
    return ERROR_READ_FAILED;
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using BuilderTy =
      cjson::DocumentBuilder<cjson::Utf8, cjson::Utf8, ErrorHandling>;
  const auto aResult = cjson::DynamicDocument::parseJson<BuilderTy>(*aJsonStr);
  if (ErrorHandling::isError(aResult)) {
    printError(std::cout, ErrorHandling::getError(aResult), *aJsonStr) << "\n";
    return ERROR_INVALID_JSON;
  } else {
    std::cout << "Document is valid\n";
  }
  return 0;
}
