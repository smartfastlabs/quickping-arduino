#include "quickping.h"
#include "quickping_wifi.h"

const int GET = 0;
const int POST = 1;
const int OPTION = 2;

void printWiFiStatus()
{
    Serial.print("[QP] SSID: ");
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    Serial.print("[QP] IP Address: ");
    Serial.println(ip);
    byte bssid[6];
    WiFi.BSSID(bssid);

    long rssi = WiFi.RSSI();
    Serial.print("[QP] Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

int connectToWiFi(char *ssid, char *password)
{
    int connectionAttempts = 2;
    int wifiStatus = WL_IDLE_STATUS;
    while (wifiStatus != WL_CONNECTED)
    {
        if (connectionAttempts-- < 0)
        {
            return NULL;
        }
        Serial.print("[QP] Attempting to connect to SSID: ");
        Serial.println(ssid);
        wifiStatus = WiFi.begin(ssid, password);

        delay(2000);
    }

    return wifiStatus;
}

bool checkWiFi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("[QP] Communication with WiFi module failed!");
        while (true)
            ;
    }

    if (WiFi.firmwareVersion() < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("[QP] Please upgrade the firmware");
    }
}

char *getMACAddress()
{
    byte mac[6];
    WiFi.macAddress(mac);
    static char macAddress[18];
    sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macAddress;
}