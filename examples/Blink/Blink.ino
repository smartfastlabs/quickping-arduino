#include <quickping.h>

const int BUTTON_PIN = 2;
bool stateIsDirty = false;

QuickPing quickPing;
QuickPingState state;
QuickPingConfig config;

void onButtonChange()
{
    stateIsDirty = true;
    digitalWrite(LED_BUILTIN, digitalRead(BUTTON_PIN));
}

void setup()
{
    state.clear(69);
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }

    config.ssid = "Moose Wifi";
    config.wifiPassword = "straderishere";
    config.uuid = "6895285d-02c0-4f96-8702-46520d76cee4";
    config.localPort = 2525;
    config.serverIP = IPAddress(192, 168, 86, 48);
    config.serverPort = 2525;
    config.debug = true;

    unsigned long startTime = millis();
    Serial.println("[QP] STARTING UP");
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonChange, CHANGE);
    WiFiServer wifiServer(80);

    // TRY TO READ CONFIG
    Serial.println("[QP] RUNNING");

    quickPing.run(&wifiServer, &config);
}

void loop()
{
    if (stateIsDirty)
    {
        state.clear(digitalRead(BUTTON_PIN));
        state.addValue("button", digitalRead(BUTTON_PIN));
        state.addValue("button2", !digitalRead(BUTTON_PIN));
        quickPing.sendPing(&state);
        stateIsDirty = false;
    }
    else
    {
        state.clear(digitalRead(BUTTON_PIN));
        state.addValue("millis", millis());
        QuickPingMessage *message = quickPing.loop(&state);
        free(message);
    }
}
