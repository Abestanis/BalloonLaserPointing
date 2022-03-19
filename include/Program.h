#pragma once

#include "SerialConnection.h"
#include "Stepper.h"
#include "Vec3.h"


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

/**
 * A position in a 3 dimensional space.
 */
typedef Vec3<double> Position;


/**
 * A handler for incoming telecommands.
 */
class Program : private SerialConnection::CommandHandler {
public:
    /**
     * Initialize the program.
     */
    Program();

    /**
     * Run the program.
     * @note This function will block and never return.
     */
    [[noreturn]] void run();

private:
    void handlePing() const override;

    void handleGps(deg_t latitude, deg_t longitude, meter_t height) override;

    void handleMotorsCalibration() override;

    /**
     * Convert the coordinates given by the GPS to Earth-Centered, Earth-Fixed frame.
     *
     * @param latitude The latitude of the GPS position in radians.
     * @param longitude The longitude of the GPS position in radians.
     * @param altitude The altitude of the GPS position in meter.
     * @return The GPS position in the Local Tangent Place reference frame.
     */
    static Position gpsToEcef(rad_t latitude, rad_t longitude, meter_t altitude);

    /**
     * Convert the coordinates given by the GPS to the Local Tangent Place reference frame.
     * The center of this reference frame being the laser.
     *
     * @param latitude The latitude of the GPS position in radians.
     * @param longitude The longitude of the GPS position in radians.
     * @param altitude The altitude of the GPS position in meter.
     * @return The GPS position in the Local Tangent Place reference frame.
     */
    Position gpsToLtp(rad_t latitude, rad_t longitude, meter_t altitude) const;

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

    /**
     * The position of the laser in the local tangent place reference frame.
     */
    Position laserPosition {0, 0, 0};

    /**
     * The position of the target in the local tangent place reference frame.
     */
    Position targetPosition {0, 0, 1};

    /** The motor that is used to turn the base plate of the laser. */
    Stepper baseMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_UPDATE_PERIOD_MICRO_S,
            Pins::baseMotor1, Pins::baseMotor2, Pins::baseMotor3,
            Pins::baseMotor4, Pins::baseMotorCalibration);

    /** The motor that is used to turn the final mirror, controlling the elevation. */
    Stepper elevationMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_UPDATE_PERIOD_MICRO_S,
            Pins::elevationMotor1, Pins::elevationMotor2, Pins::elevationMotor3,
            Pins::elevationMotor4, Pins::elevationMotorCalibration);

    /**
     * The connection to a controller that can send commands.
     */
    SerialConnection connection;
};
