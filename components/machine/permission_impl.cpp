/* Self */
#include "permission_impl.hpp"
#include <permission.hpp>

/* C++ Standard Library */
#include <format>
#include <ostream>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace machine;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Permission::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

std::string_view constexpr Permission::name_of( Permission::Kind v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );

    return ( idx < detail::PERMISSION_NAMES.size() )
        ? detail::PERMISSION_NAMES[idx]
        : "Unknown";
}

/* #endregion */// Public methods.