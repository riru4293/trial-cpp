#pragma once

/* C++ Standard Library */
#include <array>
#include <string_view>

namespace machine::property::detail
{

    /** @brief The names of each @ref Permission::Kind ​​value. */
    inline std::array<std::string_view, 4U> constexpr PERMISSION_KIND_NAMES =
    {
        "none",       "write-only",
        "read-only",  "read-write",
    };

} // namespace machine::property::detail
