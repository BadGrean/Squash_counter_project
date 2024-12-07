#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "InteractionHandler.h"
#include "DisplayControl.h"

// BH1750 I2C addresses
#define BH1750_ADDR2 0x23
#define BH1750_ADDR1 0x5C

#define CMD_MEASURE 0x10
const unsigned long THRESHOLD = 7;

bool sensor1BelowThreshold = false;
bool sensor2BelowThreshold = false;

// Variables to track scores and names
uint8_t counterid = 0;
uint8_t player1Score = 0;
uint8_t player2Score = 0;
bool matchEnd = false;
String player1_name = "XXXX";  // Default player names
String player2_name = "XXXX";

// WiFi and MQTT configuration
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "192.168.0.50";

WiFiClient espClient;
PubSubClient client(espClient);
bool mqttConnected = false; // Track MQTT connection status

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(21, 22);

  Serial.println("Initializing BH1750 sensors...");
  if (beginBH1750(BH1750_ADDR1)) {
    Serial.println("BH1750 Sensor 1 initialized successfully!");
  } else {
    Serial.println("Failed to initialize BH1750 Sensor 1.");
  }

  if (beginBH1750(BH1750_ADDR2)) {
    Serial.println("BH1750 Sensor 2 initialized successfully!");
  } else {
    Serial.println("Failed to initialize BH1750 Sensor 2.");
  }

  setupDisplayGPIO();
  displayScore(player1Score, player2Score);

  if (setupWiFi()) {
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
    mqttConnected = setupMQTT(); // Attempt MQTT setup and store result
    if (mqttConnected) {
      Serial.println("MQTT setup completed.");
    } else {
      Serial.println("MQTT not available. Proceeding without it.");
    }
  } else {
    Serial.println("WiFi setup failed.");
  }

  publishMatchData(); // Initial match data (even without MQTT)
}

void loop() {
  // If MQTT was successfully set up, process MQTT messages
  if (mqttConnected) {
    if (!client.connected()) {
      // Try reconnecting only if MQTT was initially connected
      mqttConnected = setupMQTT(); 
    }

    if (mqttConnected) {
      client.loop();
    }
  }

  // Sensor logic
  sensor1BelowThreshold = (readLightLevel(BH1750_ADDR1) < THRESHOLD);
  sensor2BelowThreshold = (readLightLevel(BH1750_ADDR2) < THRESHOLD);

  switch (processInteraction(sensor1BelowThreshold, sensor2BelowThreshold)) {
    case Player1_point:
      player1Score++;
      publishMatchData();
      displayScore(player1Score, player2Score);
      Serial.print(player1Score);
      Serial.print(" : ");
      Serial.println(player2Score);
      break;

    case Player2_point:
      player2Score++;
      publishMatchData();
      displayScore(player1Score, player2Score);
      Serial.print(player1Score);
      Serial.print(" : ");
      Serial.println(player2Score);
      break;

    case Game_restarted:
      player1Score = 0;
      player2Score = 0;
      publishMatchData();
      displayScore(player1Score, player2Score);
      break;

    case Match_ended:
      Serial.println("Match has ended!");
      displayScore(player1Score, player2Score);
      matchEnd = true;
      publishMatchData();
      matchEnd = false;
      displayScore(8, 8);
      delay(1000);
      player1Score = 0;
      player2Score = 0;
      player1_name = "XXXX";
      player2_name = "XXXX";
      publishMatchData();
      displayScore(player1Score, player2Score);
      break;

    default:
      break;
  }
}

// Function to initialize BH1750 sensor
bool beginBH1750(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(CMD_MEASURE);
  return Wire.endTransmission() == 0;
}

// Function to read light level from the BH1750
float readLightLevel(uint8_t addr) {
  uint16_t lux = 0;
  Wire.beginTransmission(addr);
  Wire.requestFrom((uint8_t)addr, (uint8_t)2);
  if (Wire.available() == 2) {
    lux = (Wire.read() << 8) | Wire.read();
  }
  return lux / 1.2;
}

bool setupWiFi() {
  Serial.println("Connecting to WiFi...");
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 5) {
    WiFi.begin(ssid, password);
    delay(1000);
    attempt++;
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nFailed to connect to WiFi after 5 attempts.");
    return false;
  }
}

bool setupMQTT() {
  Serial.println("Connecting to MQTT...");
  for (int attempt = 0; attempt < 3; attempt++) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker.");
      client.subscribe("counter_download");
      Serial.println("Subscribed to topic: counter_download");
      return true;
    }
    Serial.print("Failed, rc=");
    Serial.print(client.state());
    Serial.println(" retrying...");
    delay(1000);
  }
  Serial.println("Failed to connect to MQTT after 3 attempts.");
  return false;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "counter_download") != 0) return;

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  uint8_t receivedCounterId = doc["counter_id"];
  uint8_t receivedScore1 = doc["score1"];
  uint8_t receivedScore2 = doc["score2"];
  const char* receivedPlayer1Name = doc["player1_name"];
  const char* receivedPlayer2Name = doc["player2_name"];

  if (receivedCounterId == counterid) {
    player1Score = receivedScore1;
    player2Score = receivedScore2;
    player1_name = receivedPlayer1Name;
    player2_name = receivedPlayer2Name;
    displayScore(player1Score, player2Score);
    publishMatchData();
  }
}

void publishMatchData() {
  if (!mqttConnected) return; // Skip publishing if MQTT is not connected

  StaticJsonDocument<256> doc;
  doc["counter_id"] = counterid;
  doc["score1"] = player1Score;
  doc["score2"] = player2Score;
  doc["match_end"] = matchEnd;
  doc["player1_name"] = player1_name;
  doc["player2_name"] = player2_name;

  char buffer[256];
  size_t len = serializeJson(doc, buffer);
  client.publish("counter_upload", buffer, len);

  Serial.println("Data published to MQTT:");
  serializeJsonPretty(doc, Serial);
  Serial.println();
}

void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; address++) {
    // The I2C address must be shifted left by 1
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println();
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Found ");
    Serial.print(nDevices);
    Serial.println(" device(s) on the I2C bus.");
  }
}
