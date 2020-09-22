#ifndef CONSTEXPR_JSON_PRINTING_H
#define CONSTEXPR_JSON_PRINTING_H
#include "constexpr_json/ext/utf-8.h"

#include <iostream>

namespace cjson {
template <typename CRTPImpl, typename DocumentEncodingTy = Utf8,
          typename OutputEncodingTy = Utf8>
struct PrinterBase {
  template <typename EntityRef>
  static std::ostream &print(std::ostream &theStream,
                             const EntityRef &theEntity) {
    switch (theEntity.getType()) {
    case Entity::BOOL:
      return CRTPImpl::printBool(theStream, theEntity.toBool());
    case Entity::NUMBER:
      return CRTPImpl::printNumber(theStream, theEntity.toNumber());
    case Entity::NUL:
      return CRTPImpl::printNull(theStream);
    case Entity::STRING:
      return CRTPImpl::printString(theStream, theEntity.toString());
    case Entity::ARRAY: {
      theStream << "[";
      const auto aArray = theEntity.toArray();
      for (auto aIter = aArray.begin(), aEnd = aArray.end(); aIter != aEnd;
           ++aIter) {
        const auto &aEntity = *aIter;
        print(theStream, aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          theStream << ", ";
      }
      theStream << "]";
      return theStream;
    }
    case Entity::OBJECT: {
      theStream << "{";
      const auto aObject = theEntity.toObject();
      for (auto aIter = aObject.begin(), aEnd = aObject.end(); aIter != aEnd;
           ++aIter) {
        const auto &[aKey, aEntity] = *aIter;
        print(theStream << "\"" << aKey << "\": ", aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          theStream << ", ";
      }
      theStream << "}";
      return theStream;
    }
    }
    return theStream;
  }

  static std::ostream &printBool(std::ostream &theStream, const bool theBool) {
    return theStream << (theBool ? "true" : "false");
  }
  static std::ostream &printNumber(std::ostream &theStream,
                                   const double theNumber) {
    return theStream << theNumber;
  }
  static std::ostream &printNull(std::ostream &theStream) {
    return theStream << "null";
  }
  static std::ostream &printString(std::ostream &theStream,
                                   const std::string_view theString) {
    if constexpr (std::is_same_v<DocumentEncodingTy, OutputEncodingTy>) {
      return theStream << "\"" << theString << "\"";
    } else {
      std::string_view aRemaining = theString;
      theStream << "\"";
      while (!aRemaining.empty()) {
        const auto [aDocChar, aDocCharWidth] =
            DocumentEncodingTy::decodeFirst(aRemaining);
        aRemaining.remove_prefix(aDocCharWidth);
        const auto [aOutput, aOutputWidth] = OutputEncodingTy::encode(aDocChar);
        for (size_t aIdx = 0; aIdx < aOutputWidth; ++aIdx)
          theStream << aOutput[aIdx];
      }
      theStream << "\"";
    }
    return theStream;
  }
};

template <typename DocumentEncodingTy = Utf8, typename OutputEncodingTy = Utf8>
struct Printer
    : public PrinterBase<Printer<DocumentEncodingTy, OutputEncodingTy>,
                         DocumentEncodingTy, OutputEncodingTy> {
  using Base = PrinterBase<Printer, DocumentEncodingTy, OutputEncodingTy>;
  using Base::print;
};

} // namespace cjson
#endif // CONSTEXPR_JSON_PRINTING_H
