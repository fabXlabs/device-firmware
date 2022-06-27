#include "main.h"

void setup() {
    for (int i = 0; i < sizeof(displays) / sizeof(Display *); i++) {
        displays[i]->begin();
    }

    debug("hello, world");
}

void loop() {
    delay(2000);
    debug("and another hello world");
}