#include "quickping.h"
#include "quickping_wifi.h"
#include <WiFiNina.h>

const int BUFFER_SIZE = 128;
const int GET = 0;
const int POST = 1;
const int OPTION = 2;

WiFiUDP Udp;

QuickPingMessage *QuickPing::readMessage()
{
  int packetSize = Udp.parsePacket();
  if (!packetSize)
  {
    return NULL;
  }
  if (packetSize > PACKET_SIZE)
  {
    Serial.println("[QP] PACKET TOO LARGE TRUNCATING");
    packetSize = PACKET_SIZE;
  }

  QuickPingMessage *message = (struct QuickPingMessage *)malloc(sizeof(struct QuickPingMessage));

  char buffer[packetSize + 1];
  buffer[packetSize] = '\0'; // null terminate the string
  Udp.read(buffer, packetSize);
  if (_config.debug)
  {
    Serial.print("[QP] RECEIVED PACKET: (");
    Serial.print(packetSize);
    Serial.print(")\n\tBODY: `");
    Serial.print(buffer);
    Serial.println("`");
  }
  message->action = buffer[0];
  // TODO: Verify device_uuid
  if (false && strcmp(_config.uuid, &buffer[1]) != 0)
  {
    Serial.println("[QP] INVALID DEVICE UUID");
    return NULL;
  }
  strcpy(message->body, &buffer[39]);
  message->body[packetSize] = '\0'; // null terminate the string

  if (_config.debug)
  {
    Serial.println("[QP] GOT A MESSAGE");
    Serial.print("\tACTION: `");
    Serial.print(message->action);
    Serial.print("\n\tBODY: `");
    Serial.print(message->body);
    Serial.println("`");
  }

  return message;
}

unsigned long QuickPing::sendMessage(QuickPingMessage *message)
{
  if (_config.debug)
  {
    Serial.print("[QP] SENDING PING: local: ");
    Serial.print(WiFi.localIP());
    Serial.print(" server: ");
    Serial.println(_config.serverIP.toString());
  }

  unsigned long sentAt = millis();
  char sentAtBuffer[33];
  ltoa(sentAt, sentAtBuffer, 10);

  Udp.begin(_config.localPort);

  Serial.print("[QP] SENDING PING:");
  Serial.print(_config.serverIP.toString());
  Udp.beginPacket(_config.serverIP, _config.serverPort);
  Udp.write(_config.uuid, UUID_SIZE - 1);
  Udp.write('!');
  Udp.write(sentAtBuffer);
  Udp.write('!');
  Udp.write(message->action);
  Udp.write('!');
  Udp.write(message->body);
  Udp.write('\0');
  Udp.endPacket();
  return sentAt;
}

unsigned long QuickPing::sendPing(QuickPingState *state)
{
  QuickPingMessage message = {
    action : 'P',
  };
  for (int i = 0; i < BODY_SIZE; i++)
  {
    if (state->buffer[i] == '\0')
    {
      break;
    }
    message.body[i] = state->buffer[i];
  }
  return sendMessage(&message);
}

QuickPingState QuickPing::getConfigAsBuffer()
{
  QuickPingState state;
  state.clear();
  state.addValue("ssid", _config.ssid);
  state.addValue("password", _config.wifiPassword);
  state.addValue("server_ip", _config.serverIP);
  state.addValue("server_port", _config.serverPort);
  state.addValue("local_port", _config.localPort);
  state.addValue("ping_timeout", _config.timeOutSeconds);
  return state;
}

unsigned long QuickPing::sendRegister()
{
  QuickPingMessage message = {
    action : 'R',
  };

  QuickPingState state;
  state.clear('R');
  state.addValue("ping_timeout", _config.timeOutSeconds);
  state.addValue("millis", millis());
  for (int i = 0; i < BODY_SIZE; i++)
  {
    if (state.buffer[i] == '\0')
    {
      break;
    }
    message.body[i] = state.buffer[i];
  }

  return sendMessage(&message);
}

void QuickPing::updateState(QuickPingState *state)
{
  unsigned long time = millis();
  sendPing(state);
}

QuickPingMessage *QuickPing::loop(QuickPingState *state)
{
  unsigned long time = millis();
  QuickPingMessage *message = readMessage();
  if (message)
  {
    lastResponse = time;
    if (message->action == 'R')
    {
      Serial.println("[QP] RECEIVED REGISTER REQUEST, SENDING PING");
      sendRegister();
    }
  }
  else if (time - lastPing > 5000)
  {
    lastPing = sendPing(state);
  }

  return message;
}

void QuickPing::run(WiFiServer *wifiServer, QuickPingConfig *config)
{
  _config = *config;

  return run(wifiServer);
}

void QuickPing::run(WiFiServer *wifiServer)
{
  _wifiServer = wifiServer;

  checkWiFi();

  connectToWiFi(_config.ssid, _config.wifiPassword);

  printWiFiStatus();
  _macAddress = getMACAddress();
  Serial.print("[QP] MAC ADDRESS: ");
  Serial.println(_macAddress);
  sendRegister();
}

QuickPing::QuickPing()
{
}

void QuickPingState::clear()
{
  for (int i = 1; i < BODY_SIZE; i++)
  {
    buffer[i] = '\0';
  }
  buffer[0] = state;
  buffer[1] = '!';
}

void QuickPingState::clear(char _state)
{
  state = _state;
  clear();
}

void QuickPingState::addValue(char *key, int value)
{
  strcpy(&buffer[strlen(buffer)], "(i:");
  strcpy(&buffer[strlen(buffer)], key);
  strcpy(&buffer[strlen(buffer)], ":");
  ltoa(value, &buffer[strlen(buffer)], 10);
  strcpy(&buffer[strlen(buffer)], ")");
}

void QuickPingState::addValue(char *key, char *value)
{
  strcpy(&buffer[strlen(buffer)], "(s:");
  strcpy(&buffer[strlen(buffer)], key);
  strcpy(&buffer[strlen(buffer)], ":");
  strcpy(&buffer[strlen(buffer)], value);
  strcpy(&buffer[strlen(buffer)], ")");
}

char *QuickPingState::getString()
{
  return buffer;
}

QuickPingState::QuickPingState()
{
}