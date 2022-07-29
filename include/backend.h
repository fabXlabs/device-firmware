#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include "xdebug.h"

class Backend {
public:
    void begin(const char* backend_host, const int backend_port, const char* backend_url) {
        debug("begin");
        webSocket.beginSSL(backend_host, backend_port, backend_url, NULL, NULL);
        webSocket.onEvent(Backend::websocketEvent);
        webSocket.setAuthorization("aabbcc000000", "supersecret");
    }

    void draw(Display* display) {
        display->debug("Backend draw");

        if (got_config) {
            display->name(name);
        }
    }

    bool redraw_request() {
        return _redraw_request;
    }

    void reset_redraw_request() {
        _redraw_request = false;
    }

    void loop(wl_status_t wifi_status) {
        if (wifi_status == WL_CONNECTED) {
            webSocket.loop();
        }
    }

    void sendGetConfig();

    bool got_config = false;
    String name;

private:
    bool _redraw_request = false;

    WebSocketsClient webSocket;

    void sendToolUnlockResponse(long commandId);

    static void websocketEvent(WStype_t type, uint8_t * payload, size_t length);

    void handleText(uint8_t * payload, size_t length);

    void handleConfigurationResponse(DynamicJsonDocument & doc);

    void handleUnlockToolCommand(DynamicJsonDocument & doc);

    void debug(String message) {
        String tag = "[BE] ";
        tag.concat(message);
        xdebug(tag);
    }
};

extern Backend backend;