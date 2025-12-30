#pragma once

/* Self */
#include <permission.hpp>

/* C++ Standard Library */
#include <format>

namespace std
{

    /** @brief Formatter specialization for `machine::property::Permission::Kind`. */
    /**
     * @details
     * Formats a `machine::property::Permission::Kind` value. Examples are follows:
     *
     * - `Kind::None     `: `none(0)`
     * - `Kind::WriteOnly`: `write-only(1)`
     * - `Kind::ReadOnly `: `read-only(2)`
     * - `Kind::ReadWrite`: `read-write(3)`
     */
    template <>
    struct formatter<machine::property::Permission::Kind>
    {
        using Permission = machine::property::Permission;

        /** @brief Parse format specifiers (none supported). */
        /**
         * @param ctx [in,out] The format parse context.
         *
         * @return Iterator pointing to the next character to be parsed
         *         (no specifiers are consumed).
         */
        constexpr auto parse( std::format_parse_context &ctx ) const noexcept
            -> const char *
        {
            return ctx.begin();
        }

        /** @brief Format `Permission::Kind` value. */
        /**
         * @param v   [in]     The `Permission::Kind` value to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( Permission::Kind const &v, FormatContext &ctx ) const
        {
            std::string str = Permission::strOf( v );
            
            for ( char c : str )
            {
                *ctx.out()++ = c;
            }

            return ctx.out();
        }
    };

} // namespace std
