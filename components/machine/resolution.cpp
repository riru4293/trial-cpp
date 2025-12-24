#include "resolution_impl.hpp"
#include <resolution.hpp>

#include <cmath>
#include <ostream>

using namespace machine;

int8_t constexpr Resolution::shift_of( Resolution::Kind v ) noexcept
{
    /* 2-bit signed (two's complement) */
    auto raw = static_cast<uint8_t>( v );
    auto bit2_bit1 = ( raw >> 1 ) & 0b11; // signed 2-bit
    return bit2_bit1 - 2;
}
uint8_t constexpr Resolution::coeff_of( Resolution::Kind v ) noexcept
{
    auto raw = static_cast<uint8_t>( v );
    auto bit0 = raw & 0b1;
    return bit0 ? 5U : 1U;
}

std::string_view constexpr Resolution::name_of( Resolution::Kind v ) noexcept
{
    auto idx = static_cast<uint8_t>( v );
    assert( idx < detail::RESOLUTION_NAMES.size() );
    return detail::RESOLUTION_NAMES[idx];
}

double Resolution::scale_factor( Resolution::Kind v ) noexcept
{
    double const coeff = static_cast<double>( coeff_of( v ) );
    int8_t const shift = shift_of( v );

    return coeff * std::pow( 10.0, -shift );
}

std::ostream &operator<<(std::ostream &os, const Resolution::Kind &v)
{
    return os << std::format( "{}", v );
}
