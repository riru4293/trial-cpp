#pragma once

/**
 * @file value255_transform.hpp
 * @brief Data transformation utilities for `Value255` class.
 *     @n （ja: `Value255` クラスのデータ変換ユーティリティ）
 *
 * @details
 * This header provides advanced data transformation utilities for `Value255` instances.
 * These utilities enable flexible operations such as format conversion, validation,
 * and custom data processing with automatic locking.
 * @n @n ja: @n
 * このヘッダーは、`Value255` インスタンスの高度なデータ変換ユーティリティを提供します。
 * これらのユーティリティにより、形式変換、検証、カスタムデータ処理などの柔軟な操作が
 * 自動ロックで可能になります。
 *
 * @par Usage
 * Include this header when you need data transformation functionality:
 * @code
 * #include <value255.hpp>            // Core classes
 * #include <value255_transform.hpp>  // Transformation utilities
 * @endcode
 * ja: @n
 * データ変換機能が必要な場合に、このヘッダーをインクルードしてください。
 * 使用例については、英文説明側を参照してください。
 */

#include "value255.hpp"

namespace value
{
    /** @brief Applies a transformation function to locked data.
     *      @n （ja: ロックされたデータに変換関数を適用します） */
    /**
     * @details
     * This advanced utility provides maximum flexibility for data transformation
     * operations. It locks the source `Value255`, passes its data to the provided
     * transformation function, and returns the result.
     *
     * This is useful for custom operations beyond simple cloning, such as:
     * - Converting data to a different format
     * - Creating derived values
     * - Performing validation or checksums
     * - Extracting specific fields from structured data
     * @n @n ja: @n
     * この高度なユーティリティは、データ変換操作の最大限の柔軟性を提供します。
     * ソース `Value255` をロックし、そのデータを提供された変換関数に渡し、
     * 結果を返します。
     *
     * これは単純なクローンを超えたカスタム操作に有用です。例えば:
     * - データを異なる形式に変換
     * - 派生値の作成
     * - 検証またはチェックサムの実行
     * - 構造化データから特定のフィールドを抽出
     *
     * @tparam Fn Callable type that takes `(std::byte const*, std::uint8_t)` and returns any type.
     *         @n （ja: `(std::byte const*, std::uint8_t)` を受け取り任意の型を返す呼び出し可能型）
     *
     * @param source [in] The `Value255` instance to read from.
     *                 @n （ja: 読み取り元の `Value255` インスタンス）
     * @param fn [in] Transformation function to apply to the locked data.
     *            @n （ja: ロックされたデータに適用する変換関数）
     *
     * @return The result of the transformation function.
     *      @n （ja: 変換関数の結果）
     *
     * @par Thread Safety
     * This function is thread-safe. It acquires the source instance's lock
     * during the transformation.
     * @n @n ja: @n
     * この関数はスレッドセーフです。変換中にソースインスタンスのロックを取得します。
     *
     * @par Examples
     * @code
     * Value255 original = ...;
     *
     * // Clone using static factory method
     * auto clone1 = withLockedTransform( original, MutableValue255::create );
     * auto clone2 = withLockedTransform( original, Value255::create );
     *
     * // Calculate checksum
     * auto checksum = withLockedTransform( original,
     *     []( std::byte const *data, std::uint8_t size ) -> uint32_t {
     *         uint32_t sum = 0;
     *         for ( uint8_t i = 0; i < size; i++ ) {
     *             sum += static_cast<uint32_t>( data[i] );
     *         }
     *         return sum;
     *     } );
     *
     * // Convert to hexadecimal string
     * auto hexString = withLockedTransform( original,
     *     []( std::byte const *data, std::uint8_t size ) -> std::string {
     *         std::string result;
     *         for ( uint8_t i = 0; i < size; i++ ) {
     *             char buf[4];
     *             snprintf( buf, sizeof( buf ), "%02X ", static_cast<unsigned>( data[i] ) );
     *             result += buf;
     *         }
     *         return result;
     *     } );
     *
     * // Validate data format
     * auto isValid = withLockedTransform( original,
     *     []( std::byte const *data, std::uint8_t size ) -> bool {
     *         if ( size < 2 ) return false;
     *         // Check magic bytes
     *         return data[0] == std::byte{0xCA} && data[1] == std::byte{0xFE};
     *     } );
     * @endcode
     */
    template<typename Fn>
    [[nodiscard]]
    inline auto withLockedTransform( Value255 const &source, Fn &&fn ) noexcept(
        noexcept( fn( std::declval<std::byte const *>(), std::declval<std::uint8_t>() ) ) )
        -> decltype( fn( std::declval<std::byte const *>(), std::declval<std::uint8_t>() ) )
    {
        using ResultType =
            decltype( fn( std::declval<std::byte const *>(), std::declval<std::uint8_t>() ) );

        // Use an immediately invoked lambda to capture and return the result
        return [&]() -> ResultType {
            ResultType result{};
            source.withLockedData(
                [&result, &fn]( std::byte const *data, std::uint8_t size ) {
                    result = fn( data, size );
                } );
            return result;
        }();
    }
} // namespace value
