#pragma once

#include <Arduino.h>
#include "display.h"

SerialDisplay serial_display;

Display* displays[] = {
    &serial_display
};

void debug(String message) {
    for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
        displays[i]->debug(message);
    }
}