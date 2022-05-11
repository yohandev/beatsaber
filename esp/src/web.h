/**
 * [web.h] WiFi + HTTP utilities
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>

class Web {
    char req[1028];     // Request buffer
    char res[1028];     // Response buffer
public:
    /**
     * Attempts to connect to the given network, blocking until connected
     * or has exceeded permitted attempts(500ms each). Returns `true` if
     * connection was succesful.
     */
    bool connect(const char* network, const char* pwd, int attempts = 6) const {
        WiFi.begin(network, pwd);
        // Perform attempts
        for (int i = 0; i < attempts && WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
        }
        return WiFi.isConnected();
    }

    /**
     * Get the response buffer
     */
    const char* response() const {
        return this->res;
    }

    /**
     * Sends an HTTP GET request to `host` at `endpoint`, overwritting this
     * instance's response buffer. Operation is blocking and returns `true`
     * if it suceeded.
     */
    bool get(const char* host, const char* endpoint, ...) {
        // Build HTTP header
        int i = 0;
        va_list args;
        va_start(args, endpoint);
        i += sprintf(&this->req[i], "GET http://%s/", host);
        i += vsprintf(&this->req[i], endpoint, args);
        i += sprintf(&this->req[i], " HTTP/1.1\r\nHost: %s\r\n\r\n", host);
        va_end(args);

        // Send request
        return this->fetch(host, this->req);
    }

    /**
     * Send an HTTP POST request to `host` at `endpoint` with `body`, overwritting
     * this instance's response buffer. Operation is blocking and returns `true`
     * if it suceeded.
     */
    bool post(const char* host, const char* endpoint, const char* body, ...) {
        // Build HTTP header
        int i = 0, cl = 0;
        va_list args;
        va_start(args, body);
        i += sprintf(&this->req[i], "POST /");
        i += vsprintf(&this->req[i], endpoint, args);
        i += sprintf(&this->req[i],
            " HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n", host);
        i += sprintf(&this->req[i], "Content-Length: 000\r\n\r\n");
        // Body + Content-Length
        cl = vsprintf(&this->req[i], body, args);
        this->req[i - 7] = '0' + cl / 100;
        this->req[i - 6] = '0' + (cl / 10) % 10;
        this->req[i - 5] = '0' + cl % 10;
        va_end(args);

        // Send request
        return this->fetch(host, this->req);
    }

    /**
     * Sends a generic HTTP `req`uest to `host`, overwritting this instance's
     * response buffer. (Blocking!)
     */
    bool fetch(const char* host, const char* req, int timeout = 6000) {
        WiFiClient client;
        uint32_t start = millis();

        // Connect on port 80
        if (!client.connect(host, 80)) return false;
        // Send request
        client.print(req);

        // Read header
        while (client.connected()) {
            // Timeout
            if (millis() - start > timeout) return false;
            
            // Read line
            client.readBytesUntil('\n', this->res, sizeof(this->res));
            // End of header(delimited by blank line)
            if (strcmp(this->res, "\r") == 0) break;

            memset(this->res, 0, sizeof(this->res));
        }
        memset(this->res, 0, sizeof(this->res));

        // Read body
        int i;
        for (i = 0; i < sizeof(this->res) && client.available(); i++) {
            this->res[i] = client.read();
        }
        this->res[i] = '\0';

        // Terminate HTTP connection
        client.stop();

        return true;
    }
};