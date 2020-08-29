#ifndef CONSTEXPR_JSON_DOCUMENT_INFO_H
#define CONSTEXPR_JSON_DOCUMENT_INFO_H

namespace cjson {
struct DocumentInfo {
  ssize_t itsNumNulls = 0;
  ssize_t itsNumBools = 0;
  ssize_t itsNumNumbers = 0;
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

  constexpr operator bool() const {
    return itsNumNulls >= 0 && itsNumBools >= 0 && itsNumNumbers >= 0 &&
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
    itsNumArrayEntries += theOther.itsNumArrayEntries;
    itsNumArrays += theOther.itsNumArrays;
    itsNumBools += theOther.itsNumBools;
    itsNumChars += theOther.itsNumChars;
    itsNumNumbers += theOther.itsNumNumbers;
    itsNumNulls += theOther.itsNumNulls;
    itsNumObjectProperties += theOther.itsNumObjectProperties;
    itsNumObjects += theOther.itsNumObjects;
    itsNumStrings += theOther.itsNumStrings;
    return *this;
  }
  constexpr bool operator==(const DocumentInfo &theOther) const {
    return itsNumArrayEntries == theOther.itsNumArrayEntries &&
           itsNumArrays == theOther.itsNumArrays &&
           itsNumBools == theOther.itsNumBools &&
           itsNumChars == theOther.itsNumChars &&
           itsNumNumbers == theOther.itsNumNumbers &&
           itsNumNulls == theOther.itsNumNulls &&
           itsNumObjectProperties == theOther.itsNumObjectProperties &&
           itsNumObjects == theOther.itsNumObjects &&
           itsNumStrings == theOther.itsNumStrings;
  }
};
} // namespace cjson

#endif // CONSTEXPR_JSON_DOCUMENT_INFO_H
