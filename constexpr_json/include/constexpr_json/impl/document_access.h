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
  constexpr iterator end() const { return {itsDoc, itsBegin + size()}; }
  constexpr EntityRefImpl<DocumentRefType> operator[](size_t theIdx) const;

  constexpr size_t size() const { return itsNumElements; }
  constexpr bool empty() const { return !size(); }

private:
  DocumentRefType itsDoc;
  const Entity *itsBegin;
  size_t itsNumElements;
};

template <typename DocumentRefType> struct ObjectIteratorImpl {
  constexpr ObjectIteratorImpl() noexcept = default;
  constexpr ObjectIteratorImpl(const DocumentRefType theDoc,
                               const intptr_t &theObjectIdx,
                               const size_t thePosition) noexcept
      : itsDoc{theDoc}, itsObjectIdx{theObjectIdx}, itsPosition{thePosition} {}

  using value_type =
      std::pair<std::string_view, EntityRefImpl<DocumentRefType>>;

  constexpr value_type operator*() const;
  constexpr ObjectIteratorImpl &operator++() {
    ++itsPosition;
    return *this;
  }
  constexpr bool operator==(const ObjectIteratorImpl &theOther) const noexcept {
    return itsDoc == theOther.itsDoc && itsObjectIdx == theOther.itsObjectIdx &&
           itsPosition == theOther.itsPosition;
  }
  constexpr bool operator!=(const ObjectIteratorImpl &theOther) const noexcept {
    return !(*this == theOther);
  }

private:
  DocumentRefType itsDoc = {};
  intptr_t itsObjectIdx = 0;
  size_t itsPosition = 0;
};

template <typename DocumentRefType> struct ObjectRefImpl {
  using iterator = ObjectIteratorImpl<DocumentRefType>;

  constexpr ObjectRefImpl(const DocumentRefType theDoc, const Entity &theEntity)
      : itsDoc{theDoc}, itsObjectIdx{theEntity.itsPayload} {}

  constexpr iterator begin() const { return {itsDoc, itsObjectIdx, 0}; }
  constexpr iterator end() const { return {itsDoc, itsObjectIdx, size()}; }
  constexpr std::optional<EntityRefImpl<DocumentRefType>>
  operator[](std::string_view theKey) const;
  constexpr bool operator==(const ObjectRefImpl &theOther) const noexcept {
    for (const auto aKVPair : *this) {
      const auto aOtherVal = theOther[aKVPair.first];
      if (!aOtherVal || !(aKVPair.second == *aOtherVal))
        return false;
    }
    return true;
  }

  constexpr size_t size() const {
    return itsDoc->getNumProperties(itsObjectIdx);
  }
  constexpr bool empty() const { return !size(); }

private:
  DocumentRefType itsDoc;
  intptr_t itsObjectIdx = 0;
};

template <typename DocumentRefType> struct EntityRefImpl {
public:
  constexpr EntityRefImpl(const DocumentRefType theDoc,
                          const Entity &theEntity) noexcept
      : itsDoc(theDoc), itsEntity(&theEntity) {}
  constexpr EntityRefImpl() = delete;
  constexpr EntityRefImpl(const EntityRefImpl &) = default;
  constexpr EntityRefImpl(EntityRefImpl &&) = default;
  constexpr EntityRefImpl &operator=(const EntityRefImpl &) = default;
  constexpr EntityRefImpl &operator=(EntityRefImpl &&) = default;

  using ArrayRef = ArrayRefImpl<DocumentRefType>;
  using ObjectRef = ObjectRefImpl<DocumentRefType>;

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
  constexpr ObjectRef toObject() const { return {itsDoc, *itsEntity}; }
  std::map<std::string_view, EntityRefImpl> toMap() const {
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

template <typename DocumentRefType>
constexpr EntityRefImpl<DocumentRefType>
ArrayRefImpl<DocumentRefType>::operator[](size_t theIdx) const {
  return {itsDoc, *(itsBegin + theIdx)};
}

template <typename DocumentRefType>
constexpr typename ObjectIteratorImpl<DocumentRefType>::value_type
ObjectIteratorImpl<DocumentRefType>::operator*() const {
  return itsDoc->getProperty(itsObjectIdx, itsPosition);
}

template <typename DocumentRefType>
constexpr std::optional<EntityRefImpl<DocumentRefType>>
ObjectRefImpl<DocumentRefType>::operator[](std::string_view theKey) const {
  // TODO linear search is quite inefficient
  for (const auto aKVPair : *this) {
    if (aKVPair.first == theKey)
      return aKVPair.second;
  }
  return std::nullopt;
}

} // namespace impl
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ACCESS_H
