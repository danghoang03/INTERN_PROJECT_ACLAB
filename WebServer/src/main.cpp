#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <AdafruitIO.h>
#include <AdafruitIO_WiFi.h>
#include "SPIFFS.h"
#include "DHT20.h"
#include <Wire.h>
#include <PubSubClient.h>
#include "time.h"
#include <HTTPClient.h>


const char* ssid = "ACLAB";
const char* password = "ACLAB2023";
const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
const char* mqttUser = "dang03";
const char* mqttPassword = "Active_Key";
const char* mqttTopic = "dang03/feeds/dashboard"; 
const char* mqttTopicRelay = "dang03/feeds/relay";
const char* mqttTopicRelayHistory = "dang03/feeds/relay_history";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Json Variable to Hold Sensor Readings
JSONVar receivedData;


// Boolean array to hold relay state
bool relayState[6] = {false, false, false, false, false, false};

unsigned long relayStartTime[6] = {0, 0, 0, 0, 0, 0};
unsigned long relayElapsedTime[6] = {0, 0, 0, 0, 0, 0};

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}


void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void connectBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("Dang-server", mqttUser, mqttPassword)) {
      Serial.println("Connected");
      mqttClient.subscribe(mqttTopic);
      mqttClient.subscribe(mqttTopicRelayHistory);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  if (String(topic) == String(mqttTopic)) {
    receivedData = JSON.parse(message);
    String jsonString = JSON.stringify(receivedData);
    notifyClients(jsonString);
  } else if (String(topic) == String(mqttTopicRelayHistory)) {
    // Forward the relay history data to WebSocket clients
    ws.textAll("HISTORY|" + message); // Prefixing with 'HISTORY|' to identify the message type
  }

  receivedData = JSON.parse(message);
  String jsonString = JSON.stringify(receivedData);
  notifyClients(jsonString);
}

void initMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  connectBroker();
}

void sendRelayControl(int relayId, bool relayState) {
  String payload = "";
  payload += "RELAY"; 
  payload += String(relayId);
  payload += (relayState)? "_ON" : "_OFF";
  Serial.print("Publishing message: ");
  Serial.println(payload);

  mqttClient.publish(mqttTopicRelay, payload.c_str());
}

