#pragma once

/* Self */
#include <format.hpp>

/* C++ Standard Library */
#include <array>

namespace machine::property::detail
{

    /** @brief Minimum byte value for boolean format. */
    constexpr std::byte MIN_BOOL_VALUE = std::byte{ 0x00 };

    /** @brief Maximum byte value for boolean format. */
    constexpr std::byte MAX_BOOL_VALUE = std::byte{ 0x01 };

    /** @brief Size for boolean format. */
    constexpr std::uint8_t BOOL_SIZE = 1U;

    /** @brief Maximum size for bitset format. */
    constexpr std::uint8_t MAX_BITSET_SIZE = 4U;

    /** @brief Maximum size for numeric format. */
    constexpr std::uint8_t MAX_NUMERIC_SIZE = 4U;

    /** @brief Maximum size for string format. */
    constexpr std::uint8_t MAX_STRING_SIZE = 192U;

    /** @brief The number of bits used to represent
     *         @ref machine::property::Format::Kind. */
    constexpr std::uint8_t FORMAT_KIND_BITS = 2U;

    /** @brief A mask to extract the @ref machine::property::Format::Kind
     *         ​​from the `std::uint8_t`. */
    constexpr std::uint8_t FORMAT_KIND_MASK = ( 1U << FORMAT_KIND_BITS ) - 1U;

    /** @brief The names of each @ref machine::property::Format::Kind ​​value. */
    constexpr std::array<std::string_view, 4U> FORMAT_KIND_NAMES =
    {
        "numeric", "boolean",
        "bitset",  "string",
    };

} // namespace machine::property::detail
