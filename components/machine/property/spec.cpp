/* Self */
#include <spec.hpp>

/* C++ Standard Library */
#include <sstream>

/* ^\__________________________________________ */
/* Namespaces.                                  */
using namespace machine::property;


/* ^\__________________________________________ */
/* #region Operators.                           */

std::ostream &operator<<( std::ostream &os, Spec const &v ) noexcept
{
    return os << v.str();
}

/* #endregion */// Operators.


/* ^\__________________________________________ */
/* #region Public methods.                      */

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