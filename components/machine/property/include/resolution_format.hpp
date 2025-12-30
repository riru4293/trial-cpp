#pragma once

/* Self */
#include <resolution.hpp>

/* C++ Standard Library */
#include <format>

namespace std {

    /** @brief Formatter specialization for `machine::property::Resolution::Kind`. */
    /**
     * @details
     * Formats a `machine::property::Resolution::Kind` value. Examples are follows:
     *
     * - `Kind::X1   `: `x1(0)`
     * - `Kind::X0_01`: `x0.01(4)`
     * - `Kind::X0_5 `: `x0.5(7)`
     */
    template <>
    struct formatter<machine::property::Resolution::Kind>
    {
        using Resolution = machine::property::Resolution;

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

        /** @brief Format `Resolution::Kind` value. */
        /**
         * @param v   [in]     The `Resolution::Kind` value to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( Resolution::Kind const &v, FormatContext &ctx ) const
        {
            std::string str = Resolution::strOf( v );
            
            for ( char c : str )
            {
                *ctx.out()++ = c;
            }

            return ctx.out();
        }
    };

} // namespace std
