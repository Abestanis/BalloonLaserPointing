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
        targetBaseAngle += rotations.z * ((currentTime - lastMeasurementMillis) / 1000.0);
        targetBaseAngle = normalizeAngle(targetBaseAngle);

        // Move the motor to compensate for the rotation.
        baseMotor.setTargetAngle(targetBaseAngle);
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
    targetPosition = gpsToLtp(rad_t(latitude), rad_t(longitude), height);
}

void Program::handleMotorsCalibration() {
    Serial.print("Calibrating Motors...\n");
    baseMotor.calibrate();
}

Position Program::gpsToEcef(rad_t latitude, rad_t longitude, meter_t altitude) {
    double N = Earth::semiMajorAxis.value /
               sqrt(1 - pow(Earth::eccentricity, 2) * pow(sin(latitude.value), 2));
    return {
            (altitude.value + N) * cos(latitude.value) * cos(longitude.value),
            (altitude.value + N) * cos(latitude.value) * sin(longitude.value),
            (altitude.value + (1 - pow(Earth::eccentricity, 2)) * N) * sin(latitude.value),
    };
}

Position Program::gpsToLtp(rad_t latitude, rad_t longitude, meter_t altitude) const {
    Position b_ECEF = gpsToEcef(latitude, longitude, altitude);
    // This is a transition vector used for calculation.
    Position Transition = {
            b_ECEF.x - this->laserPosition.x,
            b_ECEF.y - this->laserPosition.y,
            b_ECEF.z - this->laserPosition.z,
    };
    return {
            Transition.x * (-sin(latitude.value)) + Transition.y * cos(longitude.value),
            Transition.x * (-cos(longitude.value) * sin(latitude.value)) +
            Transition.y * (-sin(latitude.value) * sin(longitude.value)) +
            Transition.z * cos(latitude.value),
            Transition.x * cos(latitude.value) * cos(longitude.value) +
            Transition.y * cos(latitude.value) * sin(longitude.value) +
            Transition.z * sin(latitude.value),
    };
}
