#include "Earth.h"


constexpr meter_t Earth::radius;
constexpr meter_t Earth::semiMajorAxis;
/** Parameter for the eccentricity. */
static constexpr double f =
        (Earth::semiMajorAxis - Earth::semiMinorAxis).value / Earth::semiMajorAxis.value;
const double Earth::eccentricity = std::sqrt(f * (2 - f));

meter_t Earth::radiusAt(rad_t latitude) {
    double cosLat = std::cos(latitude.value);
    double sinLat = std::sin(latitude.value);
    double t1 = Earth::semiMajorAxis.value * Earth::semiMajorAxis.value * cosLat;
    double t2 = Earth::semiMinorAxis.value * Earth::semiMinorAxis.value * sinLat;
    double t3 = Earth::semiMajorAxis.value * cosLat;
    double t4 = Earth::semiMinorAxis.value * sinLat;
    return meter_t {std::sqrt((t1 * t1 + t2 * t2) / (t3 * t3 + t4 * t4))};
}
