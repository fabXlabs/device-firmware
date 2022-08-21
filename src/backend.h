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

    void sendDeviceRestartResponse(long commandId);

    static void websocketEvent(WStype_t type, uint8_t * payload, size_t length);

    void handleText(uint8_t * payload, size_t length);

    void handleConfigurationResponse(DynamicJsonDocument & doc);

    void handleUnlockToolCommand(DynamicJsonDocument & doc);

    void handleRestartDeviceCommand(DynamicJsonDocument & doc);

    void debug(String message) {
        String tag = "[BE] ";
        tag.concat(message);
        xdebug(tag);
    }
};

extern Backend backend;

void Backend::sendGetConfig() {
    debug("sending get config");
    if (!webSocket.sendTXT("{\"type\":\"cloud.fabX.fabXaccess.device.ws.GetConfiguration\",\"commandId\":42}")) {
        debug("sending get config failed");
    }
}

void Backend::sendToolUnlockResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.ToolUnlockResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    debug("sending tool unlock response");
    debug(msg);

    if (!webSocket.sendTXT(msg)) {
        debug("sending tool unlock response failed");
    }
}

void Backend::sendDeviceRestartResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.DeviceRestartResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    debug("sending device restart response");
    debug(msg);

    if (!webSocket.sendTXT(msg)) {
        debug("sending device restart response failed");
    }
}

void Backend::handleText(uint8_t * payload, size_t length) {
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument response(1024);

    String debug_message = "text ";
    debug_message.concat((char*) payload);
    backend.debug(debug_message);

    DeserializationError deserialization_error;
    deserialization_error = deserializeJson(doc, payload);

    if (deserialization_error) {
        String debug_message = "deserialization failed: ";
        debug_message.concat(deserialization_error.f_str());
        backend.debug(debug_message);
        return;
    }

    if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.ConfigurationResponse") == 0) {
        backend.handleConfigurationResponse(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.UnlockTool") == 0) {
        backend.handleUnlockToolCommand(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.RestartDevice") == 0) {
        backend.handleRestartDeviceCommand(doc);
    }
}

void Backend::handleConfigurationResponse(DynamicJsonDocument & doc) {
    debug("handling configuration response");
    name = (const char*) doc["name"];
    debug(backend.name);

    got_config = true;
    _redraw_request = true;
}

void Backend::handleUnlockToolCommand(DynamicJsonDocument & doc) {
    debug("handling unlock tool command");
    long commandId = doc["commandId"];
    sendToolUnlockResponse(commandId);
}

void Backend::handleRestartDeviceCommand(DynamicJsonDocument & doc) {
    debug("handling restart device command");
    long commandId = doc["commandId"];
    // TODO if device is in use, send ErrorResponse and abort
    sendDeviceRestartResponse(commandId);

    delay(100);

    ESP.restart();
}

void Backend::websocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            backend.debug("disconnected");
            break;
        case WStype_CONNECTED:
            backend.debug("connected");
            backend.sendGetConfig();
            break;
        case WStype_TEXT:
            backend.handleText(payload, length);
            break;
        case WStype_ERROR:
            backend.debug("error");
            break;
        case WStype_PING:
            backend.debug("ping");
            break;
        case WStype_PONG:
            backend.debug("pong");
            break;
        default:
            backend.debug("other websocket event");
            break;
    }
}