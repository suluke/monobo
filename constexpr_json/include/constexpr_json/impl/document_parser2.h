#ifndef CONSTEXPR_JSON_DOCUMENT_PARSER2_H
#define CONSTEXPR_JSON_DOCUMENT_PARSER2_H

#include "constexpr_json/document.h"
#include "constexpr_json/document_info.h"
#include "constexpr_json/error_codes.h"
#include "constexpr_json/impl/document_allocator.h"
#include "constexpr_json/impl/parsing_utils.h"
#include <cassert>

namespace cjson {
template <typename SourceEncodingTy, typename DestEncodingTy,
          typename ErrorHandlingTy>
struct DocumentParser2 {
private:
  using p = parsing<SourceEncodingTy>;
  using ElementId = intptr_t;
  using ParentId = intptr_t;
  using Location = intptr_t;
  using Type = typename p::Type;
  struct ElementInfo {
    ElementId itsId = 0;
    Type itsType;
    ParentId itsParentId = -1;
    Location itsLocation = 0;
    ElementId itsFirstChild = -1;
    ElementId itsLastChild = -1;
    ElementId itsNextSibling = -1;
    size_t itsNumChildren = 0;
  };

public:
  template <typename DocTy>
  using ResultTy = typename ErrorHandlingTy::template ErrorOr<DocTy>;

  /// Overload that also takes over error checking on the computeDocInfo return
  /// value
  ///
  /// Using this results in shorter code because clients only need to check for
  /// errors at the very end of the parsing process.
  template <typename DocTy>
  static constexpr auto
  parseDocument(const std::string_view theJsonString,
                const typename ErrorHandlingTy::template ErrorOr<DocumentInfo>
                    &theDocInfo)
      -> std::enable_if_t<
          !std::is_same_v<
              const DocumentInfo,
              const typename ErrorHandlingTy::template ErrorOr<DocumentInfo>>,
          ResultTy<DocTy>> {
    if (ErrorHandlingTy::isError(theDocInfo))
      return ErrorHandlingTy::template convertError<DocTy>(theDocInfo);
    return parseDocument<DocTy>(theJsonString,
                                ErrorHandlingTy::unwrap(theDocInfo));
  }

