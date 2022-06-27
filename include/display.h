#pragma once

#include "Arduino.h"

class Display {
public: 
    virtual void begin() = 0;
    virtual void debug(String message) = 0;
    virtual ~Display() {}
};

class SerialDisplay : public Display {
public:
    virtual void begin() {
        Serial.begin(115200);
    }

    virtual void debug(String message) {
        Serial.print("[DEBUG] ");
        Serial.println(message);
    }
};