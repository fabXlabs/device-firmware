#pragma once

#include <Arduino.h>
#include "display.h"

extern Display* displays[2];

void xdebug(String message) {
    for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
        displays[i]->debug(message);
    }
}