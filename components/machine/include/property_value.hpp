#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <esp_heap_caps.h>
#include <compare>   // std::strong_ordering
#include <algorithm> // std::lexicographical_compare_three_way

namespace machine
{
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
            return create( other.data(), other.size() );
        }

        #pragma endregion
    private:
        static constexpr std::uint8_t INLINE_SIZE = 4;
    /* ^\__________________________________________ */
    /* Constructors.                                */
    public:
        #pragma region : constructors

        /**
         * @brief Default constructor.
         * @details
         * Initializes an empty `PropertyValue` with size 0 and no allocated memory.
         */
        PropertyValue() noexcept = default;

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
             moveFrom( std::move( other ) );
        }

        #pragma endregion
    private:
        PropertyValue( std::byte const *data, std::uint8_t size ) { set( data, size ); }
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
        constexpr bool operator == ( PropertyValue const &other ) const noexcept;

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
        constexpr auto operator <=> ( PropertyValue const &other ) const noexcept;

        #pragma endregion
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        /**
         * @brief Returns the size of the property value in bytes.
         * @return Size of the property value in bytes.
         */
        [[nodiscard]] std::uint8_t size() const noexcept { return size_; }

        /**
         * @brief Returns a pointer to the data of the property value.
         * @return Pointer to the data of the property value.
         */
        [[nodiscard]] std::byte const *data() const noexcept
        {
            return isHeapAllocated() ? storage_.heap_ptr_ : storage_.inline_buffer_;
        }
    protected:
        [[nodiscard]] bool set( std::byte const *data, std::uint8_t size ) noexcept;
    private:
        bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }
        void cleanup() noexcept;
        void moveFrom( PropertyValue &&other ) noexcept;

        #pragma region : member variables

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
} // namespace machine
