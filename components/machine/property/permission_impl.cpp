/* Self */
#include "permission_impl.hpp"

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Permission::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

Permission::Kind Permission::fromRaw( std::uint8_t const &raw ) noexcept
{
    std::uint8_t const v = raw & PERMISSION_KIND_MASK;
    return static_cast<Kind>( v );
}

std::string_view Permission::nameOf( Kind const &v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );
    auto const &names = PERMISSION_KIND_NAMES;

    return ( idx < names.size() ) ? names[idx] : "Unknown";
}

/* #endregion */// Public methods.
