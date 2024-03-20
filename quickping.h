#ifndef QUICKPING_H

#include "Arduino.h"

#if __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#else
#include <WiFiNINA.h>
#endif

const int PACKET_SIZE = 1024;
const int DEVICE_ID_SIZE = 19;
const int UUID_SIZE = 37;
const int BODY_SIZE = PACKET_SIZE - 3 - DEVICE_ID_SIZE - UUID_SIZE;

struct QuickPingMessage
{
  char action;
  char deviceState;
  char body[BODY_SIZE];
};

struct QuickPingConfig
{
  int timeOutSeconds;
  IPAddress serverIP;
  int serverPort;
  int localPort;
  char *uuid;
  char *ssid;
  char *wifiPassword;
  bool debug;
};

class QuickPingState
{
public:
  QuickPingState();
  void clear();
  void reset(char _state);
  void addValue(char *key, int value);
  void addValue(char *key, char *value);
  char *getString();

  char buffer[BODY_SIZE];
  char state;
};

class QuickPing
{
public:
  QuickPing();
  void updateState(QuickPingState *state);
  QuickPingMessage *loop(QuickPingState *state);
  int loopHTTP();
  QuickPingMessage *readMessage();
  void run(WiFiServer *wifiServer);
  void run(WiFiServer *wifiServer, QuickPingConfig *config);
  unsigned long sendMessage(QuickPingMessage *message);
  unsigned long sendPing(QuickPingState *state);
  unsigned long sendRegister();
  unsigned long lastPing;
  unsigned long lastResponse;

  void clearState();
  void setState(char *key, int value);
  void setState(char *key, char *value);
  void printWiFiStatus();
  int connectToWiFi(char *ssid, char *password);
  bool checkWiFi();
  char *getMACAddress();

private:
  QuickPingConfig _config;
  QuickPingMessage _message;
  char *_macAddress;
  char _stateBuffer[BODY_SIZE];
  WiFiServer *_wifiServer;
};

#define QUICKPING_H
#endif