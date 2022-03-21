#pragma once

#include <cstdint>
#include <limits>
#include "units.h"


/**
 * A connection via a serial port which can receive commands.
 */
class SerialConnection {
public:

    /**
     * All supported commands.
     */
    enum MessageType : uint8_t {

        /**
         * Request a PONG response
         */
        PING = 0,

        /**
         * Sets the pointing target GPS position.
         */
        GPS = 1,

        /**
         * Requests a motor calibration.
         */
        CALIBRATE_MOTORS = 2,

        /**
         * Sets the location and orientation of the laser pointing structure.
         */
        SET_LOCATION = 3,

        /**
         * No command, but indicates waiting for the header of the next command.
         */
        HEADER = std::numeric_limits<uint8_t>::max(),
    };

    /**
     * A handler for incoming telecommands.
     */
    class CommandHandler {
    public:
        /**
         * Handle a ping request.
         */
        virtual void handlePing() const = 0;

        /**
         * Handle a new pointing target GPS position.
         *
         * @param latitude The latitude in degrees.
         * @param longitude The longitude in degrees.
         * @param height The height in meter.
         */
        virtual void handleGps(deg_t latitude, deg_t longitude, meter_t height) = 0;

        /**
         * Handle a request to calibrate the motors.
         */
        virtual void handleMotorsCalibration() = 0;

        /**
         * Handle a new location and orientation for the laser pointing structure.
         *
         * @param latitude The latitude of the new location in degrees.
         * @param longitude The longitude of the new location in degrees.
         * @param height The height of the new location in meters.
         * @param orientation The new orientation in degrees from the north direction.
         */
        virtual void handleSetLocation(
                deg_t latitude, deg_t longitude, meter_t height, deg_t orientation) = 0;
    };

    /**
     * Set up the connection, but doesn't connect it.
     *
     * @param handler A handler for incoming telecommands.
     */
    explicit SerialConnection(CommandHandler &handler);

    /**
     * Check for and handle incoming messages.
     */
    void fetchMessages();

private:

    /**
     * The type of the next expected message.
     */
    MessageType nextMessageType = HEADER;

    /**
     * A handler for incoming telecommands.
     */
    CommandHandler &handler;
};

