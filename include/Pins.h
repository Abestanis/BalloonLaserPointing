/**
 * This keeps track of all pins used.
 */

#pragma once

#include <cstdint>

/**
 * A physical pin on the Arduino.
 */
class Pin {
public:
    /**
     * Create a new pin from the raw pin number.
     *
     * @param pinNumber The raw number of the pin.
     */
    explicit constexpr Pin(uint32_t pinNumber) : pinNumber(pinNumber) {
    }

    /**
     * The raw number of the pin.
     */
    const uint32_t pinNumber;
};

/**
 * All pins used in the system.
 */
class Pins {
public:
    /**
     * The first pin to control the base motor.
     */
    static constexpr Pin baseMotor1 = Pin(8);

    /**
     * The second pin to control the base motor.
     */
    static constexpr Pin baseMotor2 = Pin(9);

    /**
     * The third pin to control the base motor.
     */
    static constexpr Pin baseMotor3 = Pin(10);

    /**
     * The fourth pin to control the base motor.
     */
    static constexpr Pin baseMotor4 = Pin(11);

    /**
     * The pin indicating the 0° angle position for the base motor.
     */
    static constexpr Pin baseMotorCalibration = Pin(12);

    /**
     * The first pin to control the motor.
     */
    static constexpr Pin elevationMotor1 = Pin(3);

    /**
     * The second pin to control the elevation motor.
     */
    static constexpr Pin elevationMotor2 = Pin(4);

    /**
     * The third pin to control the elevation motor.
     */
    static constexpr Pin elevationMotor3 = Pin(5);

    /**
     * The fourth pin to control the elevation motor.
     */
    static constexpr Pin elevationMotor4 = Pin(6);

    /**
     * The pin indicating the 0° angle position for the elevation motor.
     */
    static constexpr Pin elevationMotorCalibration = Pin(7);
};
