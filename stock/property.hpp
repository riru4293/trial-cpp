#pragma once

#include <cstdint>
#include <ostream>
#include <format>
#include <vector>

#include <hash_util.hpp>

namespace machine {

    /** @brief Represents a property within a machine unit component. */
    /**
     * @details
     * @par Overview:
     * Properties represent information about a machine unit component as key-value pairs.
     * For example, a BIOS component includes properties such as
     * the version, last updated time, and capacity.
     * 
     * @par Identification (Key):
     * Each property is identified by a code. 
     * 
     * @par Value:
     * A property value consists of a byte array of 1 byte or more and a value type.
     * Value types include bit array, number (signed 32-bit integer), string, and boolean.
     * The byte array is the little endian. e.g. [0x01, 0x1F] is 0x1F01.
     * The data length is 1 to 255 bytes for string and 1 to 4 bytes for others.
     * 
     * @par Metadata:
     * Properties have metadata for value, as follows:
     * 1. Reliability
     * 2. Minimum
     * 3. Maximum
     * 4. Step
     * 5. Permission
     * 
     * @par Reliability:
     * Indicates whether the property value is reliable in 1 bit.
     * 0 means the property value is not present, anything else means it is present.
     * 
     * @par Minimum:
     * The smallest possible value for the property value.
     * This value does not exists for bit array and string types, and is fixed at 0 for boolean.
     * The data length is 0 to 4 bytes. 0 means not exists.
     * 
     * @par Maximum:
     * The largest possible value for the property value.
     * This value does not exists for string types, and is fixed at 1 for boolean.
     * For bit array, this is the value when all the enabled bits are 1;
     * in this case the undefined bits are set to 0.
     * The data length is 0 to 4 bytes. 0 means not exists.
     * 
     * @par Step:
     * Resolution of the property value expressed in 4 bits.
     * Not present for types other than number.
     * 
     * The upper 1 bit represents a coefficient of 1 or 5, as follows:
     * | Bits    | Meaning  |
     * | ------- | -------- |
     * | 0b0000  | 1        |
     * | 0b1000  | 5        |
     * 
     * The lower 3 bits represents a coefficient, as follows:
     * | Bits    | Meaning  |
     * | ------- | -------- |
     * | 0b0000  | 10^0     |
     * | 0b0001  | 10^1     |
     * | 0b0010  | 10^2     |
     * | 0b0011  | 10^3     |
     * | 0b0100  | 10^-3    |
     * | 0b0101  | 10^-2    |
     * | 0b0110  | 10^-1    |
     * 
     * For example, a value step of 0xE indicates a resolution of 5 * 10^-1 = 0.5.
     * As a concrete example, to represent a temperature of 25.5°C,
     * the value would be 51 and the resolution would be 0.5.
     * 
     * @par Permission:
     * Indicates whether the property value is read-write,
     * read-only or write-only in 2 bits, as follows:
     * | Bits    | Meaning    |
     * | ------- | ---------- |
     * | 0b00    | read-write |
     * | 0b01    | read-only  |
     * | 0b10    | write-only |

     * @par Hierarchy:
     * - Machine
     *   - Unit[] (unique: kind, index)
     *     - Component[] (unique: code, index)
     *       - Property[]  (unique: code)
     * @note This class is immutable; its state cannot be modified after construction.
     * @note This class is hashable; std::hash specialization is provided.
     * @note This class is comparable; supports equality and ordering operators.
     * @note This class is streamable; supports `operator<<`.
     */
    class Property
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    private:
        struct Flagments {
            std::uint8_t code;                  // Range 0x00–0xFF
            // --- alignmrnt boundary ---
            std::uint8_t isReliable : 1;        // 0: unreliable, 1: reliable
            std::uint8_t permission : 2;        // 0: read-write, 1: read-only, 2: write-only
            std::uint8_t step : 4;              // Range 0–7
            std::uint8_t reserved : 1;          // padding
        };
        static_assert(sizeof(Flagments) == 2, "Unexpected Flagments size");
        static_assert(alignof(Flagments) == 1, "Unexpected Flagments alignment");

