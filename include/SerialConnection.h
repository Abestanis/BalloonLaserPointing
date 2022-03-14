#pragma once

#include <cstdint>
#include <limits>
#include "CommandHandler.h"


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
         * No command, but indicates waiting for the header of the next command.
         */
        HEADER = std::numeric_limits<uint8_t>::max(),
    };
    
    /**
     * Set up the connection, but doesn't connect it.
     *
     * @param handler A handler for incoming telecommands.
     */
    explicit SerialConnection(const CommandHandler &handler);
    
    /**
     * Initialize the serial connection.
     */
    static void connect();
    
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
    const CommandHandler &handler;
};

