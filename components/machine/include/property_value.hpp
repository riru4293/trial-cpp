#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <atomic>
#include <compare>

namespace machine
{
    /** @brief Represents a property value with dynamic storage. */
    /**
     * @details
     * This class manages a property value that can be stored either
     * inline (for small sizes) or on the heap (for larger sizes).
     * It provides methods for creating, moving, and comparing property values.
     * @note 
     * Internal methods such as `set()` and `cleanup()` assume that the caller
     * has already acquired the lock. Public API methods always acquire
     * `SpinGuard`/`TwinSpinGuard` to ensure thread safety.
     * @note
     * Instances of this class are movable but not copyable.
     * @note 
     * This class is effectively immutable for external users; mutation is only
     * allowed via the derived `MutablePropertyValue`.
     * @note Critical sections are expected to be short; spinlock is chosen for minimal footprint.
     * @note This class is immutable; its state cannot be modified after construction.
     * @note This class is hashable; std::hash specialization is provided.
     * @note This class is comparable; supports equality and ordering operators.
     * @note This class is streamable; supports `operator<<`.
     * @note This class is thread-safe.
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

            return create( other.data(), other.size() );
        }

        #pragma endregion
    private:
        static constexpr std::uint8_t INLINE_SIZE = 4;

        struct SpinGuard {
            PropertyValue const &v_;
            explicit SpinGuard( PropertyValue const &v ) : v_( v ) { v_.lock(); }
            ~SpinGuard() { v_.unlock(); }
            SpinGuard( SpinGuard const & ) = delete;
            SpinGuard &operator = ( SpinGuard const & ) = delete;
        };

        struct TwinSpinGuard {
            PropertyValue const &v1_;
            PropertyValue const &v2_;

            TwinSpinGuard(PropertyValue const& v1, PropertyValue const& v2)
                : v1_(v1), v2_(v2)
            {
                if ( &v1_ < &v2_ )  { v1_.lock(); v2_.lock(); }
                else                { v2_.lock(); v1_.lock(); }
            }

            ~TwinSpinGuard() {
                if ( &v1_ < &v2_ )  { v2_.unlock(); v1_.unlock(); }
                else                { v1_.unlock(); v2_.unlock(); }
            }

            TwinSpinGuard( TwinSpinGuard const & ) = delete;
            TwinSpinGuard &operator = ( TwinSpinGuard const & ) = delete;
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
            TwinSpinGuard guard( *this, other );
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
         * 1. Releases any heap memory currently owned by this instance.
         * 2. Transfers or copies the contents from the other `PropertyValue`
         *    (heap pointer is moved, inline buffer is copied).
         * 3. Resets the other `PropertyValue` by setting its size to 0 and
         *    its pointer to nullptr.
         * @param other The other `PropertyValue` to move from.
         * @return Reference to this `PropertyValue` after the move.
         */
        PropertyValue &operator = ( PropertyValue &&other ) noexcept;

        /** @brief Equality operator. */
        /**
         * @details
         * Perform the following steps:
         * 1. If the sizes do not match, return `false`.
         * 2. If the size is `0`, return `true`.
         * 3. If the payload is an exact match, return `true`; otherwise, return `false`.
         * @param other other instance to compare with.
         * @return `true` if both instances are equal, `false` otherwise.
         */
        bool operator == ( PropertyValue const &other ) const noexcept;

        /** @brief Three-way comparison operator. */
        /**
         * @details
         * Performs the following steps:
         * 1. If this instance's size is smaller, return `std::strong_ordering::less`.
         * 2. If this instance's size is larger, return `std::strong_ordering::greater`.
         * 3. Returns the result of `std::compare_three_way`, comparing the payloads of both instances.
         * @param other The other `PropertyValue` to compare with.
         * @return `std::strong_ordering` indicating the comparison result.
         */
        auto operator <=> ( PropertyValue const &other ) const noexcept;

        #pragma endregion
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        /**
         * @brief Returns the size of the property value in bytes.
         * @return Size of the property value in bytes.
         */
        [[nodiscard]] std::uint8_t size() const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            return size_;
        }

        /**
         * @brief Returns a pointer to the data of the property value.
         * @return Pointer to the data of the property value.
         */
        [[nodiscard]] std::byte const *data() const noexcept
        {
            SpinGuard guard( *this );
            // [===> Follows: Locked]

            return isHeapAllocated() ? storage_.heap_ptr_ : storage_.inline_buffer_;
        }
    protected:
        [[nodiscard]] bool set( std::byte const *data, std::uint8_t size ) noexcept;
    private:
        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }
        void cleanup() noexcept;
        void moveFrom( PropertyValue &&other ) noexcept;
        void lock() const noexcept { while( lock_.exchange( 1, std::memory_order_acquire ) ) { /* spin */ } }
        void unlock() const noexcept { lock_.store( 0, std::memory_order_release ); }


        #pragma region : member variables

        std::atomic<uint8_t> mutable lock_{0};  //!< Spinlock for thread safety. 0=unlocked, 1=locked.
        std::uint8_t size_ = 0;

        union Storage
        {
            std::byte inline_buffer_[INLINE_SIZE];
            std::byte* heap_ptr_;

            Storage() noexcept : inline_buffer_{} {}
        } storage_;

        #pragma endregion
    }; // class PropertyValue


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
    static_assert( sizeof(machine::PropertyValue) == 16, "Unexpected PropertyValue size" );
    static_assert( alignof(machine::PropertyValue) == 8, "Unexpected PropertyValue alignment" );
} // namespace machine
