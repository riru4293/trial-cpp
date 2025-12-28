/* Self */
#include "permission_impl.hpp"
#include <permission.hpp>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace machine::property;


/* ^\__________________________________________ */
/* #region Public methods.                      */

std::string_view constexpr Permission::nameOf( Kind const &v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );
    auto const &names = detail::PERMISSION_KIND_NAMES;

    return ( idx < names.size() ) ? names[idx] : "Unknown";
}

/* #endregion */// Public methods.


/* ^\__________________________________________ */
/* #region Formatter.                           */

namespace std
{

    template <>
    struct formatter<machine::property::Permission::Kind>
    {
        using Permission = machine::property::Permission;

        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::property::Permission::Kind` value. */
        template <typename FormatContext>
        auto format( Permission::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}({})",
                Permission::nameOf( v ), static_cast<int>( v ) );
        }
    };

} // namespace std

/* #endregion */// Formatter.


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Permission::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.
