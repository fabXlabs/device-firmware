#pragma once
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <tiny_websockets/internals/wscrypto/crypto.hpp>
#include <EEPROM.h>
#include "itool.h"
#include "trace.h"
#include "states.h"
#include "config.h"
using namespace websockets;

const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n"\
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"\
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"\
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"\
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"\
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"\
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"\
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"\
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"\
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"\
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"\
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"\
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"\
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"\
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"\
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"\
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"\
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"\
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"\
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"\
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"\
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"\
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"\
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"\
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"\
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"\
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"\
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"\
"nLRbwHOoq7hHwg==\n"\
"-----END CERTIFICATE-----\n";

class Backend
{
public:
    Backend(const char* iHost, const int iPort, const char* iUrl);
    void begin();
    void loop(wl_status_t iWifiStatus, States iCurrentState);
    BackendStates getState();
    void sendGetConfig();
    CloseReason getCloseReason();
    void printTools();
    void setupSecret(bool iForceNew=false);


private:

    void sendToolUnlockResponse(long commandId);
    void sendDeviceRestartResponse(long commandId);
    void sendErrorResponse(long commandId, String message);
    void handleText(const char * iPayload);


    static void websocketEventCallback(WebsocketsEvent event, String data);
    static void websocketMsgCallback(WebsocketsMessage message);

    void handleUnlockToolCommand(DynamicJsonDocument & doc);
    void handleRestartDeviceCommand(long iId);
    void handleConfigurationResponse(DynamicJsonDocument & doc);



private:
    const char* mHost;
    const int mPort;
    const char* mUrl;
    ITool* mLastPointer = nullptr;
    WebsocketsClient mWebSocket;
    bool mInitialized = false;



    //method add authsource
    //save pointer to authsource

    //class authsource
    //get_auth return: json

public:
    String mName;
    String mBackgroundURL;
    String mSecret;
    String mMac;
    std::vector<ITool*> mTools;
    bool mRestartRequest = false;
    bool mUnlockRequest = false;
    long mCurrentCommandId = 0;
    BackendStates mState = BackendStates::INIT;

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
        X_DEBUG("Backend Begin");
        String wsString = "wss://";
        wsString += mHost;
        wsString += mUrl;
        X_DEBUG("Connection details: %s", wsString.c_str());


        if (mSecret.length() == 0){
            sBackend.setupSecret();
        }
        X_DEBUG("Device Mac: %s", mMac.c_str());
        String auth = mMac;
        auth += ":";
        auth += mSecret;
        WSString base64 = crypto::base64Encode((uint8_t *)auth.c_str(), auth.length());
        String authstr = "Basic ";
        authstr += internals::fromInternalString(base64);
        if(!mInitialized)
        {
        mWebSocket.addHeader("Authorization", authstr);
        mWebSocket.onEvent(websocketEventCallback);
        mWebSocket.onMessage(websocketMsgCallback);
        mWebSocket.setCACert(echo_org_ssl_ca_cert);
        mInitialized = true;
        }

        mWebSocket.connect(wsString);
        mState = BackendStates::INIT;

}

inline void Backend::setupSecret(bool iForceNew)
{

    EEPROM.begin(SECRET_LENGTH + 1);
	byte magic = EEPROM.readByte(0);

	if ((magic != 0x42) || iForceNew) {
		X_DEBUG("Generating new Secret!");
		//generate new secret
		EEPROM.writeByte(0, 0x42);
		for (int i = 0; i < SECRET_LENGTH; i++) {
			EEPROM.writeByte(i + 1, (byte) esp_random());
		}
	} else {
		X_DEBUG("Reading Secret from EEPROM");
	}
    sBackend.mSecret = "";
	char buffer[3];
	for (int i = 0; i < SECRET_LENGTH; i++) {
		byte b = EEPROM.readByte(i + 1);
		sprintf(buffer, "%02x", b);
		sBackend.mSecret += buffer;
	}
	EEPROM.end();
}

inline void Backend::loop(wl_status_t iWifiStatus, States iCurrentState)
{
        if (iWifiStatus == WL_CONNECTED) {
            mWebSocket.poll();
        }
        if (mRestartRequest)
        {
            mRestartRequest = false;
            if(iCurrentState == States::IDLE){
                handleRestartDeviceCommand(mCurrentCommandId);
            } 
            else 
            {
                sendErrorResponse(mCurrentCommandId, "Device in use.");
            }
        }     
}

inline BackendStates Backend::getState()
{
    return mState;
}

inline CloseReason Backend::getCloseReason()
{
    return mWebSocket.getCloseReason();
}


