#include <cmath>
#include <algorithm>
#include "arduinoSystem.h"
#include "Stepper.h"

/** The maximum amount of jitter allowed for the motor update timer in microseconds. */
#define MAX_TIMER_JITTER_MICRO_SEC 10

static std::vector<Stepper*> stepperMotors = {};


Stepper::Stepper(unsigned int numberOfSteps, unsigned long stepDelay, Pin motorPin1, Pin motorPin2,
                 Pin motorPin3, Pin motorPin4, Pin calibrationPin) :
        stepDelay(stepDelay), totalSteps(numberOfSteps), referenceStep(totalSteps),
        motorPin1(motorPin1), motorPin2(motorPin2), motorPin3(motorPin3), motorPin4(motorPin4),
        calibrationPin(calibrationPin), timer(DueTimer::getAvailable()) {

    // Set up the pins on the microcontroller.
    pinMode(this->motorPin1.pinNumber, OUTPUT);
    pinMode(this->motorPin2.pinNumber, OUTPUT);
    pinMode(this->motorPin3.pinNumber, OUTPUT);
    pinMode(this->motorPin4.pinNumber, OUTPUT);
    pinMode(this->calibrationPin.pinNumber, INPUT_PULLUP);

    stepperMotors.push_back(this);
    this->timer.attachInterrupt(&Stepper::updateMotors).start(stepDelay);
}

Stepper::~Stepper() {
    this->timer.stop();
    (void) std::remove(stepperMotors.begin(), stepperMotors.end(), this);
}

void Stepper::setTargetAngle(deg_t angle) {
    this->targetAngle = angle;
    this->targetStep = getStepForAngle(this->targetAngle);
}

void Stepper::updateMotors() {
    uint32_t now = micros();
    for (auto& motor: stepperMotors) {
        if (now - motor->lastStepTime > motor->stepDelay - MAX_TIMER_JITTER_MICRO_SEC) {
            motor->updateStep();
            motor->lastStepTime = now;
        }
    }
}

void Stepper::calibrate() {
    this->calibrationStartStep = this->currentStep;
    this->referenceStep = this->totalSteps;
}

void Stepper::updateStep() {
    if (this->referenceStep == this->totalSteps) {
        if (digitalRead(this->calibrationPin.pinNumber) == HIGH) {
            setStep((this->currentStep + 1) % this->totalSteps);
            if (this->currentStep == this->calibrationStartStep) {
                // TODO: This is a temporary fix to prevent the motor from spinning more than 360°
                //       Remove this when the restrictions are added elsewhere.
                Serial.print("Calibration failed...\n");
                this->referenceStep = this->currentStep;
            }
            return;
        }
        Serial.print("Calibration complete...\n");
        this->referenceStep = this->currentStep;
    }
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
        digitalWrite(this->motorPin1.pinNumber, HIGH);
        digitalWrite(this->motorPin2.pinNumber, HIGH);
        digitalWrite(this->motorPin3.pinNumber, LOW);
        digitalWrite(this->motorPin4.pinNumber, LOW);
        break;
    case 1:  // 0110
        digitalWrite(this->motorPin1.pinNumber, LOW);
        digitalWrite(this->motorPin2.pinNumber, HIGH);
        digitalWrite(this->motorPin3.pinNumber, HIGH);
        digitalWrite(this->motorPin4.pinNumber, LOW);
        break;
    case 2:  //0011
        digitalWrite(this->motorPin1.pinNumber, LOW);
        digitalWrite(this->motorPin2.pinNumber, LOW);
        digitalWrite(this->motorPin3.pinNumber, HIGH);
        digitalWrite(this->motorPin4.pinNumber, HIGH);
        break;
    case 3:  //1001
        digitalWrite(this->motorPin1.pinNumber, HIGH);
        digitalWrite(this->motorPin2.pinNumber, LOW);
        digitalWrite(this->motorPin3.pinNumber, LOW);
        digitalWrite(this->motorPin4.pinNumber, HIGH);
        break;
    }
    this->currentStep = step;
}

unsigned int Stepper::getStepForAngle(deg_t angle) const {
    return (lround((this->totalSteps - 1) / 360.0 * angle.value) + this->referenceStep) %
           this->totalSteps;
}

void Stepper::setCurrentAsCalibrationPoint() {
    this->referenceStep = this->currentStep;
}
