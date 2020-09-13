# CJSON
Compile-time JSON parsing for C++17 with support for different encodings

## Feature Overview
CJSON currently ships with the following features:

* Fully `constexpr` JSON parsing
* Parsing non-static JSON content at runtime using the same algorithms and (even simpler) APIs
* A CMake script to convert JSON files into easily includable C++ header files
* UTF-8 support for encoding/decoding, or **bring your own**!

Also look at the [feature wishlist](https://github.com/suluke/monobo/issues/1) to see what's in the pipeline.

## Basic usage
CJSON operates in two stages to be able to achieve `constexpr` JSON parsing:

1. SCAN: Calculate storage requirements for the parsed data structure --> [DocumentInfo](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document_info.h)
2. PARSE: Populate the data structure --> [Document](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document.h)

The central API to perform these two steps is provided by the [DocumentBuilder](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document_builder.h) class.
A full usage example would look like this:

```cpp
#include "constexpr_json/document_builder.h"
#include "constexpr_json/static_document.h"

int main() {
  using namespace cjson;
  constexpr std::string_view aJsonStr{"1234"};
  // 1. SCAN-phase
  constexpr DocumentInfo aDocInfo = DocumentBuilder<>::computeDocInfo(aJsonStr);
  // 2. PARSE-phase
  using DocTy =
        StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                      aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                      aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                      aDocInfo.itsNumObjectProperties>;
  constexpr auto aDoc = DocumentBuilder<>::parseDocument<DocTy>(aJsonStr, aDocInfo);

  // --- DONE! ---
  static_assert(aDoc); // Parse successful?
  static_assert(aDoc->getStaticRoot().toNumber() == 1234.);
  return 0;
}
```

You can also easily parse non-constexpr JSON content at runtime using almost the same APIs.
Only the doctype changes to `DynamicDocument`.

```cpp
#include "constexpr_json/dynamic_document.h"
// ...
using DocTy = DynamicDocument;
auto aDoc = DocumentBuilder<>::parseDocument<DocTy>(aJsonStr, aDocInfo);
assert(aDoc);
assert(aDoc->getRoot().toNumber() == 1234.);
```
