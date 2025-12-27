#pragma once

/* C++ Standard Library */
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

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
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /* #region Factory methods */

        /** @brief Creates a `Value255` instance from raw data. */
        /**
        * @details
        * Allocates memory as needed and copies the provided data into the new
        * instance.
        *
        * @param data Pointer to the raw data. A null pointer is only valid if size is 0.
        * @param size Size of the data in bytes.
        *
        * @return An optional containing the created `Value255` if successful;
        * `std::nullopt` otherwise.
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
        */
        static std::optional<Value255> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        /* #endregion */// Factory methods

    protected:

        /* #region SpinGuard */

        struct SpinGuard
        {
            Value255 const &a_;
            Value255 const &b_;

            explicit SpinGuard( Value255 const &v ) noexcept : SpinGuard( v, v ) {}

            explicit SpinGuard( Value255 const &a, Value255 const &b ) noexcept : a_( a ), b_( b )
            {
                // Note: To prevent deadlocks, only one if the same instance will be locked.
                if ( &a_ == &b_ ) { a_.lock();            }
                else              { a_.lock(); b_.lock(); }
            }

            ~SpinGuard()
            {
                // Note: Unlock in reverse order.
                if ( &a_ == &b_ ) {              a_.unlock(); }
                else              { b_.unlock(); a_.unlock(); }
            }

            SpinGuard( SpinGuard const & ) = delete;
            SpinGuard &operator=( SpinGuard const & ) = delete;
            SpinGuard( SpinGuard && ) = delete;
            SpinGuard &operator=( SpinGuard && ) = delete;
        };

        /* #endregion */// SpinGuard

    private:

        static constexpr std::uint8_t INLINE_SIZE = 4;

    /* #endregion */// Static members, Inner types


    /* ^\__________________________________________ */
    /* #region Constructors.                        */

    public:

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
         */
        ~Value255() noexcept
        {
            // Note: Destructor called, there's no need to lock it.
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
         * @param other The other `Value255` to move from.
         */
        Value255( Value255 &&other ) noexcept
        {
            SpinGuard guard( *this, other );
            // [===> Follows: Locked]

            moveFrom( std::move( other ) );
        }

    /* #endregion */// Constructors


    /* ^\__________________________________________ */
    /* #region Operators.                           */

    public:

        /** @brief Copy assignment operator (deleted). */
        Value255 &operator=( Value255 const & ) noexcept = delete;

        /** @brief Move assignment operator. */
        /**
         * @details
         * Performs the following steps:
         * 1. If this and other instance are same, do nothing and return *this.
         * 2. Releases any heap memory currently owned by this instance.
         * 3. Transfers or copies the contents from the other `Value255`
         *    (heap pointer is moved, inline buffer is copied).
         * 4. Resets the other `Value255` by setting its size to 0 and
         *    its pointer to nullptr.
         *
         * @param other The other `Value255` to move from.
         *
         * @return Reference to this `Value255` after the move.
         */
        Value255 &operator=( Value255 &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Perform the following steps:
         * 1. If this and other instance are same, return `true`.
         * 2. If the sizes do not match, return `false`.
         * 3. If the size is `0`, return `true`.
         * 4. If the payload is an exact match, return `true`; otherwise, return `false`.
         *
         * @param other other instance to compare with.
         *
         * @return `true` if both instances are equal, `false` otherwise.
         */
        bool operator==( Value255 const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs the following steps:
         * 1. If this and other instance are same, return `std::strong_ordering::equal`.
         * 2. If this instance's size is smaller, return `std::strong_ordering::less`.
         * 3. If this instance's size is larger, return `std::strong_ordering::greater`.
         * 4. Returns the result of `std::compare_three_way`, comparing the payloads of both instances.
         *
         * @param other The other `Value255` to compare with.
         *
         * @return `std::strong_ordering` indicating the comparison result.
         */
        auto operator<=>( Value255 const &other ) const noexcept;

    /* #endregion */// Operators


    /* ^\__________________________________________ */
    /* #region Instance members.                    */

    public:

        /**
         * @brief Returns the size of the value in bytes.
         * @return Size of the value in bytes.
         */
        [[nodiscard]] std::uint8_t size() const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            return size_;
        }

        /** @brief Returns the size of the value in bytes. */
        /**
         * @return Vector of bytes representing the value.
         */
        [[nodiscard]] std::vector<std::byte> bytes() const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            std::vector<std::byte> out;
            out.reserve( size_ );

            if ( isHeapAllocated() )
            {
                std::byte *ptr = heapPointerAsByte();
                out.insert( out.end(), ptr, ptr + size_ );
            }
            else
            {
                out.insert( out.end(), raw_data_, raw_data_ + size_ );
            }

            return out;
        }

        /** @brief Returns a string representation of the value. */
        /**
         * @details
         * The string representation is formatted as a list of hexadecimal byte values.
         * For example, a value containing the bytes 0xA5, 0xE7, 0x00, 0xFF would be represented as:
         * ```
         * [ 0xA5 0xE7 0x00 0xFF ]
         * ```
         *
         * @return String representation of the value.
         */
        [[nodiscard]] std::string str() const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            auto *ptr = data_unlocked();
            auto   sz = size_unlocked();
            std::ostringstream oss;

            oss << "[ ";

            for ( uint8_t i = 0; i < sz; i++ )
            {
                oss << std::format( "0x{:02X}", static_cast<unsigned>( ptr[i] ) );

                if ( i + 1 < sz ) oss << ' ';
            }

            oss << " ]";

            return oss.str();
        }

        /** @brief Create clone from this instance. */
        /**
        * @details
        * Creates a new `Value255` instance by copying the contents of this
        * instance.
        * Allocates memory as needed and copies the provided data into the
        * new instance.
        *
        * @return An optional containing the cloned `Value255` if successful;
        * `std::nullopt` otherwise.
        *
        * @note
        * The failure cases are:
        * - A situation where memory cannot be allocated to store a copy of `data`.
        */
        [[nodiscard]] std::optional<Value255> clone( void ) const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            std::byte    const *data = data_unlocked();
            std::uint8_t const  size = size_unlocked();

            /* Note:
                create() is public method but does not require a lock,
                so there are no deadlock issues. */
            return create( data, size );
        }

    protected:

        [[nodiscard]] bool set( std::byte const *data, std::uint8_t size ) noexcept;

    private:

        /* #region Private methods. */

        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }

        void cleanup() noexcept;

        void moveFrom( Value255 &&other ) noexcept;

        void lock() const noexcept { while(
            lock_.exchange( true, std::memory_order_acquire ) ) { /* Busy loop */ } }

        void unlock() const noexcept {
            lock_.store( false, std::memory_order_release ); }

        std::uintptr_t heapPointer() const noexcept
        {
            std::uintptr_t ptr = 0;

            std::memcpy( &ptr, raw_data_, INLINE_SIZE );

            return ptr;
        }

        std::byte *heapPointerAsByte() const noexcept {
            return reinterpret_cast<std::byte *>( heapPointer() ); }

        void *heapPointerAsVoid() const noexcept {
            return reinterpret_cast<void *>( heapPointer() ); }

        std::uint8_t size_unlocked() const noexcept { return size_; }

        std::byte const *data_unlocked() const noexcept {
            return isHeapAllocated() ? heapPointerAsByte() : raw_data_; }

        /* #endregion */// Private methods

        /* #region Member variables */

        std::atomic<bool> mutable lock_ = false;    //!< Spinlock for thread safety. false=unlocked, true=locked.
        std::uint8_t size_ = 0;                     //!< Size of the property value in bytes.
        std::byte raw_data_[INLINE_SIZE] = {};      //!< Inline storage or heap pointer.

        /* #endregion */// Member variables

    /* #endregion */// Instance members

    }; // class Value255

    /** @brief Stream output operator for `Value255`. */
    /**
     * @details
     * Outputs the string representation of the `Value255` instance to the provided
     * output stream.
     *
     * @see Value255::str() for the format of the output.
     *
     * @param os The output stream to write to.
     * @param v The `Value255` instance to output.
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
    /* ^\__________________________________________ */
    /* #region Constructors.                        */

    public:

        using Value255::Value255;

    /* #endregion */// Constructors


    /* ^\__________________________________________ */
    /* #region Instance members.                    */

    public:

        /** @brief Sets the value's data and size. */
        /**
        * @details
        * This method updates the contents of the `MutableValue255` instance
        * with the provided data and size.
        *
        * @par Thread Safety
        * This method is thread-safe and acquires the instance's spinlock
        * for the duration of the operation.
        *
        * @param data Pointer to the new raw data. A null pointer is only valid if size is 0.
        * @param size Size of the new data in bytes.
        *
        * @return `true` if the operation was successful; `false` otherwise.
        * @note
        * The failure cases are:
        * - `data` is null while `size` is greater than 0.
        * - A situation where memory cannot be allocated to store a copy of `data`.
       */
        [[nodiscard]] bool set( std::byte const *data, std::uint8_t size ) noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            return Value255::set( data, size );
        }

    /* #endregion */// Instance members

    }; // class MutableValue255

    static_assert(  sizeof(value::Value255) == 6U, "Unexpected Value255 size");
    static_assert( alignof(value::Value255) == 1U, "Unexpected Value255 alignment");

} // namespace value

namespace std // Formatter specialization
{
    /** @brief Formatter specialization for `value::Value255`. */
    /**
     * @details
     * This formatter allows `value::Value255` instances to be formatted
     * using the C++20 `<format>` library.
     *
     * @see Value255::str() for the format of the output.
     */
    template <>
    struct formatter<value::Value255>
    {
        /** @brief Parse format specifiers (no supported). */
        /**
         * @param ctx The format parse context.
         *
         * @return Iterator to the end of the parsed format specifiers.
         */
        constexpr auto parse( std::format_parse_context &ctx ) const noexcept {
            return ctx.begin(); }

        /** @brief Format the `value::Value255`. */
        /**
         * @param v The `value::Value255` instance to format.
         * @param ctx The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( value::Value255 const &v, FormatContext &ctx ) const noexcept {
            return std::ranges::copy( v.str(), ctx.out() ).out; }
    };
} // namespace std
