# CJSON
Compile-time JSON parsing for C++17 with support for different encodings and error handling mechanisms

## Feature Overview
CJSON currently ships with the following features:

* Fully `constexpr` JSON parsing
* Parsing non-static JSON content at runtime using the same algorithms and (even simpler) APIs
* A CMake script to convert JSON files into easily includable C++ header files
* UTF-8 support for encoding/decoding, or **bring your own**!
* Configurable strategies for error handling:
   1. `ErrorWillReturnNone` \[default\]: `std::nullopt` is returned when an error occurs
   2. `ErrorWillThrow`: `std::invalid_argument` is thrown when an error occurs
   3. `ErrorWillReturnDetail`: `std::variant<ErrorDetail, ?>` is returned when an error occurs
   It is also possible to provide a user-defined error strategy. Look at [error_handling.h](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/impl/error_handling.h) for reference.

Also look at the [feature wishlist](https://github.com/suluke/monobo/issues/1) to see what's in the pipeline.

## Correctness
CJSON is tested against the popular [JSONTestSuite](https://github.com/nst/JSONTestSuite).
Generating the test result is as simple as running the "JSONTestSuite" target from the generated build system.
This will automatically fetch the newest version of the JSONTestSuite, patch the runner script to become aware of cjson and run the script.

## Basic usage
CJSON operates in two stages to be able to achieve `constexpr` JSON parsing:

1. SCAN: Calculate storage requirements for the parsed data structure --> [DocumentInfo](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document_info.h)
2. PARSE: Populate the data structure --> [Document](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document.h)

The central API to perform these two steps is provided by the [DocumentBuilder](https://github.com/suluke/monobo/blob/master/constexpr_json/include/constexpr_json/document_builder.h) class.
A full usage example would [look like this](https://github.com/suluke/monobo/blob/master/constexpr_json/test/basic.cc):

```cpp
#include "constexpr_json/document_builder.h"
#include "constexpr_json/static_document.h"

int main() {
  using namespace cjson;
  constexpr std::string_view aJsonStr{"1234"};
  // 1. SCAN-phase
  constexpr auto aDocInfo = DocumentBuilder<>::computeDocInfo(aJsonStr);
  if (!aDocInfo)
    return 1;
  // 2. PARSE-phase
  using DocTy =
      StaticDocument<aDocInfo->itsNumNumbers, aDocInfo->itsNumChars,
                     aDocInfo->itsNumStrings, aDocInfo->itsNumArrays,
                     aDocInfo->itsNumArrayEntries, aDocInfo->itsNumObjects,
                     aDocInfo->itsNumObjectProperties>;
  constexpr auto aDoc = DocumentBuilder<>::parseDocument<DocTy>(aJsonStr, *aDocInfo);

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
constexpr auto aDocInfo = DocumentBuilder<>::computeDocInfo(aJsonStr);
using DocTy = DynamicDocument;
auto aDoc = DocumentBuilder<>::parseDocument<DocTy>(aJsonStr, *aDocInfo);
assert(aDoc);
assert(aDoc->getRoot().toNumber() == 1234.);
```
