#pragma once
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

class Backend
{
public:
    Backend(const char* iHost, const int iPort, const char* iUrl);
    void begin();
    void loop(wl_status_t iWifiStatus);

private:
    void sendGetConfig();
    void sendToolUnlockResponse(long commandId);
    void sendDeviceRestartResponse(long commandId);
    void handleText(uint8_t * iPayload, size_t iLength);

    static void websocketEventHandler(WStype_t iType, uint8_t * iPayload, size_t iLength);
    void handleUnlockToolCommand(DynamicJsonDocument & doc);
    void handleRestartDeviceCommand(DynamicJsonDocument & doc);
    void handleConfigurationResponse(DynamicJsonDocument & doc);



private:
    const char* mHost;
    const int mPort;
    const char* mUrl;
    WebSocketsClient mWebSocket;
    const char* mName;
    bool mConfigured = false;
    //Event handler für events vom schäffner


    //method add authsource
    //save pointer to authsource

    //class authsource
    //get_auth return: json
};

extern Backend sBackend;


inline Backend::Backend(const char* iHost, const int iPort, const char* iUrl)
: mHost(iHost)
, mPort(iPort)
, mUrl(iUrl)
{
    
}

inline void Backend::begin() 
{
        mWebSocket.beginSSL(mHost, mPort, mUrl, NULL, NULL);
        mWebSocket.onEvent(websocketEventHandler);
        mWebSocket.setAuthorization("aabbcc000000", "supersecret");
}

inline void Backend::loop(wl_status_t iWifiStatus)
{
        if (iWifiStatus == WL_CONNECTED) {
            mWebSocket.loop();
        }
}

inline void Backend::websocketEventHandler(WStype_t iType, uint8_t * iPayload, size_t iLength) 
{
        switch(iType) {
        case WStype_DISCONNECTED:
            X_DEBUG("disconnected");
            break;
        case WStype_CONNECTED:
            X_DEBUG("connected");
            sBackend.sendGetConfig();
            break;
        case WStype_TEXT:
            sBackend.handleText(iPayload, iLength);
            break;
        case WStype_ERROR:
            X_DEBUG("Backend error");
            break;
        case WStype_PING:
            X_DEBUG("ping");
            break;
        case WStype_PONG:
            X_DEBUG("pong");
            break;
        default:
            X_DEBUG("other websocket event");
            break;
    }

}

inline void Backend::sendGetConfig()
{
    X_DEBUG("sending get config");
    if (!mWebSocket.sendTXT("{\"type\":\"cloud.fabX.fabXaccess.device.ws.GetConfiguration\",\"commandId\":42}")) {
        X_DEBUG("sending get config failed");
    }
}
void Backend::sendToolUnlockResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.ToolUnlockResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    X_DEBUG("sending tool unlock response %s", msg);

    if (!mWebSocket.sendTXT(msg)) {
        X_DEBUG("sending tool unlock response failed");
    }
}

void Backend::sendDeviceRestartResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.DeviceRestartResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    X_DEBUG("sending device restart response");

    if (!mWebSocket.sendTXT(msg)) {
        X_DEBUG("sending device restart response failed");
    }
}



void Backend::handleText(uint8_t * iPayload, size_t iLength) {
    DynamicJsonDocument doc(1024);
    DynamicJsonDocument response(1024);
    X_DEBUG("text %s ", (char*) iPayload);

    DeserializationError deserialization_error;
    deserialization_error = deserializeJson(doc, iPayload);

    if (deserialization_error) {
        X_DEBUG("deserialization failed: %s ", deserialization_error.f_str());
        return;
    }

    if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.ConfigurationResponse") == 0) {
        sBackend.handleConfigurationResponse(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.UnlockTool") == 0) {
        sBackend.handleUnlockToolCommand(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.RestartDevice") == 0) {
        sBackend.handleRestartDeviceCommand(doc);
    }
}

void Backend::handleUnlockToolCommand(DynamicJsonDocument & doc) {
    X_DEBUG("handling unlock tool command");
    long commandId = doc["commandId"];
    sendToolUnlockResponse(commandId);
}

void Backend::handleRestartDeviceCommand(DynamicJsonDocument & doc) {
    X_DEBUG("handling restart device command");
    long commandId = doc["commandId"];
    // TODO if device is in use, send ErrorResponse and abort
    sendDeviceRestartResponse(commandId);

    delay(3000);

    ESP.restart();
}

void Backend::handleConfigurationResponse(DynamicJsonDocument & doc) {
    X_DEBUG("handling configuration response");
    sBackend.mName = (const char*) doc["name"];
    X_DEBUG(sBackend.mName);

    mConfigured = true;
}
