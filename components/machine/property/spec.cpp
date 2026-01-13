/* Self */
#include "permission.hpp"
#include <spec.hpp>

/* C++ Standard Library */
#include <cstring>
#include <sstream>
#include <utility>


/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

namespace machine::property
{
    std::ostream &operator<<( std::ostream &os, Spec const &v ) noexcept
    {
        return os << v.str();
    }
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Factory methods, Constructors.       */

std::optional<Spec> Spec::create( std::uint8_t permission
                                , std::uint8_t resolution
                                , std::byte const *init_val
                                , std::uint8_t init_size
                                , std::byte const *min_val
                                , std::uint8_t min_size
                                , std::byte const *max_val
                                , std::uint8_t max_size ) noexcept
{
    Permission::Kind p_kind = Permission::fromRaw( permission );
    bool is_valid_permission = ( permission == static_cast<std::uint8_t>( p_kind ) );

    Resolution::Kind r_kind = Resolution::fromRaw( resolution );
    bool is_valid_resolution = ( resolution == static_cast<std::uint8_t>( r_kind ) );

    auto init = Value::create( init_val, init_size );
    auto min  = Value::create( min_val, min_size );
    auto max  = Value::create( max_val, max_size );

    if ( is_valid_permission &&
         is_valid_resolution &&
         init.has_value() &&
         min.has_value()  &&
         max.has_value() )
    {
        return std::optional<Spec>{
            Spec{
                {
                    static_cast<std::uint8_t>(
                        Format::fromValueRange( min.value(), max.value() ) ),
                    permission,
                    resolution,
                    0
                },
                std::move( init.value() ),
                std::move( min.value() ),
                std::move( max.value() )
            }
        };
    }

    return std::nullopt;
}

Spec::Spec( Fragments frags
          , Value &&init_val
          , Value &&min_val
          , Value &&max_val ) noexcept
    : frags_( frags )
    , initVal_( std::move( init_val ) )
    , minVal_( std::move( min_val ) )
    , maxVal_( std::move( max_val ) )
{ /* Do nothing */ }

/* #endregion */// Factory methods, Constructors.


/* ^\__________________________________________ */
/* #region Operators.                           */

bool constexpr Spec::operator==( Spec const &other ) const noexcept
{
    return ( ( frags_.format     == other.frags_.format )
          && ( frags_.permission == other.frags_.permission )
          && ( frags_.resolution == other.frags_.resolution )
          && ( initVal_          == other.initVal_ )
          && ( minVal_           == other.minVal_ )
          && ( maxVal_           == other.maxVal_ ) );
}

auto constexpr Spec::operator<=>( Spec const &other ) const noexcept
    ->std::strong_ordering
{
    using R = std::strong_ordering;

    Spec const &o = other;
    Fragments const &f = this->frags_;
    Fragments const &o_f = o.frags_;

    if ( R r = f.format     <=> o_f.format;     r != 0 ) return r;
    if ( R r = f.permission <=> o_f.permission; r != 0 ) return r;
    if ( R r = f.resolution <=> o_f.resolution; r != 0 ) return r;
    if ( R r = initVal_     <=> o.initVal_;     r != 0 ) return r;
    if ( R r = minVal_      <=> o.minVal_;      r != 0 ) return r;
         R r = maxVal_      <=> o.maxVal_;               return r;
}

/* #endregion */// Operators.

/* ^\__________________________________________ */
/* #region Public methods.                      */

bool Spec::areEquals( std::uint8_t permission
                      , std::uint8_t resolution
                      , std::byte const *init_val
                      , std::uint8_t init_size
                      , std::byte const *min_val
                      , std::uint8_t min_size
                      , std::byte const *max_val
                      , std::uint8_t max_size ) noexcept
{
    return ( ( frags_.permission == permission )
          && ( frags_.resolution == resolution )
          && ( initVal_.areEquals( init_val, init_size ) )
          && ( minVal_.areEquals(  min_val,  min_size  ) )
          && ( maxVal_.areEquals(  max_val,  max_size  ) ) );
}


bool Spec::isWithinRange( std::byte const *data, std::uint8_t size ) const noexcept
{
    bool is_null = ( data == nullptr );

    if ( size == 0U )
    {
        return is_null;
    }
    // [===> Follows: Size is non-zero]

    switch ( format() )
    {

    case Format::Kind::String:
        return !is_null && ( size <= MAX_STRING_SIZE );

    case Format::Kind::BitSet:
        return !is_null && ( size <= MAX_BITSET_SIZE );

    case Format::Kind::Boolean:
        if ( !is_null && ( size == BOOL_SIZE ) )
        { // Validate value is either 0x00 or 0x01.
            std::byte b = data[0];

            return ( ( b == BOOL_FALSE ) || ( b == BOOL_TRUE ) );
        }

        return false;

    case Format::Kind::Numeric:
        if ( !is_null && ( 0U < size ) && ( size <= MAX_NUMERIC_SIZE ) )
        { // Allow if value is within the valid range.
            std::int32_t n = decodeNumericValue( data, size );
            std::int32_t min = decodeNumericValue( minVal() );
            std::int32_t max = decodeNumericValue( maxVal() );

            return ( ( min <= n ) && ( n <= max ) );
        }

        return false;

    default: // No reached
        return false;

    } // switch ( format() )
}

std::string Spec::str() const noexcept
{
    std::ostringstream oss;

    oss << "{ format: "        << format()
        << ", permission: "    << permission()
        << ", resolution: "    << resolution()
        << ", initial_value: " << initVal_.str()
        << ", minimum_value: " << minVal_.str()
        << ", maximum_value: " << maxVal_.str()
        << " }";

    return oss.str();
}

/* #endregion */// Public methods.


/* ^\__________________________________________ */
/* #region Private methods.                     */

std::int32_t Spec::decodeNumericValue( std::byte const *data, std::uint8_t size ) const noexcept
{
    if ( ( size == 0U ) || ( size > MAX_NUMERIC_SIZE ) )
    {
        return 0;
    }
    // [===> Follows: Size is 1 to 4 bytes]

    std::int32_t val = 0;
    std::memcpy( &val, data, size );

    return val;
}

/* #endregion */// Private methods.