#pragma once

/* Self */
#include <resolution.hpp>

/* C++ Standard Library */
#include <array>

namespace machine::property::detail
{

    /** @brief The number of bits used to represent
     *         @ref machine::property::Resolution::Kind. */
    constexpr std::uint8_t RESOLUTION_KIND_BITS = 3U;

    /** @brief A mask to extract the @ref machine::property::Resolution::Kind
     *         ​​from the `std::uint8_t`. */
    constexpr std::uint8_t RESOLUTION_KIND_MASK = ( 1U << RESOLUTION_KIND_BITS ) - 1U;

    /** @brief The names of each @ref machine::property::Resolution::Kind ​​value. */
    constexpr std::array<std::string_view, 8U> RESOLUTION_KIND_NAMES =
    {
        "x1",       "x5",
        "x10",      "x50",
        "x0.01",    "x0.05",
        "x0.1",     "x0.5",
    };

} // namespace machine::property::detail
