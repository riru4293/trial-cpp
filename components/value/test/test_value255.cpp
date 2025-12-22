#include <unity.h>
#include <value255.hpp>

using namespace value;


TEST_CASE("Value255 string conversion", "[Value255]")
{
    std::byte src[] = {std::byte{0xAB}, std::byte{0xCD}};
    auto v = Value255::create(src, sizeof(src));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("[ 0xAB 0xCD ]", v->str().c_str());
}
