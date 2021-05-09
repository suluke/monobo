#include "constexpr_json/document_parser.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/static_document.h"

#include <gtest/gtest.h>

using namespace cjson;

TEST(cjson_basic, static_doc) {
  constexpr std::string_view aJsonStr{"1234"};
  constexpr auto aDocInfo = DocumentParser<>::computeDocInfo(aJsonStr);
  ASSERT_TRUE(aDocInfo);
  // Static json object creation
  using StaticDocTy = CJSON_STATIC_DOCTY(*aDocInfo);
  constexpr auto aDocStatic =
      DocumentParser<>::parseDocument<StaticDocTy>(aJsonStr, aDocInfo);
  static_assert(aDocStatic);
  static_assert(aDocStatic->getRoot().toNumber() == 1234.);
}

TEST(cjson_basic, dynamic_doc) {
  // Dynamic json object creation
  auto aDocDynamic = DynamicDocument::parseJson("1234");
  ASSERT_TRUE(aDocDynamic);
  ASSERT_TRUE(*aDocDynamic);
  EXPECT_EQ((*aDocDynamic)->getRoot().toNumber(), 1234.);
}

TEST(cjson_basic, access_equal) {
  // Dynamic json object creation
  const std::string_view aJson(R"{"}""{"}(
    {
      "a": 1,
      "b": [123],
      "c": {
        "x": true,
        "y": false,
        "z": null
      }
    }
  ){"}""{"}");
  auto aDoc1 = DynamicDocument::parseJson(aJson);
  auto aDoc2 = DynamicDocument::parseJson(aJson);
  ASSERT_TRUE(aDoc1 && *aDoc1);
  ASSERT_TRUE(aDoc2 && *aDoc2);
  const auto aRoot1 = (*aDoc1)->getRoot();
  const auto aRoot2 = (*aDoc2)->getRoot();
  EXPECT_EQ(aRoot1, aRoot2);
  EXPECT_FALSE(aRoot1 != aRoot2);
}
