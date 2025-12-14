#pragma once

#include <cstdint>
#include <ostream>
#include <format>
// #include <vector>

#include <hash_util.hpp>

#include <property_spec.hpp>

namespace machine {

    class BooleanPropertySpec : public PropertySpec
    {
    /* ^\__________________________________________ */
    /* Static members, Inner types.                 */
    private:
        static constexpr std::array<std::byte, 1> MIN_VAL = {std::byte{false}};
        static constexpr std::array<std::byte, 1> MAX_VAL = {std::byte{true}};
    /* ^\__________________________________________ */
    /* Constructors, Operators.                     */
    public:
    /* ^\__________________________________________ */
    /* Instance members.                            */
    public:
        [[nodiscard]] std::span<const std::byte> initialValue( void ) const noexcept override { return initValue_; }
        [[nodiscard]] constexpr std::span<const std::byte> mininumValue( void ) const noexcept override { return MIN_VAL; };
        [[nodiscard]] constexpr std::span<const std::byte> maximumValue( void ) const noexcept override { return MIN_VAL; };
        [[nodiscard]] constexpr Resolution resolution() const noexcept override { return Resolution::Left0__Multi1;};
        [[nodiscard]] Permission permission() const noexcept override { return Permission::ReadWrite;};
    private:
        std::array<std::byte, 1> initValue_ = {std::byte{false}};
        Permission permission_ = Permission::None;
    };
    int a = sizeof(machine::BooleanPropertySpec);
    int b = alignof(machine::BooleanPropertySpec);
    int c = sizeof(std::array<std::byte, 1>);
    static_assert( sizeof(machine::BooleanPropertySpec) == 24, "Unexpected BooleanPropertySpec size" );
    static_assert( alignof(machine::BooleanPropertySpec) == 8, "Unexpected BooleanPropertySpec alignment" );

} // namespace machine
