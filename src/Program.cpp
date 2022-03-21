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
    Serial.print("Target: latitude=");
    Serial.print(latitude.value);
    Serial.print(" longitude=");
    Serial.print(longitude.value);
    Serial.print(" height=");
    Serial.println(height.value);
    this->targetPosition = {rad_t(latitude), rad_t(longitude), height};
    updateTargetMotorAngles();
}

void Program::handleMotorsCalibration() {
    Serial.println("Calibrating Motors...");
    this->baseMotor.calibrate();
    this->elevationMotor.calibrate();
}

void Program::handleSetLocation(deg_t latitude, deg_t longitude, meter_t height,
                                deg_t orientation) {
    Serial.print("New location: latitude=");
    Serial.print(latitude.value);
    Serial.print(" longitude=");
    Serial.print(longitude.value);
    Serial.print(" height=");
    Serial.print(height.value);
    Serial.print(" orientation=");
    Serial.println(orientation.value);
    laserPosition = {rad_t(latitude), rad_t(longitude), height};
    laserOrientation = orientation;
    updateTargetMotorAngles();
}

void Program::updateTargetMotorAngles() {
    LocalDirection targetDirection = LocationTransformer::directionFrom(
            this->laserPosition, this->targetPosition);
    this->targetMotorAngles.azimuth = deg_t(360) - laserOrientation + targetDirection.azimuth;
    this->targetMotorAngles.elevation = -targetDirection.elevation / 2.0;
    Serial.print("target: Azimuth=");
    Serial.print(this->targetMotorAngles.azimuth.value);
    Serial.print(" elevation=");
    Serial.println(this->targetMotorAngles.elevation.value);
    this->baseMotor.setTargetAngle(this->targetMotorAngles.azimuth);
    this->elevationMotor.setTargetAngle(this->targetMotorAngles.elevation);
}
