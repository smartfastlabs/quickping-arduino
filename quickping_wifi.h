#include <quickping.h>

#ifndef QUICKPING_WIFI_H

void printWiFiStatus();
int connectToWiFi(char *ssid, char *password);
bool checkWiFi();
char *getMACAddress();

#define QUICKPING_WIFI_H
#endif