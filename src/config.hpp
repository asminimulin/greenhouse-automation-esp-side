#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>


constexpr bool CREATE_ACCESS_POINT = true;
static constexpr auto AP_SSID = "test";
static constexpr auto AP_PASSWORD = "12345678910";

constexpr bool USE_EXISTING_NETWORK = false;
static const struct {
    const String WIFI_SSID = "4G LTE";
    const String WIFI_PASSWORD = "Hell0pwd";
} NETWORK;

#endif