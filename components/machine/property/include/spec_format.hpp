#pragma once

/* Self */
#include <spec.hpp>

/* C++ Standard Library */
#include <format>

namespace std
{

    /** @brief Formatter specialization for `machine::property::Spec`. */
    /**
     * @details
     * Formats a `machine::property::Spec` instance. Examples are follows:
     *
     * - A `Spec` with format=`Numeric`, permission=`ReadWrite`, resolution=`X1`,
     *   initial_value=`10`, minimum_value=`0`, maximum_value=`1024`
     *   will be formatted as:
     *   `{ format: numeric(0), permission: read-write(3), resolution: x1(0),
     *     initial_value: [ 0x0A ], minimum_value: [ 0x00 ], maximum_value: [ 0x00 0x04 ] }`
     */
    template <>
    struct formatter<machine::property::Spec>
    {
        using Spec = machine::property::Spec;

        /** @brief Parse format specifiers (none supported). */
        /**
         * @param ctx [in,out] The format parse context.
         *
         * @return Iterator pointing to the next character to be parsed
         *         (no specifiers are consumed).
         */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `Spec` value. */
        /**
         * @param v   [in]     The `Spec` value to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( Spec const &v, FormatContext &ctx ) const
        {
            std::string str = v.str();
            
            for ( char c : str )
            {
                *ctx.out()++ = c;
            }

            return ctx.out();
        }
    };

} // namespace std
