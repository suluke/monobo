#ifndef CONSTEXPR_JSON_DOCUMENT_ENTITIES_H
#define CONSTEXPR_JSON_DOCUMENT_ENTITIES_H
#include <cstddef>

namespace cjson {
struct Entity {
  enum KIND { NUL = 0, ARRAY, BOOL, NUMBER, OBJECT, STRING } itsKind = NUL;
  intptr_t itsPayload = 0;
};
struct Array {
  intptr_t itsPosition; // index into itsEntities
  size_t itsNumElements;
};
struct Object {
  intptr_t itsKeysPos;   // index into itsObjectProps
  intptr_t itsValuesPos; // index into itsEntities
  size_t itsNumProperties;
};
struct Property {
  intptr_t itsKeyPos; // index into itsStrings
};
struct String {
  intptr_t itsPosition; // index into itsChars
  size_t itsSize;
};
} // namespace cjson
#endif // CONSTEXPR_JSON_DOCUMENT_ENTITIES_H
