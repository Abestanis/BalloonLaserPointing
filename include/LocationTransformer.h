/**
 * Coordinate transformation functionality.
 */

#pragma once

#include "units.h"
#include "types.h"

/**
 * A direction in the local coordinate system.
 */
struct LocalDirection {
    /**
     * The azimuth angle where 0 degree represents North and 90 degree represents East.
     */
    deg_t azimuth;

    /**
     * The elevation where 0 degree points towards the horizon
     * and 90 degree points towards the zenith.
     */
    deg_t elevation;
};

/**
 * A position reported by the GPS system in the geodesic system (WGS 84).
 */
struct GpsPosition {
    /**
     * The latitude in degrees.
     */
    rad_t latitude;
    /**
     * The longitude in degrees.
     */
    rad_t longitude;
    /**
     * The height in meters above the mean sea level.
     */
    meter_t altitude;
};

/**
 * A position in the Earth local coordinate system as cartesian coordinates.
 */
struct LocalPosition : public Position {
    LocalPosition(meter_t x, meter_t y, meter_t z, meter_t radius = meter_t(0),
                  Vec3D normalVector = {0, 0, 0}) :
            Position(x, y, z), earthRadius(radius), normalVector(normalVector) {
    }

    /**
     * The Earths radius at this position.
     */
    meter_t earthRadius;

    /**
     * A vector pointing in the normal direction at this position of the Earth.
     */
    Vec3D normalVector;
};


/**
 * Transformation utilities.
 */
struct LocationTransformer {

    /**
     * Convert a GPS position into a local position.
     *
     * @param position The GPS position.
     * @return The converted GPS position in Cartesian coordinates.
     */
    static LocalPosition localPositionFrom(const GpsPosition& position);

    /**
     * Get a direction from one GPS position to another one.
     *
     * @param observer The GPS position of the observer, the origin of the direction vector.
     * @param target The GPS position of the target.
     * @return The direction from the observer to the target.
     */
    static LocalDirection directionFrom(const GpsPosition& observer, const GpsPosition& target);
};
