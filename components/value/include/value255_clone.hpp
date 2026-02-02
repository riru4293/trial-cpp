#pragma once

/**
 * @file value255_clone.hpp
 * @brief Cloning utilities for `Value255` and `MutableValue255` classes.
 *     @n （ja: `Value255` と `MutableValue255` クラスのクローン機能）
 *
 * @details
 * This header provides convenient cloning utilities for `Value255` instances.
 * These utilities allow creating clones as either immutable `Value255` or
 * mutable `MutableValue255` in a single, concise expression.
 * @n @n ja: @n
 * このヘッダーは、`Value255` インスタンスの便利なクローン機能を提供します。
 * これらのユーティリティにより、不変の `Value255` または可変の `MutableValue255`
 * として、1つの簡潔な式でクローンを作成できます。
 *
 * @par Usage
 * Include this header when you need cloning functionality:
 * @code
 * #include <value255.hpp>        // Core classes
 * #include <value255_clone.hpp>  // Cloning utilities
 * @endcode
 * ja: @n
 * クローン機能が必要な場合に、このヘッダーをインクルードしてください。
 * 使用例については、英文説明側を参照してください。
 */

#include "value255.hpp"

namespace value
{
    /** @brief Creates a clone of a `Value255` instance as a specified type.
     *      @n （ja: `Value255` インスタンスを指定された型としてクローンします） */
    /**
     * @details
     * This function provides a convenient way to clone a `Value255` instance
     * as either `Value255` or `MutableValue255` in a single expression.
     * It internally uses `withLockedData()` to safely access the source data
     * and the target type's `create()` factory method.
     * @n @n ja: @n
     * この関数は、`Value255` インスタンスを `Value255` または `MutableValue255`
     * として1つの式でクローンする便利な方法を提供します。
     * 内部的に `withLockedData()` を使用してソースデータに安全にアクセスし、
     * ターゲット型の `create()` ファクトリメソッドを使用します。
     *
     * @tparam T Target type for the clone. Must be either `Value255` or `MutableValue255`.
     *           Defaults to `MutableValue255`.
     *        @n （ja: クローンのターゲット型。`Value255` または `MutableValue255` である必要があります。
     *                 デフォルトは `MutableValue255` です）
     *
     * @param source [in] The `Value255` instance to clone from.
     *                 @n （ja: クローン元の `Value255` インスタンス）
     *
     * @return An optional containing the cloned instance if successful; `std::nullopt` otherwise.
     *      @n （ja: 成功した場合はクローンされたインスタンス、それ以外は `std::nullopt`）
     *
     * @par Failure Cases
     * Returns `std::nullopt` when:
     * - Heap memory allocation fails for the cloned instance
     * @n @n ja: @n
     * 次の場合に `std::nullopt` を返します:
     * - クローンされたインスタンスのヒープメモリ割り当てに失敗した場合
     *
     * @par Thread Safety
     * This function is thread-safe. It acquires the source instance's lock
     * during the cloning operation.
     * @n @n ja: @n
     * この関数はスレッドセーフです。クローン操作中にソースインスタンスのロックを取得します。
     *
     * @par Examples
     * @code
     * Value255 original = ...;
     *
     * // Clone as MutableValue255 (default)
     * auto mutableClone = cloneAs( original );
     * if ( mutableClone ) {
     *     mutableClone->set( newData, newSize );
     * }
     *
     * // Clone as Value255 (immutable)
     * auto immutableClone = cloneAs<Value255>( original );
     *
     * // Clone as MutableValue255 (explicit)
     * auto explicitClone = cloneAs<MutableValue255>( original );
     * @endcode
     */
    template<typename T = MutableValue255>
    [[nodiscard]]
    inline std::optional<T> cloneAs( Value255 const &source ) noexcept
    {
        std::optional<T> result;
        source.withLockedData( [&result]( std::byte const *data, std::uint8_t size ) {
            result = T::create( data, size );
        } );
        return result;
    }
} // namespace value
