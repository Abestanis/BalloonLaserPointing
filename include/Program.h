#pragma once

#include "SerialConnection.h"
#include "Stepper.h"


/** The number of individual steps that make up a full revolution of the stepper motor. */
#define MOTOR_STEPS_PER_REVOLUTION 2048

/**
 * The time to wait between updates in microseconds.
 * The datasheet (https://www.gotronic.fr/pj-1136.pdf) specifies a maximum response frequency of
 * 900 phases per seconds, e.g ~1111 microseconds need to be waited before switching to the next
 * phase (the next step).
 * To overcome initial resistance, the maximum change in phases per seconds at startup is 500,
 * e.g 2000 milliseconds between steps.
 */
#define MOTOR_UPDATE_PERIOD_MICRO_S 2000

struct coord_GPS {
    /** Angles have to be transform in radian before assignation !!! **/
    double longitude;
    double latitude;
    double altitude;
};


struct coord_xyz {
    double x;
    double y;
    double z;
};


/**
 * A handler for incoming telecommands.
 */
class Program : private SerialConnection::CommandHandler {
public:
    Program();

    [[noreturn]] void run();

private:
    void handlePing() const override;

    void handleGps(deg_t latitude, deg_t longitude, meter_t height) override;

    void handleMotorsCalibration() override;

    coord_xyz conversion_GPS_to_LTP(rad_t latitude, rad_t longitude, meter_t altitude);

    /**
     * Normalize the angle to be between 0 and 360 degrees.
     * @param angle The angle to normalize in degrees.
     * @return The normalized angle.
     */
    static deg_t normalizeAngle(deg_t angle) {
        angle = deg_t(fmod(angle.value, 360));
        if (angle < 0) {
            angle += 360;
        }
        return angle;
    }


    /** The time in milliseconds since boot when the gyroscope was last read. */
    unsigned long lastMeasurementMillis = 0;

    /** The target angle of the base motor. */
    deg_t targetBaseAngle = deg_t(0);

    /** The motor that is used to turn the base plate of the laser. */
    Stepper baseMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_UPDATE_PERIOD_MICRO_S,
            Pins::baseMotor1, Pins::baseMotor2, Pins::baseMotor3,
            Pins::baseMotor4, Pins::baseMotorCalibration);

    /**
     * The connection to a controller that can send commands.
     */
    SerialConnection connection;

    coord_xyz laserPosition;
};
