#include "Program.h"
#include "arduinoSystem.h"
#include "imu.h"


/** The radius of the Earth in meter. */
#define  Rt 6378.137e3
/** Geometrical parameters for coordinates calculation in the LTP */
static double a = 6378137.0;
static double b = 6356752.3142;
static double f = (a - b) / a;
static double e = sqrt(f * (2 - f));


Program::Program() : connection(*this) {
    coord_GPS laser_GPS = {
            /** Angle in radian !! **/
            .longitude=23,
            .latitude=45,
            .altitude= 23,
    };

    float N_laser = a / sqrt(1 - pow(e, 2) * pow(sin(laser_GPS.latitude), 2));

    laserPosition = {
            .x=(laser_GPS.altitude + N_laser) * cos(laser_GPS.latitude) * cos(laser_GPS.longitude),
            .y=(laser_GPS.altitude + N_laser) *
               cos(laser_GPS.latitude) * sin(laser_GPS.longitude),
            .z=(laser_GPS.altitude + (1 - pow(e, 2)) * N_laser) * sin(laser_GPS.latitude)
    };
    Serial.println("Booting...");
    initImu();
    Serial.println("Boot complete");
    lastMeasurementMillis = millis();
}

[[noreturn]] void Program::run() {
    while (true) {
        connection.fetchMessages();

        // Measure the rotation.
        Vec3D rotations;
        getIMUGyro(rotations);
        unsigned long currentTime = millis();

        // Calculate the angular change since the last iteration.
        targetBaseAngle += rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
        targetBaseAngle = normalizeAngle(targetBaseAngle);

        // Move the motor to compensate for the rotation.
        baseMotor.setTargetAngle(targetBaseAngle);
        lastMeasurementMillis = currentTime;
        if (serialEventRun) { // This is copied from main.cpp from the Arduino library.
            serialEventRun();
        }
    }
}

void Program::handlePing() const {
    Serial.print("PONG\n");
}

void Program::handleGps(deg_t latitude, deg_t longitude, meter_t height) {
    // TODO: Actually do something with the location
    Serial.print("latitude: ");
    Serial.print(latitude.value);
    Serial.print("\nlongitude: ");
    Serial.print(longitude.value);
    Serial.print("\nheight: ");
    Serial.print(height.value);
    Serial.print('\n');
}

coord_xyz Program::conversion_GPS_to_LTP(rad_t latitude, rad_t longitude, meter_t altitude) {
    /** This function convert the coordinates given by the GPS to the Local Tangent Place reference frame. The center of this reference frame being the laser.**/

    float N = a / sqrt(1 - pow(e, 2) * pow(sin(latitude.value), 2));

    coord_xyz b_ECEF;
    coord_xyz b_LTP;
    coord_xyz Transition; /*This is a transition vector used for calculation*/

    b_ECEF.x = (altitude.value + N) * cos(latitude.value) * cos(longitude.value);
    b_ECEF.y = (altitude.value + N) * cos(latitude.value) * sin(longitude.value);
    b_ECEF.z = (altitude.value + (1 - pow(e, 2)) * N) * sin(latitude.value);

    Transition.x = b_ECEF.x - this->laserPosition.x;
    Transition.y = b_ECEF.y - this->laserPosition.y;
    Transition.z = b_ECEF.z - this->laserPosition.z;

    b_LTP.x = Transition.x * (-sin(latitude.value)) + Transition.y * cos(longitude.value);
    b_LTP.y = Transition.x * (-cos(longitude.value) * sin(latitude.value)) +
              Transition.y * (-sin(latitude.value) * sin(longitude.value)) +
              Transition.z * cos(latitude.value);
    b_LTP.z = Transition.x * cos(latitude.value) * cos(longitude.value) +
              Transition.y * cos(latitude.value) * sin(longitude.value) +
              Transition.z * sin(latitude.value);

    return b_LTP;
}
