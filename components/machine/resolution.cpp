#include "resolution_impl.hpp"
#include <resolution.hpp>

#include <ostream>

using namespace machine;

int8_t constexpr Resolution::shift_of( Resolution::Kind v ) noexcept
{
    /* 2-bit signed (two's complement) */
    return static_cast<int8_t>(
        ( static_cast<uint8_t>( v ) >> 1 ) & 0b11 ) - 2;
}
uint8_t constexpr Resolution::coeff_of( Resolution::Kind v ) noexcept
{
    return static_cast<uint8_t>(
        ( static_cast<uint8_t>( v ) & 0b1 ) ? 5 : 1 );
}

std::string_view constexpr Resolution::name_of( Resolution::Kind v ) noexcept
{
    return detail::RESOLUTION_NAMES[static_cast<uint8_t>( v )];
}

std::ostream &operator<<(std::ostream &os, const Resolution::Kind &v)
{
    os << Resolution::name_of(v) << "(" << static_cast<int>(v) << ")";
    return os;
}
