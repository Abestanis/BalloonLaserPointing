#include <cmath>
#include <Arduino.h>
#include "Stepper.h"


Stepper::Stepper(unsigned int numberOfSteps, unsigned long stepDelay, int motorPin1, int motorPin2,
                 int motorPin3, int motorPin4) {
    this->stepDelay = stepDelay;
    this->targetAngle = 0;
    this->lastStepTime = 0;
    this->totalSteps = numberOfSteps;
    
    // Arduino pins for the motor control connection.
    this->motorPin1 = motorPin1;
    this->motorPin2 = motorPin2;
    this->motorPin3 = motorPin3;
    this->motorPin4 = motorPin4;
    
    // Set up the pins on the microcontroller.
    pinMode(this->motorPin1, OUTPUT);
    pinMode(this->motorPin2, OUTPUT);
    pinMode(this->motorPin3, OUTPUT);
    pinMode(this->motorPin4, OUTPUT);
}

void Stepper::setTargetAngle(double angle) {
    stepTo(getStepForAngle(angle));
    targetAngle = angle;
}

void Stepper::stepTo(unsigned int targetStep) {
    unsigned int stepNumber = getStepForAngle(targetAngle);
    bool increasing = stepNumber < targetStep ?
                      targetStep - stepNumber < this->totalSteps - targetStep + stepNumber :
                      stepNumber - targetStep > this->totalSteps - stepNumber + targetStep;
    
    // Move one step at a time.
    while (targetStep != stepNumber) {
        uint32_t now = micros();
        // Move only if the appropriate delay has passed.
        if (now - this->lastStepTime >= this->stepDelay) {
            // Remember the timeStamp of this step.
            this->lastStepTime = now;
            // Increment or decrement the step number.
            if (increasing) {
                stepNumber++;
                if (stepNumber == this->totalSteps) {
                    stepNumber = 0;
                }
            } else {
                if (stepNumber == 0) {
                    stepNumber = this->totalSteps;
                }
                stepNumber--;
            }
            setStep(stepNumber);
        }
    }
}

void Stepper::setStep(unsigned int step) const {
    switch (step % 4) {
    case 0:  // 1100
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, HIGH);
        digitalWrite(motorPin3, LOW);
        digitalWrite(motorPin4, LOW);
        break;
    case 1:  // 0110
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, HIGH);
        digitalWrite(motorPin3, HIGH);
        digitalWrite(motorPin4, LOW);
        break;
    case 2:  //0011
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
        digitalWrite(motorPin3, HIGH);
        digitalWrite(motorPin4, HIGH);
        break;
    case 3:  //1001
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);
        digitalWrite(motorPin3, LOW);
        digitalWrite(motorPin4, HIGH);
        break;
    }
}

unsigned int Stepper::getStepForAngle(double angle) const {
    return lround((this->totalSteps - 1) / 360.0 * angle);
}
