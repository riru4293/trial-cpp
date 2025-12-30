#pragma once

/* Self */
#include <value255.hpp>

/* C++ Standard Library */
#include <string>

// to cpp
#include <format>


namespace std // Formatter specialization
{

    /** @brief Formatter specialization for `value::Value255`. */
    /**
     * @details
     * Formats a `value::Value255` instance. Examples are follows:
     *
     * - A `Value255` containing the bytes `0xA5, 0xE7, 0x00, 0xFF`
     *   will be formatted as: `[ 0xA5 0xE7 0x00 0xFF ]`
     * - A `Value255` containing the bytes `0x12, 0x34`
     *   will be formatted as: `[ 0x12 0x34 ]`
     * - An empty `Value255` (size 0) will be formatted as: `[ ]`
     *
     * @see Value255::str() for the format of the output.
     */
    template <>
    struct formatter<value::Value255>
    {
        /** @brief Parse format specifiers (no supported). */
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

        /** @brief Format the `value::Value255`. */
        /**
         * @param v   [in]     The `value::Value255` instance to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( value::Value255 const &v, FormatContext &ctx ) const noexcept
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
