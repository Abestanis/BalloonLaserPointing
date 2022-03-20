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
    elevationMotor.calibrate();
}
