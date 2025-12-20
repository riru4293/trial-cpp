#include <algorithm>
#include <esp_heap_caps.h>

#include <value255.hpp>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace value;

/* ^\__________________________________________ */
/* #region Factory methods.                     */

std::optional<Value255> Value255::create(
    std::byte const *data, std::uint8_t size ) noexcept
{
    // Note: Creating a new instance, there's no need to lock it.

    Value255 pv;
    bool ans = pv.set( data, size );

    if ( ans )
    {
        return pv;
    }

    return std::nullopt;
}

/* #endregion */// Factory methods

/* ^\__________________________________________ */
/* #region Operators.                           */

Value255 &Value255::operator = ( Value255 &&other ) noexcept
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

bool Value255::operator == ( Value255 const &other ) const noexcept
{
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return true; }
    // [===> Follows: Not the same instance]

    if ( size_ != other.size_ ) { return false; }
    // [===> Follows: Sizes matched]

    if ( size_ == 0 ) { return true; }
    // [===> Follows: Sizes present]

    auto *a = data_unlocked();
    auto *b = other.data_unlocked();

    return std::equal( a, a + size_, b );
}

auto Value255::operator <=> ( Value255 const &other ) const noexcept
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

    auto *a = data_unlocked();
    auto *b = other.data_unlocked();

    return std::lexicographical_compare_three_way(
        a, a + size_,
        b, b + other.size_,
        std::compare_three_way()
    );
}

/* #endregion */// Operators

/* ^\__________________________________________ */
/* #region Protected methods.                   */

bool Value255::set( std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: This instance is locked]

    if ( size > 0 && data == nullptr ) { return false; }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  [ Early return on invalid parameters!! ]
    // [===> Follows: All parameters are valid]

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

/* #endregion */// Protected methods

/* ^\__________________________________________ */
/* #region Private methods.                     */

void Value255::cleanup() noexcept
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

void Value255::moveFrom( Value255 &&other ) noexcept
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

/* #endregion */// Private methods


std::ostream &operator << ( std::ostream &os, Value255 const &v )
{
    os << v.str();
    return os;
}
