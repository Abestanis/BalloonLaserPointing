#include "arduinoSystem.h"
#include "LocationTransformer.h"
#include "Earth.h"


static constexpr double e2 = 0.00669437999014;


/**
 * Convert geodetic latitude 'lat' to a geocentric latitude 'clat'.
 * Geodetic latitude is the latitude as given by GPS.
 * Geocentric latitude is the angle measured from center of Earth between a point and the equator.
 * https:#en.wikipedia.org/wiki/Latitude#Geocentric_latitude
 *
 * @param latitude A geodetic latitude.
 * @return The geocentric latitude.
 */
static rad_t geocentricLatitude(rad_t latitude) {
    return rad_t {atan((1.0 - e2) * tan(latitude.value))};
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
    // Now we use geodetic latitude to calculate normal vector from the surface, to correct for elevation.
    double cosGlat = std::cos(position.latitude.value);
    double sinGlat = std::sin(position.latitude.value);

    double nx = cosGlat * cosLon;
    double ny = cosGlat * sinLon;
    double nz = sinGlat;

    x += position.altitude * nx;
    y += position.altitude * ny;
    z += position.altitude * nz;
    return {x, y, z, radius, nx, ny, nz};
}

static LocalPosition rotateGlobe(const GpsPosition& position, meter_t radius,
                                 const GpsPosition& position2, meter_t radius2) {
    // Get modified coordinates of 'b' by rotating the globe so that 'a' is at lat=0, lon=0.
    GpsPosition br {
            .latitude=position.latitude,
            .longitude=(position.longitude - position2.longitude),
            .altitude=position.altitude,
    };
    LocalPosition brp = LocationTransformer::localPositionFrom(br);

    // Rotate brp cartesian coordinates around the z-axis by a.lon degrees,
    // then around the y-axis by a.lat degrees.
    // Though we are decreasing by a.lat degrees, as seen above the y-axis,
    // this is a positive (counterclockwise) rotation (if B's longitude is east of A's).
    // However, from this point of view the x-axis is pointing left.
    // So we will look the other way making the x-axis pointing right, the z-axis
    // pointing up, and the rotation treated as negative.

    rad_t alat = geocentricLatitude(-position2.latitude);
    double acos = std::cos(alat.value);
    double asin = std::sin(alat.value);

    meter_t bx = (brp.x * acos) - (brp.z * asin);
    meter_t by = brp.y;
    meter_t bz = (brp.x * asin) + (brp.z * acos);
    return {bx, by, bz, radius};
}

static bool normalizeVectorDiff(LocalPosition& result, const LocalPosition& position1,
                                const LocalPosition& position2) {
    // Calculate norm(b-a), where norm divides a vector by its length to produce a unit vector.
    meter_t dx = position1.x - position2.x;
    meter_t dy = position1.y - position2.y;
    meter_t dz = position1.z - position2.z;
    double dist2 = dx.value * dx.value + dy.value * dy.value + dz.value * dz.value;
    if (dist2 == 0) {
        return false;
    }
    meter_t dist = meter_t(std::sqrt(dist2));
    result.x = meter_t(dx.value / dist.value);
    result.y = meter_t(dy.value / dist.value);
    result.z = meter_t(dz.value / dist.value);
    result.radius = meter_t(1.0);
    return true;
}

/**
 * make sure a numerical value is between a minimum and a maximum.
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
    LocalPosition ap = localPositionFrom(observer);
    LocalPosition bp = localPositionFrom(target);

    // Let's use a trick to calculate azimuth:
    // Rotate the globe so that point A looks like latitude 0, longitude 0.
    // We keep the actual radii calculated based on the oblate geoid,
    // but use angles based on subtraction.
    // Point A will be at x=radius, y=0, z=0.
    // Vector difference B-A will have dz = N/S component, dy = E/W component.
    deg_t azimuth = deg_t {0};
    deg_t elevation = deg_t {0};
    LocalPosition br = rotateGlobe(target, bp.radius, observer, ap.radius);
    if (br.z.value * br.z.value + br.y.value * br.y.value > 1.0e-6) {
        deg_t theta = deg_t(std::atan2(br.z.value, br.y.value) * 180.0 / M_PI);
        azimuth = deg_t(90.0) - theta;
        if (azimuth < 0.0) {
            azimuth += 360.0;
        } else if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    }

    LocalPosition bma = {meter_t(0), meter_t(0), meter_t(0)};
    if (normalizeVectorDiff(bma, bp, ap)) {
        // Calculate altitude, which is the angle above the horizon of B as seen from A.
        // Almost always, B will actually be below the horizon, so the altitude will be negative.
        // The dot product of bma and norm = cos(zenith_angle), and zenith_angle = (90 deg) - altitude.
        // So altitude = 90 - acos(dotprod).
        elevation = deg_t(90.0) - (180.0 / M_PI) * std::acos(
                clamp(bma.x.value * ap.nx + bma.y.value * ap.ny + bma.z.value * ap.nz, 1, -1));
    }
    return {azimuth, elevation};
}
