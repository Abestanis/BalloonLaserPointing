/**
 * A class to control a four wire stepper motor, based on the Stepper library for Wiring/Arduino.
 * Original header is included below:
 *
 * Stepper.h - Stepper library for Wiring/Arduino - Version 1.1.0
 *
 * Original library        (0.1)   by Tom Igoe.
 * Two-wire modifications  (0.2)   by Sebastian Gassner
 * Combination version     (0.3)   by Tom Igoe and David Mellis
 * Bug fix for four-wire   (0.4)   by Tom Igoe, bug fix from Noah Shibley
 * High-speed stepping mod         by Eugene Kozlenko
 * Timer rollover fix              by Eugene Kozlenko
 * Five phase five wire    (1.1.0) by Ryan Orendorff
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Drives a unipolar, bipolar, or five phase stepper motor.
 *
 * When wiring multiple stepper motors to a microcontroller, you quickly run
 * out of output pins, with each motor requiring 4 connections.
 *
 * By making use of the fact that at any time two of the four motor coils are
 * the inverse of the other two, the number of control connections can be
 * reduced from 4 to 2 for the unipolar and bipolar motors.
 *
 * A slightly modified circuit around a Darlington transistor array or an
 * L293 H-bridge connects to only 2 microcontroler pins, inverts the signals
 * received, and delivers the 4 (2 plus 2 inverted ones) output signals
 * required for driving a stepper motor. Similarly the Arduino motor shields
 * 2 direction pins may be used.
 *
 * The sequence of control signals for 5 phase, 5 control wires is as follows:
 *
 * Step C0 C1 C2 C3 C4
 *    1  0  1  1  0  1
 *    2  0  1  0  0  1
 *    3  0  1  0  1  1
 *    4  0  1  0  1  0
 *    5  1  1  0  1  0
 *    6  1  0  0  1  0
 *    7  1  0  1  1  0
 *    8  1  0  1  0  0
 *    9  1  0  1  0  1
 *   10  0  0  1  0  1
 *
 * The sequence of control signals for 4 control wires is as follows:
 *
 * Step C0 C1 C2 C3
 *    1  1  0  1  0
 *    2  0  1  1  0
 *    3  0  1  0  1
 *    4  1  0  0  1
 *
 * The sequence of controls signals for 2 control wires is as follows
 * (columns C1 and C2 from above):
 *
 * Step C0 C1
 *    1  0  1
 *    2  1  1
 *    3  1  0
 *    4  0  0
 *
 * The circuits can be found at
 *
 * http://www.arduino.cc/en/Tutorial/Stepper
 */

#pragma once

#include <cstdint>

/**
 * A stepper motor that can rotate 360 degrees freely.
 */
class Stepper {
public:
    
    /**
     * Initialize a stepper motor.
     *
     * @param numberOfSteps The number of steps that the motor can take per revolution.
     * @param stepDelay The delay between steps in microseconds.
     * @param motorPin1 The pin number of the first connection to the motor.
     * @param motorPin2 The pin number of the second connection to the motor.
     * @param motorPin3 The pin number of the third connection to the motor.
     * @param motorPin4 The pin number of the fourth connection to the motor.
     */
    Stepper(unsigned int numberOfSteps, unsigned long stepDelay, int motorPin1, int motorPin2,
            int motorPin3, int motorPin4);
    
    /**
     * Set the target angle of the motor.
     *
     * @note The function will block until the motor reached the target angle.
     * @param angle The target angle in degrees.
     */
    void setTargetAngle(double angle);

private:
    
    /**
     * Move the motor to the target step. The motor will take the shortest path to the target.
     *
     * @param targetStep The step to move to.
     */
    void stepTo(unsigned int targetStep);
    
    /**
     * Set the current step of the motor.
     *
     * @param step The step to step to.
     */
    void setStep(unsigned int step) const;
    
    /**
     * Convert an angle in degrees to the corresponding step.
     *
     * @param angle The angle in degrees.
     * @return The step corresponding to the angle.
     */
    unsigned int getStepForAngle(double angle) const;
    
    /**
     * The delay in microseconds between steps.
     */
    unsigned long stepDelay;
    
    /**
     * The total number of steps that the motor can take.
     */
    unsigned int totalSteps;
    
    /**
     * The angle that the motor should point to in degrees.
     */
    double targetAngle;
    
    /**
     * The pin for the first connection to the motor.
     */
    int motorPin1;
    
    /**
     * The pin for the second connection to the motor.
     */
    int motorPin2;
    
    /**
     * The pin for the third connection to the motor.
     */
    int motorPin3;
    
    /**
     * The pin for the fourth connection to the motor.
     */
    int motorPin4;
    
    /**
     * The time stamp in microseconds when the last step was taken.
     */
    uint32_t lastStepTime;
};
