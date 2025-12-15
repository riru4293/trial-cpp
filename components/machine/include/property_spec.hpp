#pragma once

#include <cstdint>
#include <ostream>
#include <format>
#include <optional>
#include <hash_util.hpp>
#include <integer_util.hpp>

namespace machine {

    /** @brief Represents a property specification. */
    /**
     * @details
     * @par Overview:
     * A property specification consists of the following items:
     * 1. Value kind
     * 2. Value access permission
     * 3. Value resolution
     * 4. Initial value
     * 5. Minimum value
     * 6. Maximum value
     * 
     * @par Value kind:
     * The value types are as follows; if no value exists, the value is set to 0 bytes.
     * | Hex | Kind    | Details                     | Minimum value | Maximum value |
     * | --- | ------- | --------------------------- | ------------- | ------------- |
     * | 0x0 | Illegal | Does not fall following     | -             | -             |
     * | 0x1 | Numeric | Signed 1-3 byte integer     | 1-3 byte      | 1-3 byte      |
     * | 0x2 | Boolean | 1 byte; 0=false, non-0=true | Fixed 0       | Fixed 1       |
     * | 0x3 | BitSet  | 1-3 byte                    | N/A           | 1-3 byte      |
     * | 0x4 | String  | 1-192 bytes                 | N/A           | N/A           |
     * 
     * @par Value access permission:
     * Indicates whether the property value is none, read-write, read-only or write-only in 2 bits, as follows:
     * | Bits    | Permission |
     * | ------- | ---------- |
     * | 0b00    | None       |
     * | 0b01    | Write-only |
     * | 0b10    | Read-only  |
     * | 0b11    | Read-write |
     * @note ja: プロパティ値のアクセス権限。2ビットで表現され、権限なし、読み書き可能、読み取り専用、書き込み専用を示します。
     *
     * @par Value resolution:
     * The resolution of the property value, expressed in 3 bits.
     * In addition this also applies to the it initial value, minimum value, and it maximum value.
     * This resolution has already been applied to their values.
     * There is no resolution for types other than numeric values.
     * The resolution is defined as follows:
     * | Hex | Bits  | Resolution         |
     * | --- | ----- | ------------------ |
     * | 0x0 | 0b000 | `10^0  x1 =   x1 ` |
     * | 0x1 | 0b001 | `10^0  x5 =   x5 ` |
     * | 0x2 | 0b010 | `10^1  x1 =   x10` |
     * | 0x3 | 0b011 | `10^1  x5 =   x50` |
     * | 0x4 | 0b100 | `10^-2 x1 = x0.01` |
     * | 0x5 | 0b101 | `10^-2 x5 = x0.05` |
     * | 0x6 | 0b110 | `10^-1 x1 = x0.1 ` |
     * | 0x7 | 0b111 | `10^-1 x5 = x0.5 ` |
     * 
     * For example, a value resolution of 0x7 indicates a resolution of 5 * 10^-1 = 0.5.
     * As a concrete example, to represent a temperature of 25.5°C,
     * the value would be 51 and the resolution would be 0.5.
     * @note ja: プロパティ値の解像度。3ビットで表現されます。さらに、これは初期値、最小値、最大値にも適用されます。
     * この解像度は、これらの値に既に適用されています。数値以外の型には解像度はありません。常に0になります。
     * 
     * @par Initial value:
     * Default value used when no specific property value is provided, but a valid value is required.
     * 0-3 byte value, 0 byte indicates no value. If this value is not present, it defaults to 0.
     * @note ja: 特定の値を設定できない場合でも、有効な値を必ず設定するための初期値。
     * この値が存在しない場合、初期値は0とします。
     * 
     * @par Minimum value:
     * The smallest possible value for the property value.
     * @note ja: プロパティ値の最小可能値。
     * 
     * @par Maximum value:
     * The largest possible value for the property value.
     * However, if it is a bitset, it is treated as a bitmask,
     * with all valid bits set to 1 and undefined bits set to 0.
     * @note ja: プロパティ値の最大可能値。
     * ただし、ビットセットの場合はビットマスクとして扱い、有効なビットをすべて1に、未定義のビットを0に設定します。
     * 
     * @par Hierarchy:
     * - Machine
     *   - `Unit[]` (unique: kind, index)
     *     - `Component[]` (unique: code, index)
     *       - `Property[]`  (unique: code)
     *         - `std::uint8_t code`
     *         - `std::array<std::byte, 3> value`
     *         - `PropertySpec`
     * @note This class is immutable; its state cannot be modified after construction.
     * @note This class is hashable; std::hash specialization is provided.
     * @note This class is comparable; supports equality and ordering operators.
     * @note This class is streamable; supports `operator<<`.
     */
    class PropertySpec
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    public:
        #pragma region : constants and types
        /** @brief The kind of the property value. */
        enum class ValueKind : std::uint8_t {
            Illegal, //!< Illegal value kind
            Numeric, //!< Signed 1-3 byte integer
            Boolean, //!< 1 byte; 0=false, non-0=true
            BitSet,  //!< 1-3 byte
            String,  //!< 1-192 bytes
        };

