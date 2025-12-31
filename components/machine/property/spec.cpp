/* Self */
#include <spec.hpp>

/* C++ Standard Library */
#include <cstring>
#include <sstream>
#include <utility>
#include <vector>

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;
using namespace machine::property::detail;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Spec const &v ) noexcept
{
    return os << v.str();
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Factory methods, Constructors.       */

std::optional<Spec> Spec::create( Permission::Kind permission
                                , Resolution::Kind resolution
                                , std::byte const *init_val
                                , std::uint8_t init_size
                                , std::byte const *min_val
                                , std::uint8_t min_size
                                , std::byte const *max_val
                                , std::uint8_t max_size ) noexcept
{
    auto init = Value::create( init_val, init_size );
    auto min  = Value::create( min_val, min_size );
    auto max  = Value::create( max_val, max_size );

    return create( permission
                  , resolution
                  , std::move( init )
                  , std::move( min )
                  , std::move( max ) );
}

std::optional<Spec> Spec::create( Permission::Kind permission
                                , Resolution::Kind resolution
                                , Value const &init_val
                                , Value const &min_val
                                , Value const &max_val ) noexcept
{
    auto cloned_init = init_val.clone();
    auto cloned_min = min_val.clone();
    auto cloned_max = max_val.clone();

    return create( permission
                  , resolution
                  , std::move( cloned_init )
                  , std::move( cloned_min )
                  , std::move( cloned_max ) );
}

std::optional<Spec> Spec::create( Permission::Kind permission
                                , Resolution::Kind resolution
                                , std::optional<Value> &&init
                                , std::optional<Value> &&min
                                , std::optional<Value> &&max ) noexcept
{
    if ( init.has_value() &&
         min.has_value()  &&
         max.has_value() )
    {
        return std::optional<Spec>{
            Spec{
                {
                    static_cast<std::uint8_t>(
                        Format::fromValueRange( min.value(), max.value() ) ),
                    static_cast<std::uint8_t>( permission ),
                    static_cast<std::uint8_t>( resolution ),
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
/* #region Public methods.                      */

bool Spec::isWithinRange( Value const &v ) const noexcept
{
    std::uint8_t const size = v.size();

    if ( size == 0U )
    {
        return false; // Note: Must not be empty.
    }

    std::vector<std::byte> const bytes = v.bytes();

    switch ( format() )
    {

    case Format::Kind::String:
        return ( size <= MAX_STRING_SIZE );

    case Format::Kind::BitSet:
        return ( size <= MAX_BITSET_SIZE );

    case Format::Kind::Boolean:
        { // Validate value is either 0x00 or 0x01.
            bool const is_valid_size = ( size == BOOL_SIZE );
            bool const is_false_value = ( bytes.at( 0U ) == BOOL_FALSE );
            bool const is_true_value  = ( bytes.at( 0U ) == BOOL_TRUE );

            return ( is_valid_size && ( is_false_value || is_true_value ) );
        }

    case Format::Kind::Numeric:
        if ( size <= MAX_NUMERIC_SIZE )
        { // Validate value is within the valid range.
            std::int32_t n = decodeNumericValue( v );
            std::int32_t min = decodeNumericValue( minVal() );
            std::int32_t max = decodeNumericValue( maxVal() );

            return ( ( min <= n ) && ( n <= max ) );
        }
        return false; // Note: Size exceeds maximum allowed.

    default:
        return false; // Note: Unknown format.

    } // switch ( format() )

    return false; // Note: Should not reach here.
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

std::int32_t Spec::decodeNumericValue( Value const &v ) const noexcept
{
    if ( ( v.size() == 0U ) || ( v.size() > MAX_NUMERIC_SIZE ) )
    {
        return 0;
    }
    // [===> Follows: Size is 1 to 4 bytes]

    std::vector<std::byte> const &bytes = v.bytes();
    std::int32_t val = 0;
    std::memcpy( &val, bytes.data(), bytes.size() );

    return val;
}

/* #endregion */// Private methods.