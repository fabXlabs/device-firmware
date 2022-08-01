#include "backend.h"

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

    delay(3000);

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