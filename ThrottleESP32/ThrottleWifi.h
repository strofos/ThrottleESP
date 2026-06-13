#ifndef THROTTLE_WIFI_H
#define THROTTLE_WIFIMNG_H

void startWifiConnect();
void checkWifiConnection();
void saveWifiConfig(const char* newSsid, const char* newPass);
void loadWifiConfig();

#endif