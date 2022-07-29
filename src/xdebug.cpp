#include "xdebug.h"

void xdebug(String message) {
    for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
        displays[i]->debug(message);
    }
}