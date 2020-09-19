#ifndef CONSTEXPR_JSON_PRINTING_H
#define CONSTEXPR_JSON_PRINTING_H
#include <iostream>

namespace cjson {
template <typename EntityRef>
std::ostream &print(std::ostream &theStream, const EntityRef &theEntity) {
  switch (theEntity.getType()) {
  case Entity::BOOL:
    return theStream << (theEntity.toBool() ? "true" : "false");
  case Entity::NUMBER:
    return theStream << theEntity.toNumber();
  case Entity::NUL:
    return theStream << "null";
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
  case Entity::STRING:
    return theStream << "\"" << theEntity.toString() << "\"";
  }
  return theStream;
}
} // namespace cjson
#endif // CONSTEXPR_JSON_PRINTING_H
