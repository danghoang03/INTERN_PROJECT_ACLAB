#include <WiFi.h>
#include <PubSubClient.h>
#include <AdafruitIO_WiFi.h>
#include <DHT20.h>
#include <Wire.h>
#include "RelayStatus.h"

#define TXD 8
#define RXD 9

// WiFi and Adafruit IO configuration
const char* ssid = "ACLAB";
const char* password = "ACLAB2023";
const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttUser = "dang03";
const char* mqttPassword = "Active_Key";
const char* mqttTopic = "dang03/feeds/dashboard"; 
const char* mqttRelay = "dang03/feeds/relay";

WiFiClient espClient;
PubSubClient client(espClient);

// Initialize DHT20 sensor
DHT20 DHT;
#define SCL 12
#define SDA 11
#define BAUD_RATE 9600


#define lightSensor 1
#define moistureSensor 3

unsigned long lastTimeTask1 = 0;
unsigned long timerDelayTask1 = 1000;

unsigned long lastTimeTask2 = 0;
unsigned long timerDelayTask2 = 5000;

void taskMQTTRelay(void *pvParameters);
void taskReadSensors(void *pvParameters);

void sendModbusCommand(const uint8_t command[], size_t length)
{
  for (size_t i = 0; i < length; i++)
  {
    Serial2.write(command[i]);
  }
}

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectBroker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Dang-Send-Data", mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe(mqttRelay);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendSensorData() {
  if (!client.connected()) {
    connectBroker();
  }
  
  DHT.read();
  
  String payload = "{\"temperature\": " + String(DHT.getTemperature()) + ", "
                   + "\"humidity\": " + String(DHT.getHumidity()) + ", "
                   + "\"light\": " + String(analogRead(lightSensor)) + ", "
                   + "\"moisture\": " + String(map(analogRead(moistureSensor), 0, 4095, 0, 100)) + "}";
  
  Serial.print("Publishing message: ");
  Serial.println(payload);
  
  client.publish(mqttTopic, payload.c_str());
}

void mqttCallBack(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  int indexStart = message.indexOf("Y");
  int index = message[indexStart + 1] - '0';
  index--;
  Serial.println(index);
  String command = message.substring(indexStart + 3, message.length());
  Serial.println(command);
  if(command == "ON"){
    sendModbusCommand(relay_ON[index], sizeof(relay_ON[0]));
  }
  else if(command == "OFF"){
    sendModbusCommand(relay_OFF[index], sizeof(relay_OFF[0]));
  }
}

void setup() {
  Serial.begin(115200);

  Serial2.begin(BAUD_RATE, SERIAL_8N1, TXD, RXD);

  Wire.begin(SDA, SCL);
  DHT.begin();
  
  setupWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallBack);
  
  connectBroker();

  xTaskCreate(taskMQTTRelay, "Task MQTT Relay", 2048, NULL, 2, NULL);
  xTaskCreate(taskReadSensors, "Task Read Sensors", 2048, NULL, 2, NULL);
}

void loop() {
}

void taskMQTTRelay(void *pvParameters) {
  while(1) {
    if (!client.connected()) {
      connectBroker();
    }
    client.loop();

    while(millis() - lastTimeTask1 < timerDelayTask1) {
      continue;
    }
    lastTimeTask1 = millis();
  }
}

void taskReadSensors(void *pvParameters) {
  pinMode(lightSensor, OUTPUT);
  pinMode(moistureSensor, OUTPUT);

  while(1){
    sendSensorData();
    while(millis() - lastTimeTask2 < timerDelayTask2) {
      continue;
    }
    lastTimeTask2 = millis();
  }
}
