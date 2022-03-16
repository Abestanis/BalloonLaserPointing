#include <cmath>
#include <algorithm>
#include "arduinoSystem.h"
#include "Stepper.h"

/** The maximum amount of jitter allowed for the motor update timer in microseconds. */
#define MAX_TIMER_JITTER_MICRO_SEC 10

static std::vector<Stepper*> stepperMotors = {};


Stepper::Stepper(unsigned int numberOfSteps, unsigned long stepDelay, int motorPin1, int motorPin2,
                 int motorPin3, int motorPin4, int calibrationPin) :
        stepDelay(stepDelay), totalSteps(numberOfSteps), timer(DueTimer::getAvailable()) {
    // Arduino pins for the motor control connection.
    this->motorPin1 = motorPin1;
    this->motorPin2 = motorPin2;
    this->motorPin3 = motorPin3;
    this->motorPin4 = motorPin4;
    this->calibrationPin = calibrationPin;

    // Set up the pins on the microcontroller.
    pinMode(this->motorPin1, OUTPUT);
    pinMode(this->motorPin2, OUTPUT);
    pinMode(this->motorPin3, OUTPUT);
    pinMode(this->motorPin4, OUTPUT);
    pinMode(this->calibrationPin, INPUT);

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

void Stepper::calibrate() {
    for (int i = 0; i < this->totalSteps; i++) {
        if (digitalRead(this->calibrationPin) == HIGH) {
            this->referenceStep = this->currentStep;
            return;
        }
        setStep((this->currentStep + 1) % this->totalSteps);
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
    return (lround((this->totalSteps - 1) / 360.0 * angle) + this->referenceStep) %
           this->totalSteps;
}
