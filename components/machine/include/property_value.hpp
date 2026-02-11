#pragma once

/**
 * @file property_value.hpp
 *
 * @brief Declaration of the `PropertyValue` and `MutablePropertyValue` classes.
 *     @n （ja: `PropertyValue` クラスと `MutablePropertyValue` クラスの宣言）
 *
 * @details
 * This header is declaration of the `PropertyValue` and `MutablePropertyValue` classes.
 * These classes represent byte array whose storage size is limited to 255 bytes.
 *
 * `PropertyValue` is immutable and `MutablePropertyValue` is mutable.
 * This separation is intentional to make variability explicit.
 * @n @n ja: @n
 * このヘッダーは、`PropertyValue` クラスと `MutablePropertyValue` クラスの宣言です。
 * これらのクラスは、記憶域サイズが255バイトに制限されたバイト配列を表します。
 *
 * `PropertyValue` は不変で `MutablePropertyValue` は可変です。
 * この分離は、可変性を明示的にするために意図的に行われています。
 */

/* C++ Standard Library */
#include <atomic>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace machine
{
    /** @brief Value type that represents an immutable byte array with a maximum size of 255 bytes.
     *      @n （ja: 最大 255 バイトの不変バイト配列を表す値型） */
    /**
     * @details
     * This class represents an immutable byte array with a maximum size of 255 bytes.
     * It provides construction, move semantics, comparison, and streaming capabilities.
     * Instances are movable but not copyable.
     *
     * Values of 4 bytes or less are stored inline (total 6 bytes),
     * while larger values are stored in heap-allocated memory.
     *
     * | Byte array size | Memory usage                                           |
     * |:----------------|:-------------------------------------------------------|
     * | 0 to 4 bytes    | 6 bytes (inline storage)                               |
     * | 5 to 255 bytes  | 6 bytes (instance) + 5 to 255 bytes (heap allocation)  |
     *
     * ja: @n
     * このクラスは最大 255 バイトの不変バイト列を表す値型であり、
     * 構築・移動・比較・ストリーミングを提供します。インスタンスは移動可能ですがコピー不可です。
     *
     * 4 バイト以下の値はインラインで保持し（総 6 バイト）、それ以上のサイズはヒープ領域を使用します。
     * サイズ上限およびメモリモデルの詳細は英文説明を参照してください。
     *
     * @par Thread Safety
     * This class is thread-safe, so **all public methods** perform mutual exclusion.
     * To minimize memory footprint and locking overhead,
     * mutual exclusion is achieved using a spinlock method using `atomic<bool>`.
     * The tradeoff is non-reentrancy: calling a public method from within
     * another public method will result in a deadlock.
     * @n @n ja: @n
     * このクラスはスレッドセーフです。そのため、 **全てのパブリックメソッド** は排他制御を行います。
     * メモリフットプリントとロックのオーバーヘッドを最小限に抑えるため、排他制御には
     * `atomic<bool>` を用いたスピンロック方式を採用しています。代償として **非再入可能** 制約があり、
     * パブリックメソッド内からパブリックメソッドを呼び出すと、デッドロックが発生します。
     */
    class PropertyValue
    {
    public:
        // ----- Nested types -----

        /** @brief Callback type for read-only access to byte array.
         *      @n （ja: 読み取り専用のバイト配列へのアクセスのためのコールバック型） */
        /**
         * @details
         * A callback type that receives a byte array and its size.
         * This type is used by `withLockedData()` to provide safe exclusive read-only access.
         * The callback should not attempt to modify the data and should return
         * promptly to avoid holding the lock for long periods of time.
         * @n @n ja: @n
         * バイト配列とそのサイズを受け取るコールバック型です。
         * この型は `withLockedData()` によって使用され、排他制御された安全な
         * 読み取り専用アクセスを提供します。コールバックは値の変更を試みてはならず、
         * ロックを長時間保持しないように迅速に戻る必要があります。
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         */
        using DataReader = std::function<void(
            std::byte const *data, std::uint8_t size )>;

        enum class SetResult : std::uint8_t; // Forward declaration.

        // ----- Factory methods -----

        /** @brief Creates a `PropertyValue` instance from byte array.
         *      @n （ja: バイト配列から `PropertyValue` インスタンスを作成します） */
        /**
         * @pre
         *   - If `size` > 0, `data` must not be `nullptr`.
         *  @n （ja: `size` が 0 より大きい場合、`data` は `nullptr` であってはならない）
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return A created instance if successful, otherwise it becomes `std::nullopt` because:
         *      @n （ja: 成功した場合は作成されたインスタンス、それ以外は次の理由により `std::nullopt`）
         *   - Illegal argument
         *  @n （ja: 引数不正）
         *   - Insufficient heap memory
         *  @n （ja: ヒープメモリ不足）
         */
        [[nodiscard]]
        static std::optional<PropertyValue> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        // ----- Constructors and destructor -----

        /** @brief Default constructor. */
        /**
         * @details
         * Constructs an empty `PropertyValue` with size 0 and no allocated memory.
         * @n @n ja: @n
         * サイズ 0 でメモリが割り当てられていない空の `PropertyValue` を構築します。
         */
        explicit PropertyValue() noexcept = default;

        /** @brief Destructor. */
        /**
         * @details
         * Frees the heap memory it uses.
         * Acquire an exclusive lock beforehand to ensure that
         * no other threads are currently accessing it.
         * @n @n ja: @n
         * 使用しているヒープ メモリを解放します。
         * 他のスレッドが現在アクセスしていないことを確実にするために、
         * 事前に排他ロックを取得します。
         */
        ~PropertyValue() noexcept;

        /** @brief Copy constructor (deleted). */
        PropertyValue( PropertyValue const & ) = delete;

        /** @brief Move constructor. */
        /**
         * @details
         * Constructs a new `PropertyValue` by transferring ownership of the byte array
         * from the other instance. After the move, the original `PropertyValue`
         * will be empty.
         * @n @n ja: @n
         * 他の `PropertyValue` からバイト配列の所有権を移動して新しいインスタンスを構築します。
         * 移動後、元の `PropertyValue` は空になります。
         *
         * @param other [in,out] the other `PropertyValue` to move from
         *                    @n （ja: 移動元の他の `PropertyValue`）
         */
        PropertyValue( PropertyValue &&other ) noexcept;

        // ----- Operators -----

        /** @brief Copy assignment operator (deleted). */
        PropertyValue &operator=( PropertyValue const & ) noexcept = delete;

        /** @brief Move assignment operator. */
        /**
         * @details
         * Transfers ownership of the byte array from the other `PropertyValue` to this instance.
         * Before the transfer, this instance releases its currently held resources.
         * After the move, the original `PropertyValue` will be empty.
         * If the source and destination are the same, it does nothing and returns `*this`.
         * @n @n ja: @n
         * 他の `PropertyValue` からこのインスタンスへバイト配列の所有権を移動します。
         * 移動に先立ち、このインスタンスが保持している既存リソースを解放します。
         * 移動後、元の `PropertyValue` は空になります。
         * 移動元と移動先が同一である場合は何もせず `*this` を返します。
         *
         * @param other [in,out] the other `PropertyValue` to move from
         *                    @n （ja: 移動元の他の `PropertyValue`）
         *
         * @return reference to this `PropertyValue` after the move
         *      @n （ja: 移動後の、この `PropertyValue` への参照）
         */
        PropertyValue &operator=( PropertyValue &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Determines whether this `PropertyValue` instance is equal to the other instance.
         * Equality is defined by the following conditions:
         * - Both instances have the same size.
         * - If the size is non-zero, the byte array must match exactly.
         *
         * ja: @n
         * この `PropertyValue` と他のインスタンスが等しいかどうかを判定します。
         * 等価性は以下の条件で定義されます:
         * - サイズが同じであること。
         * - サイズが 0 でない場合、バイト配列の内容が完全に一致していること。
         *
         * @param other [in] the `PropertyValue` instance to compare against
         *                @n （ja: 比較対象となる `PropertyValue` インスタンス）
         *
         * @return `true` if both instances are equal, `false` otherwise.
         *      @n （ja: 両インスタンスが等しい場合は `true` で、それ以外は `false`）
         */
        bool operator==( PropertyValue const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs a strong three-way comparison between this `PropertyValue` and the other instance.
         * Ordering is defined as follows:
         * - If the sizes differ, the instance with the smaller size is considered less.
         * - If the sizes are equal, the byte array are compared in lexicographical order.
         *
         * ja: @n
         * この `PropertyValue` と他のインスタンスを強い三方比較で判定します。
         * 順序は次の規則で定義されます:
         * - サイズが異なる場合、サイズが小さい方を小さいとみなします。
         * - サイズが同じ場合、バイト配列を辞書順で比較します。
         *
         * @param other [in] the `PropertyValue` instance to compare against
         *                @n （ja: 比較対象となる `PropertyValue` インスタンス）
         * @return the comparison result
         *      @n （ja: 比較結果）
         */
        std::strong_ordering operator<=>( PropertyValue const &other ) const noexcept;

        // ----- Public member methods -----

        /** @brief Provides thread-safe access to byte array via callback.
         *      @n （ja: コールバックを介してスレッドセーフなバイト配列へのアクセスを提供します） */
        /**
         * @details
         * Calls the provided callback with a byte array and its size.
         * The byte array and size will remain valid and unchanged during
         * the execution of the callback.
         * @n @n ja: @n
         * 提供されたコールバックをバイト配列とそのサイズで呼び出します。
         * コールバックの実行中、バイト配列とサイズは有効かつ不変であることが保証されます。
         *
         * @param callback [in] the `DataReader`
         *
         * @example
         * @code
         * myValue.withLockedData( [&driver]( std::byte const *data, std::uint8_t size ) {
         *     driver->write( data, size );
         * });
         * @endcode
         */
        void withLockedData( DataReader const &callback ) const noexcept;

        /** @brief Compares the value with external byte array.
         *      @n （ja: 外部のバイト配列との比較を行います） */
        /**
         * @details
         * Compares this instance's data with external data;
         * the caller must ensure that the external data is not modified during the comparison.
         * @n @n ja: @n
         * このインスタンスのバイト配列と外部のバイト配列を比較します。
         * 呼び出し元は、比較中に外部のバイト配列が変更されないことを保証する必要があります。
         *
         * @param data [in] external byte array for comparison
         *                  `nullptr` is only allowed if size is 0.
         *               @n （ja: 比較対象となる外部のバイト配列。
         *                        `nullptr`はサイズが 0 の場合のみ許可される）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return `true` if size match and byte array is identical, `false` otherwise.
         *      @n （ja: サイズとバイト配列が同一である場合は `true`、それ以外は `false`）
         */
        [[nodiscard]]
        bool areEquals( std::byte const *data, std::uint8_t size ) const noexcept;

        /** @brief Returns the number of bytes of the byte array.
         *      @n （ja: バイト配列のバイト数を返します） */
        /**
         * @return number of bytes of the byte array
         *      @n （ja: バイト配列のバイト数）
         */
        [[nodiscard]]
        std::uint8_t size() const noexcept;

        /** @brief Returns the byte array as a vector of bytes.
         *      @n （ja: バイト配列をバイトのベクターとして返します） */
        /**
         * @return the vector of bytes representing the byte array
         *      @n （ja: バイト配列を表すバイトのベクター）
         * @note The returned vector is a copy of the internal byte array.
         * @n @n ja: @n
         * 返されるベクターは内部バイト配列のコピーです。
         */
        [[nodiscard]]
        std::vector<std::byte> bytes() const noexcept;

        /** @brief Returns a string representation of the byte array.
         *      @n （ja: バイト配列の文字列表現を返します） */
        /**
         * @details
         * The string representation is formatted as a list of hexadecimal byte values.
         * For example, a value containing the bytes 0xA5, 0xE7, 0x00, 0xFF
         * would be represented as:
         * ```
         * [ 0xA5 0xE7 0x00 0xFF ]
         * ```
         * ja: @n
         * バイト配列の文字列表現は、16進数のバイト値のリストとしてフォーマットされます。
         * 表現例については、値が 0xA5、0xE7、0x00、0xFF の場合の例を
         * 英文説明側に記載していますので、そちらを参照してください。
         *
         * @return String representation of the byte array.
         *      @n （ja: バイト配列の文字列表現）
         */
        [[nodiscard]]
        std::string str() const noexcept;

    protected:
        // ----- Nested types -----

        /** @brief Scoped spinlock for thread safety.
         *      @n （ja: スレッドセーフのためのスコープ付きスピンロック） */
        /**
         * @details
         * A helper class that acquires a spinlock on construction
         * and releases it on destruction.
         * @n @n ja: @n
         * コンストラクション時にスピンロックを取得し、
         * デストラクション時に解放するヘルパークラスです。
         *
         * @note This class is intended for internal use only.
         * @n @n ja: @n
         * このクラスは内部使用のみを目的としています。
         */
        class ScopedSpinLock; // Forward declaration.

        // ----- Protected member methods -----

        /** @brief Set the new byte array.
         *      @n （ja: 新しいバイト配列を設定します） */
        /**
         * @details
         * This is a simplified wrapper that calls `setWithResult()` internally
         * and converts the detailed result to a boolean for convenience.
         * It converts `SetResult::Success` and `SetResult::NoChange` to `true`,
         * and others to `false`.
         * @n @n ja: @n
         * これは簡略化されたラッパーで、内部的に `setWithResult()` を呼び出し、
         * 結果をブール値に変換して利便性を提供します。
         * `SetResult::Success` および `SetResult::NoChange` の場合に
         * `true` それ以外の場合に `false` へ変換します。
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return `true` if successful (either `Success` or `NoChange`),
         *         `false` otherwise.
         */
        [[nodiscard]]
        bool set( std::byte const *data, std::uint8_t size ) noexcept;

        /** @brief Set the new byte array with detailed result.
         *      @n （ja: バイト配列を詳細な結果付きで設定します） */
        /**
         * @details
         * This is the primary implementation for setting data. It provides detailed
         * result information via the `SetResult` enum, distinguishing between
         * different failure scenarios.
         * @n @n ja: @n
         * これはバイト配列設定の主要な実装です。
         * `SetResult` 列挙型を介して詳細な結果情報を提供し、異なる失敗シナリオを区別します。
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return
         * - `SetResult::Success`         - Byte array successfully updated
         *                               @n （ja: バイト配列が正常に更新された）
         * - `SetResult::NoChange`        - New byte array is identical to current byte array
         *                               @n （ja: 新しいバイト配列が現在のバイト配列と同一である）
         * - `SetResult::IllegalArgument` - `data` is null while `size > 0`
         *                               @n （ja: `size > 0` の場合に `data` が `nullptr` である）
         * - `SetResult::OutOfMemory`     - Heap allocation failed
         *                               @n （ja: ヒープメモリの割り当てに失敗した）
         */
        [[nodiscard]]
        SetResult setWithResult( std::byte const *data, std::uint8_t size ) noexcept;

    private:
        // ----- Static constants -----

        static constexpr std::uint8_t INLINE_SIZE = 4;

        // ----- Member methods -----

        void lock() const noexcept;
        void unlock() const noexcept;
        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }
        void cleanup() noexcept;
        std::uintptr_t heapPointerAsUint() const noexcept;
        std::byte *heapPointerAsByte() const noexcept;
        void *heapPointerAsVoid() const noexcept;
        void moveFrom( PropertyValue &&other ) noexcept;
        bool areEqualsUnlocked( std::byte const *data, std::uint8_t size ) const noexcept;
        std::byte const *dataUnlocked() const noexcept;

        // ----- Member variables -----

        std::atomic<bool> mutable lock_ = false;    //!< Spinlock for thread safety.
        std::uint8_t size_ = 0;                     //!< Size of the value in bytes.
        std::byte raw_data_[INLINE_SIZE] = {};      //!< Inline storage or heap pointer.

    }; // class PropertyValue

    /** @brief Stream output operator for `PropertyValue`. */
    /**
     * @details
     * Outputs the string representation of the `PropertyValue` instance to the provided
     * output stream.
     *
     * @see PropertyValue::str() for the format of the output.
     *
     * @param os [out] The output stream to write to.
     * @param v  [in]  The `PropertyValue` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, PropertyValue const &v ) noexcept;

    /** @brief Mutable counterpart of `Value255`. */
    /**
    * @details
    * This class provides the only mechanism to modify the contents of a
    * `Value255` instance. While `Value255` behaves as an immutable value type
    * for external users, `MutableValue255` exposes a controlled mutation API
    * through the `set()` method.
    *
    * Internally, this class does not introduce additional state; it simply
    * inherits the storage and locking behavior of `Value255`. All thread-safety
    * guarantees, locking rules, and non-reentrancy constraints of `Value255`
    * apply equally to `MutableValue255`.
    *
    * @note
    * - Mutation is performed in-place and is protected by the same per-instance
    *   spinlock used by `Value255`.
    * - After mutation, the instance remains a valid `Value255` and can be used
    *   wherever an immutable value is expected.
    *
    * @attention
    * - As with `Value255`, this class is **not reentrant**. Calling a public
    *   method from within another public method will result in deadlock.
    * - Avoid long-running operations inside `set()`, as the lock is held for
    *   the entire duration of the mutation.
    */
    class MutableValue255 : public Value255
    {
    public:
        // ----- Static methods -----

        /** @brief Creates a `MutableValue255` instance from byte array.
         *      @n （ja: バイト配列から `MutableValue255` インスタンスを作成します） */
        /**
         * @pre
         *   - If `size` > 0, `data` must not be `nullptr`.
         *  @n （ja: `size` が 0 より大きい場合、`data` は `nullptr` であってはならない）
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return A created instance if successful, otherwise it becomes `std::nullopt` because:
         *      @n （ja: 成功した場合は作成されたインスタンス、それ以外は次の理由により `std::nullopt`）
         *   - Illegal argument
         *  @n （ja: 引数不正）
         *   - Insufficient heap memory
         *  @n （ja: ヒープメモリ不足）
         */
        static std::optional<MutableValue255> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        // ----- Constructors -----

        using Value255::Value255;

        // ----- Public member methods -----

        /** @brief Set the new byte array.
         *      @n （ja: 新しいバイト配列を設定します） */
        /**
         * @details
         * This is a simplified wrapper that calls `setWithResult()` internally
         * and converts the detailed result to a boolean for convenience.
         * It converts `SetResult::Success` and `SetResult::NoChange` to `true`,
         * and others to `false`.
         * @n @n ja: @n
         * これは簡略化されたラッパーで、内部的に `setWithResult()` を呼び出し、
         * 結果をブール値に変換して利便性を提供します。
         * `SetResult::Success` および `SetResult::NoChange` の場合に
         * `true` それ以外の場合に `false` へ変換します。
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return `true` if successful (either `Success` or `NoChange`),
         *         `false` otherwise.
         */
        [[nodiscard]]
        bool set( std::byte const *data, std::uint8_t size ) noexcept;

        /** @brief Set the new byte array with detailed result.
         *      @n （ja: バイト配列を詳細な結果付きで設定します） */
        /**
         * @details
         * This is the primary implementation for setting data. It provides detailed
         * result information via the `SetResult` enum, distinguishing between
         * different failure scenarios.
         * @n @n ja: @n
         * これはバイト配列設定の主要な実装です。
         * `SetResult` 列挙型を介して詳細な結果情報を提供し、異なる失敗シナリオを区別します。
         *
         * @param data [in] byte array
         *               @n （ja: バイト配列）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return
         * - `SetResult::Success`         - Byte array successfully updated
         *                               @n （ja: バイト配列が正常に更新された）
         * - `SetResult::NoChange`        - New byte array is identical to current byte array
         *                               @n （ja: 新しいバイト配列が現在のバイト配列と同一である）
         * - `SetResult::IllegalArgument` - `data` is null while `size > 0`
         *                               @n （ja: `size > 0` の場合に `data` が `nullptr` である）
         * - `SetResult::OutOfMemory`     - Heap allocation failed
         *                               @n （ja: ヒープメモリの割り当てに失敗した）
         */
        [[nodiscard]]
        SetResult setWithResult( std::byte const *data, std::uint8_t size ) noexcept;

    }; // class MutableValue255

    /** @brief Result codes for the `setWithResult` operation.
     *      @n （ja: `setWithResult` 操作の結果コード）          */
    /**
     * @details
     * This enum class defines the outcomes of the `setWithResult` operation
     * in the `Value255` and its derived classes.
     * @n @n ja: @n
     * この列挙型クラスは `Value255` クラスと、その派生クラスにおける
     * `setWithResult` 操作の結果を定義します。
     */
    enum class Value255::SetResult : std::uint8_t
    {
        Success = 0U,         //!< Data successfully updated.
        NoChange = 1U,        //!< New data is identical to current data.
        IllegalArgument = 2U, //!< Invalid arguments provided.
        OutOfMemory = 3U,     //!< Memory allocation failed.
    };

    /* Note:
        このクラスはポインタのサイズが 4 バイトであることを前提としているため、
        それを静的アサーションで確認しています。
    */
    static_assert(  sizeof( std::uintptr_t ) == 4U );

    static_assert(  sizeof(Value255) == 6U );
    static_assert( alignof(Value255) == 1U );
    static_assert( std::is_standard_layout<Value255>::value );

    static_assert(  sizeof(MutableValue255) == 6U );
    static_assert( alignof(MutableValue255) == 1U );
    static_assert( std::is_standard_layout<MutableValue255>::value );

} // namespace value
