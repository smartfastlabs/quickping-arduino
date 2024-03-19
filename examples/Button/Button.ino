#include <WiFiS3.h>
#include <quickping.h>
#include <quickping_wifi.h>
#include <quickping_http.h>

const int BUTTON_PIN = 2;
bool stateIsDirty = false;

QuickPing quickPing;
QuickPingState state;

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

  unsigned long startTime = millis();
  Serial.println("[QP] STARTING UP");
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonChange, CHANGE);
  WiFiServer wifiServer(80);

  // TRY TO READ CONFIG
  Serial.println("[QP] RUNNING");

  quickPing.loadConfig(&wifiServer);
  quickPing.run(&wifiServer);
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