  template <typename DocTy>
  static constexpr ResultTy<DocTy>
  parseDocument(const std::string_view theJsonString,
                const DocumentInfo &theDocInfo) {
    if (!theDocInfo)
      return makeError<DocTy>("Using illegal DocInfo for parsing");
    const auto aElementInfosOrError =
        computeElementInfos<DocTy>(theJsonString, theDocInfo);
    if (ErrorHandlingTy::isError(aElementInfosOrError))
      return makeError<DocTy>("Failed to compute element infos");
    const auto &aElementInfos = ErrorHandlingTy::unwrap(aElementInfosOrError);
    const ElementInfo *aCurrentElm = &aElementInfos.front();
    DocumentAllocator<DocTy, ErrorHandlingTy> aAlloc;
    intptr_t aNextEntityIdx = 1;
    intptr_t aNextPropIdx = 0;
    DocTy aResult{theDocInfo};
    // Invariant: if aCurrentElm is null, aEntity has been set its ElementInfo
    // index before as its payload
    for (Entity &aEntity : aResult.itsEntities) {
      if (!aCurrentElm)
        aCurrentElm = &aElementInfos[aEntity.itsPayload];
      std::string_view aSubJson =
          theJsonString.substr(aCurrentElm->itsLocation);
      // if the parent is an object, consume the key first
      if (aCurrentElm->itsParentId >= 0 &&
          aElementInfos[aCurrentElm->itsParentId].itsType == Type::OBJECT) {
        std::string_view aStr = p::readString(aSubJson);
        size_t aLenRead = aStr.size();
        aStr = p::stripQuotes(aStr);
        const auto aAllocStr =
            aAlloc.template allocateTranscodeString<SourceEncodingTy, DestEncodingTy>(
                aResult, aStr);
        if (ErrorHandlingTy::isError(aAllocStr))
          return ErrorHandlingTy::template convertError<DocTy>(aAllocStr);
        aResult.itsObjectProps[aNextPropIdx++].itsKeyPos =
            ErrorHandlingTy::unwrap(aAllocStr).itsPayload;
        aSubJson.remove_prefix(aLenRead);
        aSubJson = p::removeLeadingWhitespace(aSubJson);
        const auto aDecoded = SourceEncodingTy::decodeFirst(aSubJson);
        assert(aDecoded.second > 0 && aDecoded.first == ':' &&
               "Expected colon");
        aSubJson.remove_prefix(aDecoded.second);
        aSubJson = p::removeLeadingWhitespace(aSubJson);
      }

      // now get the value
      switch (aCurrentElm->itsType) {
      case Type::NUL:
        aEntity = {Entity::NUL, 0};
        break;
      case Type::BOOL:
        aEntity = {Entity::BOOL, p::parseBool(aSubJson).first};
        break;
      case Type::NUMBER:
        aEntity =
            aAlloc.allocateNumber(aResult, p::parseNumber(aSubJson).first);
        break;
      case Type::STRING: {
        const auto aAllocStr =
            aAlloc.template allocateTranscodeString<SourceEncodingTy, DestEncodingTy>(
                aResult, p::stripQuotes(p::readString(aSubJson)));
        if (ErrorHandlingTy::isError(aAllocStr))
          return ErrorHandlingTy::template convertError<DocTy>(aAllocStr);
        aEntity = ErrorHandlingTy::unwrap(aAllocStr);
        break;
      }
      case Type::ARRAY:
      case Type::OBJECT: {
        if (aCurrentElm->itsType == Type::OBJECT) {
          aEntity = aAlloc.allocateObject(aResult, aCurrentElm->itsNumChildren);
        } else {
          aEntity = aAlloc.allocateArray(aResult, aCurrentElm->itsNumChildren);
        }
        // maintain invariant
        if (aCurrentElm->itsFirstChild >= 0) {
          aResult.itsEntities[aNextEntityIdx].itsPayload =
              aCurrentElm->itsFirstChild;
          aNextEntityIdx += aCurrentElm->itsNumChildren;
        }
        break;
      }
      }
      if (aCurrentElm->itsNextSibling != -1) {
        aCurrentElm = &aElementInfos[aCurrentElm->itsNextSibling];
      } else {
        aCurrentElm = nullptr;
      }
    }
#undef PARSE_STRING
    return aResult;
  }

private:
  template <typename DocTy>
  using ElementInfos =
      typename DocTy::Storage::template Buffer<ElementInfo,
                                               DocTy::Storage::MAX_ENTITIES()>;

  static constexpr std::string_view
  consumeObjectKey(const std::string_view theString) {
    std::string_view aRemaining = theString;
    aRemaining.remove_prefix(p::readString(aRemaining).size());
    aRemaining = p::removeLeadingWhitespace(aRemaining);
    const auto aDecoded = SourceEncodingTy::decodeFirst(aRemaining);
    assert(aDecoded.second > 0 && aDecoded.first == ':' && "Expected colon");
    aRemaining.remove_prefix(aDecoded.second);
    aRemaining = p::removeLeadingWhitespace(aRemaining);
    return aRemaining;
  }