        /** @brief Helper to get name of @ref machine::PropertySpec::ValueKind. */
        struct ValueKindName
        {
            /**
             * @brief Convert `machine::PropertySpec::ValueKind` enum value to name.
             * @param v value to convert
             * @return name string view
             */
            static constexpr std::string_view of(ValueKind v) noexcept {
                using T = ValueKind;
                switch (v) {
                    case T::Illegal:    return "Illegal";
                    case T::Numeric:    return "Numeric";
                    case T::Boolean:    return "Boolean";
                    case T::BitSet:     return "BitSet";
                    case T::String:     return "String";
                    default:            return "Unknown";
                }
            }
        };

        /** @brief A mask to extract read permission value ​​from uint8_t value. */
        static constexpr std::uint8_t READ_PERMISSION_MASK = 0b00000010;

        /** @brief A mask to extract write permission value ​​from uint8_t value. */
        static constexpr std::uint8_t WRITE_PERMISSION_MASK = 0b00000001;

        /** @brief A mask to extract permission value ​​from uint8_t value. */
        static constexpr std::uint8_t PERMISSION_MASK = (READ_PERMISSION_MASK | WRITE_PERMISSION_MASK);

        /** @brief The permission for the property value access. */
        enum class Permission : std::uint8_t {
            None = 0b00,        //!< No access permission
            WriteOnly = 0b01,   //!< Write-only access permission
            ReadOnly  = 0b10,   //!< Read-only access permission
            ReadWrite = 0b11,   //!< Read-write access permission
        };

        /** @brief Helper to get name of @ref machine::PropertySpec::Permission. */
        struct PermissionName
        {
            /**
             * @brief Convert `machine::PropertySpec::Permission` enum value to name.
             * @param v value to convert
             * @return name string view
             */
            static constexpr std::string_view of(Permission v) noexcept {
                using T = Permission;
                switch (v) {
                    case T::None:       return "None";
                    case T::WriteOnly:  return "WriteOnly";
                    case T::ReadOnly:   return "ReadOnly";
                    case T::ReadWrite:  return "ReadWrite";
                    default:            return "Unknown";
                }
            }
        };

        /** @brief A mask to extract resolution value ​​from uint8_t value. */
        static constexpr std::uint8_t RESOLUTION_MASK = 0b00000111;

        /** @brief The resolution of the property value, initial value, minimum value, and maximum value. */
        /** 
         * @details Resolution encoding (3-bit).
         * - bit0: coefficient (0 = ×1, 1 = ×5)
         * - bit1,2: signed shift amount N (2's complement)
         *   - 0b00 = +0 (10^0)
         *   - 0b01 = +1 (10^1)
         *   - 0b10 = -2 (10^-2)
         *   - 0b11 = -1 (10^-1)
         * 
         * @par Examples:
         * - ShiftR1x5 (0b0011) → 10^+1 × 5 = 50
         * - ShiftL2x1 (0b0100) → 10^-2 × 1 = 0.01
         */
        enum class Resolution : std::uint8_t {
            ShiftR0x1 = 0b0000, //!< 10^0 x1 = x1
            ShiftR0x5 = 0b0001, //!< 10^0 x5 = x5
            ShiftR1x1 = 0b0010, //!< 10^1 x1 = x10
            ShiftR1x5 = 0b0011, //!< 10^1 x5 = x50
            ShiftL2x1 = 0b0100, //!< 10^-2 x1 = x0.01
            ShiftL2x5 = 0b0101, //!< 10^-2 x5 = x0.05
            ShiftL1x1 = 0b0110, //!< 10^-1 x1 = x0.1
            ShiftL1x5 = 0b0111, //!< 10^-1 x5 = x0.5
        };

        /** @brief Helper to get name of @ref machine::PropertySpec::Resolution. */
        struct ResolutionName
        {
            /**
             * @brief Convert `machine::PropertySpec::Resolution` enum value to name.
             * @param v value to convert
             * @return name string view
             */
            static constexpr std::string_view of(Resolution v) noexcept {
                using T = Resolution;
                switch (v) {
                    case T::ShiftR0x1:  return "ShiftR0x1";
                    case T::ShiftR0x5:  return "ShiftR0x5";
                    case T::ShiftR1x1:  return "ShiftR1x1";
                    case T::ShiftR1x5:  return "ShiftR1x5";
                    case T::ShiftL2x1:  return "ShiftL2x1";
                    case T::ShiftL2x5:  return "ShiftL2x5";
                    case T::ShiftL1x1:  return "ShiftL1x1";
                    case T::ShiftL1x5:  return "ShiftL1x5";
                    default:            return "Unknown";
                }
            }
        };

