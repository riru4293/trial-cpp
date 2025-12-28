#pragma once

/* C++ Standard Library */
#include <array>
#include <string_view>

namespace machine::property::detail
{

    /** @brief The names of each @ref machine::property::Format::Kind ​​value. */
    inline std::array<std::string_view, 4U> constexpr FORMAT_KIND_NAMES =
    {
        "numeric", "boolean",
        "bitset",  "string",
    };

} // namespace machine::property::detail
