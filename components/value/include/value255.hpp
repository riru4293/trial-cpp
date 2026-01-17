#pragma once

/* C++ Standard Library */
#include <atomic>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/* Utilities */
#include <spin_guard.hpp>

namespace value
{
    /** @brief Represents an opaque value with dynamic storage up to 255 bytes. */
    /**
    * @details
    * This class manages an opaque value that may be stored either inline
    * (4 bytes) or on the heap (for larger sizes). It provides mechanisms for
    * constructing, moving, comparing, and streaming member values.
    * Instances are movable but not copyable.
    *
    * For external users, this class behaves as an immutable value type.
    * Mutation is only permitted through the derived `MutableValue255`.
    *
    * Critical sections are intentionally kept short; a spinlock is chosen
    * to minimize memory footprint and locking overhead.
    *
    * The class supports equality comparison, ordering comparison,
    * and stream output via `operator<<`.
    *
    * @par Thread Safety
    * All **public methods** of this class acquire an `atomic<bool>`-based
    * spinlock to ensure thread safety.
    * Locking is performed per instance and held for the entire duration
    * of each public method.
    *
    * @note
    * Private/internal methods such as `set()` and `cleanup()` assume that
    * the caller has already acquired the lock. They must not be invoked
    * directly from outside the class.
    *
    * @attention
    * - This class is **not reentrant**. Calling a public method from within
    *   another public method will result in deadlock.
    * - Avoid long-running operations inside public methods, as they hold
    *   the lock for their entire execution.
    * - Locking granularity is coarse (per instance), limiting concurrency
    *   to a single thread at a time.
    */
    class Value255
    {
    // ========== Supporting Section ==========
    // (Helper/factory functions that require forward implements)

    protected:
        // ----- Guard factories -----

        static auto makeGuard( Value255 const &v ) noexcept
        {
            auto lock = [&v]() noexcept { v.lock(); };
            auto unlock = [&v]() noexcept { v.unlock(); };
            return util::SpinGuard( lock, unlock, lock, unlock, true );
        }

        static auto makeGuard( Value255 const &a, Value255 const &b ) noexcept
        {
            return util::SpinGuard(
                [&a]() noexcept { a.lock(); }, [&a]() noexcept { a.unlock(); },
                [&b]() noexcept { b.lock(); }, [&b]() noexcept { b.unlock(); },
                ( &a == &b )
            );
        }

    // ========== Main Implementation ==========

    public:
        // ----- Nested types -----

        enum class SetResult : std::uint8_t; // Forward declaration.

        // ----- Factory methods -----

        /** @brief Creates a `Value255` instance from raw data. */
        /**
        * @details
        * Allocates memory as needed and copies the provided data into the new
        * instance.
        *
        * @param data [in] Pointer to the raw data. A null pointer is only valid if size is 0.
        * @param size [in] Size of the data in bytes.
        *
        * @return An optional containing the created `Value255` if successful;
        * `std::nullopt` otherwise.
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
        */
        [[nodiscard]]
        static std::optional<Value255> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        // ----- Constructors and destructor -----

        /** @brief Default constructor. */
        /**
         * @details
         * Initializes an empty `Value255` with size 0 and no allocated memory.
         */
        explicit Value255() noexcept = default;

        /** @brief Destructor. */
        /**
         * @details
         * Cleans up any heap-allocated memory when the `Value255` instance is destroyed.
         * The spinlock is acquired before cleanup to ensure that no other thread is
         * currently accessing the data.
         */
        ~Value255() noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            cleanup();
        }

        /** @brief Copy constructor (deleted). */
        Value255( Value255 const & ) = delete;

        /** @brief Move constructor. */
        /**
         * @details
         * Transfers ownership of the data from the other `Value255` to this instance.
         * After the move, the other `Value255` is left in a valid but unspecified state.
         *
         * @param other [in,out] The other `Value255` to move from.
         */
        Value255( Value255 &&other ) noexcept;

        // ----- Operators -----

        /** @brief Copy assignment operator (deleted). */
        Value255 &operator=( Value255 const & ) noexcept = delete;

