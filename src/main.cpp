#include "main.h"

void setup() {
    for (int i = 0; i < sizeof(displays) / sizeof(Display *); i++) {
        displays[i]->begin();
    }

    debug("hello, world");

    wifi.begin(ssid, password);
}

void loop() {
    delay(2000);
    debug("loop");

    // draw if there was a redraw request in last loop iteration
    if (drawing) {
        for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
            wifi.draw(displays[i]);
        }
        wifi.reset_redraw_request();
        drawing = false;
    }

    // execute module loops
    wifi.loop();

    // check for redraw requests
    drawing |= wifi.redraw_request();
}

void debug(String msg) {
    String tag = "[main] ";
    tag.concat(msg);
    xdebug(tag);
};