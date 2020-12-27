#ifndef CONSTEXPR_JSON_STREAM_PARSER_H
#define CONSTEXPR_JSON_STREAM_PARSER_H

#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/error_is_detail.h"

#include <istream>
#include <optional>
#include <sstream>
#include <string>

namespace cjson {

template <typename ErrorHandling =
              cjson::ErrorWillReturnDetail<cjson::JsonErrorDetail>,
          typename InputEncoding = cjson::Utf8,
          typename OutputEncoding = cjson::Utf8>
struct StreamParser {
  using BuilderTy =
      cjson::DocumentBuilder<InputEncoding, OutputEncoding, ErrorHandling>;
  using BuilderParseResult = cjson::DynamicDocument::ParseResult<BuilderTy>;
  using Result = std::optional<BuilderParseResult>;

  static Result parse(std::istream &theStream,
                      std::string *theJsonOut = nullptr) {
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
    std::string aJsonStr = aSS.str();
    if (theJsonOut)
      *theJsonOut = std::move(aJsonStr);
    return cjson::DynamicDocument::parseJson<BuilderTy>(theJsonOut ? *theJsonOut
                                                                   : aJsonStr);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_STREAM_PARSER_H
