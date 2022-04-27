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
    Serial.print("Target: Latitude=");
    Serial.print(latitude.value);
    Serial.print(" Longitude=");
    Serial.print(longitude.value);
    Serial.print(" Height=");
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
    Serial.print("New location: Latitude=");
    Serial.print(latitude.value);
    Serial.print(" Longitude=");
    Serial.print(longitude.value);
    Serial.print(" Height=");
    Serial.print(height.value);
    Serial.print(" Orientation=");
    Serial.println(orientation.value);
    laserPosition = {rad_t(latitude), rad_t(longitude), height};
    laserOrientation = orientation;
    updateTargetMotorAngles();
}

void Program::updateTargetMotorAngles() {
    LocalDirection targetDirection = LocationTransformer::directionFrom(
            this->laserPosition, this->targetPosition);
    // TODO: Investigate why it's -targetDirection.azimuth when testing with Google Maps.
    this->targetMotorAngles.azimuth = targetDirection.azimuth - laserOrientation;
    this->targetMotorAngles.elevation = targetDirection.elevation / 2.0 - deg_t(90);
    Serial.print("Target: Azimuth=");
    Serial.print(this->targetMotorAngles.azimuth.value);
    Serial.print(" Elevation=");
    Serial.println(this->targetMotorAngles.elevation.value);
    this->baseMotor.setTargetAngle(this->targetMotorAngles.azimuth);
    this->elevationMotor.setTargetAngle(this->targetMotorAngles.elevation);
}

void Program::handleSetMotorPosition(SerialConnection::Motor motor, deg_t position) {
    switch (motor) {
    case SerialConnection::AZIMUTH_MOTOR:
        this->baseMotor.setTargetAngle(position);
        break;
    case SerialConnection::ELEVATION_MOTOR:
        this->elevationMotor.setTargetAngle(position);
        break;
    }
}

void Program::handleSetCalibrationPoint(SerialConnection::Motor motor) {
    switch (motor) {
    case SerialConnection::AZIMUTH_MOTOR:
        this->baseMotor.setCurrentAsCalibrationPoint();
        Serial.println("Azimuth motor calibration point set");
        break;
    case SerialConnection::ELEVATION_MOTOR:
        this->elevationMotor.setCurrentAsCalibrationPoint();
        Serial.println("Elevation motor calibration point set");
        break;
    }
}
