#ifndef CONSTEXPR_JSON_DOCUMENT_ACCESS_H
#define CONSTEXPR_JSON_DOCUMENT_ACCESS_H
#include "constexpr_json/impl/document_entities.h"

#include <map>
#include <vector>

namespace cjson {
namespace impl {
template <typename DocumentRefType> struct EntityRefImpl;

template <typename DocumentRefType> struct ArrayIteratorImpl {
  constexpr ArrayIteratorImpl() noexcept = default;
  constexpr ArrayIteratorImpl(const DocumentRefType theDoc,
                              const Entity *thePosition) noexcept
      : itsDoc{theDoc}, itsPosition{thePosition} {}
  constexpr EntityRefImpl<DocumentRefType> operator*() const;
  constexpr ArrayIteratorImpl &operator++() {
    ++itsPosition;
    return *this;
  }
  constexpr bool operator==(const ArrayIteratorImpl &theOther) const noexcept {
    return itsDoc == theOther.itsDoc && itsPosition == theOther.itsPosition;
  }
  constexpr bool operator!=(const ArrayIteratorImpl &theOther) const noexcept {
    return !(*this == theOther);
  }

private:
  DocumentRefType itsDoc = {};
  const Entity *itsPosition = nullptr;
};

template <typename DocumentRefType> struct ArrayRefImpl {
  using iterator = ArrayIteratorImpl<DocumentRefType>;

  constexpr ArrayRefImpl(const DocumentRefType theDoc, const Entity &theEntity)
      : itsDoc{theDoc}, itsBegin{theDoc->array_begin(theEntity.itsPayload)},
        itsNumElements{theDoc->array_size(theEntity.itsPayload)} {}

  constexpr iterator begin() const { return {itsDoc, itsBegin}; }
  constexpr iterator end() const { return {itsDoc, itsBegin + itsNumElements}; }

  constexpr size_t size() const { return itsNumElements; }
  constexpr bool empty() const { return !size(); }

private:
  DocumentRefType itsDoc;
  const Entity *itsBegin;
  size_t itsNumElements;
};

template <typename DocumentRefType> struct EntityRefImpl {
public:
  constexpr EntityRefImpl(const DocumentRefType theDoc, const Entity &theEntity) noexcept
      : itsDoc(theDoc), itsEntity(&theEntity) {}
  constexpr EntityRefImpl() = delete;
  constexpr EntityRefImpl(const EntityRefImpl &) = default;
  constexpr EntityRefImpl(EntityRefImpl &&) = default;
  constexpr EntityRefImpl &operator=(const EntityRefImpl &) = default;
  constexpr EntityRefImpl &operator=(EntityRefImpl &&) = default;

  using ArrayRef = ArrayRefImpl<DocumentRefType>;

  constexpr bool operator==(const EntityRefImpl &theOther) const noexcept {
    if (theOther.getType() != getType())
      return false;
    switch (getType()) {
    case Entity::NUL:
      return true;
    case Entity::ARRAY: {
      auto aOtherIter = theOther.toArray().begin();
      for (const EntityRefImpl aEntry : toArray()) {
        if (!(aEntry == *aOtherIter))
          return false;
        ++aOtherIter;
      }
      return true;
    }
    case Entity::BOOL:
      return toBool() == theOther.toBool();
    case Entity::NUMBER:
      return toNumber() == theOther.toNumber();
    case Entity::OBJECT:
      // TODO not constexpr yet - needs ObjectRef
      return toObject() == theOther.toObject();
      break;
    case Entity::STRING:
      return toString() == theOther.toString();
    }
    return false;
  }

  constexpr Entity::KIND getType() const { return itsEntity->itsKind; }
  constexpr bool toBool() const { return itsEntity->itsPayload; }
  constexpr double toNumber() const {
    return itsDoc->getNumber(itsEntity->itsPayload);
  }
  constexpr std::string_view toString() const {
    return itsDoc->getString(itsEntity->itsPayload);
  }
  constexpr ArrayRef toArray() const { return ArrayRef{itsDoc, *itsEntity}; }
  std::vector<EntityRefImpl> toVector() const {
    intptr_t aIdx = itsEntity->itsPayload;
    std::vector<EntityRefImpl> aArray;
    aArray.reserve(itsDoc->array_size(aIdx));
    for (const Entity *aIter = itsDoc->array_begin(aIdx),
                      *aEnd = itsDoc->array_end(aIdx);
         aIter != aEnd; ++aIter) {
      aArray.emplace_back(itsDoc, *aIter);
    }
    return aArray;
  }
  std::map<std::string_view, EntityRefImpl> toObject() const {
    intptr_t aObjectIdx = itsEntity->itsPayload;
    std::map<std::string_view, EntityRefImpl> aObject;
    for (size_t aPropIdx = 0u; aPropIdx < itsDoc->getNumProperties(aObjectIdx);
         ++aPropIdx)
      aObject.emplace(itsDoc->getProperty(aObjectIdx, aPropIdx));
    std::ignore = aObject;
    return aObject;
  }

private:
  DocumentRefType itsDoc = {};
  const Entity *itsEntity = nullptr;
};

template <typename DocumentRefType>
constexpr EntityRefImpl<DocumentRefType>
ArrayIteratorImpl<DocumentRefType>::operator*() const {
  return {itsDoc, *itsPosition};
}

} // namespace impl
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ACCESS_H
