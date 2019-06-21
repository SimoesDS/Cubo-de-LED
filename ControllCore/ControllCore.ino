#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <string.h>

#define _GPIO1  15
#define _GPIO2  12
#define _GPIO3  13
#define _GPIO4  2

#define LENGTHARRAY(x)  (sizeof(x) / sizeof((x)[0]))

static const int GPIO[] = {_GPIO1, _GPIO2, _GPIO3};

static const char* PROPERTY_GPIO1   = "GPIO1";
static const char* PROPERTY_GPIO2   = "GPIO2";
static const char* PROPERTY_GPIO3   = "GPIO3";
static const char* PROPERTY_GPIO4   = "GPIO4";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void initLEDs();
void initSerial();
void initWiFi(char * ssid, char * password, int mode);
void initWSConfigWiFi();
void initWebServer();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void pageControllGPIO();
void getHtml(char *pathPage);
void process(const uint8_t *payload, size_t length);

void initLEDs() {
  for (int i = 0; i < LENGTHARRAY(GPIO); i++) {
    pinMode(GPIO[i], OUTPUT);
    digitalWrite(GPIO[i], LOW);  
  }
}

void initSerial() {
  Serial.begin(115200);
  delay(10);
  Serial.println("\n\n\n");
  Serial.println("Start");
}

void getHtml(char *pathPage) {
  Serial.printf("getHtml %s", pathPage);
  File fileHootPage = SPIFFS.open(pathPage, "r");
  String webpage = "";

  if (!fileHootPage) {
    server.send(404, "text/plain", "Page not found!!");
    return;
  }
  
  while (fileHootPage.available() > 0) {
    char buffer = fileHootPage.read();
    webpage += buffer ;
  }
  server.send(200, "text/html", webpage);
}

void pageControllGPIO() {
  getHtml("/controllGPIO/index.html");
}

void initWSCuboLED(){
  server.on("/", pageControllGPIO);
  server.serveStatic("/controllGPIO/app.js", SPIFFS, "/controllGPIO/app.js");
  server.serveStatic("/controllGPIO/style.css", SPIFFS, "/controllGPIO/style.css");
}

void initWiFi(char * ssid, char * password, int mode) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("HotSpt IP: ");
  Serial.println(WiFi.localIP());
}

void process(const uint8_t *payload, size_t length) {

  StaticJsonDocument<1> doc;
  DeserializationError error = deserializeJson (doc, payload);
  if(error) {
    Serial.print("deserializeJson() with code");
    Serial.println(error.c_str());
  }

  Serial.printf("Se pá foi!!");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  Serial.println("Passou");
  switch(type){
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
    break;
    case WStype_TEXT: {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      process(payload, length);
      break;
    }
  }
}

void setup() {
  initLEDs();
  initSerial();
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  initWiFi("Minha Rede_2.4GHz", "Qa$3!adUYad", WIFI_STA);
  initWSCuboLED();
  initWebServer();
}

void loop() {
  server.handleClient();
  webSocket.loop();
}

void initWebServer() {
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("HTTP server started");
}