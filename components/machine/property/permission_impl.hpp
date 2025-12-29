#pragma once

/* Self */
#include <permission.hpp>

/* C++ Standard Library */
#include <array>

namespace machine::property::detail
{

    /** @brief The number of bits used to represent
     *         @ref machine::property::Permission::Kind. */
    constexpr std::uint8_t PERMISSION_KIND_BITS = 2U;

    /** @brief A mask to extract the @ref machine::property::Permission::Kind
     *         ​​from the `std::uint8_t`. */
    constexpr std::uint8_t PERMISSION_KIND_MASK = ( 1U << PERMISSION_KIND_BITS ) - 1U;

    /** @brief The names of each @ref machine::property::Permission::Kind ​​value. */
    constexpr std::array<std::string_view, 4U> PERMISSION_KIND_NAMES =
    {
        "none",       "write-only",
        "read-only",  "read-write",
    };

} // namespace machine::property::detail
