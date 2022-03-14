#pragma once


/**
 * A handler for incoming telecommands.
 */
class CommandHandler {
public:
    /**
     * Handle a ping request.
     */
    void handlePing() const;
    
    /**
     * Handle a new pointing target GPS position.
     *
     * @param latitude The latitude in degrees.
     * @param longitude The longitude in degrees.
     * @param height The height in meter.
     */
    void handleGps(double latitude, double longitude, double height) const;
};
