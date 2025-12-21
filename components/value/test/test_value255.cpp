#include <unity.h>
#include <value255.hpp>

using namespace value;

// テスト前の準備
void setUp(void) {
    // 各テストの前に実行される
}

// テスト後のクリーンアップ
void tearDown(void) {
    // 各テストの後に実行される
}


TEST_CASE("Value255 string conversion", "[Value255]")
{
    std::byte src[] = {std::byte{0xAB}, std::byte{0xCD}};
    auto v = Value255::create(src, sizeof(src));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("[ 0xAB 0xCD ]", v->str().c_str());
}

TEST_CASE("Value255 empty value", "[Value255]")
{
    auto result = Value255::create(nullptr, 0);
    TEST_ASSERT_TRUE(result.has_value());
    auto bytes = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(0, bytes.size());
}

TEST_CASE("Value255 inline value", "[Value255]")
{
    std::byte data[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    auto result = Value255::create(data, 3);
    
    TEST_ASSERT_TRUE(result.has_value());
    auto retrieved = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(3, retrieved.size());
    TEST_ASSERT_EQUAL_UINT8(0x01, static_cast<uint8_t>(retrieved[0]));
    TEST_ASSERT_EQUAL_UINT8(0x02, static_cast<uint8_t>(retrieved[1]));
    TEST_ASSERT_EQUAL_UINT8(0x03, static_cast<uint8_t>(retrieved[2]));
}

TEST_CASE("Value255 heap value", "[Value255]")
{
    std::byte data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = std::byte{static_cast<uint8_t>(i)};
    }
    
    auto result = Value255::create(data, 100);
    
    TEST_ASSERT_TRUE(result.has_value());
    auto retrieved = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(100, retrieved.size());
    
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_UINT8(i, static_cast<uint8_t>(retrieved[i]));
    }
}