void sendRelayHistory(int relayId, bool relayState) {
  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  String dateTime = String(timeinfo.tm_year + 1900) + "-" + 
                    String(timeinfo.tm_mon + 1) + "-" + 
                    String(timeinfo.tm_mday) + " " + 
                    String(timeinfo.tm_hour) + ":" + 
                    String(timeinfo.tm_min) + ":" + 
                    String(timeinfo.tm_sec);

  String state = relayState ? "ON" : "OFF"; 
  String payload = "{";
  payload += "\"relayId\": " + String(relayId) + ",";
  payload += "\"state\": \"" + state + "\",";
  payload += "\"timestamp\": \"" + dateTime + "\"";
  payload += "}";

  Serial.print("Publishing relay history: ");
  Serial.println(payload);
  mqttClient.publish(mqttTopicRelayHistory, payload.c_str());
}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String message = String((char*)data);
    Serial.println("Received WebSocket message: " + message);
    
    if (message == "RELAY1") {
      relayState[0] =!relayState[0];
      sendRelayControl(1, relayState[0]);
      sendRelayHistory(1, relayState[0]);
      String message = "";
      if (relayState[0]) {
        message = "RELAY1_ON";
        relayStartTime[0] = millis();
      }
      else {
        message = "RELAY1_OFF";
        relayElapsedTime[0] = 0;
      }
      ws.textAll(message);
    }

    else if (message == "RELAY2") {
      relayState[1] = !relayState[1];
      sendRelayControl(2, relayState[1]);
      sendRelayHistory(2, relayState[1]);
      String message = "";
      if (relayState[1]) {
        message = "RELAY2_ON";
        relayStartTime[1] = millis();
      }
      else {
        message = "RELAY2_OFF";
        relayElapsedTime[1] = 0;
      }
      ws.textAll(message);
    }

    else if (message == "RELAY3") {
      relayState[2] =!relayState[2];
      sendRelayControl(3, relayState[2]);
      sendRelayHistory(3, relayState[2]);
      String message = "";
      if (relayState[2]) {
        message = "RELAY3_ON";
        relayStartTime[2] = millis();
      }
      else {
        message = "RELAY3_OFF";
        relayElapsedTime[2] = 0;
      }
      ws.textAll(message);
    }

    else if (message == "RELAY4") {
      relayState[3] =!relayState[3];
      sendRelayControl(4, relayState[3]);
      sendRelayHistory(4, relayState[3]);
      String message = "";
      if (relayState[3]) {
        message = "RELAY4_ON";
        relayStartTime[3] = millis();
      }
      else {
        message = "RELAY4_OFF";
        relayElapsedTime[3] = 0;
      }
      ws.textAll(message);
    }

    else if (message == "RELAY5") {
      relayState[4] =!relayState[4];
      sendRelayControl(5, relayState[4]);
      sendRelayHistory(5, relayState[4]);
      String message = "";
      if (relayState[4]) {
        message = "RELAY5_ON";
        relayStartTime[4] = millis();
      }
      else {
        message = "RELAY5_OFF";
        relayElapsedTime[4] = 0;
      }
      ws.textAll(message);
    }

    else if (message == "RELAY6") {
      relayState[5] =!relayState[5];
      sendRelayControl(6, relayState[5]);
      sendRelayHistory(6, relayState[5]);
      String message = "";
      if (relayState[5]) {
        message = "RELAY6_ON";
        relayStartTime[5] = millis();
      }
      else {
        message = "RELAY6_OFF";
        relayElapsedTime[5] = 0;
      }
      ws.textAll(message);
    }
  

    // Example: You can process messages for controlling relays or other actions
    else if (message == "getReadings") {
      String sensorReadings = JSON.stringify(receivedData); // Using readings updated by MQTT
      ws.textAll(sensorReadings);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String outputState(int relayId) {
  return relayState[relayId - 1]? "checked" : "";
}

String processor(const String& var){
  if (var == "STATE1") {
    return relayState[0] ? "Đang hoạt động" : "Đang tắt";
  }
  else if (var == "STATE2") {
    return relayState[1] ? "Đang hoạt động" : "Đang tắt";
  }
  else if (var == "STATE3") {
    return relayState[2] ? "Đang hoạt động" : "Đang tắt";
  }
  else if (var == "STATE4") {
    return relayState[3] ? "Đang hoạt động" : "Đang tắt";
  }
  else if (var == "STATE5") {
    return relayState[4] ? "Đang hoạt động" : "Đang tắt";
  }
  else if (var == "STATE6") {
    return relayState[5] ? "Đang hoạt động" : "Đang tắt";
  }
  
  if (var == "STYLE1") {
    return relayState[0] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }
  
  else if (var == "STYLE2") {
    return relayState[1] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE3") {
    return relayState[2] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE4") {
    return relayState[3] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE5") {
    return relayState[4] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE6") {
    return relayState[5] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  if (var == "STATE_MONITOR1") {
    return relayState[0] ? "Đang hoạt động" : "Đang tắt";
  }

  else if (var == "STATE_MONITOR2") {
    return relayState[1] ? "Đang hoạt động" : "Đang tắt";
  }

  else if (var == "STATE_MONITOR3") {
    return relayState[2] ? "Đang hoạt động" : "Đang tắt";
  }

  else if (var == "STATE_MONITOR4") {
    return relayState[3] ? "Đang hoạt động" : "Đang tắt";
  }

  else if (var == "STATE_MONITOR5") {
    return relayState[4] ? "Đang hoạt động" : "Đang tắt";
  }

  else if (var == "STATE_MONITOR6") {
    return relayState[5] ? "Đang hoạt động" : "Đang tắt";
  }

  if (var == "STYLE1_MONI") {
    return relayState[0] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE2_MONI") {
    return relayState[1] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE3_MONI") {
    return relayState[2] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE4_MONI") {
    return relayState[3] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE5_MONI") {
    return relayState[4] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  else if (var == "STYLE6_MONI") {
    return relayState[5] ? "style=\"color: blue;\"" : "style=\"color: red;\"";
  }

  if (var == "TOGGLE1") {
    return relayState[0] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"1\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"1\"> <span class=\"slider\"></span></label>";
  }
  else if (var == "TOGGLE2") {
    return relayState[1] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"2\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"2\"> <span class=\"slider\"></span></label>";
  }
  else if (var == "TOGGLE3") {
    return relayState[2] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"3\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"3\"> <span class=\"slider\"></span></label>";
  }
  else if (var == "TOGGLE4") {
    return relayState[3] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"4\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"4\"> <span class=\"slider\"></span></label>";
  }
  else if (var == "TOGGLE5") {
    return relayState[4] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"5\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"5\"> <span class=\"slider\"></span></label>";
  }
  else if (var == "TOGGLE6") {
    return relayState[5] ? "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"6\" checked> <span class=\"slider\"></span></label>"
                         : "<label class=\"switch\"> <input type=\"checkbox\" onclick=\"toggleRelay(this)\" id=\"6\"> <span class=\"slider\"></span></label>";
  }

  if (var == "UPTIME1") {
    unsigned long elapsed = relayElapsedTime[0];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }
  else if (var == "UPTIME2") {
    unsigned long elapsed = relayElapsedTime[1];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }
  else if (var == "UPTIME3") {
    unsigned long elapsed = relayElapsedTime[2];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }
  else if (var == "UPTIME4") {
    unsigned long elapsed = relayElapsedTime[3];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }
  else if (var == "UPTIME5") {
    unsigned long elapsed = relayElapsedTime[4];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }
  else if (var == "UPTIME6") {
    unsigned long elapsed = relayElapsedTime[5];
    int hours = (elapsed / 3600000);
    int minutes = (elapsed % 3600000) / 60000;
    return String(hours) + " giờ " + String(minutes) + " phút";
  }

  return String();
}

void sendRelayTimeToClients() {
    String message = "{ \"relayTimes\": [";
    for (int i = 0; i < 6; i++) {
        if (i > 0) message += ", ";
        message += String(relayElapsedTime[i]);
    }
    message += "] }";
    ws.textAll(message);
}

void updateRelayElapsedTime() {
  for (int i = 0; i < 6; i++) {
    if (relayState[i]) {
      relayElapsedTime[i] = millis() - relayStartTime[i];
    }
  }
  sendRelayTimeToClients();
}

HTTPClient http;

void handleGetHistory(AsyncWebServerRequest *request) {
  String url = "https://io.adafruit.com/api/v2/dang03/feeds/relay-history/data?limit=100&X-AIO-Key=Active_Key"; // Fetch last 100 entries
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    request->send(200, "application/json", payload);
  } else {
    request->send(500, "text/plain", "Error fetching data");
  }
  http.end();
}


void setup() {
  Serial.begin(115200);
  initWiFi();
  initWebSocket();
  initMQTT();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if(!SPIFFS.begin(true)){
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
}

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/dashboard.html", "text/html", false);
  });

  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css","text/css");
  });  
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js","text/javascript");
  });

  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/dashboard.html", "text/html", false);
  });

  server.on("/control-device", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/control.html", "text/html", false, processor);
  });

  server.on("/styles2.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles2.css","text/css");
  });  

  server.on("/script2.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script2.js","text/javascript");
  });

  server.on("/monitor-device", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/monitor.html", "text/html", false, processor);
  });
  
  server.on("/styles3.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles3.css","text/css");
  });

  server.on("/script3.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script3.js","text/javascript");
  });

  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/history.html", "text/html", false, processor);
  });
  
  server.on("/styles4.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles4.css","text/css");
  });

  server.on("/script4.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script4.js","text/javascript");
  });

  server.on("/getHistory", HTTP_GET, [](AsyncWebServerRequest *request){
    handleGetHistory(request);
});

  // Start server
  server.begin();
}

void loop() {
  if (!mqttClient.connected()) {
    connectBroker();
  }
  mqttClient.loop();

  if(millis() - lastTime >= timerDelay) {
    updateRelayElapsedTime();
    lastTime = millis();
  }

  ws.cleanupClients();
}