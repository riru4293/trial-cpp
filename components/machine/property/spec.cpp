/* Self */
#include "format_impl.hpp"
#include <cstdint>
#include <spec.hpp>

/* C++ Standard Library */
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <vector>

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Spec const &v ) noexcept
{
    return os << v.str();
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

bool Spec::isWithinRange( Value const &v ) const noexcept
{
    std::uint8_t const size = v.size();

    if ( size == 0U )
    {
        return false; // Note: Must not be empty.
    }

    std::vector<std::byte> const bytes = v.bytes();

    switch ( format() )
    {

    case Format::Kind::String:
        return ( size <= MAX_STRING_SIZE );

    case Format::Kind::BitSet:
        return ( size <= MAX_BITSET_SIZE );

    case Format::Kind::Boolean:
        { // Validate value is either 0x00 or 0x01.
            bool const is_valid_size = ( size == BOOL_SIZE );
            bool const is_false_value = ( bytes.at( 0U ) == MIN_BOOL_VALUE );
            bool const is_true_value  = ( bytes.at( 0U ) == MAX_BOOL_VALUE );

            return ( is_valid_size && ( is_false_value || is_true_value ) );
        }

    case Format::Kind::Numeric:
        if ( size <= MAX_NUMERIC_SIZE )
        { // Validate value is within the valid range.
            std::int32_t n = decodeNumericValue( v );
            std::int32_t min = decodeNumericValue( minVal() );
            std::int32_t max = decodeNumericValue( maxVal() );

            return ( ( min <= n ) && ( n <= max ) );
        }
        return false; // Note: Size exceeds maximum allowed.

    default:
        return false; // Note: Unknown format.

    } // switch ( format() )

    return false; // Note: Should not reach here.
}

std::string Spec::str() const noexcept
{
    std::ostringstream oss;

    oss << "{ format: "        << format()
        << ", permission: "    << permission()
        << ", resolution: "    << resolution()
        << ", initial_value: " << initVal_.str()
        << ", minimum_value: " << minVal_.str()
        << ", maximum_value: " << maxVal_.str()
        << " }";

    return oss.str();
}

/* #endregion */// Public methods.


/* ^\__________________________________________ */
/* #region Private methods.                     */

std::int32_t Spec::decodeNumericValue( Value const &v ) const noexcept
{
    if ( ( v.size() == 0U ) || ( v.size() > MAX_NUMERIC_SIZE ) )
    {
        return 0;
    }
    // [===> Follows: Size is 1 to 4 bytes]

    std::vector<std::byte> const &bytes = v.bytes();
    std::int32_t val = 0;
    std::memcpy( &val, bytes.data(), bytes.size() );

    return val;
}

/* #endregion */// Private methods.