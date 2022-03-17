#include "Earth.h"


constexpr meter_t Earth::radius;
constexpr meter_t Earth::semiMajorAxis;
/** Parameter for the eccentricity. */
static constexpr double f =
        (Earth::semiMajorAxis - Earth::semiMinorAxis).value / Earth::semiMajorAxis.value;
const double Earth::eccentricity = std::sqrt(f * (2 - f));
