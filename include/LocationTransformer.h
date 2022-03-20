// This code is adapted from
// https://javascript.plainenglish.io/calculating-azimuth-distance-and-altitude-from-a-pair-of-gps-locations-36b4325d8ab0

#pragma once

#include "units.h"
#include "types.h"

struct LocalDirection {
    deg_t azimuth;
    deg_t elevation;
};

struct GpsPosition {
    rad_t latitude;
    rad_t longitude;
    meter_t altitude;
};

struct LocalPosition : public Position {
    LocalPosition(meter_t x, meter_t y, meter_t z, meter_t radius = meter_t(0),
                  double nx = 0, double ny = 0, double nz = 0) :
            Position(x, y, z), radius(radius), nx(nx), ny(ny), nz(nz) {
    }

    meter_t radius;
    double nx;
    double ny;
    double nz;
};

class LocationTransformer {
public:
    static LocalPosition localPositionFrom(const GpsPosition& position);

    static LocalDirection directionFrom(const GpsPosition& observer, const GpsPosition& target);
};
