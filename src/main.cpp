#include "main.h"

void setup() {
    for (int i = 0; i < sizeof(displays) / sizeof(Display *); i++) {
        displays[i]->begin();
    }

    debug("hello, world");

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    M5.begin();
#endif

    wifi.begin(ssid, password);

    ntp.begin(timezone_info, ntp_server);
    backend.begin(backend_host, backend_port, backend_url);
}

void loop() {
    // draw if there was a redraw request in last loop iteration
    if (drawing) {
        for(int i = 0; i < sizeof(displays) / sizeof(Display*); i++) {
            displays[i]->start_draw();
            wifi.draw(displays[i]);
            ntp.draw(displays[i]);
            backend.draw(displays[i]);
            displays[i]->end_draw();
        }
        wifi.reset_redraw_request();
        ntp.reset_redraw_request();
        backend.reset_redraw_request();
        drawing = false;
    }

    // execute module loops
    wifi.loop();
    ntp.loop(wifi.status());
    backend.loop(wifi.status());

    // check for redraw requests
    drawing |= wifi.redraw_request();
    drawing |= ntp.redraw_request();
    drawing |= backend.redraw_request();

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_Core2)
    M5.update();

    // TODO introduce abstraction for input
    if (M5.BtnA.wasClicked()) {
        debug("BTN A CLICK");
    }
#endif
}

void debug(String msg) {
    String tag = "[main] ";
    tag.concat(msg);
    xdebug(tag);
};