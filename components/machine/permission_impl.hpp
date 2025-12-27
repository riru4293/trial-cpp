#pragma once

/* C++ Standard Library */
#include <string_view>
#include <array>

namespace machine
{
    namespace detail
    {

        /** @brief The names of each @ref Permission::Kind ​​value. */
        inline std::array<std::string_view, 4U> constexpr PERMISSION_KIND_NAMES =
        {
            "none",       "write-only",
            "read-only",  "read-write",
        };

    } // namespace machine::detail
} // namespace machine
