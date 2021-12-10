#include <cmath>
#include <algorithm>
#include <Arduino.h>
#undef max  // See https://github.com/kekyo/gcc-toolchain/issues/3
#undef min
#include "Stepper.h"

/** The maximum amount of jitter allowed for the motor update timer in microseconds. */
#define MAX_TIMER_JITTER_MICRO_SEC 10

static std::vector<Stepper*> stepperMotors = {};


Stepper::Stepper(unsigned int numberOfSteps, unsigned long stepDelay, int motorPin1, int motorPin2,
                 int motorPin3, int motorPin4) : timer(DueTimer::getAvailable()) {
    this->stepDelay = stepDelay;
    this->totalSteps = numberOfSteps;
    this->targetAngle = 0;
    this->targetStep = 0;
    this->currentStep = 0;
    this->lastStepTime = 0;
    
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
    
    stepperMotors.push_back(this);
    this->timer.attachInterrupt(&Stepper::updateMotors).start(stepDelay);
}

Stepper::~Stepper() {
    this->timer.stop();
    (void) std::remove(stepperMotors.begin(), stepperMotors.end(), this);
}

void Stepper::setTargetAngle(double angle) {
    this->targetAngle = angle;
    this->targetStep = getStepForAngle(this->targetAngle);
}

void Stepper::updateMotors() {
    uint32_t now = micros();
    for (auto &motor: stepperMotors) {
        if (now - motor->lastStepTime > motor->stepDelay - MAX_TIMER_JITTER_MICRO_SEC) {
            motor->updateStep();
            motor->lastStepTime = now;
        }
    }
}

void Stepper::updateStep() {
    if (this->targetStep != this->currentStep) {
        bool increasing = this->currentStep < this->targetStep ?
                          this->targetStep - this->currentStep <
                          this->totalSteps - this->targetStep + this->currentStep :
                          this->currentStep - this->targetStep >
                          this->totalSteps - this->currentStep + this->targetStep;
        unsigned int newStep;
        if (increasing) {
            newStep = this->currentStep + 1;
            if (newStep == this->totalSteps) {
                newStep = 0;
            }
        } else if (this->currentStep == 0) {
            newStep = this->totalSteps;
        } else {
            newStep = this->currentStep - 1;
        }
        setStep(newStep);
    }
}

void Stepper::setStep(unsigned int step) {
    switch (step % 4) {
    case 0:  // 1100
        digitalWrite(this->motorPin1, HIGH);
        digitalWrite(this->motorPin2, HIGH);
        digitalWrite(this->motorPin3, LOW);
        digitalWrite(this->motorPin4, LOW);
        break;
    case 1:  // 0110
        digitalWrite(this->motorPin1, LOW);
        digitalWrite(this->motorPin2, HIGH);
        digitalWrite(this->motorPin3, HIGH);
        digitalWrite(this->motorPin4, LOW);
        break;
    case 2:  //0011
        digitalWrite(this->motorPin1, LOW);
        digitalWrite(this->motorPin2, LOW);
        digitalWrite(this->motorPin3, HIGH);
        digitalWrite(this->motorPin4, HIGH);
        break;
    case 3:  //1001
        digitalWrite(this->motorPin1, HIGH);
        digitalWrite(this->motorPin2, LOW);
        digitalWrite(this->motorPin3, LOW);
        digitalWrite(this->motorPin4, HIGH);
        break;
    }
    this->currentStep = step;
}

unsigned int Stepper::getStepForAngle(double angle) const {
    return lround((this->totalSteps - 1) / 360.0 * angle);
}
