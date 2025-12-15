#include <property_value.hpp>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <esp_heap_caps.h>
#include <compare>   // std::strong_ordering
#include <algorithm> // std::lexicographical_compare_three_way

using namespace machine;

PropertyValue const &PropertyValue::operator = ( PropertyValue &&other ) noexcept
{
    if ( this != &other ) {
        cleanup();
        moveFrom( std::move( other ) );
    }

    return *this;
}

//         /**
//          * @brief Equality operator.
//          * @details
//          * It does a deep comparison: exact content matches are considered equal.
//          * @param other The other PropertyValue to compare with.
//          * @return true if both PropertyValue instances are equal, false otherwise.
//          */
//         constexpr bool operator == ( const PropertyValue & other ) const noexcept
//         {
//             if ( size_ != other.size_ ) { return false; }
//             // [===> Follows: Sizes matched]

//             if ( size_ == 0 ) { return true; }
//             // [===> Follows: Sizes present]

//             bool isHeap = isHeapAllocated();

//             const std::byte* lhs = isHeap ? storage_.heap_ptr_ : storage_.inline_buffer_;
//             const std::byte* rhs = isHeap ? other.storage_.heap_ptr_ : other.storage_.inline_buffer_;

//             return std::equal( lhs, lhs + size_, rhs );
//         }   

//         /** 
//          * @brief Three-way comparison operator.
//          * @details
//          * First, size is compared, then content is compared lexicographically.
//          * @param other The other PropertyValue to compare with.
//          * @return std::strong_ordering indicating the comparison result.
//          */
//         constexpr auto operator <=> ( const PropertyValue &other) const noexcept
//         {
//             if ( size_ < other.size_ ) { return std::strong_ordering::less; }
//             if ( size_ > other.size_ ) { return std::strong_ordering::greater; }
//             // [===> Follows: Sizes matched]

//             return std::lexicographical_compare_three_way(
//                 data(), data() + size_,
//                 other.data(), other.data() + other.size_,
//                 std::compare_three_way() );
//         }
//         #pragma endregion
//     /* ^\__________________________________________ */
//     /* Instance members.                            */
//     public:
//         void set( std::byte const *data, std::uint8_t size ) noexcept
//         {
//             if ( size <= INLINE_SIZE )
//             {
//                 cleanup();
//                 std::copy_n( data, size, storage_.inline_buffer_ );
//             }
//             else
//             {
//                 if ( size > size_ )
//                 {
//                     cleanup();
//                     storage_.heap_ptr_ = static_cast<std::byte *>(
//                         heap_caps_malloc( size, MALLOC_CAP_DEFAULT ) );
//                 }

//                 std::copy_n( data, size, storage_.heap_ptr_ );
//             }

//             size_ = size;
//         }

//         std::byte const *data() const noexcept
//         {
//             return isHeapAllocated() ? storage_.heap_ptr_ : storage_.inline_buffer_;
//         }

//         std::uint8_t size() const noexcept { return size_; }
//     private:

//         std::uint8_t size_ = 0;

//         union Storage
//         {
//             std::byte inline_buffer_[INLINE_SIZE];
//             std::byte* heap_ptr_;

//             Storage() noexcept : inline_buffer_{} {}
//         } storage_;

//         bool isHeapAllocated() const noexcept { return size_ > INLINE_SIZE; }

//         void cleanup() noexcept
//         {
//             if ( isHeapAllocated() )
//             {
//                 heap_caps_free( storage_.heap_ptr_ );
//                 storage_.heap_ptr_ = nullptr;
//             }
//         }

//         void moveFrom( PropertyValue &&other ) noexcept
//         {
//             size_ = other.size_;

//             if ( isHeapAllocated() )
//             {
//                 storage_.heap_ptr_ = other.storage_.heap_ptr_;
//                 other.storage_.heap_ptr_ = nullptr;
//             }
//             else
//             {
//                 std::copy_n( other.storage_.inline_buffer_, size_, storage_.inline_buffer_ );
//             }

//             other.size_ = 0;
//         }
//     };


// };
