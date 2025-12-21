#include <cstddef>
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

// テスト1: 空のValue255を作成
void test_create_empty_value(void) {
    auto result = Value255::create(nullptr, 0);
    TEST_ASSERT_TRUE(result.has_value());
    auto bytes = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(0, bytes.size());
}

// テスト2: 小さいデータ（インラインストレージ）を使ったValue255を作成
void test_create_inline_value(void) {
    std::byte data[] = {std::byte{0x01}, std::byte{0x02}, std::byte{0x03}};
    auto result = Value255::create(data, 3);
    
    TEST_ASSERT_TRUE(result.has_value());
    auto retrieved = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(3, retrieved.size());
    TEST_ASSERT_EQUAL_UINT8(0x01, static_cast<uint8_t>(retrieved[0]));
    TEST_ASSERT_EQUAL_UINT8(0x02, static_cast<uint8_t>(retrieved[1]));
    TEST_ASSERT_EQUAL_UINT8(0x03, static_cast<uint8_t>(retrieved[2]));
}

// テスト3: 大きいデータ（ヒープストレージ）を使ったValue255を作成
void test_create_heap_value(void) {
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

// テスト4: 最大サイズ（255バイト）のValue255を作成
void test_create_max_size_value(void) {
    std::byte data[255];
    for (int i = 0; i < 255; i++) {
        data[i] = std::byte{static_cast<uint8_t>(i)};
    }
    
    auto result = Value255::create(data, 255);
    
    TEST_ASSERT_TRUE(result.has_value());
    auto bytes = result->bytes();
    TEST_ASSERT_EQUAL_UINT8(255, bytes.size());
}

// テスト5: 無効なパラメータでの作成（nullptrで非ゼロサイズ）
void test_create_invalid_parameters(void) {
    auto result = Value255::create(nullptr, 10);
    TEST_ASSERT_FALSE(result.has_value());
}

// テスト6: クローン機能
void test_clone_value(void) {
    std::byte data[] = {std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}};
    auto original = Value255::create(data, 3);
    TEST_ASSERT_TRUE(original.has_value());
    
    auto cloned = Value255::clone(*original);
    TEST_ASSERT_TRUE(cloned.has_value());
    auto cloned_data = cloned->bytes();
    TEST_ASSERT_EQUAL_UINT8(3, cloned_data.size());
    
    TEST_ASSERT_EQUAL_UINT8(0xAA, static_cast<uint8_t>(cloned_data[0]));
    TEST_ASSERT_EQUAL_UINT8(0xBB, static_cast<uint8_t>(cloned_data[1]));
    TEST_ASSERT_EQUAL_UINT8(0xCC, static_cast<uint8_t>(cloned_data[2]));
}

// テスト7: 等価比較演算子
void test_equality_operator(void) {
    std::byte data1[] = {std::byte{0x01}, std::byte{0x02}};
    std::byte data2[] = {std::byte{0x01}, std::byte{0x02}};
    std::byte data3[] = {std::byte{0x01}, std::byte{0x03}};
    
    auto value1 = Value255::create(data1, 2);
    auto value2 = Value255::create(data2, 2);
    auto value3 = Value255::create(data3, 2);
    
    TEST_ASSERT_TRUE(value1.has_value() && value2.has_value() && value3.has_value());
    TEST_ASSERT_TRUE(*value1 == *value2);
    TEST_ASSERT_FALSE(*value1 == *value3);
}

// テスト8: 比較演算子（順序付け）
void test_comparison_operator(void) {
    std::byte data1[] = {std::byte{0x01}, std::byte{0x02}};
    std::byte data2[] = {std::byte{0x01}, std::byte{0x03}};
    std::byte data3[] = {std::byte{0x01}};
    
    auto value1 = Value255::create(data1, 2);
    auto value2 = Value255::create(data2, 2);
    auto value3 = Value255::create(data3, 1);
    
    TEST_ASSERT_TRUE(value1.has_value() && value2.has_value() && value3.has_value());
    
    // 等価演算子を使った比較テスト
    TEST_ASSERT_FALSE(*value1 == *value2);
    TEST_ASSERT_FALSE(*value1 == *value3);
    TEST_ASSERT_FALSE(*value2 == *value3);
}

// テスト9: ムーブ代入演算子
void test_move_assignment(void) {
    std::byte data[] = {std::byte{0xDE}, std::byte{0xAD}};
    auto value1 = Value255::create(data, 2);
    TEST_ASSERT_TRUE(value1.has_value());
    
    Value255 value2;
    value2 = std::move(*value1);
    
    auto retrieved = value2.bytes();
    TEST_ASSERT_EQUAL_UINT8(2, retrieved.size());
    TEST_ASSERT_EQUAL_UINT8(0xDE, static_cast<uint8_t>(retrieved[0]));
    TEST_ASSERT_EQUAL_UINT8(0xAD, static_cast<uint8_t>(retrieved[1]));
}

// テスト10: 文字列変換
void test_str_conversion(void) {
    std::byte data[] = {std::byte{0x48}, std::byte{0x65}, std::byte{0x6C}, std::byte{0x6C}, std::byte{0x6F}}; // "Hello"
    auto value = Value255::create(data, 5);
    TEST_ASSERT_TRUE(value.has_value());
    
    std::string str = value->str();
    TEST_ASSERT_EQUAL_STRING("[ 0x48 0x65 0x6C 0x6C 0x6F ]", str.c_str());
}

// メインのテスト実行関数
extern "C" void app_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_empty_value);
    RUN_TEST(test_create_inline_value);
    RUN_TEST(test_create_heap_value);
    RUN_TEST(test_create_max_size_value);
    RUN_TEST(test_create_invalid_parameters);
    RUN_TEST(test_clone_value);
    RUN_TEST(test_equality_operator);
    RUN_TEST(test_comparison_operator);
    RUN_TEST(test_move_assignment);
    RUN_TEST(test_str_conversion);
    
    UNITY_END();
}
