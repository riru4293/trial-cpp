#pragma once

/* Self */
#include <format.hpp>

/* C++ Standard Library */
#include <format>

namespace std // Formatter specialization
{

    /** @brief Formatter specialization for `machine::property::Format::Kind`. */
    /**
     * @details
     * Formats a `machine::property::Format::Kind` value. Examples are follows:
     *
     * - `Kind::Numeric`: `numeric(0)`
     * - `Kind::Boolean`: `boolean(1)`
     * - `Kind::BitSet `: `bitset(2)`
     * - `Kind::String `: `string(3)`
     */
    template <>
    struct formatter<machine::property::Format::Kind>
    {
        using Format = machine::property::Format;

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

        /** @brief Format `Format::Kind` value. */
        /**
         * @param v   [in]     The `Format::Kind` value to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( Format::Kind const &v, FormatContext &ctx ) const
        {
            std::string str = Format::strOf( v );
            
            for ( char c : str )
            {
                *ctx.out()++ = c;
            }

            return ctx.out();
        }
    };

} // namespace std
