#include <value_resolution.hpp>

#include <ostream>

std::ostream &operator<<(std::ostream &os, const machine::Resolution &v)
{
    os << machine::name_of(v) << "(" << static_cast<int>(v) << ")";
    return os;
}