        static constexpr std::uint8_t MAX_VALUE_SIZE = 3;    // in bytes

        /** @brief Helper to get string representation of the property value. */
        struct PropertyValueString
        {
            /**
             * @brief Convert property value to string representation.
             * @param v value to convert
             * @return name string
             */
            static constexpr std::string of(const ValueKind &kind,
                const std::array<std::byte, MAX_VALUE_SIZE> &val,
                const std::uint8_t &size) noexcept
            {
                if (size == 0) return "N/A";

                switch (kind) {
                    case ValueKind::Numeric:
                        return std::to_string(util::toInt32LE(val));
                    case ValueKind::Boolean:
                        return (std::to_integer<uint8_t>(val.at(0)) > 0) ? "true" : "false";
                    case ValueKind::BitSet:
                        return std::to_string(util::toUInt32LE(val));
                    case ValueKind::Illegal:
                    case ValueKind::String:
                    default:
                        return "N/A";
                }
            }
        };

        /** @brief Create a PropertySpec instance with given parameters. */
        /**
         * @param permission Access permission. Must be one of Permission enum values.
         * @param resolution Value resolution. Must be one of Resolution enum values.
         * @param initVal Initial property value. Must be 0-3 bytes.
         * @param minVal Minimum property value. Must be 0-3 bytes.
         * @param maxVal Maximum property value. Must be 0-3 bytes.
         * @return PropertySpec instance if parameters are valid; std::nullopt otherwise.
         */
        static std::optional<PropertySpec> create(Permission permission, Resolution resolution,
            std::span<std::byte> initVal, std::span<std::byte> minVal, std::span<std::byte> maxVal) {

            // validate arguments
            if (initVal.size() <= MAX_VALUE_SIZE && minVal.size() <= MAX_VALUE_SIZE && maxVal.size() <= MAX_VALUE_SIZE) {

                PropertySpec instance(
                    Flagments{ static_cast<std::uint8_t>(permission),
                               static_cast<std::uint8_t>(resolution) },
                    ValueSizes{ static_cast<std::uint8_t>(initVal.size()),
                                static_cast<std::uint8_t>(minVal.size()),
                                static_cast<std::uint8_t>(maxVal.size()) },
                    initVal, minVal, maxVal
                );

                return std::optional<PropertySpec>( std::move(instance) );
            }

            return std::nullopt;
        }
        #pragma endregion
    private:
        struct Flagments {
            std::uint8_t permission : 2;
            std::uint8_t resolution : 3;
            const std::uint8_t reserved : 3 = 0;
        };

        struct ValueSizes {
            std::uint8_t initVal : 2;
            std::uint8_t minVal : 2;
            std::uint8_t maxVal : 2;
            const std::uint8_t reserved : 2 = 0;
        };
    /* ^\__________________________________________ */
    /* Constructors, Operators.                     */
    public:
        #pragma region : constructors
        ~PropertySpec() noexcept = default;                                         //!< Destructor (default).
        PropertySpec(const PropertySpec &) noexcept = default;                      //!< Copy constructor (default).
        PropertySpec(PropertySpec &&) noexcept = default;                           //!< Move constructor (default).
        #pragma endregion

        #pragma region : operators
        PropertySpec &operator=(const PropertySpec &) noexcept = default;           //!< Copy operator (default).
        PropertySpec &operator=(PropertySpec &&) noexcept = default;                //!< Move operator (default).
        constexpr bool operator==(const PropertySpec &) const noexcept = default;   //!< Equality operator (default).
        constexpr auto operator<=>(const PropertySpec &) const noexcept = default;  //!< Three-way comparison operator (default).
        #pragma endregion
    private:
        #pragma region : private constructor
        explicit constexpr PropertySpec(Flagments flags, ValueSizes sizes, std::span<std::byte> init,
            std::span<std::byte> min, std::span<std::byte> max) noexcept
            : flags_{flags}, valSizes_{sizes}, initVal_{}, minVal_{}, maxVal_{}
        {
            std::copy_n(init.begin(), init.size(), initVal_.begin());
            std::copy_n(min.begin(), min.size(), minVal_.begin());
            std::copy_n(max.begin(), max.size(), maxVal_.begin());
        }
        #pragma endregion
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        #pragma region : getter methods

