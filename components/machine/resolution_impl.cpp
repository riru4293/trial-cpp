/* Self */
#include "resolution_impl.hpp"
#include <resolution.hpp>

/* C++ Standard Library */
#include <cmath>
#include <format>
#include <ostream>


/* ^\__________________________________________ */
/* Namespaces.                                  */

using namespace machine;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Resolution::Kind const &v )
{
    return os << std::format( "{}", v );
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

std::int8_t constexpr Resolution::shift_of( Resolution::Kind v ) noexcept
{
    auto raw = static_cast<std::uint8_t>( v ); // Note: 0 to 7

    std::uint8_t bit2_bit1 = static_cast<std::uint8_t>( (raw >> 1) & 0b11 ); // Note: 0 to 3

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

std::uint8_t constexpr Resolution::coeff_of( Resolution::Kind v ) noexcept
{
    auto raw = static_cast<std::uint8_t>( v );
    auto bit0 = raw & 0b1;
    return bit0 ? 5U : 1U; // 5 if bit0 == 1, 1 if bit0 == 0
}

std::string_view constexpr Resolution::name_of( Resolution::Kind v ) noexcept
{
    auto idx = static_cast<std::uint8_t>( v );

    return ( idx < detail::RESOLUTION_KIND_NAMES.size() )
        ? detail::RESOLUTION_KIND_NAMES[idx]
        : "Unknown";
}

double constexpr Resolution::scale_factor( Resolution::Kind v ) noexcept
{
    double coeff = static_cast<double>( coeff_of( v ) );
    std::int8_t shift = shift_of( v );

    return coeff * std::pow( 10.0, shift );
}

/* #endregion */// Public methods.
