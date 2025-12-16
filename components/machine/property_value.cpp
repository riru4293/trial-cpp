#include <property_value.hpp>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <esp_heap_caps.h>
#include <compare>   // std::strong_ordering
#include <algorithm> // std::lexicographical_compare_three_way


/* ___________________________________/ _/ _/ .*/
/* == [ Namespaces. ] */
using namespace machine;


/* ___________________________________/ _/ _/ .*/
/* == [ Static members. ] */
std::optional<PropertyValue> PropertyValue::create(
    std::byte const *data, std::uint8_t size ) noexcept
{
    PropertyValue pv;
    bool ans = pv.set( data, size );

    if ( ans )
    {
        return pv;
    }

    return std::nullopt;
}


/* ___________________________________/ _/ _/ .*/
/* == [ Operators. ] */
PropertyValue &PropertyValue::operator = ( PropertyValue &&other ) noexcept
{
    if ( this != &other ) {
        cleanup();
        moveFrom( std::move( other ) );
    }

    return *this;
}

constexpr bool PropertyValue::operator == ( PropertyValue const &other ) const noexcept
{
    if ( size_ != other.size_ ) { return false; }
    // [===> Follows: Sizes matched]

    if ( size_ == 0 ) { return true; }
    // [===> Follows: Sizes present]

    bool is_heap = isHeapAllocated();

    std::byte const *lhs = is_heap ? storage_.heap_ptr_ : storage_.inline_buffer_;
    std::byte const *rhs = is_heap ? other.storage_.heap_ptr_ : other.storage_.inline_buffer_;

    return std::equal( lhs, lhs + size_, rhs );
}

constexpr auto PropertyValue::operator <=> ( PropertyValue const &other ) const noexcept
{
    if ( size_ < other.size_ ) { return std::strong_ordering::less; }
    if ( size_ > other.size_ ) { return std::strong_ordering::greater; }
    // [===> Follows: Sizes matched]

    return std::lexicographical_compare_three_way(
        data(), data() + size_,
        other.data(), other.data() + other.size_,
        std::compare_three_way() );
}


/* ___________________________________/ _/ _/ .*/
/* == [ Protected methods. ] */
bool PropertyValue::set( std::byte const *data, std::uint8_t size ) noexcept
{
    if ( size <= INLINE_SIZE )
    {
        cleanup();
        // [===> Follows: Heap memory has already been freed]
        
        std::copy_n( data, size, storage_.inline_buffer_ );
    }
    else
    {
        if ( size > size_ ) // Allocate or reallocate.
        {
            cleanup();

            storage_.heap_ptr_ = static_cast<std::byte *>(
                heap_caps_malloc( size, MALLOC_CAP_DEFAULT ) );

            if ( storage_.heap_ptr_ == nullptr )
            {
                size_ = 0;
                return false;
            }
        }
        // [===> Follows: Heap memory allocation completed]

        std::copy_n( data, size, storage_.heap_ptr_ );
    }

    size_ = size;

    return true;
}


/* ___________________________________/ _/ _/ .*/
/* == [ Private methods. ] */
void PropertyValue::cleanup() noexcept
{
    if ( isHeapAllocated() )
    {
        heap_caps_free( storage_.heap_ptr_ );
        storage_.heap_ptr_ = nullptr;
    }
}

void PropertyValue::moveFrom( PropertyValue &&other ) noexcept
{
    size_ = other.size_;
    // [===> Follows: This instance has size]

    if ( other.isHeapAllocated() )
    {
        storage_.heap_ptr_ = other.storage_.heap_ptr_;
        other.storage_.heap_ptr_ = nullptr;
    }
    else
    {
        std::copy_n( other.storage_.inline_buffer_, size_, storage_.inline_buffer_ );
    }
    // [===> Follows: Other instance has no heap memory]
    // [===> Follows: This instance has data copied]

    other.size_ = 0;
    // [===> Follows: Other instances has no size]
}
