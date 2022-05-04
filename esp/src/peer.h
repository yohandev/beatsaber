/**
 * [peer.h] Wireless peer-to-peer bidirectional communication between ESP32's. 
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include "num.h"

/**
 * A single connection to another ESP32
 */
class Peer {
    u8 mac[6];          // Remote peer's MAC address
public:
    /**
     * Utility method to get this ESP32's MAC address.
     */
    static String addr() {
        // Station mode
        WiFi.mode(WIFI_MODE_STA);
        // MAC address
        return WiFi.macAddress();
    }

    /**
     * Register the given callback to be notified whenever data is received
     * from any peer.
     */
    static void recv(esp_now_recv_cb_t cb) {
        // Station mode
        WiFi.mode(WIFI_MODE_STA);
        // Initialize ESP NOW
        if (esp_now_init() != ESP_OK) {
            Serial.println("Failed to initialize ESP NOW!");
        }
        // Add callback
        esp_now_register_recv_cb(cb);
    }

    /**
     * Create a new remote peer at the given MAC address.
     */
    Peer(const u8* addr) {
        memcpy(this->mac, addr, sizeof(this->mac));
    }

    /**
     * Begin the connection to the remote peer. Returns `true` if connection
     * was succesful.
     */
    bool begin() {
        // Station mode
        WiFi.mode(WIFI_MODE_STA);
        // Initialize ESP-NOW
        if (esp_now_init() != ESP_OK) {
            return false;
        }

        // Register peer
        esp_now_peer_info_t info = {
            .channel = 0,
            .encrypt = false,
        };
        memcpy(info.peer_addr, this->mac, sizeof(this->mac));

        return esp_now_add_peer(&info) == ESP_OK;
    }

    /**
     * Send data to the remote peer. Returns `true` if operation was succesful.
     */
    bool send(const u8* buf, u32 len) {
        return esp_now_send(this->mac, buf, len) == ESP_OK;
    }
};