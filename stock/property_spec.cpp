#include <property_spec.hpp>


ValueKind PropertySpec::valueKind() const noexcept
{
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
