/**
 * This code is adapted from
 * https://javascript.plainenglish.io/calculating-azimuth-distance-and-altitude-from-a-pair-of-gps-locations-36b4325d8ab0
 */

#include "arduinoSystem.h"
#include "LocationTransformer.h"
#include "Earth.h"


/**
 * Convert geodetic latitude to a geocentric latitude.
 * Geodetic latitude is the latitude as given by GPS.
 * Geocentric latitude is the angle measured from center of Earth between a point and the equator.
 * @see https://en.wikipedia.org/wiki/Latitude#Geocentric_latitude
 *
 * @param latitude A geodetic latitude.
 * @return The geocentric latitude.
 */
static rad_t geocentricLatitude(rad_t latitude) {
    return rad_t {atan((1.0 - (Earth::eccentricity * Earth::eccentricity)) * tan(latitude.value))};
}


LocalPosition LocationTransformer::localPositionFrom(const GpsPosition& position) {
    // Convert (lat, lon, elv) to (x, y, z).
    meter_t radius = Earth::radiusAt(position.latitude);
    rad_t clat = geocentricLatitude(position.latitude);

    double cosLon = std::cos(position.longitude.value);
    double sinLon = std::sin(position.longitude.value);
    double cosLat = std::cos(clat.value);
    double sinLat = std::sin(clat.value);
    meter_t x = radius * cosLon * cosLat;
    meter_t y = radius * sinLon * cosLat;
    meter_t z = radius * sinLat;

    // We used geocentric latitude to calculate (x,y,z) on the Earth's ellipsoid.
    // Now we use geodetic latitude to calculate normal vector from the surface,
    // to correct for elevation.
    double cosGeoLat = std::cos(position.latitude.value);
    double sinGeoLat = std::sin(position.latitude.value);

    double nx = cosGeoLat * cosLon;
    double ny = cosGeoLat * sinLon;
    double nz = sinGeoLat;

    x += position.altitude * nx;
    y += position.altitude * ny;
    z += position.altitude * nz;
    return {x, y, z, radius, {nx, ny, nz}};
}

/**
 * Rotate the coordinate system so that the observation point (position2)
 * is at a pretend equator and prime meridian.
 *
 * @param position The target position to point to.
 * @param radius The Earths radius at the target point.
 * @param position2 The observation point.
 * @return The rotated target position.
 */
static LocalPosition rotateGlobe(const GpsPosition& position, meter_t radius,
                                 const GpsPosition& position2) {
    // Get modified coordinates of 'position' positionY rotating the globe
    // so that 'position2' is at lat=0, lon=0.
    GpsPosition rotatedPosition {
            .latitude=position.latitude,
            .longitude=(position.longitude - position2.longitude),
            .altitude=position.altitude,
    };
    LocalPosition rotatedLocalPosition = LocationTransformer::localPositionFrom(rotatedPosition);

    // Rotate rotatedLocalPosition cartesian coordinates around the z-axis positionY position2.lon degrees,
    // then around the y-axis positionY position2.lat degrees.
    // Though we are decreasing positionY position2.lat degrees, as seen above the y-axis,
    // this is a positive (counterclockwise) rotation
    // (if position's longitude is east of position2's).
    // However, from this point of view the x-axis is pointing left.
    // So we will look the other way making the x-axis pointing right, the z-axis
    // pointing up, and the rotation treated as negative.

    rad_t position2Lat = geocentricLatitude(-position2.latitude);
    double position2Cos = std::cos(position2Lat.value);
    double position2sin = std::sin(position2Lat.value);

    meter_t positionX = (rotatedLocalPosition.x * position2Cos)
                        - (rotatedLocalPosition.z * position2sin);
    meter_t positionY = rotatedLocalPosition.y;
    meter_t positionZ = (rotatedLocalPosition.x * position2sin)
                        + (rotatedLocalPosition.z * position2Cos);
    return {positionX, positionY, positionZ, radius};
}

/**
 * Calculate the normalized difference between two positions / vectors.
 *
 * @param result The normalized difference.
 * @param position1 The first position.
 * @param position2 The second position.
 * @return Whether or not the two positions are actually different.
 */
static bool normalizeVectorDiff(LocalPosition& result, const LocalPosition& position1,
                                const LocalPosition& position2) {
    // Calculate norm(position1 - position2),
    // where norm divides a vector by its length to produce a unit vector.
    meter_t deltaX = position1.x - position2.x;
    meter_t deltaY = position1.y - position2.y;
    meter_t deltaZ = position1.z - position2.z;
    double squaredDistance =
            deltaX.value * deltaX.value + deltaY.value * deltaY.value + deltaZ.value * deltaZ.value;
    if (squaredDistance == 0) {
        return false;
    }
    meter_t distance = meter_t(std::sqrt(squaredDistance));
    result.x = meter_t(deltaX.value / distance.value);
    result.y = meter_t(deltaY.value / distance.value);
    result.z = meter_t(deltaZ.value / distance.value);
    result.earthRadius = meter_t(1.0);
    return true;
}

/**
 * Make sure a numerical value is between a minimum and a maximum.
 *
 * @param value The value to check.
 * @param upper The maximum allowed value.
 * @param lower The minimum allowed value.
 * @return The clamped value, which is lower <= value <= upper.
 */
static double clamp(double value, double upper, double lower) {
    if (value > upper) {
        return upper;
    } else if (value < lower) {
        return lower;
    }
    return value;
}

LocalDirection LocationTransformer::directionFrom(const GpsPosition& observer,
                                                  const GpsPosition& target) {
    LocalPosition observerPosition = localPositionFrom(observer);
    LocalPosition targetPosition = localPositionFrom(target);

    // Let's use a trick to calculate azimuth:
    // Rotate the globe so that point observer looks like latitude 0, longitude 0.
    // We keep the actual radii calculated based on the oblate geoid,
    // but use angles based on subtraction.
    // Point observer will be at x=earthRadius, y=0, z=0.
    // Vector difference target - observer will have dz = N/S component, dy = E/W component.
    deg_t azimuth = deg_t {0};
    deg_t elevation = deg_t {0};
    LocalPosition rotatedTargetPosition = rotateGlobe(target, targetPosition.earthRadius, observer);
    if (rotatedTargetPosition.z.value * rotatedTargetPosition.z.value +
        rotatedTargetPosition.y.value * rotatedTargetPosition.y.value > 1.0e-6) {
        deg_t theta = deg_t(std::atan2(
                rotatedTargetPosition.z.value, rotatedTargetPosition.y.value) * 180.0 / M_PI);
        azimuth = deg_t(90.0) - theta;
        if (azimuth < 0.0) {
            azimuth += 360.0;
        } else if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    }

    LocalPosition pointingVector = {meter_t(0), meter_t(0), meter_t(0)};
    if (normalizeVectorDiff(pointingVector, targetPosition, observerPosition)) {
        // Calculate altitude, which is the angle above the horizon of the target as seen from
        // the observer. Almost always, the target will actually be below the horizon,
        // so the altitude will be negative. The dot product of pointingVector
        // and norm = cos(zenith_angle), and zenith_angle = (90 deg) - altitude.
        // So altitude = 90 - acos(dot product).
        elevation = deg_t(90.0) - (180.0 / M_PI) * std::acos(clamp(
                pointingVector.x.value * observerPosition.normalVector.x +
                pointingVector.y.value * observerPosition.normalVector.y +
                pointingVector.z.value * observerPosition.normalVector.z, 1, -1));
    }
    return {azimuth, elevation};
}
