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
    /**
     * The semi-major axis of an orbit describing the Earths shape.
     * Represents the equatorial radius in meters.
     */
    static constexpr meter_t semiMajorAxis = radius;
    /**
     * The semi-minor axis of an orbit describing the Earths shape.
     * Represents the polar radius in meters.
     */
    static constexpr meter_t semiMinorAxis = meter_t(6356752.3142);
    /** The eccentricity of an orbit describing the Earths shape. */
    static const double eccentricity;

    /**
     * Get the radius of the Earth at a specific latitude.
     * This functions assumes that the Earths shape can be described by a simple ellipsoid.
     *
     * @param latitude The latitude of a point on Earth.
     * @return The height of the given point.
     */
    static meter_t radiusAt(rad_t latitude);
};
