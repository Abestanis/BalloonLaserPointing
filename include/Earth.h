/**
 * Constants related to Earth.
 */

#pragma once

#include "units.h"

/**
 * A container for Earth related constants.
 */
struct Earth {
    /** The radius of the Earth in meter. */
    static constexpr meter_t radius = meter_t(6378.137e3);
    /** The semi-major axis of an orbit describing the Earths shape. */
    static constexpr meter_t semiMajorAxis = radius;
    /** The semi-minor axis of an orbit describing the Earths shape. */
    static constexpr meter_t semiMinorAxis = meter_t(6356752.3142);
    /** The eccentricity of an orbit describing the Earths shape. */
    static const double eccentricity;
};
