#include <filesystem>
#include <fstream>
#include <memory>

#include "constexpr_json/document_builder.h"
#include "constexpr_json/dynamic_document.h"

static constexpr int ERROR_INVALID_JSON = 1;
static constexpr int ERROR_WRONG_ARGC = 11;
static constexpr int ERROR_OPEN_FAILED = 12;
static constexpr int ERROR_READ_FAILED = 13;

namespace fs = std::filesystem;

template <typename Builder = cjson::DocumentBuilder<cjson::Utf8, cjson::Utf8>>
static std::unique_ptr<cjson::DynamicDocument>
parseJson(const std::string_view theJson) {
  using namespace cjson;
  DocumentInfo aDocInfo =
      DocumentInfo::compute<typename Builder::src_encoding, typename Builder::dest_encoding>(theJson).first;
  if (!aDocInfo)
    return nullptr;
  const auto aDoc = Builder::template parseDocument<DynamicDocument>(theJson, aDocInfo);
  if (aDoc) {
    auto aResult = std::make_unique<DynamicDocument>(aDocInfo);
    *aResult = std::move(*aDoc);
    return aResult;
  }
  return nullptr;
}

int main(int argc, const char **argv) {
  if (argc != 2)
    return ERROR_WRONG_ARGC;
  fs::path aJsonPath{argv[1]};
  std::ifstream aIStream{aJsonPath, std::ios::in | std::ios::ate};
  if (!aIStream)
    return ERROR_OPEN_FAILED;
  const auto aFileSize = aIStream.tellg();
  std::string aJsonStr(aFileSize, '\0');
  aIStream.seekg(0);
  if (!aIStream.read(aJsonStr.data(), aFileSize))
    return ERROR_READ_FAILED;
  const auto aResult = parseJson(aJsonStr);
  if (!aResult)
    return ERROR_INVALID_JSON;
  return 0;
}
