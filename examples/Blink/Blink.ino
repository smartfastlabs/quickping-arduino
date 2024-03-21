// Uncommnet the next line For the Arduino UNO R4 WiFi
#include <WiFiS3.h>

// Uncommnent the next line For most othe boards
// #include <WiFiNINA.h>

#include <quickping.h>

bool ledState = true;

QuickPing quickPing;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    QuickPingConfig config = {
        .serverIP = IPAddress(192, 168, 86, 48),
        .serverPort = 2525,
        .localPort = 2525,
        .uuid = "f2e3e3e3-02c0-4f96-8702-46520d76cee4",
        .ssid = "",
        .wifiPassword = "",
        .debug = true};

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("[QP] RUNNING");

    quickPing.run(&config);
}

void loop()
{
    QuickPingMessage *message = quickPing.loop(ledState ? '1' : '0');
    if (message && message->action == 'C' && message->body[0] == 'T')
    {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }

    free(message);
}
