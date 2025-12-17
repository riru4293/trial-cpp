#include <cstddef>
#include <cstdint>
#include <algorithm>            // For std::copy_n and std::equal
#include <esp_heap_caps.h>      // For heap_caps_malloc and heap_caps_free
#include <compare>              // For std::strong_ordering
#include <algorithm>            // For std::lexicographical_compare_three_way

#include <property_value.hpp>


/* ___________________________________/ _/ _/ .*/
/* == [ Namespaces. ] */
using namespace machine;


/* ___________________________________/ _/ _/ .*/
/* == [ Factory methods. ] */
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
    TwinSpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this != &other ) {
        cleanup();
        moveFrom( std::move( other ) );
    }

    return *this;
}

bool PropertyValue::operator == ( PropertyValue const &other ) const noexcept
{
    TwinSpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( size_ != other.size_ ) { return false; }
    // [===> Follows: Sizes matched]

    if ( size_ == 0 ) { return true; }
    // [===> Follows: Sizes present]

    bool is_heap = isHeapAllocated();

    std::byte const *lhs = is_heap ? storage_.heap_ptr_ : storage_.inline_buffer_;
    std::byte const *rhs = is_heap ? other.storage_.heap_ptr_ : other.storage_.inline_buffer_;

    return std::equal( lhs, lhs + size_, rhs );
}

auto PropertyValue::operator <=> ( PropertyValue const &other ) const noexcept
{
    TwinSpinGuard guard( *this, other );
    // [===> Follows: Locked]

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
    if ( size <= INLINE_SIZE )  // Note: Use inline storage.
    {
        cleanup();
        // [===> Follows: Heap memory has already been freed]
        
        std::copy_n( data, size, storage_.inline_buffer_ );
        // [===> Follows: Data copied to inline buffer]
    }
    else // Caution: Return false if allocation fails.
    {
        if ( size > size_ ) // Note: Allocate or reallocate.
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
        else // Do nothing
        {
            // Note: Existing heap memory is sufficient; no action needed.
        }
        // [===> Follows: Heap memory allocation completed]

        std::copy_n( data, size, storage_.heap_ptr_ );
        // [===> Follows: Data copied to heap memory]
    }

    size_ = size;
    // [===> Follows: Size updated]

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
    // [===> Follows: This instance has data copied]
    // [===> Follows: Other instance has no heap memory]

    other.size_ = 0;
    // [===> Follows: Other instances has no size]
}
