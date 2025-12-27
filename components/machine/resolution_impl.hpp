#pragma once

/* C++ Standard Library */
#include <string_view>
#include <array>

namespace machine
{
    namespace detail
    {

        /** @brief The names of each @ref Resolution::Kind ​​value. */
        inline std::array<std::string_view, 8U> constexpr RESOLUTION_KIND_NAMES =
        {
            "x1",       "x5",
            "x10",      "x50",
            "x0.01",    "x0.05",
            "x0.1",     "x0.5",
        };

    } // namespace machine::detail
} // namespace machine
