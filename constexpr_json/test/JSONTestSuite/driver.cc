#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "constexpr_json/document_builder.h"
#include "constexpr_json/dynamic_document.h"

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_WRONG_ARGC = 11;
static constexpr int ERROR_OPEN_FAILED = 12;
static constexpr int ERROR_READ_FAILED = 13;

namespace fs = std::filesystem;

static std::ostream &printError(std::ostream &theOS,
                                const cjson::ErrorDetail &theError,
                                const std::string_view theJson) {
  theOS << "ERROR: " << theError.what();

  if (theError.itsPosition >= 0) {
    const auto [aLineNo, aColNo] =
        theError.computeLocation<cjson::Utf8>(theJson);
    theOS << " - in line " << (aLineNo + 1) << " near\n"
          << theJson.substr(theError.itsPosition, 10);
  }
  return theOS;
}

template <typename Builder = cjson::DocumentBuilder<cjson::Utf8, cjson::Utf8>>
static typename Builder::error_handling::ErrorOr<
    std::unique_ptr<cjson::DynamicDocument>>
parseJson(const std::string_view theJson) {
  using namespace cjson;
  using ErrorHandling = typename Builder::error_handling;
  using ResultTy = std::unique_ptr<DynamicDocument>;
  const auto aDocInfoOrError =
      DocumentInfo::compute<typename Builder::src_encoding,
                            typename Builder::dest_encoding,
                            typename Builder::error_handling>(theJson);

  if (ErrorHandling::isError(aDocInfoOrError))
    return ErrorHandling::template convertError<ResultTy>(aDocInfoOrError);
  const auto aDocInfoAndLen = ErrorHandling::unwrap(aDocInfoOrError);
  const DocumentInfo aDocInfo = aDocInfoAndLen.first;
  const ssize_t aDocSize = aDocInfoAndLen.second;
  assert(aDocInfo);
  if (static_cast<size_t>(aDocSize) != theJson.size())
    return ErrorHandling::template makeError<ResultTy>(
        ErrorCode::TRAILING_CONTENT, aDocSize);
  const auto aDocOrError =
      Builder::template parseDocument<DynamicDocument>(theJson, aDocInfo);
  if (ErrorHandling::isError(aDocOrError))
    return ErrorHandling::template convertError<ResultTy>(aDocOrError);
  auto aResult = std::make_unique<DynamicDocument>(aDocInfo);
  *aResult = std::move(ErrorHandling::unwrap(aDocOrError));
  return aResult;
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
  using ErrorHandling = cjson::ErrorWillReturnDetail;
  using BuilderTy =
      cjson::DocumentBuilder<cjson::Utf8, cjson::Utf8, ErrorHandling>;
  const auto aResult = parseJson<BuilderTy>(aJsonStr);
  if (ErrorHandling::isError(aResult)) {
    printError(std::cout, std::get<cjson::ErrorDetail>(aResult), aJsonStr)
        << "\n";
    return ERROR_INVALID_JSON;
  }
  return 0;
}