  template <typename DocTy>
  static constexpr auto
  computeElementInfos(const std::string_view theJsonString,
                      const DocumentInfo &theDocInfo)
      -> ResultTy<ElementInfos<DocTy>> {
    // state variables
    ElementInfos<DocTy> aEntities{
        DocTy::Storage::template createBuffer<ElementInfo,
                                              DocTy::Storage::MAX_ENTITIES()>(
            static_cast<size_t>(theDocInfo.itsNumArrayEntries +
                                theDocInfo.itsNumObjectProperties + 1))};
    ParentId aCurrentParent = -1;
    Type aCurrentParentType = Type::NUL;
    bool aIsFirstChild = true;
    std::string_view aRemaining = theJsonString;

    for (size_t aElmIdx = 0; aElmIdx < aEntities.size(); ++aElmIdx) {
      aRemaining = p::removeLeadingWhitespace(aRemaining);
      // leave surrounding element(s)
      for (;;) {
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aCharWidth <= 0)
          return makeElmInfoError<DocTy>(
              "Unexpected EOF or failed to decode character");
        if ((aCurrentParentType == Type::OBJECT && aChar == '}') ||
            (aCurrentParentType == Type::ARRAY && aChar == ']')) {
          // move one level up again
          aRemaining.remove_prefix(aCharWidth);
          aRemaining = p::removeLeadingWhitespace(aRemaining);
          const auto &aOldParentInfo = aEntities[aCurrentParent];
          aCurrentParent = aOldParentInfo.itsParentId;
          aIsFirstChild = false;
          if (aCurrentParent >= 0) {
            const auto &aNewParentInfo = aEntities[aCurrentParent];
            assert((aNewParentInfo.itsType == Type::OBJECT ||
                    aNewParentInfo.itsType == Type::ARRAY) &&
                   "Parent element must be array or object");
            aCurrentParentType = aNewParentInfo.itsType;
          } else {
            aCurrentParentType = Type::NUL;
            break;
          }
          continue;
        }
        break;
      }
      // sanity check
      assert(
          (aIsFirstChild || aCurrentParentType != Type::NUL) &&
          "Second element in root scope. You may be using the wrong DocType");
      // consume comma
      if (!aIsFirstChild) {
        const auto [aChar, aCharWidth] =
            SourceEncodingTy::decodeFirst(aRemaining);
        if (aChar != ',')
          return makeElmInfoError<DocTy>("Expected comma");
        aRemaining.remove_prefix(aCharWidth);
        aRemaining = p::removeLeadingWhitespace(aRemaining);
      }

      // location is registered before the key is consumed
      const Location aLocation = theJsonString.size() - aRemaining.size();

      // consume key
      if (aCurrentParentType == Type::OBJECT) {
        aRemaining = consumeObjectKey(aRemaining);
      }
      // detect type
      auto aTypeOpt = p::detectElementType(aRemaining);
      if (!aTypeOpt) {
        return makeElmInfoError<DocTy>("Failed to detect element type");
      }

      // set element properties
      aEntities[aElmIdx].itsId = aElmIdx;
      aEntities[aElmIdx].itsType = *aTypeOpt;
      aEntities[aElmIdx].itsParentId = aCurrentParent;
      aEntities[aElmIdx].itsLocation = aLocation;
      if (aCurrentParent >= 0) {
        auto &aParent = aEntities[aCurrentParent];
        ++aParent.itsNumChildren;
        if (aIsFirstChild) {
          aParent.itsFirstChild = aElmIdx;
        }
        const auto aPrevSibIdx = aParent.itsLastChild;
        aParent.itsLastChild = aElmIdx;
        if (aPrevSibIdx != -1) {
          aEntities[aPrevSibIdx].itsNextSibling = aElmIdx;
        }
      }
      // setup for reading next element
      aIsFirstChild = false;
      switch (*aTypeOpt) {
      case Type::NUL:
        aRemaining.remove_prefix(p::readNull(aRemaining).size());
        break;
      case Type::BOOL:
        aRemaining.remove_prefix(p::parseBool(aRemaining).second);
        break;
      case Type::NUMBER:
        aRemaining.remove_prefix(p::readNumber(aRemaining).size());
        break;
      case Type::STRING:
        aRemaining.remove_prefix(p::readString(aRemaining).size());
        break;
      case Type::ARRAY:
      case Type::OBJECT: {
        if (*aTypeOpt == Type::ARRAY) {
          aRemaining.remove_prefix(SourceEncodingTy::encode('[').second);
          aCurrentParentType = Type::ARRAY;
        } else {
          aRemaining.remove_prefix(SourceEncodingTy::encode('{').second);
          aCurrentParentType = Type::OBJECT;
        }
        aCurrentParent = aElmIdx;
        aIsFirstChild = true;
        break;
      }
      }
    }
    return aEntities;
  }

  template <typename DocTy>
  static constexpr auto makeElmInfoError(const char *const theMsg) ->
      typename ErrorHandlingTy::template ErrorOr<ElementInfos<DocTy>> {
    return ErrorHandlingTy::template makeError<ElementInfos<DocTy>>(
        ErrorCode::UNKNOWN, -1);
  }
  template <typename DocTy>
  static constexpr auto makeError(const char *const theMsg,
                                  const ErrorCode theCode = ErrorCode::UNKNOWN)
      -> typename ErrorHandlingTy::template ErrorOr<DocTy> {
    return ErrorHandlingTy::template makeError<DocTy>(theCode, -1);
  }
}; // namespace cjson
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_PARSER2_H
