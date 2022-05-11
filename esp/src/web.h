/**
 * [web.h] WiFi + HTTP utilities
 */
#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

class Web {
    char res[1028];     // Response buffer
    char req[1028];     // Request buffer
    
    WiFiClientSecure cli;
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
    bool get(const char* cert, const char* host, const char* endpoint, ...) {
        // Build HTTP header
        int i = 0;
        va_list args;
        va_start(args, endpoint);
        i += sprintf(&this->req[i], "GET http://%s/", host);
        i += vsprintf(&this->req[i], endpoint, args);
        i += sprintf(&this->req[i], " HTTP/1.1\r\nHost: %s\r\n\r\n", host);
        va_end(args);

        // Send request
        return this->fetch(host, this->req, cert);
    }

    /**
     * Send an HTTP POST request to `host` at `endpoint` with `body`, overwritting
     * this instance's response buffer. Operation is blocking and returns `true`
     * if it suceeded.
     */
    bool post(const char* cert, const char* host, const char* endpoint, const char* body, ...) {
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
        return this->fetch(host, this->req, cert);
    }

    /**
     * Sends a generic HTTP `req`uest to `host`, overwritting this instance's
     * response buffer. (Blocking!)
     */
    bool fetch(const char* host, const char* req, const char* cert = NULL, int timeout = 6000) {
        uint32_t start = millis();
        uint16_t port = cert ? 443 : 80;

        this->cli.setHandshakeTimeout(cert ? 30 : 0);
        this->cli.setCACert(cert);

        // Connect on port 80
        if (!this->cli.connect(host, port, timeout)) return false;
        // Send request
        this->cli.print(req);

        // Read header
        while (this->cli.connected()) {
            // Timeout
            if (millis() - start > timeout) return false;
            
            // Read line
            this->cli.readBytesUntil('\n', this->res, sizeof(this->res));
            // End of header(delimited by blank line)
            if (strcmp(this->res, "\r") == 0) break;

            memset(this->res, 0, sizeof(this->res));
        }
        memset(this->res, 0, sizeof(this->res));

        // Read body
        int i;
        for (i = 0; i < sizeof(this->res) && this->cli.available(); i++) {
            this->res[i] = this->cli.read();
        }
        this->res[i] = '\0';

        // Terminate HTTP connection
        this->cli.stop();

        return true;
    }
};