        struct ValueLengths {
            std::uint8_t valueLength : 3;       // Range 0–4
            std::uint8_t minimumValueLength : 3;// Range 0–4
            std::uint8_t reserved : 2;          // padding
            // --- alignment boundary ---
            std::uint8_t maximumValueLength : 3;// Range 0–4
            std::uint8_t reserved : 5;          // padding
        };
        static_assert(sizeof(ValueLengths) == 2, "Unexpected ValueLengths size");
        static_assert(alignof(ValueLengths) == 1, "Unexpected ValueLengths alignment");

        union OptionalValue {
            const bool isEmpty;
            const std::array<std::byte,4> data;

            constexpr OptionalValue() : isEmpty(0) {}
            constexpr OptionalValue(const std::array<std::byte,4>& bytes) : data(bytes) {}
        };
    static std::array<std::byte,4> toArray(std::span<const std::byte> s) {
        std::array<std::byte,4> arr{};
        auto nn = std::max<std::size_t>(s.size(), arr.size());
        auto n = std::min<std::size_t>(s.size(), arr.size());
        std::copy_n(s.begin(), n, arr.begin());
        return arr;
    }

    /* ^\__________________________________________ */
    /* Constructors, Operators.                     */
    public:
        /** @brief Construct with given code. */
        /**
         * @param code property code
         */
        // explicit constexpr Property(const std::uint8_t code, std::span<std::byte> value) noexcept
        //     : code_(code), value_(std::copy(value.begin(), value.end(), value_.begin())), valueLength_(), minimumValue_(), minimumValueLength_()
        //     , maximumValue_ (), maximumValueLength_ (), step_ (), permission_ () {}
        explicit constexpr Property(std::span<std::byte> minimum) noexcept
            : minimum_(!minimum.empty() ? OptionalValue(toArray(minimum)) : OptionalValue()) {}
    
        ~Property() noexcept = default;                           //!< Destructor (default).
        Property(const Property &) noexcept = default;            //!< Copy constructor (default).
        Property(Property &&) noexcept = default;                 //!< Move constructor (default).
        Property &operator=(const Property &) noexcept = delete;  //!< Copy operator (deleted).
        Property &operator=(Property &&) noexcept = delete;       //!< Move operator (deleted).
        constexpr bool operator==(const Property &) const noexcept = default;   //!< Equality operator (default).
        constexpr auto operator<=>(const Property &) const noexcept = default;  //!< Three-way comparison operator (default).
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        [[nodiscard]] constexpr std::uint8_t code() const noexcept { return code_; }
    private:
        // TODO PropertySpec
        std::array<std::byte, 4> value_;
        const OptionalValue minimumValue_;
        const OptionalValue maximumValue_;
        const Flagments flagments_{};
        const ValueLengths valueLengths_{};
        const std::uint8_t code_;
    };

    inline namespace ostream_support {
        std::ostream &operator<<(std::ostream &os, const Property &v)
        {
            os  << "Property{"
                            << "code="  << static_cast<int>(v.code())
                    // << ", " << "index=" << static_cast<int>(v.index())
                    // << ", " << "level=" << static_cast<int>(v.level())
                << "}";
            return os;
        }
    }

} // namespace machine

namespace std {

#pragma region : hash specialization

    /** @brief Hash specialization for `machine::Component`. */
    /**
     * @details
     * This specialization enables hashing of the `Component` class by
     * combining the hashes of its `code` and `index` members.
     * It utilizes the `util::makeHash` function to combine the
     * individual hashes into a single hash value.
     * @note This specialization is necessary because user-defined
     * types do not have a default hash function in the standard library.
     * @see util::makeHash
     */
    template <>
    struct hash<machine::Property> {
        std::size_t operator()(const machine::Property &v) const noexcept {
            std::size_t h1 = std::hash<int>{}(v.code());
            // std::size_t h2 = std::hash<int>{}(v.index());
            return util::makeHash(h1 /*, h2 */);
        }
    };

#pragma endregion

#pragma region : formatter specialization
    /** @brief Formatter specialization for `machine::Component`. */
    /**
     * @details
     * Formats a `Component` instance to `Component{code=Number, index=Number, level=Number}`.
     * @note This specialization is necessary because user-defined
     * types do not have a default formatter in the standard library.
     */
    template <>
    struct formatter<machine::Property> {
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
