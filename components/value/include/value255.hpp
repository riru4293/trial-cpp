#pragma once

/**
 * @file value255.hpp
 * @brief Opaque value types with a maximum size of 255 bytes.
 *     @n ja: （最大サイズが 255 バイトの不透明な値型）
 *
 * @details
 * This header declares the `Value255` and `MutableValue255` classes.
 * These types represent opaque binary values whose storage size is
 * limited to 255 bytes.
 *
 * `Value255` is immutable and `MutableValue255` is mutable.
 * The separation is intentional to make mutability explicit at the type level.
 * @n @n ja: @n
 * このヘッダーは、`Value255` クラスと `MutableValue255` クラスを宣言します。
 * これらの型は、記憶域サイズが255バイトに制限された不透明なバイナリ値を表します。
 * `Value255` は不変で、 `MutableValue255` は変更可能です。
 * この分離は、型レベルで変更可能性を明示的にするために意図的に行われています。
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

namespace value
{
    /** @brief Represents an immutable opaque value with dynamic storage up to 255 bytes.
     *      @n （ja: 最大 255 バイトの動的ストレージを持つ不変で不透明な値を表す） */
    /**
     * @details
     * This class acts as an immutable value type, managing opaque values ​​up to 255 bytes in size.
     * It provides mechanisms for construction, moving, comparison, and streaming.
     * Instances are movable but not copyable. If the value is larger than 4 bytes,
     * heap memory is allocated to store the value.
     * @n @n ja: @n
     * このクラスは不変の値型として機能し、最大 255 バイトの不透明な値を管理します。
     * また、構築/移動/比較/ストリーミングのメカニズムを提供します。
     * インスタンスは移動可能ですが、コピーはできません。
     * 値が4バイトより大きなサイズの場合は、ヒープメモリを確保して値を格納します。
     *
     * @par Thread Safety
     * This class is thread-safe, so **all public methods** perform mutual exclusion.
     * To minimize memory footprint and locking overhead,
     * mutual exclusion is achieved using a spinlock method using `atomic<bool>`.
     * The tradeoff is non-reentrancy: calling a public method from within
     * another public method will result in a deadlock.
     * @n @n ja: @n
     * このクラスはスレッドセーフです。そのため、 **全てのパブリックメソッド** は排他制御を行います。
     * メモリフットプリントとロックのオーバーヘッドを最小限に抑えるため、
     * 排他制御には `atomic<bool>` を用いたスピンロック方式を採用しています。
     * 代償として **非再入可能** 制約があり、パブリックメソッド内からパブリックメソッドを
     * 呼び出すと、デッドロックが発生します。
     */
    class Value255
    {
    public:
        // ----- Nested types -----

        /** @brief Callback type for read-only access to raw value.
         *      @n （ja: 読み取り専用の生の値へのアクセスのためのコールバック型） */
        /**
         * @details
         * A callback type that receives a pointer to a raw byte array and its size.
         * This type is used by `withLockedData()` to provide safe exclusive read-only access.
         * The callback should not attempt to modify the data and should return
         * promptly to avoid holding the lock for long periods of time.
         * @n @n ja: @n
         * 生の値の先頭とそのサイズを受け取るコールバック型です。
         * この型は `withLockedData()` によって使用され、排他制御された安全な
         * 読み取り専用アクセスを提供します。コールバックは値の変更を試みてはならず、
         * ロックを長時間保持しないように迅速に戻る必要があります。
         *
         * @param data [in] pointer to the raw value
         *               @n （ja: 生の値の先頭）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         */
        using DataReader = std::function<void( std::byte const *data, std::uint8_t size )>;

        /** @brief Result codes for the `setWithResult` operation.
         *      @n （ja: `setWithResult` 操作の結果コード）          */
        /**
        * @details
        * This enum class defines the outcomes of the `setWithResult` operation
        * in the `Value255` and `MutableValue255` classes.
         * @n @n ja: @n
        * この列挙型クラスは、`Value255` クラスと `MutableValue255` クラスの
        * `setWithResult` 操作の結果を定義します。
        */
        enum class SetResult : std::uint8_t; // Forward declaration.

        // ----- Factory methods -----

        /** @brief Creates a `Value255` instance from raw value.
         *      @n （ja: 生の値から `Value255` インスタンスを作成します） */
        /**
         * @details
         * Allocates heap memory as needed and copies the provided value into the new instance.
         * If the size is 4 bytes or less, inline storage is used and no heap memory is allocated.
         * @n @n ja: @n
         * 必要に応じてヒープメモリを確保し、提供された値を新しいインスタンスにコピーします。
         * サイズが4バイト以下の場合はインラインストレージを使用し、ヒープメモリは使用しません。
         *
         * @pre
         * - If `size` > 0, `data` must not be `nullptr`.
         *   @n （ja: `size` が 0 より大きい場合、`data` は `nullptr` であってはならない）
         *
         * @param data [in] pointer to the raw value
         *               @n （ja: 生の値の先頭）
         * @param size [in] number of bytes in `data`
         *               @n （ja: 引数 `data` のバイト数）
         *
         * @return A created value if successful, otherwise it becomes `std::nullopt` because:
         *      @n （ja: 成功した場合は作成された値、それ以外は次の理由により `std::nullopt`）
         * - Illegal argument
         *   @n (ja: 引数不正)
         * - Insufficient heap memory to store a copy of `data`
         *   @n (ja: ヒープメモリ不足により `data` のコピーを確保できない)
         */
        [[nodiscard]]
        static std::optional<Value255> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        // ----- Constructors and destructor -----

        /** @brief Default constructor. */
        /**
         * @details
         * Constructs an empty `Value255` with size 0 and no allocated memory.
         * @n @n ja: @n
         * サイズ 0 でメモリが割り当てられていない空の `Value255` を構築します。
         */
        explicit Value255() noexcept = default;

        /** @brief Destructor. */
        /**
         * @details
         * Cleans up any heap-allocated memory when the `Value255` instance is destroyed.
         * An exclusive lock is acquired before cleanup to ensure that no other
         * threads are currently accessing it.
         * @n @n ja: @n
         * `Value255` インスタンスが破棄されるときに、ヒープに割り当てられた
         * メモリをクリーンアップします。他のスレッドが現在アクセスしていないことを
         * 確実にするために、クリーンアップ前に排他ロックを取得します。
         */
        ~Value255() noexcept;

        /** @brief Copy constructor (deleted). */
        Value255( Value255 const & ) = delete;

        /** @brief Move constructor. */
        /**
         * @details
         * Constructs a new `Value255` by transferring ownership of the value
         * from the other instance. No existing resources need to be released
         * because this object is being created.
         * After the move, the original `Value255` will be left in an empty state.
         * @n @n ja: @n
         * 他の `Value255` から値の所有権を移動して新しいインスタンスを構築します。
         * このオブジェクトは構築中のため、既存リソースの解放は不要です。
         * 移動後、元の `Value255` は空の状態になります。
         *
         * @param other [in,out] the other `Value255` to move from
         *                    @n （ja: 移動元の他の `Value255`）
         */
        Value255( Value255 &&other ) noexcept;

        // ----- Operators -----

        /** @brief Copy assignment operator (deleted). */
        Value255 &operator=( Value255 const & ) noexcept = delete;

        /** @brief Move assignment operator. */
        /**
         * @details
         * Transfers ownership of the value from the other `Value255` to this instance.
         * Before the transfer, this instance releases its currently held resources.
         * If this and the other instance are the same, no action is taken and `*this` is returned.
         * After the move, the original `Value255` will be left in an empty state.
         * @n @n ja: @n
         * 他の `Value255` からこのインスタンスへ値の所有権を移動します。
         * 移動に先立ち、このインスタンスが保持している既存リソースを解放します。
         * 他のインスタンスと同一である場合は何もせず `*this` を返します。
         * 移動後、元の `Value255` は空の状態になります。
         *
         * @param other [in,out] the other `Value255` to move from
         *                    @n （ja: 移動元の他の `Value255`）
         *
         * @return reference to this `Value255` after the move
         *      @n （ja: 移動後の、この `Value255` への参照）
         */
        Value255 &operator=( Value255 &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Determines whether this `Value255` instance is equal to the other instance.
         * Equality is defined by the following conditions:
         * - Both instances have the same size.
         * - If the size is non-zero, value must match exactly.
         *
         * ja: @n
         * この `Value255` と他のインスタンスが等しいかどうかを判定します。
         * 等価性は以下の条件で定義されます:
         * - サイズが同じであること。
         * - サイズが 0 でない場合、値の内容が完全に一致していること。
         *
         * @param other [in] the `Value255` instance to compare against
         *                @n （ja: 比較対象となる `Value255` インスタンス）
         *
         * @return `true` if both instances are equal, `false` otherwise.
         *      @n （ja: 両インスタンスが等しい場合は `true` で、それ以外は `false`）
         */
        bool operator==( Value255 const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs a strong three-way comparison between this `Value255` and the other instance.
         * Ordering is defined as follows:
         * - If the sizes differ, the instance with the smaller size is considered less.
         * - If the sizes are equal, the value are compared in lexicographical order.
         *
         * ja: @n
         * この `Value255` と他のインスタンスを強い三方比較で判定します。
         * 順序は次の規則で定義されます:
         * - サイズが異なる場合、サイズが小さい方を小さいとみなします。
         * - サイズが同じ場合、値を辞書順で比較します。
         *
         * @param other [in] the `Value255` instance to compare against
         *                @n （ja: 比較対象となる `Value255` インスタンス）
         * @return the comparison result
         *      @n （ja: 比較結果）
         */
        std::strong_ordering operator<=>( Value255 const &other ) const noexcept;

        // ----- Public member methods -----

        /** @brief Provides thread-safe access to raw value via callback.
         *      @n （ja: コールバックを介してスレッドセーフな生の値へのアクセスを提供します） */
        /**
         * @details
         * Calls the provided callback with a pointer to the raw value and its size.
         * The value pointer and size will remain valid and unchanged during
         * the execution of the callback.
         * @n @n ja: @n
         * 提供されたコールバックを生の値の先頭とそのサイズで呼び出します。
         * コールバックの実行中、生の値とサイズは有効かつ変更されません。
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

        /** @brief Compares the value with external value.
         *      @n （ja: 外部の値との比較を行います） */
        /**
         * @details
         * Compares this instance's data with external data;
         * the caller must ensure that the external data is not modified during the comparison.
         * @n @n ja: @n
         * このインスタンスの値と外部の値を比較します。
         * 呼び出し元は、比較中に外部の値が変更されないことを保証する必要があります。
         *
         * @param data [in] pointer to external value for comparison
         *                  `nullptr` is only allowed if size is 0.
         *               @n （ja: 比較対象となる外部の値の先頭。
         *                        `nullptr`はサイズが 0 の場合のみ許可される）
         * @param size [in] number of bytes of external value
         *               @n （ja: 外部の値のバイト数）
         *
         * @return `true` if size match and value is identical, `false` otherwise.
         *      @n （ja: サイズと値が同一である場合は `true`、それ以外は `false`）
         */
        [[nodiscard]]
        bool areEquals( std::byte const *data, std::uint8_t size ) const noexcept;

        /** @brief Returns the number of bytes of the value.
         *      @n （ja: 値のバイト数を返します） */
        /**
         * @return number of bytes of the value
         *      @n （ja: 値のバイト数）
         */
        [[nodiscard]]
        std::uint8_t size() const noexcept;

        /** @brief Returns the value as a vector of bytes.
         *      @n （ja: 値をバイトのベクターとして返します） */
        /**
         * @return the vector of bytes representing the value
         *      @n （ja: 値を表すバイトのベクター）
         */
        [[nodiscard]]
        std::vector<std::byte> bytes() const noexcept;

        /** @brief Returns a string representation of the value.
         *      @n （ja: 値の文字列表現を返します） */
        /**
         * @details
         * The string representation is formatted as a list of hexadecimal byte values.
         * For example, a value containing the bytes 0xA5, 0xE7, 0x00, 0xFF
         * would be represented as:
         * ```
         * [ 0xA5 0xE7 0x00 0xFF ]
         * ```
         * ja: @n
         * 値の文字列表現は、16進数のバイト値のリストとしてフォーマットされます。
         * 表現例については、値が 0xA5、0xE7、0x00、0xFF の場合の例を
         * 英文説明側に記載していますので、そちらを参照してください。
         *
         * @return String representation of the value.
         *      @n （ja: 値の文字列表現）
         */
        [[nodiscard]]
        std::string str() const noexcept;

        /** @brief Create clone from this instance.
         *      @n （ja: このインスタンスからクローンを作成します） */
        /**
        * @details
        * Creates a new `Value255` instance by copying the contents of this
        * instance.
        * @n @n ja: @n
        * このインスタンスの内容をコピーして新しい `Value255` インスタンスを作成します。
        *
        * @return An optional containing the cloned `Value255` if successful;
        *         `std::nullopt` otherwise.
        *      @n （ja: 成功した場合はクローンした値、それ以外は次の理由により `std::nullopt`）
        * - Insufficient heap memory to create a clone
        *   @n (ja: ヒープメモリ不足によりクローンを作成できない)
        */
        [[nodiscard]]
        std::optional<Value255> clone( void ) const noexcept;

    protected:
        // ----- Nested types -----

        class ScopedSpinLock; // Forward declaration.

        // ----- Protected member methods -----

        /** @brief Sets the value and it size.
         *      @n （ja: 値とそのサイズを設定します） */
        /**
         * @details
         * This is a simplified wrapper that calls `setWithResult()` internally
         * and converts the detailed result to a boolean for convenience.
         *
         * Failure cases:
         * - `data` is null while `size` is greater than 0 → returns `false`
         * - Memory allocation fails → returns `false`
         *
         * @param data [in] Pointer to the new raw data.
         * @param size [in] Size of the new data in bytes.
         *
         * @return `true` if successful (either `Success` or `NoChange`),
         *         `false` otherwise.
         *
         * @note
         * Use this method for simple boolean result checking.
         * Use `setWithResult()` if you need to distinguish between different
         * failure cases.
         *
         * @par Internal Note
         * This method must be called with the instance locked.
         * The caller is responsible for lock acquisition.
         */
        [[nodiscard]]
        bool set( std::byte const *data, std::uint8_t size ) noexcept;

        /** @brief Sets the value's data and size with detailed result (protected). */
        /**
         * @details
         * This is the primary implementation for setting data. It provides detailed
         * result information via the `SetResult` enum, distinguishing between
         * different failure scenarios.
         *
         * @param data [in] Pointer to the new raw data.
         * @param size [in] Size of the new data in bytes.
         *
         * @return
         * - `SetResult::Success` - Data successfully updated
         * - `SetResult::NoChange` - New data is identical to current data
         * - `SetResult::IllegalArgument` - `data` is null while `size > 0`
         * - `SetResult::OutOfMemory` - Heap allocation failed
         *
         * @par Internal Note
         * This method must be called with the instance locked.
         * The caller is responsible for lock acquisition (see `makeGuard`).
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
        void moveFrom( Value255 &&other ) noexcept;
        bool areEqualsUnlocked( std::byte const *data, std::uint8_t size ) const noexcept;
        std::byte const *dataUnlocked() const noexcept;

        // ----- Member variables -----

        std::atomic<bool> mutable lock_ = false;    //!< Spinlock for thread safety.
        std::uint8_t size_ = 0;                     //!< Size of the value in bytes.
        std::byte raw_data_[INLINE_SIZE] = {};      //!< Inline storage or heap pointer.

    }; // class Value255

    /** @brief Stream output operator for `Value255`. */
    /**
     * @details
     * Outputs the string representation of the `Value255` instance to the provided
     * output stream.
     *
     * @see Value255::str() for the format of the output.
     *
     * @param os [out] The output stream to write to.
     * @param v  [in]  The `Value255` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Value255 const &v ) noexcept;

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

        /** @brief Creates a `MutableValue255` instance from raw data. */
        /**
        * @details
        * Allocates memory as needed and copies the provided data into the new
        * instance.
        *
        * @param data [in] Pointer to the raw data. A null pointer is only valid if size is 0.
        * @param size [in] Size of the data in bytes.
        *
        * @return An optional containing the created `MutableValue255` if successful;
        * `std::nullopt` otherwise.
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
        */
        static std::optional<MutableValue255> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        // ----- Constructors -----

        using Value255::Value255;

        // ----- Public member methods -----

        /** @brief Sets the value's data and size. */
        /**
        * @details
        * This method updates the contents of the `MutableValue255` instance
        * with the provided data and size.
        *
        * If the operation fails, this instance keeps its previous data unchanged.
        *
        * @par Thread Safety
        * This method is thread-safe and acquires the instance's spinlock
        * for the duration of the operation.
        *
        * @param data [in] Pointer to the new raw data.
        *                  A null pointer is only valid if size is 0.
        * @param size [in] Size of the new data in bytes.
        *
        * @return `true` if the operation was successful; `false` otherwise.
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
       */
        [[nodiscard]]
        bool set( std::byte const *data, std::uint8_t size ) noexcept;

        /** @brief Sets the value's data and size. */
        /**
        * @details
        * This method updates the contents of the `MutableValue255` instance
        * with the provided data and size.
        *
        * If the operation fails (illegal argument or out-of-memory), this
        * instance keeps its previous data unchanged.
        *
        * @par Thread Safety
        * This method is thread-safe and acquires the instance's spinlock
        * for the duration of the operation.
        *
        * @param data [in] Pointer to the new raw data.
        *                  A null pointer is only valid if size is 0.
        * @param size [in] Size of the new data in bytes.
        *
        * @return the `SetResult`
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
       */
        [[nodiscard]]
        SetResult setWithResult( std::byte const *data, std::uint8_t size ) noexcept;

    }; // class MutableValue255

    /** @brief Result codes for the `setWithResult` operation. */
    /** @details
    * This enum class defines the possible outcomes of the `setWithResult` operation
    * in the `Value255` and `MutableValue255` classes.
    */
    enum class Value255::SetResult : std::uint8_t
    {
        Success = 0U,         //!< Data successfully updated.
        NoChange = 1U,        //!< New data is identical to current data.
        IllegalArgument = 2U, //!< Invalid arguments provided.
        OutOfMemory = 3U,     //!< Memory allocation failed.
    };

    static_assert(  sizeof( std::uintptr_t ) == 4U );

    static_assert(  sizeof(Value255) == 6U );
    static_assert( alignof(Value255) == 1U );
    static_assert( std::is_standard_layout<Value255>::value );

    static_assert(  sizeof(MutableValue255) == 6U );
    static_assert( alignof(MutableValue255) == 1U );
    static_assert( std::is_standard_layout<MutableValue255>::value );

} // namespace value