        /** @brief Move assignment operator. */
        /**
         * @details
         * Performs the following steps:
         *
         * 1. If this and other instance are same, do nothing and return *this.
         * 2. Releases any heap memory currently owned by this instance.
         * 3. Transfers or copies the contents from the other `Value255`
         *    (heap pointer is moved, inline buffer is copied).
         * 4. Resets the other `Value255` by setting its size to 0 and
         *    its pointer to nullptr.
         *
         * @param other [in,out] The other `Value255` to move from.
         *
         * @return Reference to this `Value255` after the move.
         */
        Value255 &operator=( Value255 &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Perform the following steps:
         *
         * 1. If this and other instance are same, return `true`.
         * 2. If the sizes do not match, return `false`.
         * 3. If the size is `0`, return `true`.
         * 4. If the payload is an exact match, return `true`; otherwise, return `false`.
         *
         * @param other [in] other instance to compare with.
         *
         * @return `true` if both instances are equal, `false` otherwise.
         */
        bool operator==( Value255 const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs the following steps:
         *
         * 1. If this and other instance are same, return `std::strong_ordering::equal`.
         * 2. If this instance's size is smaller, return `std::strong_ordering::less`.
         * 3. If this instance's size is larger, return `std::strong_ordering::greater`.
         * 4. Returns the result of `std::compare_three_way`, comparing the payloads of
         *    both instances.
         *
         * @param other [in] The other `Value255` to compare with.
         *
         * @return `std::strong_ordering` indicating the comparison result.
         */
        auto operator<=>( Value255 const &other ) const noexcept
            ->std::strong_ordering;

        // ----- Public member functions -----

        /** @brief Compares the value with external data. */
        /**
         * @details
         * Performs a byte-by-byte comparison between this instance's data and the
         * provided external data. Only this instance's spinlock is held during
         * comparison; the external data is not protected.
         *
         * @param data [in] Pointer to external data for comparison.
         *                  A null pointer is only valid if size is 0.
         * @param size [in] Size of external data in bytes.
         *
         * @return `true` if sizes match and payloads are identical, `false` otherwise.
         *
         * @par Thread Safety
         * This method is thread-safe for this instance. However, the external data
         * pointed to by `data` is NOT protected by this method's lock.
         *
         * @attention
         * - The caller is responsible for ensuring that `data` remains valid and
         *   unchanged during this method's execution.
         * - If another thread modifies the external data simultaneously, the result
         *   is undefined.
         * - This design is intentional and suitable for comparing with driver buffers
         *   or fixed immutable data.
         */
        [[nodiscard]]
        bool areEquals( std::byte const *data, std::uint8_t size ) const noexcept;

        /** @brief Returns the size of the value in bytes. */
        /**
         * @return Size of the value in bytes.
         */
        [[nodiscard]]
        std::uint8_t size() const noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            return size_;
        }

        /** @brief Returns the value as a vector of bytes. */
        /**
         * @return Vector of bytes representing the value.
         */
        [[nodiscard]]
        std::vector<std::byte> bytes() const noexcept;

        /** @brief Provides thread-safe access to raw data via callback. */
        /**
         * @details
         * Acquires the instance's spinlock, invokes the provided callback with
         * a const pointer to the raw data and its size, and releases the lock
         * when the callback completes.
         *
         * This method ensures that the data pointer and size remain valid and
         * unchanged during the callback execution. The lock is held for the
         * entire duration of the callback, guaranteeing exclusivity.
         *
         * @par Thread Safety
         * This method is thread-safe. The spinlock is held for the entire duration
         * of callback execution, ensuring that the data remains unchanged.
         *
         * @tparam Callable A callable type (function, lambda, or functor) with
         * signature compatible with `void(std::byte const *, std::uint8_t)`.
         *
         * @param callback [in] A callable object to be invoked with the data
         * pointer and size. The callback should complete quickly, as the lock
         * is held during its execution.
         *
         * @return The return value of the callback (if any).
         *
         * @example
         * @code
         * myValue.withData( [driver]( auto const *data, auto size ) {
         *     driver->write( data, size );
         * });
         * @endcode
         */
        template<typename Callable>
        [[nodiscard]]
        auto withData( Callable &&callback ) const noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            return std::forward<Callable>( callback )( dataUnlocked(), size_ );
        }

