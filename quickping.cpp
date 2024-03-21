#include "quickping.h"

const int BUFFER_SIZE = 128;
const int GET = 0;
const int POST = 1;
const int OPTION = 2;

WiFiUDP Udp;

void QuickPing::printWiFiStatus()
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

int QuickPing::connectToWiFi(char *ssid, char *password)
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

bool QuickPing::checkWiFi()
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

char *QuickPing::getMACAddress()
{
  byte mac[6];
  WiFi.macAddress(mac);
  static char macAddress[18];
  sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return macAddress;
}

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
  Udp.beginPacket(_config.serverIP, _config.serverPort);
  Udp.write(_config.uuid, UUID_SIZE - 1);
  Udp.write('!');
  Udp.write(sentAtBuffer);
  Udp.write('!');
  Udp.write(message->action);
  Udp.write('!');
  Udp.write(message->deviceState);
  Udp.write('!');
  if (message->targetDeviceUUID)
  {
    Udp.write(message->targetDeviceUUID, UUID_SIZE - 1);
  }
  Udp.write('!');
  Udp.write(message->body);
  Udp.write('\0');
  Udp.endPacket();
  return sentAt;
}

unsigned long QuickPing::sendPing(char state, char *body)
{
  QuickPingMessage message = {
    action : 'P',
    deviceState : state,
  };
  for (int i = 0; i < BODY_SIZE; i++)
  {
    if (body[i] == '\0')
    {
      break;
    }
    message.body[i] = body[i];
  }
  return sendMessage(&message);
}

unsigned long QuickPing::sendRegister()
{
  QuickPingMessage message = {
    action : 'R',
    deviceState : 'L',
  };

  char *msg = "Hey there ;)";
  for (int i = 0; i < BODY_SIZE; i++)
  {
    if (msg[i] == '\0')
    {
      break;
    }
    message.body[i] = msg[i];
  }

  return sendMessage(&message);
}

QuickPingMessage *QuickPing::loop(char state)
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
    char *ping = "PING";
    lastPing = sendPing(state, ping);
  }

  return message;
}

void QuickPing::run()
{

  WiFiServer wifiServer(80);
  run(&wifiServer);
}

void QuickPing::run(QuickPingConfig *config)
{
  _config = *config;

  run();
}

void QuickPing::run(WiFiServer *wifiServer, QuickPingConfig *config)
{
  _config = *config;

  run(wifiServer);
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