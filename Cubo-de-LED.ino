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

unsigned int getStrLength(const uint8_t * _data){
  int length = 0;
  while(_data[length] != '\0') length++;
  return length;
}

void substrData(uint8_t *returnData, const uint8_t *_data, const uint8_t startData, const uint8_t length){
  for (int i = 0, k = startData; i < length; k++, i++) 
      returnData[i] = _data[k];
  returnData[length] = '\0';
}

int getInfData(uint8_t *returnData, uint8_t *startData, uint8_t *lengthData, const uint8_t *dataReceiver, const char *prefix){
  const int lengthPrefix = strlen(prefix);

  for(int i = 0, k = 0; i < getStrLength(dataReceiver); i++){
    if( dataReceiver[i] == prefix[k]){   
      if(k >= lengthPrefix - 1){
        *startData = i + 1;
        break;
      } 
      k++;
    } else k = 0;
  }  

  if( *(dataReceiver + *startData) == '\0') return -1;
  
  // Descobre o tamanho da string a partir do startData até a ','
  for (int k = *startData; (dataReceiver[k] != '\0') && (dataReceiver[k] != ','); k++, *(lengthData) +=1);
  return 1;
}

uint8_t * getDataReceiver(uint8_t *returnData, const uint8_t * data, const char *prefix){
  uint8_t lengthData = 0;
  uint8_t startData = 0;
  getInfData(returnData, &startData, &lengthData, data, prefix);
  returnData = (uint8_t *) malloc(lengthData + 1 * sizeof(uint8_t));
  if(!returnData) printf("Error: Can't allocate memory for name network");
  substrData(returnData, data, startData, lengthData);
  return returnData;
}

uint8_t *getNameNetwork(uint8_t *returnData, const uint8_t * data){
  return getDataReceiver(returnData, data, PREFIX_NAMENETWORK);
}

uint8_t *getPassNetwork(uint8_t *returnData, const uint8_t * data){
  return getDataReceiver(returnData, data, PREFIX_PASSNETWORK);
}

void initSerial() {
  Serial.begin(115200);
  delay(10);
  Serial.println("\nStart...");  
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
  // length é o tamanho da pagina
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
      
      uint8_t *strNameNetwork;
      strNameNetwork = getNameNetwork(strNameNetwork, payload);
      Serial.printf("Name: %s\n", strNameNetwork);

      uint8_t *strPassNetwork;
      strPassNetwork = getPassNetwork(strPassNetwork, payload);
      Serial.printf("Pass: %s\n", strPassNetwork);
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
  if (!SPIFFS.begin()) return;
  initWiFi("Controle Cubo de LED", "SimoesDS");
  initWebServer();
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
