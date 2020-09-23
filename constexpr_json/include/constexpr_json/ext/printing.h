#ifndef CONSTEXPR_JSON_PRINTING_H
#define CONSTEXPR_JSON_PRINTING_H
#include "constexpr_json/ext/utf-8.h"
#include "constexpr_json/impl/document_entities.h"

#include <iostream>

namespace cjson {
template <typename CRTPImpl, typename DocumentEncodingTy = Utf8,
          typename OutputEncodingTy = Utf8,
          typename StreamStateTy = std::ostream &>
struct PrinterBase {
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
  using EncodedCharTy = decltype(OutputEncodingTy::encode(' '));
  using StreamStateHolder =
      std::conditional_t<std::is_reference_v<StreamStateTy>, RefWrapper,
                         StreamStateTy>;
  static inline constexpr EncodedCharTy OUT_COMMA =
      OutputEncodingTy::encode(',');
  static inline constexpr EncodedCharTy OUT_COLON =
      OutputEncodingTy::encode(':');
  static inline constexpr EncodedCharTy OUT_QUOTE =
      OutputEncodingTy::encode('"');
  static inline constexpr EncodedCharTy OUT_ARRAY_BEGIN =
      OutputEncodingTy::encode('[');
  static inline constexpr EncodedCharTy OUT_ARRAY_END =
      OutputEncodingTy::encode(']');
  static inline constexpr EncodedCharTy OUT_OBJECT_BEGIN =
      OutputEncodingTy::encode('{');
  static inline constexpr EncodedCharTy OUT_OBJECT_END =
      OutputEncodingTy::encode('}');

  template <typename EntityRef>
  constexpr static StreamStateTy print(const StreamStateTy theStream,
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
      StreamStateHolder aStream{theStream};
      aStream = printEncodedChar(aStream, OUT_ARRAY_BEGIN);
      const auto aArray = theEntity.toArray();
      for (auto aIter = aArray.begin(), aEnd = aArray.end(); aIter != aEnd;
           ++aIter) {
        const auto &aEntity = *aIter;
        aStream = print(aStream, aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          aStream = printEncodedChar(aStream, OUT_COMMA);
      }
      aStream = printEncodedChar(aStream, OUT_ARRAY_END);
      return aStream;
    }
    case Entity::OBJECT: {
      StreamStateHolder aStream{theStream};
      aStream = printEncodedChar(aStream, OUT_OBJECT_BEGIN);
      const auto aObject = theEntity.toObject();
      for (auto aIter = aObject.begin(), aEnd = aObject.end(); aIter != aEnd;
           ++aIter) {
        const auto &[aKey, aEntity] = *aIter;
        aStream = CRTPImpl::printString(aStream, aKey);
        aStream = printEncodedChar(aStream, OUT_COLON);
        aStream = print(aStream, aEntity);
        auto aNext = aIter;
        ++aNext;
        if (aNext != aEnd)
          aStream = printEncodedChar(aStream, OUT_COMMA);
      }
      aStream = printEncodedChar(aStream, OUT_OBJECT_END);
      return aStream;
    }
    }
    return theStream;
  }

  constexpr static StreamStateTy printBool(const StreamStateTy theStream,
                                           const bool theBool) {
    return printAsciiChars(theStream, theBool ? "true" : "false");
  }
  constexpr static StreamStateTy printNumber(const StreamStateTy theStream,
                                             const double theNumber) {
    return theStream << theNumber;
  }
  constexpr static StreamStateTy printNull(const StreamStateTy theStream) {
    return printAsciiChars(theStream, "null");
  }
  constexpr static StreamStateTy printString(const StreamStateTy theStream,
                                   const std::string_view theString) {
    StreamStateHolder aStream{theStream};
    if constexpr (std::is_same_v<DocumentEncodingTy, OutputEncodingTy>) {
      aStream = printEncodedChar(aStream, OUT_QUOTE);
      aStream = (static_cast<StreamStateTy>(aStream) << theString);
      aStream = printEncodedChar(aStream, OUT_QUOTE);
    } else {
      aStream = printEncodedChar(aStream, OUT_QUOTE);
      aStream = printDocChars(aStream, theString);
      aStream = printEncodedChar(aStream, OUT_QUOTE);
    }
    return aStream;
  }

protected:
  /// Takes a sequence of document-encoded characters and prints it
  /// encoded according to the OutputEncodingTy to the given stream
  constexpr static StreamStateTy printDocChars(const StreamStateTy theStream,
                                     const std::string_view theChars) {
    std::string_view aRemaining = theChars;
    StreamStateHolder aStream{theStream};
    while (!aRemaining.empty()) {
      const auto [aDocChar, aDocCharWidth] =
          DocumentEncodingTy::decodeFirst(aRemaining);
      aRemaining.remove_prefix(aDocCharWidth);
      const auto aEncoded = OutputEncodingTy::encode(aDocChar);
      aStream = printEncodedChar(aStream, aEncoded);
    }
    return aStream;
  }

  constexpr static StreamStateTy printAsciiChars(const StreamStateTy theStream,
                                       const std::string_view theChars) {
    StreamStateHolder aStream{theStream};
    for (const char aChar : theChars)
      aStream = printEncodedChar(aStream, OutputEncodingTy::encode(aChar));
    return aStream;
  }

  constexpr static StreamStateTy printEncodedChar(const StreamStateTy theStream,
                                        const EncodedCharTy &theChar) {
    StreamStateHolder aStream{theStream};
    for (size_t aIdx = 0; aIdx < theChar.second; ++aIdx)
      aStream = (static_cast<StreamStateTy>(aStream) << theChar.first[aIdx]);
    return aStream;
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
  using Base::print;
};

} // namespace cjson
#endif // CONSTEXPR_JSON_PRINTING_H
