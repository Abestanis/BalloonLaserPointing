#include <Arduino.h>
#include <Stepper.h>
#include "imu.h"


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

/** The motor that is used to turn the base plate of the laser. */
static Stepper baseMotor(MOTOR_STEPS_PER_REVOLUTION, 8, 10, 9, 11);

/** The time in milliseconds since boot when the gyroscope was last read. */
static unsigned long lastMeasurementMillis = 0;

void setup() {
    // Initialize the serial port.
    Serial.begin(9600);
    Serial.println("Boot");
    // Set the motor speed.
    baseMotor.setSpeed(
            (60L * 1000L * 1000L / MOTOR_STEPS_PER_REVOLUTION) / MOTOR_UPDATE_PERIOD_MICRO_S);
    initImu();
    Serial.println("Boot complete");
    lastMeasurementMillis = millis();
}


void loop() {
    // Measure the rotation.
    Vec3D rotations;
    getIMUGyro(rotations);
    unsigned long currentTime = millis();
    
    // Calculate the angular change since the last iteration.
    double angleChange = rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
    // Move the motor to compensate for the rotation.
    baseMotor.step(lround(MOTOR_STEPS_PER_REVOLUTION / 360.0 * angleChange));
    lastMeasurementMillis = currentTime;
}
