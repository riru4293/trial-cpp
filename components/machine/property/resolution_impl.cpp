/* Self */
#include "resolution_impl.hpp"
#include <resolution.hpp>

/* C++ Standard Library */
#include <cmath>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace machine::property;


/* ^\__________________________________________ */
/* #region Public methods.                      */

std::string_view constexpr Resolution::nameOf( Kind const &v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );
    auto const &names = detail::RESOLUTION_KIND_NAMES;

    return ( idx < names.size() ) ? names[idx] : "Unknown";
}

std::int8_t constexpr Resolution::shiftOf( Kind const &v ) noexcept
{
    auto raw = static_cast<std::uint8_t>( v ); // Note: 0 to 7

    std::uint8_t bit2_bit1 = static_cast<std::uint8_t>( ( raw >> 1 ) & 0b11 ); // Note: 0 to 3

    /* Note: Calculation details.
        + ------- + ------------------ + ---------------- + ------ +
        | Step0:  | Step1:             | Step2:           | Step3: |
        | Origin  | << 6 (as unsigned) | >> 6 (as signed) | Result |
        + ------- + ------------------ + ---------------- + ------ +
        |  0b'00  |      0b'0000'0000  |    0b'0000'0000  |    +0  |
        |  0b'01  |      0b'0100'0000  |    0b'0000'0001  |    +1  |
        |  0b'10  |      0b'1000'0000  |    0b'1111'1110  |    -2  |
        |  0b'11  |      0b'1100'0000  |    0b'1111'1111  |    -1  |
        + ------- + ------------------ + ---------------- + ------ +
    */
    return static_cast<std::int8_t>( bit2_bit1 << 6 ) >> 6; // Note: -2 to +1
}

std::uint8_t constexpr Resolution::coeffOf( Kind const &v ) noexcept
{
    auto raw = static_cast<std::uint8_t>( v );
    auto bit0 = raw & 0b1;
    return bit0 ? 5U : 1U; // 5 if bit0 == 1, 1 if bit0 == 0
}

double constexpr Resolution::scaleFactorOf( Kind const &v ) noexcept
{
    double coeff = static_cast<double>( coeffOf( v ) );
    std::int8_t shift = shiftOf( v );

    return coeff * std::pow( 10.0, shift );
}

/* #endregion */// Public methods.


/* ^\__________________________________________ */
/* #region Formatter.                           */

namespace std
{

    template <>
    struct formatter<machine::property::Resolution::Kind>
    {
        using Resolution = machine::property::Resolution;

        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::property::Resolution::Kind` value. */
        template <typename FormatContext>
        auto format( Resolution::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}({})",
                Resolution::nameOf( v ), static_cast<int>( v ) );
        }
    };

} // namespace std

/* #endregion */// Formatter.


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Resolution::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.
