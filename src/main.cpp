#include "main.h"

void setup() {
    for (int i = 0; i < sizeof(displays) / sizeof(Display *); i++) {
        displays[i]->begin();
    }

    debug("hello, world");

    wifi.begin(ssid, password);
    ntp.begin(timezone_info, ntp_server);
}

void loop() {
    // draw if there was a redraw request in last loop iteration
    if (drawing) {
        for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
            displays[i]->clear();
            wifi.draw(displays[i]);
            ntp.draw(displays[i]);
        }
        wifi.reset_redraw_request();
        ntp.reset_redraw_request();
        drawing = false;
    }

    // execute module loops
    wifi.loop();
    ntp.loop(wifi.status());

    // check for redraw requests
    drawing |= wifi.redraw_request();
    drawing |= ntp.redraw_request();
}

void debug(String msg) {
    String tag = "[main] ";
    tag.concat(msg);
    xdebug(tag);
};