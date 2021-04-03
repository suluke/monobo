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
  using ParserTy =
      cjson::DocumentParser<InputEncoding, OutputEncoding, ErrorHandling>;
  using ParserParseResult = cjson::DynamicDocument::ParseResult<ParserTy>;

  /// Hold the parsing result iff there is no error reading the input stream.
  using Result = std::optional<ParserParseResult>;

  /// @return nullopt in case there was an error reading from the stream.
  ///         Parsing result otherwise.
  static Result parse(std::istream &theStream,
                      std::string *theJsonOut = nullptr,
                      const InputEncoding theInEnc = {},
                      const OutputEncoding theOutEnc = {}) {
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
    return cjson::DynamicDocument::parseJson<ParserTy>(
        theJsonOut ? *theJsonOut : aJsonStr, theInEnc, theOutEnc);
  }
};
} // namespace cjson
#endif // CONSTEXPR_JSON_STREAM_PARSER_H