        /** @brief Returns a string representation of the value. */
        /**
         * @details
         * The string representation is formatted as a list of hexadecimal byte values.
         * For example, a value containing the bytes 0xA5, 0xE7, 0x00, 0xFF
         * would be represented as:
         * ```
         * [ 0xA5 0xE7 0x00 0xFF ]
         * ```
         *
         * @return String representation of the value.
         */
        [[nodiscard]]
        std::string str() const noexcept;

        /** @brief Create clone from this instance. */
        /**
        * @details
        * Creates a new `Value255` instance by copying the contents of this
        * instance.
        *
        * Allocates memory as needed and copies the provided data into the
        * new instance.
        *
        * @return An optional containing the cloned `Value255` if successful;
        *         `std::nullopt` otherwise.
        *
        * @note
        * The failure cases are:
        * - A situation where memory cannot be allocated to store a copy of `data`.
        */
        [[nodiscard]]
        std::optional<Value255> clone( void ) const noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            /* Note: create() is public method but does not require a lock,
                     so there are no deadlock issues. */
            return create( dataUnlocked(), size_ );
        }

    protected:
        // ----- Protected member functions -----

        /** @brief Sets the value's data and size (protected). */
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

        // ----- Member variables -----

        std::atomic<bool> mutable lock_ = false;    //!< Spinlock for thread safety.
        std::uint8_t size_ = 0;                     //!< Size of the value in bytes.
        std::byte raw_data_[INLINE_SIZE] = {};      //!< Inline storage or heap pointer.

        // ----- Private member functions -----

        void lock() const noexcept;

        void unlock() const noexcept
        {
            lock_.store( false, std::memory_order_release );
        }

        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }

        void cleanup() noexcept;

        void moveFrom( Value255 &&other ) noexcept;

        std::uintptr_t heapPointer() const noexcept;

        std::byte *heapPointerAsByte() const noexcept
        {
            return reinterpret_cast<std::byte *>( heapPointer() );
        }

        void *heapPointerAsVoid() const noexcept
        {
            return reinterpret_cast<void *>( heapPointer() );
        }

        std::byte const *dataUnlocked() const noexcept
        {
            return isHeapAllocated() ? heapPointerAsByte() : raw_data_;
        }

        bool areEqualsUnlocked( std::byte const *data, std::uint8_t size ) const noexcept;

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

        // ----- Public member functions -----

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
        bool set( std::byte const *data, std::uint8_t size ) noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            return Value255::set( data, size );
        }

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
        SetResult setWithResult( std::byte const *data, std::uint8_t size ) noexcept
        {
            auto guard_until_scope_end = makeGuard( *this );
            // [===> Follows: Locked]

            return Value255::setWithResult( data, size );
        }

    }; // class MutableValue255

    /** @brief Result codes for the `setWithResult` operation. */
    /** @details
    * This enum class defines the possible outcomes of the `setWithResult` operation
    * in the `Value255` and `MutableValue255` classes.
    */
    enum class Value255::SetResult : std::uint8_t
    {
        Success = 0,         //!< Data successfully updated.
        NoChange = 1,        //!< New data is identical to current data.
        IllegalArgument = 2, //!< Invalid arguments provided.
        OutOfMemory = 3,     //!< Memory allocation failed.
    };

    static_assert(  sizeof( std::uintptr_t ) == 4U );

    static_assert(  sizeof(Value255) == 6U );
    static_assert( alignof(Value255) == 1U );
    static_assert(std::is_standard_layout<Value255>::value);

    static_assert(  sizeof(MutableValue255) == 6U );
    static_assert( alignof(MutableValue255) == 1U );
    static_assert(std::is_standard_layout<MutableValue255>::value);

} // namespace value
