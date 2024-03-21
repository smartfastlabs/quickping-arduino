// Uncommnet the next line For the Arduino UNO R4 WiFi
#include <WiFiS3.h>

// Uncommnent the next line For most othe boards
// #include <WiFiNINA.h>

#include <quickping.h>

const int BUTTON_PIN = 2;
bool stateIsDirty = false;

QuickPing quickPing;

void onButtonChange()
{
  stateIsDirty = true;
  digitalWrite(LED_BUILTIN, digitalRead(BUTTON_PIN));
}

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
      .uuid = "6895285d-02c0-4f96-8702-46520d76cee4",
      .ssid = "",
      .wifiPassword = "",
      .debug = true};

  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonChange, CHANGE);

  Serial.println("[QP] RUNNING");

  quickPing.run(&config);
}

char *ON = "{\"state\":\"ON\"}";
char *OFF = "{\"state\":\"OFF\"}";

void loop()
{
  if (stateIsDirty)
  {
    quickPing.sendPing(
        digitalRead(BUTTON_PIN) ? '1' : '0',
        digitalRead(BUTTON_PIN) ? ON : OFF);

    QuickPingMessage msg = {
        .action = 'C',
        .deviceState = '0',
        .body = {'T', '\0'},
        .targetDeviceUUID = "f2e3e3e3-02c0-4f96-8702-46520d76cee4"};
    quickPing.sendMessage(&msg);
    stateIsDirty = false;
  }
  else
  {
    free(quickPing.loop(digitalRead(BUTTON_PIN) ? '1' : '0'));
  }
}
