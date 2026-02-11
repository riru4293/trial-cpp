/* Self */
#include <property.hpp>


/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine;
using namespace machine::property;


std::optional<Value255> create( std::span<std::byte const> bytes ) noexcept
{
    static uint8_t constexpr HEADER_SIZE = 2U; // code(1byte) + payload_size(1byte)

    if ( bytes.size() < HEADER_SIZE )
    {
        return std::nullopt;
    }
    /* [===> Follows: Contains header in `bytes` argument
                      (ja: 引数 `bytes` にヘッダーが含まれている）] */

    uint8_t code = static_cast<uint8_t>( bytes.at( 0 ) );
    uint8_t payload_size = static_cast<uint8_t>( bytes.at( 1 ) );

    if ( payload_size > ( UINT8_MAX - HEADER_SIZE ) )
    {
        return std::nullopt;
    }
    /* [===> Follows: Payload size is below limit
                      (ja: ペイロードサイズが制限内である）] */

    uint8_t total_size = payload_size + HEADER_SIZE;

    if ( bytes.size() != total_size )
    {
        return std::nullopt;
    }
    /* [===> Follows: Valid size of `bytes` argument
                      (ja: 引数 `bytes` のサイズが正しい）] */

    std::span<std::byte const> payload = bytes.subspan( HEADER_SIZE );
    switch ( payload.at( 0 ) )
    {
        case std::byte{ 0x00 }:
        case std::byte{ 0x01 }:
        case std::byte{ 0x02 }:
        case std::byte{ 0x03 }:
        case std::byte{ 0x04 }:
            break;

        default:
            return std::nullopt;
    }

    return Value255::create( bytes.data(), static_cast<std::uint8_t>( bytes.size() ) );
}
/* ^\__________________________________________ */
/* #region Operators.                           */

namespace machine
{
    std::ostream &operator<<( std::ostream &os, Property const &v ) noexcept
    {
        return os << v.str();
    }
}

/* #endregion */// Operators.

Property::SetResult Property::setValue( std::byte const *data, std::uint8_t size ) noexcept
{
    if ( isWritable() )
    {
        if ( isValidValue( data, size ) )
        { // SetResult::Success or SetResult::NoChange
            return updateValue( data, size );
        }
        else
        {
            return SetResult::IllegalArgument;
        }
    }
    else
    {
        return SetResult::Forbidden;
    }
}



bool Property::isWritable() noexcept
{
    property::Permission::Kind const permission = spec_.permission();

    return ( ( permission == property::Permission::Kind::WriteOnly )
          || ( permission == property::Permission::Kind::ReadWrite ) );
}

bool Property::isValidValue( std::byte const *data, std::uint8_t size ) noexcept
{
    return spec_.isWithinRange( data, size );
}

Property::SetResult Property::updateValue( std::byte const *data
                                         , std::uint8_t size ) noexcept
{
    Value::SetResult const ans = value_.setWithResult( data, size );

    switch( ans )
    {
        case Value::SetResult::Success:
            return SetResult::Success;

        case Value::SetResult::NoChange:
            return SetResult::NoChange;

        default:
            return SetResult::InternalError;
    }
}