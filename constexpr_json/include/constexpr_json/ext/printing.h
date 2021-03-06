#ifndef CONSTEXPR_JSON_PRINTING_H
#define CONSTEXPR_JSON_PRINTING_H
#include "constexpr_json/ext/utf-8.h"
#include "constexpr_json/impl/document_entities.h"

#include <limits>
#include <iostream>

namespace cjson {
template <size_t BufferSize> struct StaticStream {
  std::array<char, BufferSize> itsBuffer;
  size_t itsSize{0};

  constexpr StaticStream() noexcept : itsBuffer{} {}

  constexpr StaticStream operator<<(const char theChar) const noexcept {
    StaticStream aCopy{*this};
    aCopy.itsBuffer[itsSize] = theChar;
    ++aCopy.itsSize;
    return aCopy;
  }
  constexpr StaticStream
  operator<<(const std::string_view theString) const noexcept {
    StaticStream aCopy{*this};
    for (const char aChar : theString)
      aCopy = (aCopy << aChar);
    return aCopy;
  }
  constexpr std::string_view str() const noexcept {
    return {&itsBuffer.front(), itsSize};
  }
};

template <typename CRTPImpl, typename DocumentEncodingTy = Utf8,
          typename OutputEncodingTy = Utf8,
          typename StreamStateTy = std::ostream &>
struct PrinterBase {
  constexpr PrinterBase(const OutputEncodingTy theOutEnc = {})
      : itsOutEnc{theOutEnc} {}

  struct RefWrapper {
    using T = typename std::remove_reference_t<StreamStateTy>;
    constexpr RefWrapper(T &theRef) noexcept : itsPtr{&theRef} {}
    constexpr RefWrapper &operator=(T &theRef) noexcept {
      itsPtr = &theRef;
      return *this;
    }
    constexpr operator T &() const noexcept { return *itsPtr; }
    T *itsPtr;
  };
  using EncodedCharTy = decltype(std::declval<OutputEncodingTy>().encode(' '));
  using StreamStateHolder =
      std::conditional_t<std::is_reference_v<StreamStateTy>, RefWrapper,
                         StreamStateTy>;
  constexpr EncodedCharTy OUT_COMMA() const { return itsOutEnc.encode(','); }
  constexpr EncodedCharTy OUT_COLON() const { return itsOutEnc.encode(':'); }
  constexpr EncodedCharTy OUT_QUOTE() const { return itsOutEnc.encode('"'); }
  constexpr EncodedCharTy OUT_ARRAY_BEGIN() const {
    return itsOutEnc.encode('[');
  }
  constexpr EncodedCharTy OUT_ARRAY_END() const {
    return itsOutEnc.encode(']');
  }
  constexpr EncodedCharTy OUT_OBJECT_BEGIN() const {
    return itsOutEnc.encode('{');
  }
  constexpr EncodedCharTy OUT_OBJECT_END() const {
    return itsOutEnc.encode('}');
  }