inline void Backend::websocketEventCallback(WebsocketsEvent event, String data) 
{
    CloseReason reason = sBackend.getCloseReason();

    switch(event) {
        case WebsocketsEvent::ConnectionOpened:
            X_DEBUG("connected");
            sBackend.sendGetConfig();
            break;
        case WebsocketsEvent::ConnectionClosed:
            switch (reason) {
                case CloseReason::CloseReason_PolicyViolation:
                    X_DEBUG("Policy Violation");
                    sBackend.mState = BackendStates::PROVISIONING;
                    break;
            }
            X_DEBUG("disconnected reason: %d", reason);
            break;       
        case WebsocketsEvent::GotPing:
            X_DEBUG("ping");
            break;
        case WebsocketsEvent::GotPong:
            X_DEBUG("pong");
            break;
        default:
            X_DEBUG("other websocket event");
            break;
    }
}

inline void Backend::websocketMsgCallback(WebsocketsMessage message)
{
    sBackend.handleText(message.data().c_str());
}

inline void Backend::sendGetConfig()
{
    X_DEBUG("sending get config");
    int commandId = (int) esp_random();
    mCurrentCommandId = commandId;
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.GetConfiguration\",\"commandId\":";
    msg += commandId;
     msg += "}";
    if (!mWebSocket.send(msg)) {
        X_DEBUG("sending get config failed");
    }
}
inline void Backend::sendToolUnlockResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.ToolUnlockResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    X_DEBUG("sending tool unlock response %s", msg.c_str());

    if (!mWebSocket.send(msg)) {
        X_DEBUG("sending tool unlock response failed");
    }
}

inline void Backend::sendDeviceRestartResponse(long commandId) {
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.DeviceRestartResponse\",\"commandId\":";
    msg += commandId;
    msg += "}";

    X_DEBUG("sending device restart response");

    if (!mWebSocket.send(msg)) {
        X_DEBUG("sending device restart response failed");
    }
}

inline void Backend::sendErrorResponse(long commandId, String message) 
{
    String msg = "{\"type\":\"cloud.fabX.fabXaccess.device.ws.ErrorResponse\",\"message\":\"";
    msg += message;
    msg += "\",\"commandId\":";
    msg += commandId;
    msg += "\"parameters\":{},\"correlationId\":null";
    msg += "}";
    X_DEBUG("sending error response");
    if (!mWebSocket.send(msg)) {
        X_DEBUG("sending device restart response failed");
    }
}



inline void Backend::handleText(const char * iPayload) {
    DynamicJsonDocument doc(2048);
    DynamicJsonDocument response(1024);
    X_DEBUG("text %s ", (char*) iPayload);

    DeserializationError deserialization_error;
    deserialization_error = deserializeJson(doc, iPayload);

    if (deserialization_error) {
        X_DEBUG("deserialization failed: %s ", deserialization_error.f_str());
        return;
    }
    sBackend.mCurrentCommandId = doc["commandId"];
    if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.ConfigurationResponse") == 0) {
        sBackend.handleConfigurationResponse(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.UnlockTool") == 0) {
        sBackend.handleUnlockToolCommand(doc);
    } else if (strcmp(doc["type"], "cloud.fabX.fabXaccess.device.ws.RestartDevice") == 0) {
        sBackend.mRestartRequest = true;
    }
}

inline void Backend::handleUnlockToolCommand(DynamicJsonDocument & doc) {
    X_DEBUG("handling unlock tool command");
    long commandId = doc["commandId"];
    sendToolUnlockResponse(commandId);
}

inline void Backend::handleRestartDeviceCommand(long iId) {
    X_DEBUG("handling restart device command");
    sendDeviceRestartResponse(iId);
    delay(3000);
    ESP.restart();
}


inline void Backend::handleConfigurationResponse(DynamicJsonDocument & doc) {
    mTools.clear();
    X_DEBUG("handling configuration response");
    if (doc["commandId"] != mCurrentCommandId) return;
    sBackend.mName = String((const char*)doc["name"]);
    sBackend.mBackgroundURL = String((const char*)doc["background"]);
    int i = 0;
    JsonObject attachedTools = doc["attachedTools"];
    for(JsonPair tool: attachedTools)
    {
        //parse Json
        JsonObject toolData = tool.value();
        int pin = atoi(tool.key().c_str());
        bool r2FA = toolData["requires2FA"];
        int time = toolData["time"];
        String name = toolData["name"];
        ToolType type;
        if(toolData["type"] == "UNLOCK") type = ToolType::UNLOCK;
        if(toolData["type"] == "KEEP") type = ToolType::KEEP;
        IdleState idleState;
        if(toolData["idleState"] == "IDLE_LOW") idleState = IdleState::IDLE_LOW;
        if(toolData["idleState"] == "IDLE_HIGH") idleState = IdleState::IDLE_HIGH;
        //create tool
        String id = toolData["id"];
        ITool* currentTool = new ITool(name, type, r2FA, time, idleState, pin, id);
        if(!currentTool)
        {
            X_DEBUG("Allocation error");
            return;
        }
        mLastPointer = currentTool;
        mTools.push_back(currentTool);
        ITool* testTool = mTools.back();
        Serial.printf("Tool: Name: %s, Pin: %d ID: %s\n", testTool->mName.c_str(), testTool->mPin, testTool->mToolId.c_str());
        //TODO save this to the sd card
    }

    mState = BackendStates::CONFIGURED;
}
