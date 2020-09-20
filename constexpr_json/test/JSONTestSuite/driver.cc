#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/error_is_detail.h"

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_WRONG_ARGC = 11;
static constexpr int ERROR_OPEN_FAILED = 12;
static constexpr int ERROR_READ_FAILED = 13;

namespace fs = std::filesystem;

static std::ostream &printError(std::ostream &theOS,
                                const cjson::JsonErrorDetail &theError,
                                const std::string_view theJson) {
  theOS << "ERROR: " << theError.what();

  if (theError.itsPosition >= 0) {
    const auto [aLineNo, aColNo] =
        theError.computeLocation<cjson::Utf8>(theJson);
    theOS << " - in line " << (aLineNo + 1) << " near "
          << "\"" << theJson.substr(theError.itsPosition, 10) << "\"";
  }
  return theOS;
}

int main(int argc, const char **argv) {
  if (argc != 2) {
    std::cout << "Usage: jsontestsuite_driver <jsonpath>\n";
    return ERROR_WRONG_ARGC;
  }
  fs::path aJsonPath{argv[1]};
  std::ifstream aIStream{aJsonPath, std::ios::in | std::ios::ate};
  if (!aIStream)
    return ERROR_OPEN_FAILED;
  const auto aFileSize = aIStream.tellg();
  std::string aJsonStr(aFileSize, '\0');
  aIStream.seekg(0);
  if (!aIStream.read(aJsonStr.data(), aFileSize))
    return ERROR_READ_FAILED;
  using ErrorHandling = cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>;
  using BuilderTy =
      cjson::DocumentBuilder<cjson::Utf8, cjson::Utf8, ErrorHandling>;
  const auto aResult = cjson::DynamicDocument::parseJson<BuilderTy>(aJsonStr);
  if (ErrorHandling::isError(aResult)) {
    printError(std::cout, ErrorHandling::getError(aResult), aJsonStr)
        << "\n";
    return ERROR_INVALID_JSON;
  }
  return 0;
}