        [[nodiscard]] ValueKind valueKind() const noexcept {
            bool hasMin = valSizes_.minVal > 0;
            bool hasMax = valSizes_.maxVal > 0;

            if ( !hasMin && !hasMax ) {
                return ValueKind::String;
            }
            else if ( hasMin && hasMax &&
                        (std::to_integer<uint8_t>(minVal_.at(0)) == 0) &&
                        (std::to_integer<uint8_t>(maxVal_.at(0)) == 1) ) {
                return ValueKind::Boolean;
            }
            else if ( !hasMin && hasMax && (util::toUInt32LE(maxVal_) > 0) ) {
                return ValueKind::BitSet;
            }
            else if ( hasMin && hasMax && (util::toInt32LE(maxVal_) >= util::toInt32LE(minVal_)) ) {
                return ValueKind::Numeric;
            }
            else {
                return ValueKind::Illegal;
            }
        }
        [[nodiscard]] const std::array<std::byte,3> initVal() const noexcept { return initVal_; }
        [[nodiscard]] const std::array<std::byte,3> minVal() const noexcept { return minVal_; }
        [[nodiscard]] const std::array<std::byte,3> maxVal() const noexcept { return maxVal_; }
        [[nodiscard]] const std::uint8_t initValSize() const noexcept { return valSizes_.initVal; }
        [[nodiscard]] const std::uint8_t minValSize() const noexcept { return valSizes_.minVal; }
        [[nodiscard]] const std::uint8_t maxValSize() const noexcept { return valSizes_.maxVal; }
        [[nodiscard]] Permission permission() const noexcept { return static_cast<Permission>(flags_.permission); }
        [[nodiscard]] Resolution resolution() const noexcept { return static_cast<Resolution>(flags_.resolution); }
        #pragma endregion
    private:
        #pragma region : member variables
        Flagments flags_;
        ValueSizes valSizes_;
        std::array<std::byte,3> initVal_;
        std::array<std::byte,3> minVal_;
        std::array<std::byte,3> maxVal_;
        #pragma endregion
    };
    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert( sizeof(machine::PropertySpec) == 11, "Unexpected PropertySpec size" );
    static_assert( alignof(machine::PropertySpec) == 1, "Unexpected PropertySpec alignment" );

    inline namespace ostream_support {
        std::ostream &operator<<(std::ostream &os, const PropertySpec::ValueKind &v)
        {
            os << machine::PropertySpec::ValueKindName::of(v) << "(" << static_cast<int>(v) << ")";
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const PropertySpec::Permission &v)
        {
            os << machine::PropertySpec::PermissionName::of(v) << "(" << static_cast<int>(v) << ")";
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const PropertySpec::Resolution &v)
        {
            os << machine::PropertySpec::ResolutionName::of(v) << "(" << static_cast<int>(v) << ")";
            return os;
        }

        std::ostream &operator<<(std::ostream &os, const PropertySpec &v)
        {
            os  << "PropertySpec{"
                            << "valueKind="  << v.valueKind()   // reuse `operator<<` for machine::PropertySpec::ValueKind
                    << ", " << "permission=" << v.permission()  // reuse `operator<<` for machine::PropertySpec::Permission
                    << ", " << "resolution=" << v.resolution()  // reuse `operator<<` for machine::PropertySpec::Resolution
                    << ", " << "initVal=" << machine::PropertySpec::PropertyValueString::of(
                                        v.valueKind(), v.initVal(), v.initValSize())
                    << ", " << "minVal=" << machine::PropertySpec::PropertyValueString::of(
                                        v.valueKind(), v.minVal(), v.minValSize())
                    << ", " << "maxVal=" << machine::PropertySpec::PropertyValueString::of(
                                        v.valueKind(), v.maxVal(), v.maxValSize())
                << "}";
            return os;
        }
    }
} // namespace machine

namespace std {

#pragma region : formatter specialization
    /** @brief Formatter specialization for `machine::PropertySpec`. */
    /**
     * @details
     * Formats a `PropertySpec` instance. Examples are follows:
     * - Numeric: `PropertySpec{valueKind=Numeric, permission=ReadOnly, resolution=ShiftL1x5, initVal=0, minVal=-3, maxVal=3}`
     * - Boolean: `PropertySpec{valueKind=Boolean, permission=ReadWrite, resolution=ShiftR0x1, initVal=false, minVal=0, maxVal=1}`
     * @note This specialization is necessary because user-defined
     * types do not have a default formatter in the standard library.
     */
    template <>
    struct formatter<machine::PropertySpec> {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        /** @brief Format `machine::Component` value. */
        /**
         * @details
         * Formats a `Component` instance to `Component{code=Number, index=Number}`.
         */
        template <typename FormatContext>
        auto format(const machine::Property &v, FormatContext &ctx) const {
            return std::format_to(ctx.out(), "Property{{code={}, index={}, level={}}}",
                v.code(), static_cast<int>(v.index()), static_cast<int>(v.level()));
        }
    };

#pragma endregion

} // namespace std
