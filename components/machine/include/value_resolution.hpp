#pragma once

#include <cstdint>

namespace machine {

    /** @brief A mask to extract the @ref Resolution ​​from the `uint8_t`. */
    static constexpr std::uint8_t RESOLUTION_MASK = 0b111;

    /** @brief The %Resolution of the number. */
    /** 
        * @details %Resolution encoding (3-bit).
        * - bit0: coefficient ( 0=x1, 1=x5 )
        * - bit1,2: signed shift amount N (2's complement)
        *   - `0b00 = +0 (10^+0)`
        *   - `0b01 = +1 (10^+1)`
        *   - `0b10 = -2 (10^-2)`
        *   - `0b11 = -1 (10^-1)`
        * 
        * @par Examples:
        * - `ShiftR1x5 (0b011) = 10^+1 × 5 = 50`
        * - `ShiftL2x1 (0b100) = 10^-2 × 1 = 0.01`
        */
    enum class Resolution : std::uint8_t {
        ShiftR0x1 = 0b0000, //!< `10^+0 x1 = x1`
        ShiftR0x5 = 0b0001, //!< `10^+0 x5 = x5`
        ShiftR1x1 = 0b0010, //!< `10^+1 x1 = x10`
        ShiftR1x5 = 0b0011, //!< `10^+1 x5 = x50`
        ShiftL2x1 = 0b0100, //!< `10^-2 x1 = x0.01`
        ShiftL2x5 = 0b0101, //!< `10^-2 x5 = x0.05`
        ShiftL1x1 = 0b0110, //!< `10^-1 x1 = x0.1`
        ShiftL1x5 = 0b0111, //!< `10^-1 x5 = x0.5`
    };

    // /** @brief Helper to get name of the @ref Resolution. */
    // struct ResolutionName
    // {
    //     /**
    //         * @brief Convert `machine::PropertySpec::Resolution` enum value to name.
    //         * @param v value to convert
    //         * @return name string view
    //         */
    //     static constexpr std::string_view of(Resolution v) noexcept {
    //         using T = Resolution;
    //         switch (v) {
    //             case T::ShiftR0x1:  return "ShiftR0x1";
    //             case T::ShiftR0x5:  return "ShiftR0x5";
    //             case T::ShiftR1x1:  return "ShiftR1x1";
    //             case T::ShiftR1x5:  return "ShiftR1x5";
    //             case T::ShiftL2x1:  return "ShiftL2x1";
    //             case T::ShiftL2x5:  return "ShiftL2x5";
    //             case T::ShiftL1x1:  return "ShiftL1x1";
    //             case T::ShiftL1x5:  return "ShiftL1x5";
    //             default:            return "Unknown";
    //         }
    //     }
    // };

    // std::ostream &operator<<(std::ostream &os, const PropertySpec::Resolution &v)
    // {
    //     os << machine::PropertySpec::ResolutionName::of(v) << "(" << static_cast<int>(v) << ")";
    //     return os;
    // }

} // namespace machine

// namespace std {

// #pragma region : formatter specialization
//     /** @brief Formatter specialization for `machine::PropertySpec`. */
//     /**
//      * @details
//      * Formats a `PropertySpec` instance. Examples are follows:
//      * - Numeric: `PropertySpec{valueKind=Numeric, permission=ReadOnly, resolution=ShiftL1x5, initVal=0, minVal=-3, maxVal=3}`
//      * - Boolean: `PropertySpec{valueKind=Boolean, permission=ReadWrite, resolution=ShiftR0x1, initVal=false, minVal=0, maxVal=1}`
//      * @note This specialization is necessary because user-defined
//      * types do not have a default formatter in the standard library.
//      */
//     template <>
//     struct formatter<machine::PropertySpec> {
//         /** @brief Parse format specifiers (none supported). */
//         constexpr auto parse(std::format_parse_context &ctx) {
//             return ctx.begin();
//         }

//         /** @brief Format `machine::Component` value. */
//         /**
//          * @details
//          * Formats a `Component` instance to `Component{code=Number, index=Number}`.
//          */
//         template <typename FormatContext>
//         auto format(const machine::Property &v, FormatContext &ctx) const {
//             return std::format_to(ctx.out(), "Property{{code={}, index={}, level={}}}",
//                 v.code(), static_cast<int>(v.index()), static_cast<int>(v.level()));
//         }
//     };

// #pragma endregion

// } // namespace std
