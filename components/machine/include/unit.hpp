#pragma once

#include <cstdint>
#include <ostream>
#include <format>

#include <hash_util.hpp>

namespace machine {

    /** @brief Represents a machine unit. */
    /**
     * @details
     * @par Overview:
     * A Unit represents a component within a machine.
     * For example, Board Unit, Expansion Board Unit, Thermal Unit, Storage Unit, Power Unit, Light Unit.
     * 
     * @par Identification:
     * Each unit is identified by a kind and an index.
     *
     * @par Indexing:
     * Index 0 is the primary unit for each kind; indices 1 and above are secondary units.
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
    class Unit
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    public:
        constexpr static std::uint8_t PRIMARY_IDX = 0; //!< Primary unit index.

        /** @brief Device unit kind. */
        enum class Kind : std::uint8_t
        {
            Board,          //!< Board unit
            ExpansionBoard, //!< Expansion Board unit
            Thermal,        //!< Thermal unit
            Storage,        //!< Storage unit
            Power,          //!< Power unit
            Light,          //!< Light unit
        };

        /** @brief Helper to get name of @ref machine::Unit::Kind. */
        struct KindName
        {
            /**
             * @brief Convert `machine::Unit::Kind` enum value to name.
             * @param v value to convert
             * @return name string view
             */
            static constexpr std::string_view of(Kind v) noexcept {
                using T = Kind;
                switch (v) {
                    case T::Board:          return "Board";
                    case T::ExpansionBoard: return "ExpansionBoard";
                    case T::Thermal:        return "Thermal";
                    case T::Storage:        return "Storage";
                    case T::Power:          return "Power";
                    case T::Light:          return "Light";
                    default:                return "Unknown";
                }
            }
        };
    /* ^\__________________________________________ */
    /* Constructors, Operators.                     */
    public:
        /** @brief Construct with given kind and index. */
        /**
         * @param kind Unit kind.
         * @param index Unit index. 0 is primary unit. 1 or greater is sub units.
         */
        explicit constexpr Unit(const Kind kind, const std::uint8_t index) noexcept
            : kind_(kind), index_(index) {}

        ~Unit() noexcept = default;                       //!< Destructor (default).
        Unit(const Unit &) noexcept = default;            //!< Copy constructor (default).
        Unit(Unit &&) noexcept = default;                 //!< Move constructor (default).
        Unit &operator=(const Unit &) noexcept = delete;  //!< Copy operator (deleted).
        Unit &operator=(Unit &&) noexcept = delete;       //!< Move operator (deleted).
        constexpr bool operator==(const Unit &) const noexcept = default;   //!< Equality operator (default).
        constexpr auto operator<=>(const Unit &) const noexcept = default;  //!< Three-way comparison operator (default).
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        [[nodiscard]] constexpr Kind kind() const noexcept { return kind_; }
        [[nodiscard]] constexpr std::uint8_t index() const noexcept { return index_; }
        [[nodiscard]] constexpr bool isPrimary() const noexcept { return index_ == PRIMARY_IDX; }
    private:
        const Kind kind_;
        const std::uint8_t index_;
    };

    inline namespace ostream_support {
        std::ostream &operator<<(std::ostream &os, const Unit::Kind &v)
        {
            os << machine::Unit::KindName::of(v) << "(" << static_cast<int>(v) << ")";
            return os;
        }
    
        std::ostream &operator<<(std::ostream &os, const Unit &v)
        {
            os  << "Unit{"
                            << "kind="  << v.kind()   // reuse `operator<<` for machine::Unit::Kind
                    << ", " << "index=" << static_cast<int>(v.index())
                << "}";
            return os;
        }
    }

} // namespace machine

namespace std {

#pragma region : hash specialization

    /** @brief Hash specialization for `machine::Unit::Kind` enum. */
    /**
     * @details
     * This specialization enables hashing of the `Unit::Kind` enum by
     * converting the enum value to its underlying integral type and
     * applying the standard hash function for that type.
     * @note This specialization is necessary because enums do not
     * have a default hash function in the standard library.
     */
    template <>
    struct hash<machine::Unit::Kind> {
        using Underlying = std::underlying_type_t<machine::Unit::Kind>;

        std::size_t operator()(const machine::Unit::Kind &v) const noexcept {
            return std::hash<Underlying>{}(static_cast<Underlying>(v));
        }
    };

    /** @brief Hash specialization for `machine::Unit`. */
    /**
     * @details
     * This specialization enables hashing of the `Unit` class by
     * combining the hashes of its `kind` and `index` members.
     * It utilizes the `util::makeHash` function to combine the
     * individual hashes into a single hash value.
     * @note This specialization is necessary because user-defined
     * types do not have a default hash function in the standard library.
     * @see util::makeHash
     */
    template <>
    struct hash<machine::Unit> {
        std::size_t operator()(const machine::Unit &v) const noexcept {
            std::size_t h1 = std::hash<machine::Unit::Kind>{}(v.kind()); // reuse hash specialization for machine::Unit::Kind
            std::size_t h2 = std::hash<int>{}(v.index());
            return util::makeHash(h1, h2);
        }
    };

#pragma endregion

#pragma region : formatter specialization

    /** @brief Formatter specialization for `machine::Unit::Kind` enum. */
    /**
     * @details
     * Formats a `Unit::Kind` enum to `Name(Number)`.
     * @note This specialization is necessary because enums do not
     * have a default formatter in the standard library.
     */
    template <>
    struct formatter<machine::Unit::Kind> {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        /** @brief Format `machine::Unit::Kind` value. */
        /**
         * @details
         * Formats a `Unit::Kind` enum to `Name(Number)`.
         */
        template <typename FormatContext>
        auto format(const machine::Unit::Kind &v, FormatContext &ctx) const {
            return std::format_to(ctx.out(), "{}({})", machine::Unit::KindName::of(v), static_cast<int>(v));
        }
    };

    /** @brief Formatter specialization for `machine::Unit`. */
    /**
     * @details
     * Formats a `Unit` instance to `Unit{kind=Name(Number), index=Number}`.
     * @note This specialization is necessary because user-defined
     * types do not have a default formatter in the standard library.
     */
    template <>
    struct formatter<machine::Unit> {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse(std::format_parse_context &ctx) {
            return ctx.begin();
        }

        /** @brief Format `machine::Unit` value. */
        /**
         * @details
         * Formats a `Unit` instance to `Unit{kind=Name(Number), index=Number}`.
         */
        template <typename FormatContext>
        auto format(const machine::Unit &v, FormatContext &ctx) const {
            return std::format_to(ctx.out(), "Unit{{kind={}, index={}}}",
                v.kind(),   // reuse `formatter<machine::Unit::Kind>`
                static_cast<int>(v.index()));
        }
    };
    
#pragma endregion

} // namespace std
