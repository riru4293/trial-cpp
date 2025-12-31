/* Self */
#include "format_impl.hpp"

/* C++ Standard Library */
#include <sstream>

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Format::Kind const &v )
{
    return os << Format::strOf( v );
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

Format::Kind Format::fromRaw( std::uint8_t raw ) noexcept
{
    std::uint8_t const v = raw & FORMAT_KIND_MASK;
    return static_cast<Kind>( v );
}

Format::Kind Format::fromValueRange( Value const &min, Value const &max ) noexcept
{
    auto min_size = min.size();
    auto max_size = max.size();

    if ( ( min_size == 0U ) && ( max_size == 0U ) )
    {
        return Kind::String;
    }

    if ( ( min_size == 0U ) && ( max_size != 0U ) )
    {
        return Kind::BitSet;
    }

    if ( ( min_size == BOOL_SIZE ) && ( min.bytes().at( 0U ) == BOOL_FALSE ) &&
         ( max_size == BOOL_SIZE ) && ( max.bytes().at( 0U ) == BOOL_TRUE ) )
    {
        return Kind::Boolean;
    }

    return Kind::Numeric;
}

std::string_view Format::nameOf( Kind const &v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );
    auto const &names = FORMAT_KIND_NAMES;

    return ( idx < names.size() ) ? names[idx] : "Unknown";
}

std::string Format::strOf( Kind const &v ) noexcept
{
    std::ostringstream oss;

    oss << nameOf( v ) << "(" << static_cast<unsigned>( v ) << ")";

    return oss.str();
}

/* #endregion */// Public methods.
