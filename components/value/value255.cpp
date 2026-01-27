/* Self */
#include <value255.hpp>

/* C++ Standard Library */
#include <algorithm>
#include <bit>
#include <cstring>
#include <format>
#include <sstream>
#include <utility>

/* FreeRTOS Library */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* ESP-IDF Library */
#include <esp_heap_caps.h>


using namespace value;

// ----- Nested types implements -----

class Value255::ScopedSpinLock
{
public:
    explicit ScopedSpinLock( Value255 const &v ) noexcept
        : ScopedSpinLock( v, v ) {}

    explicit ScopedSpinLock( Value255 const &a, Value255 const &b ) noexcept
        : a_( a ), b_( b )
    {
        // Note: To prevent deadlocks, only one if the same instance will be locked.
        if ( &a_ == &b_ ) { a_.lock();            }
        else              { a_.lock(); b_.lock(); }
    }

    ~ScopedSpinLock()
    {
        // Note: Unlock in reverse order.
        if ( &a_ == &b_ ) {              a_.unlock(); }
        else              { b_.unlock(); a_.unlock(); }
    }

    ScopedSpinLock( ScopedSpinLock const & ) = delete;
    ScopedSpinLock &operator=( ScopedSpinLock const & ) = delete;

private:
    Value255 const &a_;
    Value255 const &b_;
};


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

Value255::~Value255() noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    cleanup();
}

Value255::Value255( Value255 &&other ) noexcept
{
    ScopedSpinLock lock( *this, other );
    // [===> Follows: Locked]

    moveFrom( std::move( other ) );
}

// ----- Operators -----

Value255 &Value255::operator=( Value255 &&other ) noexcept
{
    ScopedSpinLock lock( *this, other );
    // [===> Follows: Locked]

    if ( this != &other )
    {
        cleanup();
        moveFrom( std::move( other ) );
    }

    return *this;
}

bool Value255::operator==( Value255 const &other ) const noexcept
{
    ScopedSpinLock lock( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return true; }
    // [===> Follows: Not the same instance]

    return areEqualsUnlocked( other.dataUnlocked(), other.size_ );
}

std::strong_ordering Value255::operator<=>( Value255 const &other ) const noexcept
{
    ScopedSpinLock lock( *this, other );
    // [===> Follows: Locked]

    if ( this == &other ) { return std::strong_ordering::equal; }
    // [===> Follows: Not the same instance]

    if ( size_ != other.size_ ) { return size_ <=> other.size_; }
    // [===> Follows: Sizes matched]

    if ( size_ == 0 ) { return std::strong_ordering::equal; }
    // [===> Follows: Sizes present]

    return std::lexicographical_compare_three_way(
        dataUnlocked(), dataUnlocked() + size_,
        other.dataUnlocked(), other.dataUnlocked() + other.size_,
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

// ----- Public member methods -----

void Value255::withLockedData( DataReader const &callback ) const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    callback( dataUnlocked(), size_ );
}

bool Value255::areEquals( std::byte const *other_data, std::uint8_t other_size ) const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    return areEqualsUnlocked( other_data, other_size );
}

std::uint8_t Value255::size() const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    return size_;
}

std::vector<std::byte> Value255::bytes() const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    std::byte const *ptr = dataUnlocked();
    return std::vector<std::byte>( ptr, ptr + size_ );
}

std::string Value255::str() const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    std::byte const *ptr = dataUnlocked();
    std::ostringstream oss;

    oss << "[ ";

    for ( uint8_t i = 0; i < size_; i++ )
    {
        oss << std::format( "0x{:02X}", static_cast<unsigned>( ptr[i] ) );
        if ( i + 1 < size_ ) { oss << ' '; }
    }

    oss << " ]";

    return oss.str();
}

std::optional<Value255> Value255::clone( void ) const noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    /* Note: create() is public method but does not require a lock,
             so there are no deadlock issues. */
    return create( dataUnlocked(), size_ );
}

