#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define LED_RED 15
#define LED_GREEN 12
#define LED_BLUE 13
#define LED_ON_ESP 2

#define SWITCH 4

#define LDR A0

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char* PREFIX_NAMENETWORK = "NN:";
static const char* PREFIX_PASSNETWORK = "PN:";
static const char* PREFIX_CONFIGNETWORK = "CN:";
static const char  SUFIX_COMMA = ',';

unsigned int getStrLength(const uint8_t * _data){
  int length = 0;
  while(_data[length] != '\0') length++;
  return length;
}

unsigned int srcInitPrefix(const uint8_t * _data, const char *prefix, size_t lengthData){
  if(!lengthData) lengthData = getStrLength(_data);
  const int lengthPrefix = strlen(prefix);

  for(int i = 0, k = 0; i < lengthData; i++){
    if( _data[i] == prefix[k]){
      if(k >= lengthPrefix - 1){
        return i + 1;
      }
      k++;
    } else k = 0;
  }
  return 0;
}

void substrData(uint8_t *returnData, const uint8_t *_data, const uint8_t startData, const uint8_t length){
  for (int i = 0, k = startData; i < length; k++, i++) 
      returnData[i] = _data[k];
  returnData[length] = '\0';
}

int getInfData(uint8_t *startData, uint8_t *lengthDataSrc, const uint8_t *dataReceiver, const size_t lengthData, const char *prefix, const char sufix){
  *startData = srcInitPrefix(dataReceiver, prefix, lengthData);

  if( *(dataReceiver + *startData) == '\0') return -1;
  
  // Descobre o tamanho da string a partir do startData até a sufix
  for (int k = *startData; (dataReceiver[k] != '\0') && (dataReceiver[k] != sufix); k++, *(lengthDataSrc) +=1);
  return 1;
}

uint8_t * getDataReceiver(uint8_t *returnData, const uint8_t *data, const size_t lengthData, const char *prefix, const char sufix){
  uint8_t startData = 0;
  uint8_t lengthDataSrc = 0;
  getInfData(&startData, &lengthDataSrc, data, lengthData, prefix, sufix);
  returnData = (uint8_t *) malloc(lengthDataSrc + 1 * sizeof(uint8_t));
  if(!returnData) printf("Error: Can't allocate memory for name network");
  substrData(returnData, data, startData, lengthDataSrc);
  return returnData;
}

uint8_t *getNameNetwork(uint8_t *returnData, const uint8_t *data, const size_t lengthData){
  return getDataReceiver(returnData, data, lengthData, PREFIX_NAMENETWORK, SUFIX_COMMA);
}

uint8_t *getPassNetwork(uint8_t *returnData, const uint8_t *data, const size_t lengthData){
  return getDataReceiver(returnData, data, lengthData, PREFIX_PASSNETWORK, SUFIX_COMMA);
}

uint8_t isConfigNetwork(uint8_t *data){
  return srcInitPrefix(data, PREFIX_CONFIGNETWORK, strlen(PREFIX_CONFIGNETWORK));
}

void listDir(char * path){
  Dir dir = SPIFFS.openDir(path);
  while (dir.next())
    Serial.println(dir.fileName());
}

void initSerial() {
  Serial.begin(115200);
  delay(10);
  Serial.println("\n\n\n");
  Serial.println("Start...");
}

void pageRoot() {
  File fileHootPage = SPIFFS.open("/index.html", "r");
  String webpage = "";

  if (!fileHootPage) {
    server.send(404, "text/plain", "Página não encontrada!");
    return;
  }
  
  while (fileHootPage.available() > 0) {
    char buffer = fileHootPage.read();
    webpage += buffer ;
  }
  server.send(200, "text/html", webpage);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  // num: numero do cliente
  // WStype_t: tipo de conexão, pode ser de texto, binaria, messagem de desconectado
  // payload: dados que estão recebendo
  // length é o tamanho do payload
  switch(type){
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, "Connected");
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
    break;
    case WStype_TEXT:
    {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      
      if(isConfigNetwork(payload)){
        uint8_t *strNameNetwork;
        strNameNetwork = getNameNetwork(strNameNetwork, payload, length);
        Serial.printf("Name: %s\n", strNameNetwork);

        uint8_t *strPassNetwork;
        strPassNetwork = getPassNetwork(strPassNetwork, payload, length);
        Serial.printf("Pass: %s\n", strPassNetwork);
      }
    }
  }
}

void initWebServer() {
  server.on("/", pageRoot);
  server.serveStatic("/app.js", SPIFFS, "/app.js");
  server.serveStatic("/style.css", SPIFFS, "/style.css");

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("HTTP server started");
}

void initWiFi(char * ssid, char * password) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP(); //Get IP address
  Serial.print("HotSpt IP: ");
  Serial.println(myIP);
}

void setup() {
  initSerial();
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  initWiFi("Controle Cubo de LED", "SimoesDS");
  initWebServer();
  //listDir("/");
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
