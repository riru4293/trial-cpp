/* Self */
#include "format_impl.hpp"
#include <format_format.hpp>

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Format::Kind const &v )
{
    return os << std::format( "{}", v );
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

    if ( ( min_size == 1U ) && ( min.bytes().at( 0U ) == MIN_BOOL_VALUE ) &&
         ( max_size == 1U ) && ( max.bytes().at( 0U ) == MAX_BOOL_VALUE ) )
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

/* #endregion */// Public methods.
