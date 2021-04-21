#include "json_schema/util/variant.h"
#include "json_schema/util/type_tag.h"
#include <gtest/gtest.h>

using namespace json_schema;

struct Foo {
  int itsValue = 21;
};

struct Bar {
  const char *itsValue;

  constexpr Bar(const char *const theValue) noexcept : itsValue{theValue} {}
  constexpr Bar &operator=(const char *const theValue) {
    itsValue = theValue;
    return *this;
  }
};

struct Baz {};

using ShouldBeBar = decltype(impl::InitSelect<Bar, void>{}(""))::type;
static_assert(std::is_same_v<Bar, ShouldBeBar>);

enum class Result { Ok, Err };

template<typename V>
Result checkGet(const V&, ...) {
  return Result::Err;
}
template<typename V, typename T>
Result checkGet(const V& theVariant, const type_tag<T>& theType) {
  get<T>(theVariant);
  return Result::Ok;
}

TEST(VariantTest, BasicAssertions) {
  Variant<Foo, Bar, Baz> aVariant;
  EXPECT_TRUE(holds_alternative<Foo>(aVariant));
  EXPECT_EQ(21, get<Foo>(aVariant).itsValue);
  aVariant = "This is a test";
  EXPECT_TRUE(holds_alternative<Bar>(aVariant));
  EXPECT_STREQ("This is a test", get<Bar>(aVariant).itsValue);
  EXPECT_EQ(Result::Err, checkGet(type_tag<void>{}, aVariant));
}
