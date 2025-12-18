#include <algorithm>
#include <esp_heap_caps.h>

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
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this != &other )
    {
        cleanup();
        // [===> Follows: All resources were released and cleared]

        moveFrom( std::move( other ) );
        // [===> Follows: All Resources were moved from other]
    }

    return *this;
}

bool PropertyValue::operator == ( PropertyValue const &other ) const noexcept
{
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return true; }
    // [===> Follows: Not the same instance]

    if ( size_ != other.size_ ) { return false; }
    // [===> Follows: Sizes matched]

    if ( size_ == 0 ) { return true; }
    // [===> Follows: Sizes present]

    return std::equal( data(), data() + size_, other.data() );
}

auto PropertyValue::operator <=> ( PropertyValue const &other ) const noexcept
{
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return std::strong_ordering::equal; }
    // [===> Follows: Not the same instance]

    if ( size_ < other.size_ ) { return std::strong_ordering::less; }
    if ( size_ > other.size_ ) { return std::strong_ordering::greater; }
    // [===> Follows: Sizes matched]

    if (size_ == 0) { return std::strong_ordering::equal; }
    // [===> Follows: Sizes present]

    return std::lexicographical_compare_three_way(
        data(), data() + size_,
        other.data(), other.data() + other.size_,
        std::compare_three_way() );
}


/* ___________________________________/ _/ _/ .*/
/* == [ Protected methods. ] */
bool PropertyValue::set( std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: This instance is locked]

    if ( size <= INLINE_SIZE )  // Note: Use inline storage.
    {
        cleanup();
        // [===> Follows: All resources were released and cleared]
        
        std::memcpy( raw_data_, data, size );
        // [===> Follows: Data copied to inline buffer]
    }
    else // [!! Caution !!]__  Contains early returns.  __[!! Caution !!]
    {
        if ( size > size_ ) // Note: Allocate or reallocate.
        {
            cleanup();
            // [===> Follows: All resources were released and cleared]

            void* p = heap_caps_malloc( size, MALLOC_CAP_DEFAULT );
            if ( !p ) { return false; }
            // ~~~~~~~~~~~~~~~~~~~~~~~  [ Early return on allocation failure!! ]
            // [===> Follows: Heap memory reallocated]

            std::uintptr_t addr = reinterpret_cast<std::uintptr_t>( p );
            static_assert( sizeof( std::uintptr_t ) == 4, "The `uintptr_t` must be 4 bytes." );
            std::memcpy( raw_data_, &addr, INLINE_SIZE );
        }
        // [===> Follows: Heap memory allocation completed]

        std::memcpy( heapPointerAsVoid(), data, size );
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
    // [===> Prerequisite: This instance is locked]

    if ( isHeapAllocated() )
    {
        heap_caps_free( heapPointerAsVoid() );
    }
    // [===> Follows: This instance has no heap memory]

    std::memset( raw_data_, 0, INLINE_SIZE );
    // [===> Follows: This instance has no data]

    size_ = 0;
    // [===> Follows: This instance has no size]
}

void PropertyValue::moveFrom( PropertyValue &&other ) noexcept
{
    // [===> Prerequisite: This and other instance are locked]
    // [===> Prerequisite: This instance has no heap memory]

    size_ = other.size_;
    // [===> Follows: This instance has size copied]

    std::memcpy( raw_data_, other.raw_data_, INLINE_SIZE );
    // [===> Follows: This instance has data copied]

    std::memset( other.raw_data_, 0, INLINE_SIZE );
    // [===> Follows: Other instance has no data]

    other.size_ = 0;
    // [===> Follows: Other instance has no size]
}
