/* Self */
#include <property.hpp>


/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine;
using namespace machine::property;


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