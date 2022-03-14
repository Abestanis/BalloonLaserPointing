#include <algorithm>
#include "arduinoSystem.h"
#include "SerialConnection.h"


/** The first byte of a message header, used to detect the start of the header */
constexpr uint8_t SYNC_BYTE_1 = 0xAA;
/** The second byte of a message header, used to detect the start of the header */
constexpr uint8_t SYNC_BYTE_2 = 0x55;

/**
 * The structure of a GPS message.
 */
typedef struct [[gnu::packed]] {
    /** The latitude in degrees. */
    double latitude;
    /** The longitude in degrees. */
    double longitude;
    /** The height in meters. */
    double height;
} GpsMessage;

/** The start of every message. */
typedef struct [[gnu::packed]] {
    /** Synchronization bytes to allow to detect the start of a message. */
    uint8_t sync[2];
    /** The type of the message, that will be send directly after the header. */
    SerialConnection::MessageType type;
} MessageHeader;


SerialConnection::SerialConnection(const CommandHandler &handler) : handler(handler) {
}

void SerialConnection::connect() {
    Serial.begin(9600);
}

void SerialConnection::fetchMessages() {
    size_t expectedSize = 0;
    switch (nextMessageType) {
    case PING:
        break;
    case GPS:
        expectedSize = sizeof(GpsMessage);
        break;
    case HEADER:
        expectedSize = sizeof(MessageHeader);
        break;
    }
    if ((size_t) Serial.available() < expectedSize) {
        return;
    }
    switch (nextMessageType) {
    case PING:
        handler.handlePing();
        break;
    case GPS:
        GpsMessage gpsData;
        Serial.readBytes(reinterpret_cast<uint8_t*>(&gpsData), sizeof(gpsData));
        handler.handleGps(gpsData.latitude, gpsData.longitude, gpsData.height);
        break;
    case HEADER:
        if (Serial.read() != SYNC_BYTE_1 || Serial.read() != SYNC_BYTE_2) {
            // Short circuit return to avoid consuming
            // a potential valid sync sequence after an invalid one.
            return;
        }
        MessageHeader header;
        Serial.readBytes(reinterpret_cast<uint8_t*>(&header) + sizeof(header.sync),
                expectedSize - sizeof(header.sync));
        nextMessageType = header.type;
        return; // Don't set the type back to HEADER, return early.
    }
    nextMessageType = HEADER;
}