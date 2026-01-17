/* Self */
#include <value255.hpp>

/* C++ Standard Library */
#include <algorithm>
#include <cstring>
#include <format>
#include <sstream>

/* ESP-IDF, FreeRTOS Library */
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


using namespace value;


// ----- Factory methods -----

std::optional<Value255> Value255::create(
    std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: Creating a new instance - no lock needed]

    Value255 pv;
    bool ans = pv.set( data, size );

    if ( ans )
    {
        return pv;
    }

    return std::nullopt;
}

std::optional<MutableValue255> MutableValue255::create(
    std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: Creating a new instance - no lock needed]

    MutableValue255 pv;
    bool ans = pv.set( data, size );

    if ( ans )
    {
        return pv;
    }

    return std::nullopt;
}


// ----- Constructors and destructor -----

Value255::Value255( Value255 &&other ) noexcept
{
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    moveFrom( std::move( other ) );
}


// ----- Operators -----

Value255 &Value255::operator=( Value255 &&other ) noexcept
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

bool Value255::operator==( Value255 const &other ) const noexcept
{
    SpinGuard guard( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return true; }
    // [===> Follows: Not the same instance]

    return areEqualsUnlocked( other.dataUnlocked(), other.size_ );
}

auto Value255::operator<=>( Value255 const &other ) const noexcept
    ->std::strong_ordering
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

    std::byte const *a = dataUnlocked();
    std::byte const *b = other.dataUnlocked();

    return std::lexicographical_compare_three_way(
        a, a + size_,
        b, b + other.size_,
        std::compare_three_way()
    );
}

namespace value
{
    std::ostream &operator<<( std::ostream &os, Value255 const &v ) noexcept
    {
        os << v.str();
        return os;
    }
}


// ----- Public member functions -----

bool Value255::areEquals( std::byte const *other_data, std::uint8_t other_size ) const noexcept
{
    SpinGuard guard( *this );
    // [===> Follows: Locked]

    return areEqualsUnlocked( other_data, other_size );
}

std::vector<std::byte> Value255::bytes() const noexcept
{
    SpinGuard guard( *this );
    // [===> Follows: Locked]

    std::vector<std::byte> out;
    out.reserve( size_ );

    std::byte const *ptr = dataUnlocked();
    out.insert( out.end(), ptr, ptr + size_ );

    return out;
}

std::string Value255::str() const noexcept
{
    SpinGuard guard( *this );
    // [===> Follows: Locked]

    std::byte const *ptr = dataUnlocked();
    std::ostringstream oss;

    oss << "[ ";

    for ( uint8_t i = 0; i < size_; i++ )
    {
        oss << std::format( "0x{:02X}", static_cast<unsigned>( ptr[i] ) );

        if ( i + 1 < size_ ) oss << ' ';
    }

    oss << " ]";

    return oss.str();
}


// ----- Protected member functions -----

bool Value255::set( std::byte const *data, std::uint8_t size ) noexcept
{
    SetResult ret = setWithResult( data, size );

    return ( ret == SetResult::Success  )
        || ( ret == SetResult::NoChange );
}

Value255::SetResult Value255::setWithResult( std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: This instance is locked]

    if ( size > 0U && data == nullptr )
    {
        cleanup();
        return SetResult::IllegalArgument;
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  [ Early return on invalid parameters!! ]
    // [===> Follows: All parameters are valid]

    if ( size_ == size )
    {
        if ( areEqualsUnlocked( data, size ) )
        {
            return SetResult::NoChange;
        }
    }
    // [===> Follows: Data is different]

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
            if ( !p ) { return SetResult::OutOfMemory; }
            // ~~~~~~~~~~~~~~~~~~~~~~~  [ Early return on allocation failure!! ]
            // [===> Follows: Heap memory reallocated]

            std::uintptr_t addr = reinterpret_cast<std::uintptr_t>( p );
            std::memcpy( raw_data_, &addr, INLINE_SIZE );
        }
        // [===> Follows: Heap memory allocation completed]

        std::memcpy( heapPointerAsVoid(), data, size );
        // [===> Follows: Data copied to heap memory]
    }

    size_ = size;
    // [===> Follows: Size updated]

    return SetResult::Success;
}


// ----- Private member functions -----

void Value255::lock() const noexcept
{
    while( lock_.exchange( true, std::memory_order_acquire ) )
    {
        /* Yield to other tasks while waiting for lock */
        taskYIELD();
    }
}

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

    other.size_ = 0U;
    // [===> Follows: Other instance has no size]
}

std::uintptr_t Value255::heapPointer() const noexcept
{
    std::uintptr_t ptr = 0;

    std::memcpy( &ptr, raw_data_, INLINE_SIZE );

    return ptr;
}

bool Value255::areEqualsUnlocked( std::byte const *other_data, std::uint8_t other_size ) const noexcept
{
    // [===> Prerequisite: This instance is already locked by caller]

    if ( size_ != other_size ) { return false; }
    // [===> Follows: Sizes matched]

    if ( other_size == 0U ) { return true; }
    // [===> Follows: Sizes present]

    if( other_data == nullptr ) { return false; }
    // [===> Follows: Other data is valid]

    std::byte const *a = dataUnlocked();
    std::byte const *b = other_data;

    return std::equal( a, a + size_, b );
}
