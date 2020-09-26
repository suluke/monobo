#ifndef CONSTEXPR_JSON_DOCUMENT_ACCESS_H
#define CONSTEXPR_JSON_DOCUMENT_ACCESS_H
#include "constexpr_json/impl/document_entities.h"

#include <map>
#include <vector>

namespace cjson {
namespace impl {
template <typename DocumentTy> struct EntityRefImpl;

template <typename DocumentTy> struct ArrayIteratorImpl {
  constexpr ArrayIteratorImpl() noexcept = default;
  constexpr ArrayIteratorImpl(const DocumentTy &theDoc,
                              const Entity *thePosition) noexcept
      : itsDoc{&theDoc}, itsPosition{thePosition} {}
  constexpr EntityRefImpl<DocumentTy> operator*() const;
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
  const DocumentTy *itsDoc = {};
  const Entity *itsPosition = nullptr;
};

template <typename DocumentTy> struct ArrayRefImpl {
  using iterator = ArrayIteratorImpl<DocumentTy>;

  constexpr ArrayRefImpl(const DocumentTy &theDoc, const Entity &theEntity)
      : itsDoc{&theDoc}, itsBegin{itsDoc->array_begin(theEntity.itsPayload)},
        itsNumElements{itsDoc->array_size(theEntity.itsPayload)} {}

  constexpr iterator begin() const { return {*itsDoc, itsBegin}; }
  constexpr iterator end() const { return {*itsDoc, itsBegin + size()}; }
  constexpr EntityRefImpl<DocumentTy> operator[](size_t theIdx) const;
  template <typename OtherRefTy>
  constexpr bool operator==(const OtherRefTy &theOther) {
    auto aOtherIter = theOther.begin();
    for (const EntityRefImpl aEntry : *this) {
      if (!(aEntry == *aOtherIter))
        return false;
      ++aOtherIter;
    }
    return true;
  }

  constexpr size_t size() const { return itsNumElements; }
  constexpr bool empty() const { return !size(); }

private:
  const DocumentTy *itsDoc;
  const Entity *itsBegin;
  size_t itsNumElements;
};

template <typename DocumentTy> struct ObjectIteratorImpl {
  constexpr ObjectIteratorImpl() noexcept = default;
  constexpr ObjectIteratorImpl(const DocumentTy &theDoc,
                               const intptr_t &theObjectIdx,
                               const size_t thePosition) noexcept
      : itsDoc{&theDoc}, itsObjectIdx{theObjectIdx}, itsPosition{thePosition} {}

  using EntityRef = EntityRefImpl<DocumentTy>;
  using value_type = std::pair<std::string_view, EntityRef>;

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
  const DocumentTy *itsDoc = {};
  intptr_t itsObjectIdx = 0;
  size_t itsPosition = 0;
};

template <typename DocumentTy> struct ObjectRefImpl {
  using EntityRef = EntityRefImpl<DocumentTy>;
  using iterator = ObjectIteratorImpl<DocumentTy>;

  constexpr ObjectRefImpl(const DocumentTy &theDoc, const Entity &theEntity)
      : itsDoc{&theDoc}, itsObjectIdx{theEntity.itsPayload} {}

  constexpr iterator begin() const { return {*itsDoc, itsObjectIdx, 0}; }
  constexpr iterator end() const { return {*itsDoc, itsObjectIdx, size()}; }
  constexpr std::optional<EntityRef> operator[](std::string_view theKey) const;
  template<typename OtherRefTy>
  constexpr bool operator==(const OtherRefTy &theOther) const noexcept {
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
  const DocumentTy *itsDoc;
  intptr_t itsObjectIdx = 0;
};

template <typename DocumentTy> struct EntityRefImpl {
public:
  constexpr EntityRefImpl(const DocumentTy &theDoc,
                          const Entity &theEntity) noexcept
      : itsDoc(&theDoc), itsEntity(&theEntity) {}
  constexpr EntityRefImpl() = delete;
  constexpr EntityRefImpl(const EntityRefImpl &) = default;
  constexpr EntityRefImpl(EntityRefImpl &&) = default;
  constexpr EntityRefImpl &operator=(const EntityRefImpl &) = default;
  constexpr EntityRefImpl &operator=(EntityRefImpl &&) = default;

  using ArrayRef = ArrayRefImpl<DocumentTy>;
  using ObjectRef = ObjectRefImpl<DocumentTy>;

  template <typename OtherRefTy>
  constexpr bool operator==(const OtherRefTy &theOther) const noexcept {
    if (theOther.getType() != getType())
      return false;
    switch (getType()) {
    case Entity::NUL:
      return true;
    case Entity::ARRAY:
      return toArray() == theOther.toArray();
    case Entity::BOOL:
      return toBool() == theOther.toBool();
    case Entity::NUMBER:
      return toNumber() == theOther.toNumber();
    case Entity::OBJECT:
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
  constexpr ArrayRef toArray() const { return ArrayRef{*itsDoc, *itsEntity}; }
  std::vector<EntityRefImpl> toVector() const {
    intptr_t aIdx = itsEntity->itsPayload;
    std::vector<EntityRefImpl> aArray;
    aArray.reserve(itsDoc->array_size(aIdx));
    for (const Entity *aIter = itsDoc->array_begin(aIdx),
                      *aEnd = itsDoc->array_end(aIdx);
         aIter != aEnd; ++aIter) {
      aArray.emplace_back(*itsDoc, *aIter);
    }
    return aArray;
  }
  constexpr ObjectRef toObject() const { return {*itsDoc, *itsEntity}; }
  std::map<std::string_view, EntityRefImpl> toMap() const {
    intptr_t aObjectIdx = itsEntity->itsPayload;
    std::map<std::string_view, EntityRefImpl> aObject;
    for (size_t aPropIdx = 0u; aPropIdx < itsDoc->getNumProperties(aObjectIdx);
         ++aPropIdx) {
      const auto [aKey, aValuePtr] = itsDoc->getProperty(aObjectIdx, aPropIdx);
      aObject.emplace(std::make_pair(aKey, EntityRefImpl{*itsDoc, *aValuePtr}));
    }
    std::ignore = aObject;
    return aObject;
  }

private:
  const DocumentTy *itsDoc = {};
  const Entity *itsEntity = nullptr;
};

template <typename DocumentTy>
constexpr EntityRefImpl<DocumentTy>
ArrayIteratorImpl<DocumentTy>::operator*() const {
  return {*itsDoc, *itsPosition};
}

template <typename DocumentTy>
constexpr EntityRefImpl<DocumentTy>
ArrayRefImpl<DocumentTy>::operator[](size_t theIdx) const {
  return {*itsDoc, *(itsBegin + theIdx)};
}

template <typename DocumentTy>
constexpr typename ObjectIteratorImpl<DocumentTy>::value_type
ObjectIteratorImpl<DocumentTy>::operator*() const {
  const auto [aKey, aValuePtr] = itsDoc->getProperty(itsObjectIdx, itsPosition);
  return {aKey, EntityRef{*itsDoc, *aValuePtr}};
}

template <typename DocumentTy>
constexpr std::optional<EntityRefImpl<DocumentTy>>
ObjectRefImpl<DocumentTy>::operator[](std::string_view theKey) const {
  const Entity *const aPropPtr = itsDoc->getProperty(itsObjectIdx, theKey);
  if (!aPropPtr)
    return std::nullopt;
  return EntityRef{*itsDoc, *aPropPtr};
}

} // namespace impl
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ACCESS_H
