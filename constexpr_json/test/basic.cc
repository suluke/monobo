#include "constexpr_json/document_parser.h"
#include "constexpr_json/dynamic_document.h"
#include "constexpr_json/ext/error_is_except.h"
#include "constexpr_json/static_document.h"

#include <gtest/gtest.h>

using namespace cjson;

static std::unique_ptr<DynamicDocument>
parseJson(const std::string_view theJsonStr) {
  using Parser = DocumentParser<Utf8, Utf8, ErrorWillThrow<>>;
  return DynamicDocument::parseJson<Parser>(theJsonStr);
}

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
  auto aDoc1 = parseJson(aJson);
  auto aDoc2 = parseJson(aJson);
  const auto aRoot1 = aDoc1->getRoot();
  const auto aRoot2 = aDoc2->getRoot();
  EXPECT_EQ(aRoot1, aRoot2);
  EXPECT_FALSE(aRoot1 != aRoot2);
}
TEST(cjson_basic, access_inequal) {
  {
    auto aDoc1 = parseJson(R"({ "a": 1, "b": true })");
    auto aDoc2 = parseJson(R"({ "a": 1 })");
    EXPECT_NE(aDoc1->getRoot(), aDoc2->getRoot());
    EXPECT_NE(aDoc2->getRoot(), aDoc1->getRoot());
  }
  {
    auto aDoc1 = parseJson(R"([ "a", "b" ])");
    auto aDoc2 = parseJson(R"([ "a" ])");
    EXPECT_NE(aDoc1->getRoot(), aDoc2->getRoot());
    EXPECT_NE(aDoc2->getRoot(), aDoc1->getRoot());
  }
}
