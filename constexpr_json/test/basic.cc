#include "constexpr_json/document_builder.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/static_document.h"

#include <cassert>

int main() {
  using namespace cjson;
  constexpr std::string_view aJsonStr{"1234"};
  constexpr DocumentInfo aDocInfo = DocumentBuilder<>::computeDocInfo(aJsonStr);
  // Static json object creation
  using StaticDocTy =
      StaticDocument<aDocInfo.itsNumNumbers, aDocInfo.itsNumChars,
                     aDocInfo.itsNumStrings, aDocInfo.itsNumArrays,
                     aDocInfo.itsNumArrayEntries, aDocInfo.itsNumObjects,
                     aDocInfo.itsNumObjectProperties>;
  constexpr auto aDocStatic =
      DocumentBuilder<>::parseDocument<StaticDocTy>(aJsonStr, aDocInfo);
  static_assert(aDocStatic);
  static_assert(aDocStatic->getStaticRoot().toNumber() == 1234.);

  // Dynamic json object creation
  using DynamicDocTy = DynamicDocument;
  auto aDocDynamic =
      DocumentBuilder<>::parseDocument<DynamicDocTy>(aJsonStr, aDocInfo);
  assert(aDocDynamic);
  assert(aDocDynamic->getRoot().toNumber() == 1234.);

  return 0;
}