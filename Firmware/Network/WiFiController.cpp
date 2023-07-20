#include "WiFiController.h"

#include <cstdio>

#include <pico/cyw43_arch.h>

namespace Network
{

WiFiController::WiFiController()
{
    if (cyw43_arch_init() != 0) {
        puts("cyw43_arch_init failed");
        return;
    }

    cyw43_arch_enable_sta_mode();

    _initialized = true;
}

WiFiController::~WiFiController()
{
    cyw43_arch_deinit();
}

void WiFiController::run()
{
    if (!_initialized) {
        return;
    }

    const auto connected = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_JOIN;

    if (connected != _connected) {
        if (connected) {
            puts("WiFi: connected");
        } else {
            puts("WiFi: disconnected");
        }

        _connected = connected;
    }

    if (_connected) {
        return;
    }

    puts("WiFi: connecting");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        puts("cyw43_arch_wifi_connect_timeout_ms failed");
    }
}

bool WiFiController::isConnected() const
{
    return _connected;
}

}