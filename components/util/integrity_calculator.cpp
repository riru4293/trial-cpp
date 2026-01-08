/* Self */
#include <integrity_calculator.hpp>

using namespace util;


std::uint8_t constexpr IntegrityCalculator::computeCrc8(
    std::byte const *data, std::size_t size ) noexcept
{
    std::uint8_t crc = 0U;

    for ( std::size_t i = 0U; i < size; i++ )
    {
        std::uint8_t const in = std::to_integer<std::uint8_t>( data[i] );
        crc ^= in;

        for ( int b = 0; b < 8; b++ )
        {
            bool const msb = ( ( crc & 0x80U ) != 0 );
            unsigned v = static_cast<unsigned>( crc ) << 1;
            if ( msb )
            {
                v ^= 0x07U;
            }
            crc = static_cast<std::uint8_t>( v & 0xFFU );
        }
    }

    return crc;
}