  template <typename EntityRef>
  constexpr StreamStateTy print(const StreamStateTy theStream,
                                const EntityRef &theEntity) const {
    const auto &aImpl = static_cast<const CRTPImpl &>(*this);
    switch (theEntity.getType()) {
    case Entity::BOOL:
      return aImpl.printBool(theStream, theEntity.toBool());
    case Entity::NUMBER:
      return aImpl.printNumber(theStream, theEntity.toNumber());
    case Entity::NUL:
      return aImpl.printNull(theStream);
    case Entity::STRING:
      return aImpl.printString(theStream, theEntity.toString());
    case Entity::ARRAY: {
      StreamStateHolder aStream{theStream};
      aStream = printEncodedChar(aStream, OUT_ARRAY_BEGIN());
      const auto aArray = theEntity.toArray();
      for (auto aIter = aArray.begin(), aEnd = aArray.end(); aIter != aEnd;
           ++aIter) {
        const auto &aEntity = *aIter;
        aStream = print(aStream, aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          aStream = printEncodedChar(aStream, OUT_COMMA());
      }
      aStream = printEncodedChar(aStream, OUT_ARRAY_END());
      return aStream;
    }
    case Entity::OBJECT: {
      StreamStateHolder aStream{theStream};
      aStream = printEncodedChar(aStream, OUT_OBJECT_BEGIN());
      const auto aObject = theEntity.toObject();
      for (auto aIter = aObject.begin(), aEnd = aObject.end(); aIter != aEnd;
           ++aIter) {
        const auto &[aKey, aEntity] = *aIter;
        aStream = aImpl.printString(aStream, aKey);
        aStream = printEncodedChar(aStream, OUT_COLON());
        aStream = print(aStream, aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          aStream = printEncodedChar(aStream, OUT_COMMA());
      }
      aStream = printEncodedChar(aStream, OUT_OBJECT_END());
      return aStream;
    }
    }
    return theStream;
  }

  constexpr StreamStateTy printBool(const StreamStateTy theStream,
                                    const bool theBool) const {
    return printAsciiChars(theStream, theBool ? "true" : "false");
  }
  constexpr StreamStateTy printNumber(const StreamStateTy theStream,
                                      const double theNumber) const {
    const auto [aBuf, aStrLen] = formatNumber(theNumber);
    std::string_view aStr{&aBuf.front(), aStrLen};
    return theStream << aStr;
  }
  constexpr StreamStateTy printNull(const StreamStateTy theStream) const {
    return printAsciiChars(theStream, "null");
  }
  constexpr StreamStateTy printString(const StreamStateTy theStream,
                                      const std::string_view theString) const {
    StreamStateHolder aStream{theStream};
    if constexpr (std::is_same_v<DocumentEncodingTy, OutputEncodingTy>) {
      aStream = printEncodedChar(aStream, OUT_QUOTE());
      aStream = (static_cast<StreamStateTy>(aStream) << theString);
      aStream = printEncodedChar(aStream, OUT_QUOTE());
    } else {
      aStream = printEncodedChar(aStream, OUT_QUOTE());
      aStream = printDocChars(aStream, theString);
      aStream = printEncodedChar(aStream, OUT_QUOTE());
    }
    return aStream;
  }

private:
  OutputEncodingTy itsOutEnc;

protected:
  /// Takes a sequence of document-encoded characters and prints it
  /// encoded according to the OutputEncodingTy to the given stream
  constexpr StreamStateTy printDocChars(const StreamStateTy theStream,
                                        const std::string_view theChars) const {
    std::string_view aRemaining = theChars;
    StreamStateHolder aStream{theStream};
    while (!aRemaining.empty()) {
      const auto [aDocChar, aDocCharWidth] =
          DocumentEncodingTy::decodeFirst(aRemaining);
      aRemaining.remove_prefix(aDocCharWidth);
      const auto aEncoded = itsOutEnc.encode(aDocChar);
      aStream = printEncodedChar(aStream, aEncoded);
    }
    return aStream;
  }

  constexpr StreamStateTy
  printAsciiChars(const StreamStateTy theStream,
                  const std::string_view theChars) const {
    StreamStateHolder aStream{theStream};
    for (const char aChar : theChars)
      aStream = printEncodedChar(aStream, itsOutEnc.encode(aChar));
    return aStream;
  }

  constexpr StreamStateTy printEncodedChar(const StreamStateTy theStream,
                                           const EncodedCharTy &theChar) const {
    StreamStateHolder aStream{theStream};
    for (size_t aIdx = 0; aIdx < theChar.second; ++aIdx)
      aStream = (static_cast<StreamStateTy>(aStream) << theChar.first[aIdx]);
    return aStream;
  }

  constexpr static inline size_t DOUBLE_MAX_PRECISION =
      std::numeric_limits<double>::max_digits10;
  constexpr static std::pair<std::array<char, DOUBLE_MAX_PRECISION + 1>, size_t>
  formatNumber(double theNumber) noexcept {
    // TODO this is a naive implementation
    StaticStream<DOUBLE_MAX_PRECISION + 1> aStream;
    constexpr int aRadix = 10;
    constexpr char aDecSep = '.';
    using IntMantissaTy = size_t;

    if (theNumber < 0) {
      aStream = (aStream << '-');
      theNumber *= -1.;
    }

    // determine highest power of 10 fitting into theNumber
    IntMantissaTy aPowRadix = 1;
    while (theNumber >= aPowRadix)
      aPowRadix *= aRadix;
    aPowRadix /= aRadix;
    aPowRadix = (aPowRadix == 0 ? 1 : aPowRadix);

    // Print integer part
    {
      // print at least one digit before the decimal separator
      IntMantissaTy aCurrentPowRadix = aPowRadix;
      IntMantissaTy aRemainder = static_cast<IntMantissaTy>(theNumber);
      while (aCurrentPowRadix != 0) {
        int aDigit = 0;
        while (aRemainder >= aCurrentPowRadix) {
          ++aDigit;
          aRemainder -= aCurrentPowRadix;
        }
        aStream = (aStream << ('0' + aDigit));
        aCurrentPowRadix /= aRadix;
      }
    }
    // Print decimal part
    {
      double aRemainder =
          theNumber -
          static_cast<double>(static_cast<IntMantissaTy>(theNumber));
      double aEpsilon = aPowRadix * std::numeric_limits<double>::epsilon();
      if (aRemainder >= aEpsilon) {
        aStream = (aStream << aDecSep);
        do {
          int aDigit = static_cast<int>(aRemainder * 10);
          aStream = (aStream << ('0' + aDigit));
          aRemainder = (aRemainder * 10.) - aDigit;
          aEpsilon *= aRadix;
        } while (aRemainder >= aEpsilon);
      }
    }
    return {aStream.itsBuffer, aStream.itsSize};
  }
};

template <typename DocumentEncodingTy = Utf8, typename OutputEncodingTy = Utf8,
          typename StreamStateTy = std::ostream &>
struct Printer
    : public PrinterBase<
          Printer<DocumentEncodingTy, OutputEncodingTy, StreamStateTy>,
          DocumentEncodingTy, OutputEncodingTy, StreamStateTy> {
  using Base =
      PrinterBase<Printer, DocumentEncodingTy, OutputEncodingTy, StreamStateTy>;
  constexpr Printer(const OutputEncodingTy theOutEnc = {}) : Base{theOutEnc} {}
  using Base::print;
};

} // namespace cjson
#endif // CONSTEXPR_JSON_PRINTING_H
