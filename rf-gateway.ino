#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "arduino_secrets.h"

#define WIFI_RECOVER_TIME_MS 10000 // Wait time after a failed connection attempt

#define OTA_HOSTNAME "rf-gateway-esp32"

#define MQTT_BROKER_PORT 8883
#define MQTT_CLIENT_ID "rf-gateway"
#define MQTT_RECOVER_TIME_MS 10000 // Wait time after a failed connection attempt
#define RF_GATEWAY_TOPIC "home/rf-gateway"

#define RF_INTERRUPT_GPIO 17
#define RF_FREQUENCY 433.00
#define RF_GATEWAY_ADDRESS 1

RH_RF69 rf69Driver(SS, RF_INTERRUPT_GPIO);
RHReliableDatagram radioManager(rf69Driver, RF_GATEWAY_ADDRESS);

WiFiClientSecure wifiClientSecure;
PubSubClient mqttClient(MQTT_BROKER_HOST, MQTT_BROKER_PORT, wifiClientSecure);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  ensureWifiConnection();
  initRadio();
  initMQTT();
  initOTA();
}

uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop()
{
  ensureWifiConnection();
  ensureMqttConnection();

  if (radioManager.available())
  {
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (radioManager.recvfromAck(buf, &len, &from))
    {
      Serial.print("[RFM69] Message received from node ");
      Serial.print(from);
      Serial.print(": ");
      Serial.println((char *)buf);
      
      char payload[80];
      sprintf(payload, "%s;%s", from, buf);
      mqttClient.publish(RF_GATEWAY_TOPIC, payload, true);
    }
  }

  mqttClient.loop();
}

void ensureWifiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WIFI] Connecting");
    WiFi.enableSTA(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    if (WiFi.waitForConnectResult() == WL_CONNECTED)
    {
      Serial.println("[WIFI] Connected");
      Serial.print("[WIFI] IP address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("[WIFI] Failed");
      delay(WIFI_RECOVER_TIME_MS);
      ESP.restart();
    }
  }
}

void initRadio()
{
  if (radioManager.init())
  {
    Serial.println("[RFM69] Init success");
  }
  else
  {
    Serial.println("[RFM69] Init failed");
  }
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (rf69Driver.setFrequency(RF_FREQUENCY))
  {
    Serial.println("[RFM69] Frequency set to 433MHz");
  }
  else
  {
    Serial.println("[RFM69] Failed to set frequency");
  }

  uint8_t encryptionKey[] = SECRET_RF_ENCRYPTION_KEY;
  rf69Driver.setEncryptionKey(encryptionKey);
}

void initMQTT()
{
  wifiClientSecure.setCACert(root_ca_certificate);
  wifiClientSecure.setCertificate(client_certificate);
  wifiClientSecure.setPrivateKey(client_private_key);

  mqttClient.setSocketTimeout(10);
}

void initOTA()
{
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPasswordHash(OTA_PASSWORD_MD5_HASH);

  ArduinoOTA.onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
        type = "sketch";
    }
    else
    {
        type = "filesystem";
    }
    Serial.println("[OTA] Start updating " + type);
    })
    .onEnd([]()
    { Serial.println("\n[OTA] End"); })
    .onProgress([](unsigned int progress, unsigned int total)
    { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); })
    .onError([](ota_error_t error)
    {
      Serial.printf("[OTA] Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void ensureMqttConnection()
{
    if (!mqttClient.connected())
    {
        Serial.println("[MQTT] Connecting");

        if (mqttClient.connect(MQTT_CLIENT_ID))
        {
            Serial.println("[MQTT] Connected to broker");
        }
        else
        {
            Serial.print("[MQTT] Connection failed, rc=");
            Serial.println(mqttClient.state());
            delay(MQTT_RECOVER_TIME_MS);
        }
    }
}