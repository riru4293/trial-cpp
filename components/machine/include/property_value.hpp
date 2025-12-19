#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <atomic>
#include <ostream>
#include <format>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace machine
{
    /** @brief Represents a property value with dynamic storage. */
    /**
     * @details
     * This class manages a property value that may be stored either inline
     * (4 bytes) or on the heap (for larger sizes). It provides mechanisms for
     * constructing, moving, comparing, and streaming property values.
     * Instances are movable but not copyable.
     *
     * For external users, this class behaves as an immutable value type.
     * Mutation is only permitted through the derived `MutablePropertyValue`.
     *
     * Critical sections are intentionally kept short; a spinlock is chosen
     * to minimize memory footprint and locking overhead.
     *
     * The class supports equality comparison, ordering comparison,
     * and stream output via `operator<<`.
     *
     * @thread_safety
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
    class PropertyValue
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    public:
        #pragma region : factory methods

        /** @brief Creates a `PropertyValue` instance from raw data. */
        /**
         * @details
         * Allocates memory as needed and copies the provided data into the new instance.
         * @param data Pointer to the raw data.
         * @param size Size of the data in bytes.
         * @return An optional containing the created `PropertyValue` if successful; `std::nullopt` otherwise
         */
        static std::optional<PropertyValue> create(
            std::byte const *data, std::uint8_t size ) noexcept;

        /** @brief Clones an existing `PropertyValue` instance. */
        /**
         * @details
         * Creates a new `PropertyValue` instance by copying the contents of the provided instance.
         * Allocates memory as needed and copies the provided data into the new instance.
         * @param other The `PropertyValue` instance to clone.
         * @return An optional containing the cloned `PropertyValue` if successful; `std::nullopt` otherwise
         */
        static std::optional<PropertyValue> clone( PropertyValue const &other ) noexcept
        {
            SpinGuard guard( other );
            // [===> Follows: Locked]

            auto const size = other.size_unlocked();
            auto const *ptr = other.data_unlocked();

            return create( ptr, size );
        }

        #pragma endregion
    private:
        static constexpr std::uint8_t INLINE_SIZE = 4;

        struct SpinGuard
        {
            PropertyValue const &a_;
            PropertyValue const &b_;

            explicit SpinGuard( PropertyValue const &v ) : SpinGuard( v, v ) {}
            explicit SpinGuard( PropertyValue const &a, PropertyValue const &b ) : a_( a ), b_( b )
            {
                if ( &a_ == &b_ )
                {
                    a_.lock();
                }
                else
                {
                    a_.lock();
                    b_.lock();
                }
            }
            ~SpinGuard()
            {
                if ( &a_ == &b_ )
                {
                    a_.unlock();
                }
                else
                {
                    b_.unlock();
                    a_.unlock();
                }
            }
            SpinGuard( SpinGuard const & ) = delete;
            SpinGuard &operator = ( SpinGuard const & ) = delete;
        };
    /* ^\__________________________________________ */
    /* Constructors.                                */
    public:
        #pragma region : constructors

        /**
         * @brief Default constructor.
         * @details
         * Initializes an empty `PropertyValue` with size 0 and no allocated memory.
         */
        explicit PropertyValue() noexcept = default;

        /**
         * @brief Destructor.
         * @details
         * Cleans up any heap-allocated memory when the `PropertyValue` instance is destroyed.
         */
        ~PropertyValue() { cleanup(); }

        /** @brief Copy constructor (deleted). */
        PropertyValue( PropertyValue const & ) = delete;

        /** @brief Move constructor. */
        /**
         * @details
         * Transfers ownership of the data from the other `PropertyValue` to this instance.
         * After the move, the other `PropertyValue` is left in a valid but unspecified state.
         * @param other The other `PropertyValue` to move from.
         */
        PropertyValue( PropertyValue &&other ) noexcept
        {
            SpinGuard guard( *this, other );
            // [===> Follows: Locked]

            moveFrom( std::move( other ) );
        }

        #pragma endregion
    /* ^\__________________________________________ */
    /* Operators.                                   */
    public:
        #pragma region : operators

        /** @brief Copy assignment operator (deleted). */
        PropertyValue &operator = ( PropertyValue const & ) = delete;

        /** @brief Move assignment operator. */
        /**
         * @details
         * Performs the following steps:
         * 1. If this and other instance are same, do nothing and return *this.
         * 2. Releases any heap memory currently owned by this instance.
         * 3. Transfers or copies the contents from the other `PropertyValue`
         *    (heap pointer is moved, inline buffer is copied).
         * 4. Resets the other `PropertyValue` by setting its size to 0 and
         *    its pointer to nullptr.
         * @param other The other `PropertyValue` to move from.
         * @return Reference to this `PropertyValue` after the move.
         */
        PropertyValue &operator = ( PropertyValue &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Perform the following steps:
         * 1. If this and other instance are same, return `true`.
         * 2. If the sizes do not match, return `false`.
         * 3. If the size is `0`, return `true`.
         * 4. If the payload is an exact match, return `true`; otherwise, return `false`.
         * @param other other instance to compare with.
         * @return `true` if both instances are equal, `false` otherwise.
         */
        bool operator == ( PropertyValue const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs the following steps:
         * 1. If this and other instance are same, return `std::strong_ordering::equal`.
         * 2. If this instance's size is smaller, return `std::strong_ordering::less`.
         * 3. If this instance's size is larger, return `std::strong_ordering::greater`.
         * 4. Returns the result of `std::compare_three_way`, comparing the payloads of both instances.
         * @param other The other `PropertyValue` to compare with.
         * @return `std::strong_ordering` indicating the comparison result.
         */
        auto operator <=> ( PropertyValue const &other ) const noexcept;

        #pragma endregion
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        /**
         * @brief Returns the property value as a vector of bytes.
         * @return Vector of bytes representing the property value.
         */
        [[nodiscard]] std::vector<std::byte> bytes() const
        {
            SpinGuard guard( *this );

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

        [[nodiscard]] std::string str() const
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            auto v = bytes();

            std::ostringstream oss;
            oss << "[ ";

            for ( size_t i = 0; i < v.size(); i++ )
            {
                oss << "0x"
                    << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<unsigned>( v[i] );

                if ( i + 1 < v.size() ) oss << ' ';
            }

            oss << " ]";
            return oss.str();
        }
    protected:
        [[nodiscard]] bool set( std::byte const *data, std::uint8_t size ) noexcept;
    private:
        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }
        void cleanup() noexcept;
        void moveFrom( PropertyValue &&other ) noexcept;
        void lock() const noexcept { while( lock_.exchange( true, std::memory_order_acquire ) ) { /* Busy loop */ } }
        void unlock() const noexcept { lock_.store( false, std::memory_order_release ); }
        uintptr_t heapPointer() const noexcept
        {
            uintptr_t ptr = 0;

            std::memcpy( &ptr, raw_data_, INLINE_SIZE );

            return ptr;
        }

        std::byte *heapPointerAsByte() const noexcept
        {
            return reinterpret_cast<std::byte *>( heapPointer() );
        }

        void *heapPointerAsVoid() const noexcept
        {
            return reinterpret_cast<void *>( heapPointer() );
        }

        [[nodiscard]] std::uint8_t size_unlocked() const noexcept
        {
            return size_;
        }

        [[nodiscard]] std::byte const *data_unlocked() const noexcept
        {
            return isHeapAllocated() ? heapPointerAsByte() : raw_data_;
        }

        #pragma region : member variables

        std::atomic<bool> mutable lock_ = false; //!< Spinlock for thread safety. false=unlocked, true=locked.
        std::uint8_t size_ = 0; //<! Size of the property value in bytes.
        std::byte raw_data_[INLINE_SIZE] = {};  //!< Inline storage or heap pointer.

        #pragma endregion
    }; // class PropertyValue

    std::ostream &operator << ( std::ostream &os, PropertyValue const &v );


    class MutablePropertyValue : public PropertyValue
    {
    public:
        using PropertyValue::PropertyValue;

        [[nodiscard]] bool set(const std::byte* data, std::uint8_t size) {
            return PropertyValue::set(data, size);
        }
    }; // class MutablePropertyValue


    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert( sizeof(machine::PropertyValue) == 6, "Unexpected PropertyValue size" );
    static_assert( alignof(machine::PropertyValue) == 1, "Unexpected PropertyValue alignment" );
} // namespace machine



namespace std {

#pragma region : formatter specialization
    /** @brief Formatter specialization for `machine::PropertyValue`. */
    /**
     * @details
     * Formats a `PropertyValue` instance.
     * For example, a value of 0xA5 0xE7 0x00 0xFF would be formatted as follows:
     * ```
     * [ 0xA5 0xE7, 0x00 0xFF ]
     * ```
     */
    template <>
    struct formatter<machine::PropertyValue>
    {
        /** @brief Parse format specifiers (no supported). */
        constexpr auto parse( std::format_parse_context &ctx ) {
            return ctx.begin();
        }

        /** @brief Format the `machine::PropertyValue`. */
        template <typename FormatContext>
            auto format( machine::PropertyValue const &v, FormatContext &ctx ) const
            {
                return std::ranges::copy( v.str(), ctx.out() ).out;
            }
    };

#pragma endregion

} // namespace std
