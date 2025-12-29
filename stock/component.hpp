#pragma once

#include <cstdint>
#include <ostream>
#include <format>

#include <hash_util.hpp>

namespace machine {

    /** @brief Represents a component within a machine unit. */
    /**
     * @details
     * @par Overview:
     * A Component represents a component within a machine unit.
     * For example, a board unit includes components such as
     * the CPU, memory, sound controller, BIOS, VRM, and clock generator.
     *
     * @par Identification:
     * Each component is identified by a code and an index.
     *
     * @par Indexing:
     * Index 0 is the primary component for each code; indices 1 and above are secondary components.
     *
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
    class Component
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    public:
        constexpr static std::uint8_t PRIMARY_IDX = 0; //!< Primary component index.
        constexpr static std::uint8_t ROOT_LEVEL = 0;   //!< Root component level.
    /* ^\__________________________________________ */
    /* Constructors, Operators.                     */
    public:
        /** @brief Construct with given code and index. */
        /**
         * @param code Component code.
         * @param index Component index. 0 is primary component. 1 or greater is sub components.
         * @note This constructor sets the component level to @ref ROOT_LEVEL.
         */
        explicit constexpr Component(const std::uint8_t code, const std::uint8_t index) noexcept
            : Component(code, index, ROOT_LEVEL) {}

        /** @brief Construct with given code, index, and level. */
        /**
         * @param code Component code.
         * @param index Component index. 0 is primary component. 1 or greater is sub components.
         * @param level Component level (hierarchical depth).
         */
        explicit constexpr Component(const std::uint8_t code, const std::uint8_t index, const std::uint8_t level) noexcept
            : code_(code), index_(index), level_(level) {}

        ~Component() noexcept = default;                            //!< Destructor (default).
        Component(const Component &) noexcept = default;            //!< Copy constructor (default).
        Component(Component &&) noexcept = default;                 //!< Move constructor (default).
        Component &operator=(const Component &) noexcept = delete;  //!< Copy operator (deleted).
        Component &operator=(Component &&) noexcept = delete;       //!< Move operator (deleted).
        constexpr bool operator==(const Component &) const noexcept = default;   //!< Equality operator (default).
        constexpr auto operator<=>(const Component &) const noexcept = default;  //!< Three-way comparison operator (default).
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        [[nodiscard]]
        constexpr std::uint8_t code() const noexcept { return code_; }

        [[nodiscard]]
        constexpr std::uint8_t index() const noexcept { return index_; }

        [[nodiscard]]
        constexpr std::uint8_t level() const noexcept { return level_; }

        [[nodiscard]]
        constexpr bool isPrimary() const noexcept { return index_ == PRIMARY_IDX; }

    private:
        const std::uint8_t code_;
        const std::uint8_t index_;
        const std::uint8_t level_;
    };

    inline namespace ostream_support {
        std::ostream &operator<<(std::ostream &os, const Component &v)
        {
            os  << "Component{"
                            << "code="  << static_cast<int>(v.code())
                    << ", " << "index=" << static_cast<int>(v.index())
                    << ", " << "level=" << static_cast<int>(v.level())
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
    struct hash<machine::Component> {
        std::size_t operator()(const machine::Component &v) const noexcept {
            std::size_t h1 = std::hash<int>{}(v.code());
            std::size_t h2 = std::hash<int>{}(v.index());
            return util::makeHash(h1, h2);
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
    struct formatter<machine::Component> {
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
        auto format(const machine::Component &v, FormatContext &ctx) const {
            return std::format_to(ctx.out(), "Component{{code={}, index={}, level={}}}",
                v.code(), static_cast<int>(v.index()), static_cast<int>(v.level()));
        }
    };

#pragma endregion

} // namespace std