bool MutableValue255::set( std::byte const *data, std::uint8_t size ) noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    return Value255::set( data, size );
}

Value255::SetResult MutableValue255::setWithResult( std::byte const *data, std::uint8_t size ) noexcept
{
    ScopedSpinLock lock( *this );
    // [===> Follows: Locked]

    return Value255::setWithResult( data, size );
}

// ----- Protected member methods -----

bool Value255::set( std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: This instance is locked; only if mutable]

    SetResult ret = setWithResult( data, size );
    return ( ret == SetResult::Success ) || ( ret == SetResult::NoChange );
}

Value255::SetResult Value255::setWithResult(
    std::byte const *data, std::uint8_t size ) noexcept
{
    // [===> Prerequisite: This instance is locked; only if mutable]

    SetResult ret;

    do // Single-shot transaction; break on failure.
    {
        if ( size > 0U && data == nullptr )
        {
            ret = SetResult::IllegalArgument;
    /*->*/  break;
        }
        // [===> Follows: All parameters are valid]

        if ( ( size_ == size ) && areEqualsUnlocked( data, size ) )
        {
            ret = SetResult::NoChange;
    /*->*/  break;
        }
        // [===> Follows: Data is different]

        bool to_be_used_heap = ( size > INLINE_SIZE );
        bool need_allocation = ( to_be_used_heap && ( size > size_ ) );

        if ( !to_be_used_heap )
        {
            cleanup();
            std::memcpy( raw_data_, data, size );
        }
        else
        {
            if ( need_allocation )
            {
                void *allocated = heap_caps_malloc( size, MALLOC_CAP_DEFAULT );
                if ( !allocated )
                {
                    ret = SetResult::OutOfMemory;
    /*->*/          break;
                }

                cleanup();
                std::uintptr_t addr = std::bit_cast<std::uintptr_t>( allocated );
                std::memcpy( raw_data_, &addr, INLINE_SIZE );
            }

            std::memcpy( heapPointerAsVoid(), data, size );
        }

        size_ = size;
        ret = SetResult::Success;

    } while ( false );

    return ret;
}

// ----- Private member methods -----

void Value255::lock() const noexcept
{
    while( lock_.exchange( true, std::memory_order_acquire ) )
    {
        /* Yield to other tasks while waiting for lock */
        taskYIELD();
    }
}

void Value255::unlock() const noexcept
{
    lock_.store( false, std::memory_order_release );
}

void Value255::cleanup() noexcept
{
    // [===> Prerequisite: This instance is locked]

    if ( isHeapAllocated() ) { heap_caps_free( heapPointerAsVoid() ); }
    std::memset( raw_data_, 0, INLINE_SIZE );
    size_ = 0;
}

void Value255::moveFrom( Value255 &&other ) noexcept
{
    // [===> Prerequisite: This and other instance are locked]
    // [===> Prerequisite: This instance has been cleaned up]

    size_ = other.size_;
    std::memcpy( raw_data_, other.raw_data_, INLINE_SIZE );
    // [===> Follows: This instance has been data copied]

    std::memset( other.raw_data_, 0, INLINE_SIZE );
    other.size_ = 0U;
    // [===> Follows: Other instance has been cleaned up]
}

std::uintptr_t Value255::heapPointerAsUint() const noexcept
{
    std::uintptr_t ptr = 0U;
    std::memcpy( &ptr, raw_data_, INLINE_SIZE );
    return ptr;
}

std::byte *Value255::heapPointerAsByte() const noexcept
{
    return std::bit_cast<std::byte *>( heapPointerAsUint() );
}

void *Value255::heapPointerAsVoid() const noexcept
{
    return std::bit_cast<void *>( heapPointerAsUint() );
}

std::byte const *Value255::dataUnlocked() const noexcept
{
    return isHeapAllocated() ? heapPointerAsByte() : raw_data_;
}

bool Value255::areEqualsUnlocked(
    std::byte const *other_data, std::uint8_t other_size ) const noexcept
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
