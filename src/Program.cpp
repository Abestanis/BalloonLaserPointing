#include "Program.h"
#include "arduinoSystem.h"
#include "imu.h"
#include "Earth.h"


Program::Program() : connection(*this) {
    Serial.println("Booting...");
    initImu();
    Serial.println("Boot complete");
    lastMeasurementMillis = millis();
}

[[noreturn]] void Program::run() {
    while (true) {
        connection.fetchMessages();

        // Measure the rotation.
        Vec3D rotations;
        getIMUGyro(rotations);
        unsigned long currentTime = millis();

        // Calculate the angular change since the last iteration.
        targetMotorAngles.azimuth += rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
        targetMotorAngles.azimuth = normalizeAngle(targetMotorAngles.azimuth);
        // TODO: Depending on IMU orientation, add elevation compensation.

        // Move the motor to compensate for the rotation.
        baseMotor.setTargetAngle(targetMotorAngles.azimuth);
        elevationMotor.setTargetAngle(targetMotorAngles.elevation);
        lastMeasurementMillis = currentTime;
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
}

void Program::handleMotorsCalibration() {
    Serial.print("Calibrating Motors...\n");
    baseMotor.calibrate();
    elevationMotor.calibrate();
}
