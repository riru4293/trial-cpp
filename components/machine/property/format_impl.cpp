/* Self */
#include "format_impl.hpp"
#include <format.hpp>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace machine::property;


/* ^\__________________________________________ */
/* #region Public methods.                      */

Format::Kind constexpr Format::fromValueRange( Value const &min, Value const &max ) noexcept
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

    if ( ( min_size == 1U ) && ( min.bytes().at( 0U ) == BOOL_MIN ) &&
         ( max_size == 1U ) && ( max.bytes().at( 0U ) == BOOL_MAX ) )
    {
        return Kind::Boolean;
    }

    return Kind::Numeric;
}

std::string_view constexpr Format::nameOf( Kind const &v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );
    auto const &names = detail::FORMAT_KIND_NAMES;

    return ( idx < names.size() ) ? names[idx] : "Unknown";
}

/* #endregion */// Public methods.


/* ^\__________________________________________ */
/* #region Formatter.                           */

namespace std
{

    template <>
    struct formatter<machine::property::Format::Kind>
    {
        using Format = machine::property::Format;

        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::property::Format::Kind` value. */
        template <typename FormatContext>
        auto format( Format::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}({})",
                Format::nameOf( v ), static_cast<int>( v ) );
        }
    };

} // namespace std

/* #endregion */// Formatter.


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Format::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.
