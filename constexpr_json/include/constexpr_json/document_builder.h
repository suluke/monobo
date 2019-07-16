#ifndef CONSTEXPR_JSON_DOCUMENT_BUILDER_H
#define CONSTEXPR_JSON_DOCUMENT_BUILDER_H

#include "document.h"
#include "utils/parsing.h"
#include "utils/unicode.h"
#include <memory>

namespace cjson {
template <size_t N> using StrLiteralRef = const char (&)[N];

struct DocumentInfo {
  ssize_t itsNumNulls = 0;
  ssize_t itsNumBools = 0;
  ssize_t itsNumDoubles = 0;
  ssize_t itsNumChars = 0;
  ssize_t itsNumStrings = 0;
  ssize_t itsNumArrays = 0;
  ssize_t itsNumArrayEntries = 0;
  ssize_t itsNumObjects = 0;
  ssize_t itsNumObjectProperties = 0;

  constexpr static DocumentInfo error() {
    DocumentInfo aDI{-1, -1, -1, -1, -1, -1, -1, -1, -1};
    return aDI;
  }

  operator bool() const {
    return itsNumNulls >= 0 && itsNumBools >= 0 && itsNumDoubles >= 0 &&
           itsNumChars >= 0 && itsNumStrings >= 0 && itsNumArrays >= 0 &&
           itsNumArrayEntries >= 0 && itsNumObjects >= 0 &&
           itsNumObjectProperties >= 0;
  }
  constexpr DocumentInfo operator+(const DocumentInfo &theOther) {
    DocumentInfo aDI = *this;
    aDI += theOther;
    return aDI;
  }
  constexpr DocumentInfo &operator+=(const DocumentInfo &theOther) {
    this->itsNumArrayEntries += theOther.itsNumArrayEntries;
    this->itsNumArrays += theOther.itsNumArrays;
    this->itsNumBools += theOther.itsNumBools;
    this->itsNumChars += theOther.itsNumChars;
    this->itsNumDoubles += theOther.itsNumDoubles;
    this->itsNumNulls += theOther.itsNumNulls;
    this->itsNumObjectProperties += theOther.itsNumObjectProperties;
    this->itsNumObjects += theOther.itsNumObjects;
    this->itsNumStrings += theOther.itsNumStrings;
    return *this;
  }
};

template <typename EncodingTy>
constexpr DocumentInfo computeDocInfo(const std::string_view theJsonString) {
  constexpr const DocumentInfo aErrorResult = DocumentInfo::error();
  struct InfoElement {
    Entity::KIND itsType = Entity::NUL;
    DocumentInfo itsDocInfo;
    constexpr void setBool(bool theBool) {
      itsType = Entity::BOOLEAN;
      ++itsDocInfo.itsNumBools;
    }
    constexpr void setNumber(double theNum) {
      itsType = Entity::DOUBLE;
      ++itsDocInfo.itsNumDoubles;
    }
    constexpr void setString(const std::string_view theStr) {
      itsType = Entity::STRING;
      ++itsDocInfo.itsNumStrings;
      itsDocInfo.itsNumChars += theStr.size();
    }
    constexpr void setNull() {
      itsType = Entity::NUL;
      ++itsDocInfo.itsNumNulls;
    }
    constexpr void setArray() {
      itsType = Entity::ARRAY;
      ++itsDocInfo.itsNumArrays;
    }
    constexpr void addArrayEntry(const InfoElement &theEntry) {
      ++itsDocInfo.itsNumArrayEntries;
      itsDocInfo += theEntry.itsDocInfo;
    }
    constexpr void setObject() {
      itsType = Entity::OBJECT;
      ++itsDocInfo.itsNumObjects;
    }
    constexpr void addObjectProperty(const std::string_view theKey,
                                     const InfoElement &theValue) {
      ++itsDocInfo.itsNumObjectProperties;
      itsDocInfo.itsNumChars += theKey.size();
      itsDocInfo += theValue.itsDocInfo;
    }
    constexpr static InfoElement null() { return InfoElement{}; }
  };
  const auto [aParsed, aParsedLen] =
      parsing<EncodingTy>::template parseElement<InfoElement>(theJsonString);
  if (aParsedLen <= 0)
    return aErrorResult;
  return aParsed.itsDocInfo;
}

template <size_t N, typename DocTy>
constexpr bool parseDocument(StrLiteralRef<N> theJsonString, DocTy &theDoc) {
  return true;
}

template <size_t N>
std::unique_ptr<DocumentBase> buildDocument(StrLiteralRef<N> theJsonString) {
  constexpr DocumentInfo aDocInfo = computeDocInfo(theJsonString);
  if (!aDocInfo)
    return nullptr;
  using DocTy = Document<aDocInfo.itsNumDoubles, aDocInfo.itsNumChars,
                         aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                         aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                         aDocInfo.itsNumObjectProperties>;
  std::unique_ptr<DocTy> aDoc = std::make_unique<DocTy>();
  auto aParseResult = parseDocument(theJsonString, *aDoc);
  if (!aParseResult) {
    return nullptr;
  }
  return aDoc;
}

} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_BUILDER_H
