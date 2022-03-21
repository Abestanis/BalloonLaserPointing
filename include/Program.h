#pragma once

#include "SerialConnection.h"
#include "Stepper.h"
#include "LocationTransformer.h"


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

/** Whether or not the IMU should be used to compensate rotations of the laser structure. */
#define USE_IMU false


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

    void handleSetLocation(deg_t latitude, deg_t longitude, meter_t height,
                           deg_t orientation) override;

    void handleSetMotorPosition(SerialConnection::Motor motor, deg_t position) override;

    void handleSetCalibrationPoint(SerialConnection::Motor motor) override;

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

    /**
     * Update the motor angles for the current target and laser locations.
     */
    void updateTargetMotorAngles();

#if USE_IMU
    /** The time in milliseconds since boot when the gyroscope was last read. */
    unsigned long lastMeasurementMillis = 0;
#endif /* USE_IMU */

    /**
     * The position of the laser in the local tangent place reference frame.
     */
    GpsPosition laserPosition {rad_t(0), rad_t(0), meter_t(0)};

    /**
     * The position of the target in the local tangent place reference frame.
     */
    GpsPosition targetPosition {rad_t(0), rad_t(0), meter_t(1)};

    /**
     * The orientation of the laser pointing structure in relation to the geographical nord.
     * 0° -> pointing directly nord in the 0 base motor position.
     */
    deg_t laserOrientation = deg_t(0);

    /**
     * The target angles for the motors.
     */
    LocalDirection targetMotorAngles = {deg_t(0), deg_t(0)};

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
