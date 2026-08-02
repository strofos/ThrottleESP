#ifndef THROTTLE_WIFI_H
#define THROTTLE_WIFIMNG_H

void startWifiConnect();
void checkWifiConnection();
void saveWifiConfig(String newSsid, String newPass);
void loadWifiConfig();

#endif