#include "CommandHandler.h"
#include "arduinoSystem.h"


void CommandHandler::handlePing() const {
    Serial.print("PONG\n");
}

void CommandHandler::handleGps(double latitude, double longitude, double height) const {
    // TODO: Actually do something with the location
    Serial.print("latitude: ");
    Serial.print(latitude);
    Serial.print("\nlongitude: ");
    Serial.print(longitude);
    Serial.print("\nheight: ");
    Serial.print(height);
    Serial.print('\n');
}
