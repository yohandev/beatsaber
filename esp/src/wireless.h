/**
 * [wireless.h] Bidirectional communication between ESP32s over the ~air~
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "num.h"

/**
 * Wireless event
 */
enum Event {
    None,               // No event this frame
    Connection,         // New connection this frame
    Data,               // Data received this frame
};

/**
 * A Wi-Fi access point
 */
class Host {
    const char* ssid;   // Network ID
    const char* pwd;    // Passphrase

    WiFiServer server;   // Web server
    WiFiClient client;  // Web connection

    u8 rx[128];         // Receive buffer
    u32 rxi;            // Buffer position
public:
    /**
     * 
     */
    Host(const char* ssid, const char* pwd = "1234", u16 port = 80)
        : ssid(ssid), pwd(pwd), server(port) { }
    
    /**
     * Start the web-server and wait for incoming connection.
     * Returns `true` if the operation was succesful.
     */
    bool begin() {
        if (!WiFi.softAP(this->ssid, this->pwd)) {
            return false;
        }
        this->server.begin();

        return true;
    }

    /**
     * Get this network's IP address. Usually 192.168.4.1
     */
    IPAddress ip() const {
        return WiFi.softAPIP();
    }

    /**
     * Poll for new events - new connection, data, or none. Optionally accepts
     * an `rx` argument, which will raise the data event only when the receive
     * buffer is filled to that length(in bytes).
     */
    Event poll(u32 rx = 1) {
        // Accept incoming connection
        if (!this->client) {
            // Remove stale connection
            this->client.stop();

            if (this->client = this->server.available()) {
                return Connection;
            }
            return None;
        }
        // Read data
        if (this->client.available()) {
            this->rx[this->rxi++] = this->client.read();
            // Buffer filled
            if (this->rxi >= rx) {
                this->rxi = 0;
                return Data;
            }
        }
        return None;
    }

    /**
     * Get the received data buffer
     */
    const u8* read() const {
        return this->rx;
    }

    /**
     * Send data to the peer. Returns `true` if operation was succesful.
     */
    bool write(const u8* buf, u32 len) {
        // Predicate that there is an active connection
        if (!this->client || !this->client.available()) {
            return false;
        }
        this->client.write(buf, len);

        return true;
    }
};

/**
 * A Wi-Fi station
 */
class Peer {
    
};