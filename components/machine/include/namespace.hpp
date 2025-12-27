#pragma once

/**
 * @namespace machine
 * @brief Provides foundational functionality for communication with external machine.
 *
 * @details
 * This namespace defines abstractions and implementations used to identify,
 * access, and operate on external machine through a hierarchical addressing model.
 *
 * Interaction targets are represented by properties, which form the minimal
 * operational unit. Properties are addressed through a structured hierarchy
 * that enables clear identification without assigning functional behavior
 * to intermediate levels.
 *
 * The logical structure can be illustrated as follows:
 *
 * @code
 * machine
 *  |
 *  `-- unit[]
 *       |
 *       `-- component[]
 *            |
 *            `-- property[]
 * @endcode
 *
 * Only properties represent meaningful operation targets.
 * Intermediate levels (unit, component) exist solely as parts of an address
 * hierarchy and do not carry independent behavior.
 *
 * Classes that directly participate in communication, such as value
 * representations, address definitions, and transport mechanisms,
 * are expected to reside directly under this namespace.
 */
namespace machine
{
    /**
     * @namespace machine::property
     * @brief Groups concepts and operations related to machine properties.
     *
     * @details
     * A property represents the smallest meaningful unit of interaction
     * with an external machine. It encapsulates both identity and semantics,
     * such as readable or writable values.
     *
     * As property-related concepts tend to grow in number and variety,
     * this namespace provides a dedicated scope to organize types associated
     * with property values, addressing, encoding, and communication behavior.
     */
    namespace property
    {
    }
}
