#include "Program.h"
#include "arduinoSystem.h"
#include "Earth.h"

#if USE_IMU
#  include "imu.h"
#endif /* USE_IMU */


Program::Program() : connection(*this) {
    Serial.println("Booting...");
#if USE_IMU
    initImu();
    lastMeasurementMillis = millis();
#endif /* USE_IMU */
    Serial.println("Boot complete");
}

[[noreturn]] void Program::run() {
    while (true) {
        connection.fetchMessages();

#if USE_IMU
        // Measure the rotation.
        Vec3D rotations;
        getIMUGyro(rotations);
        unsigned long currentTime = millis();

        // Calculate the angular change since the last iteration.
        targetMotorAngles.azimuth += rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
        targetMotorAngles.azimuth = normalizeAngle(targetMotorAngles.azimuth);
        // TODO: Depending on IMU orientation, add elevation compensation.
        lastMeasurementMillis = currentTime;
        // Move the motor to compensate for the rotation.
        this->baseMotor.setTargetAngle(this->targetMotorAngles.azimuth);
        this->elevationMotor.setTargetAngle(this->targetMotorAngles.elevation);
#endif /* USE_IMU */
        if (serialEventRun) { // This is copied from main.cpp from the Arduino library.
            serialEventRun();
        }
    }
}

void Program::handlePing() const {
    Serial.print("PONG\n");
}

void Program::handleGps(deg_t latitude, deg_t longitude, meter_t height) {
    Serial.print("latitude: ");
    Serial.print(latitude.value);
    Serial.print(" longitude: ");
    Serial.print(longitude.value);
    Serial.print(" height: ");
    Serial.print(height.value);
    Serial.print('\n');
    this->targetPosition = {rad_t(latitude), rad_t(longitude), height};
    LocalDirection targetDirection = LocationTransformer::directionFrom(
            this->laserPosition, this->targetPosition);
    this->targetMotorAngles.azimuth = deg_t(360) - laserOrientation + targetDirection.azimuth;
    this->targetMotorAngles.elevation = -targetDirection.elevation / 2.0;
    this->baseMotor.setTargetAngle(this->targetMotorAngles.azimuth);
    this->elevationMotor.setTargetAngle(this->targetMotorAngles.elevation);
}

void Program::handleMotorsCalibration() {
    Serial.print("Calibrating Motors...\n");
    this->baseMotor.calibrate();
    this->elevationMotor.calibrate();
}
