/**
 * Various data types.
 */

#pragma once

#include <cmath>
#include "WrapperType.h"


/**
 * An angle in degree.
 */
class deg_t : public WrapperType<double, deg_t> {
public:
    using WrapperType::WrapperType;
};

/**
 * An angle in radian.
 */
class rad_t : public WrapperType<double, rad_t> {
public:
    using WrapperType::WrapperType;

    /**
     * Convert an angle from degree to radian.
     * @param angle The angle in degree.
     */
    explicit rad_t(deg_t angle) : WrapperType<double, rad_t>(angle.value * (2 * M_PI / 360)) {
    };
};

/**
 * A distance in meter.
 */
class meter_t : public WrapperType<double, meter_t> {
public:
    using WrapperType::WrapperType;
};
