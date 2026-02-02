#pragma once

/* Self */
#include <value255.hpp>

/* C++ Standard Library */
#include <format>
#include <string>


namespace std // Formatter specialization
{

    /** @brief Formatter specialization for `value::Value255`.
     *      @n （ja: `value::Value255` のフォーマッタ特殊化） */
    /**
     * @details
     * Formats a `value::Value255` instance. Examples are follows:
     * - A `Value255` containing the bytes `0xA5, 0xE7, 0x00, 0xFF`
     *   will be formatted as: `[ 0xA5 0xE7 0x00 0xFF ]`.
     * - A `Value255` containing the bytes `0x12, 0x34`
     *   will be formatted as: `[ 0x12 0x34 ]`.
     * - An empty `Value255` (size 0) will be formatted as: `[ ]`.
     *
     * ja: @n
     * `value::Value255` インスタンスをフォーマットします。 使用例は以下の通りです:
     * - バイト列が `0xA5, 0xE7, 0x00, 0xFF` の `Value255` は
     *   `[ 0xA5 0xE7 0x00 0xFF ]` とフォーマットされます。
     * - バイト列が `0x12, 0x34` の `Value255` は
     *   `[ 0x12 0x34 ]` とフォーマットされます。
     * - 空の `Value255` (サイズ 0) は `[ ]` とフォーマットされます。
     */
    template <>
    struct formatter<value::Value255>
    {
        /** @brief Parse format specifiers (no supported).
         *      @n （ja: フォーマット指定子の解析（未対応）） */
        /**
         * @param ctx [in,out] The format parse context.
         *                  @n （ja: フォーマット解析コンテキスト）
         *
         * @return Iterator pointing to the next character to be parsed (no specifiers are consumed).
         *      @n （ja: 次に解析される文字を指すイテレータ（指定子は消費されません））
         */
        char constexpr const *parse( std::format_parse_context &ctx ) const noexcept
        {
            return ctx.begin();
        }

        /** @brief Format the `value::Value255`.
         *      @n （ja: `value::Value255` のフォーマット） */
        /**
         * @param v [in] The `value::Value255` instance to format.
         *            @n （ja: フォーマットする `value::Value255` インスタンス）
         * @param ctx [in,out] The format context.
         *                  @n （ja: フォーマットコンテキスト）
         *
         * @return Iterator to the end of the formatted output.
         *      @n （ja: フォーマットされた出力の末尾を指すイテレータ）
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
