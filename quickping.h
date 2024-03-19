#ifndef QUICKPING_H

#include "Arduino.h"
#include <WiFiNINA.h>

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
  void clear(char _state);
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
  QuickPingState getConfigAsBuffer();

  void clearState();
  void setState(char *key, int value);
  void setState(char *key, char *value);

private:
  QuickPingConfig _config;
  QuickPingMessage _message;
  char *_macAddress;
  char _stateBuffer[BODY_SIZE];
  WiFiServer *_wifiServer;
};

#define QUICKPING_H
#endif