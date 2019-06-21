#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
/*#include <Arduino_JSON.h>*/
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

static const char* PREFIX_NAMENETWORK   = "NN:";
static const char* PREFIX_PASSNETWORK   = "PN:";
static const char* PREFIX_CONFIGNETWORK = "CN:";
static const char* PREFIX_CONTROLLED    = "CL:";
static const char* PREFIX_RED           = "LR:";
static const char* PREFIX_GREEN         = "LG:";
static const char* PREFIX_BLUE          = "LB:";
static const char* PREFIX_EFFECT        = "LE:";
static const char  SUFIX_COMMA = ',';

int isSetting = 0;
int activeEffectLeds = 0;

void initLEDs();
void initSerial();
void initWiFi(char * ssid, char * password, int mode);
void initWSConfigWiFi();
void initWebServer();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
uint8_t isConfigNetwork(uint8_t *data);
uint8_t isControlLED(uint8_t *data);
void changeMode(uint8_t *payload, size_t length);
unsigned int srcInitPrefix(const uint8_t * _data, const char *prefix, size_t lengthData);
unsigned int getStrLength(const uint8_t * _data);
char *getNameNetwork(char *returnData, const uint8_t *data, const size_t lengthData);
char *getDataReceiver(char *returnData, const uint8_t *data, const size_t lengthData, const char *prefix, const char sufix);
int getInfData(uint8_t *startData, uint8_t *lengthDataSrc, const uint8_t *dataReceiver, const size_t lengthData, const char *prefix, const char sufix);
void substrData(char *returnData, const uint8_t *_data, const uint8_t startData, const uint8_t length);
void listDir(char * path);
void pageSettingWiFi();
void pageControlLED();
void getHtml(char *pathPage);

unsigned int getStrLength(const uint8_t * _data) {
  int length = 0;
  while(_data[length] != '\0') length++;
  return length;
}

unsigned int srcInitPrefix(const uint8_t * _data, const char *prefix, size_t lengthData) {
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

void substrData(char *returnData, const uint8_t *_data, const uint8_t startData, const uint8_t length) {
  for (int i = 0, k = startData; i < length; k++, i++) 
      returnData[i] = _data[k];
  returnData[length] = '\0';
}

int getInfData(uint8_t *startData, uint8_t *lengthDataSrc, const uint8_t *dataReceiver, const size_t lengthData, const char *prefix, const char sufix) {
  *startData = srcInitPrefix(dataReceiver, prefix, lengthData);

  if( *(dataReceiver + *startData) == '\0') return -1;
  
  // Descobre o tamanho da string a partir do startData até a sufix
  for (int k = *startData; (dataReceiver[k] != '\0') && (dataReceiver[k] != sufix); k++, *(lengthDataSrc) +=1);
  return 1;
}

char * getDataReceiver(char *returnData, const uint8_t *data, const size_t lengthData, const char *prefix, const char sufix) {
  uint8_t startData = 0;
  uint8_t lengthDataSrc = 0;
  getInfData(&startData, &lengthDataSrc, data, lengthData, prefix, sufix);
  returnData = (char *) malloc(lengthDataSrc + 1 * sizeof(char));
  if(!returnData) printf("Error: Can't allocate memory for name network");
  substrData(returnData, data, startData, lengthDataSrc);
  return returnData;
}

char *getNameNetwork(char *returnData, const uint8_t *data, const size_t lengthData) {
  return getDataReceiver(returnData, data, lengthData, PREFIX_NAMENETWORK, SUFIX_COMMA);
}

char *getPassNetwork(char *returnData, const uint8_t *data, const size_t lengthData) {
  return getDataReceiver(returnData, data, lengthData, PREFIX_PASSNETWORK, SUFIX_COMMA);
}

uint8_t isConfigNetwork(uint8_t *data) {
  return srcInitPrefix(data, PREFIX_CONFIGNETWORK, strlen(PREFIX_CONFIGNETWORK));
}

uint8_t isControlLED(uint8_t *data) {
  return srcInitPrefix(data, PREFIX_CONTROLLED, strlen(PREFIX_CONTROLLED));
}

void listDir(char * path) {
  Dir dir = SPIFFS.openDir(path);
  while (dir.next())
    Serial.println(dir.fileName());
}

void initLEDs() {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
}

void initSerial() {
  Serial.begin(115200);
  delay(10);
  Serial.println("\n\n\n");
  Serial.println("Start...");
}

void getHtml(char *pathPage) {
  Serial.printf("getHtml %s", pathPage);
  File fileHootPage = SPIFFS.open(pathPage, "r");
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

void pageSettingWiFi() {
  getHtml("/setting/index.html");
}

void pageControlLED() {
  getHtml("/controlLed/index.html");
}

void initWSConfigWiFi(){
  server.close();
  server.on("/", pageSettingWiFi);
  server.serveStatic("/app.js", SPIFFS, "/setting/app.js");
  server.serveStatic("/style.css", SPIFFS, "/setting/style.css");
}

void initWSCuboLED(){
  webSocket.close();
  server.close();
  server.on("/led", pageControlLED);
  server.serveStatic("/controlLed/app.js", SPIFFS, "/controlLed/app.js");
  server.serveStatic("/controlLed/style.css", SPIFFS, "/controlLed/style.css");
}

void initWiFi(char * ssid, char * password, int mode) {
  IPAddress myIP;
  char *strMode;

  switch(mode){
    case WIFI_AP:{
      char tempstr[] = "WIFI_AP";
      strMode = tempstr;
      break;
    }
    case WIFI_STA:{
      char tempstr[] = "WIFI_STA";
      strMode = tempstr;
      break;
    }
    case WIFI_AP_STA:{
      char tempstr[] = "WIFI_AP_STA";
      strMode = tempstr;
      break;
    }
  }

  Serial.printf("WiFi mode set to %s: %s\n", strMode, WiFi.mode((WiFiMode_t)mode) ? "Success" : "Failed!!");
  if(mode == WIFI_AP){
    WiFi.softAP(ssid, password);
    myIP = WiFi.softAPIP();
  } else 
    if( mode == WIFI_STA ){
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println();
      myIP = WiFi.localIP();
    } else 
        Serial.println("WiFi mode not found");

  Serial.print("HotSpt IP: ");
  Serial.println(myIP);
}

void changeMode(uint8_t *payload, size_t length) {
  char *strNameNetwork;
  strNameNetwork = getNameNetwork(strNameNetwork, payload, length);
  Serial.printf("Name: %s\n", strNameNetwork);

  char *strPassNetwork;
  strPassNetwork = getPassNetwork(strPassNetwork, payload, length);
  Serial.printf("Pass: %s\n", strPassNetwork);

  isSetting = 1;
  initWiFi(strNameNetwork, strPassNetwork, WIFI_STA);
  isSetting = 0;  
  initWSCuboLED();
  initWebServer();
}

void effectLEDs() {
  //  int arrConstPinLed[] = {LED_RED, LED_GREEN, LED_BLUE};
  
  //  for (int i = 0; i < 3; i++) {
  //    digitalWrite(arrConstPinLed[i], 1);
  //    delay(300);
  //  }

  //  for (int i = 0; i < 3; i++) {
  //    digitalWrite(arrConstPinLed[i], 0);
  //    delay(300);
  //  }    
}

void onOffLED(uint8_t * data, const char *prefix) {
  if(srcInitPrefix(data, prefix, 0) > 0){
    char *isLEDOn = getDataReceiver(isLEDOn, data, 0, prefix, SUFIX_COMMA);
    Serial.printf("%s %i:\n", prefix, isLEDOn[0] - '0');
    if(srcInitPrefix(data, PREFIX_RED, 0) > 0)
      digitalWrite(LED_RED, isLEDOn[0] - '0');
    else if(srcInitPrefix(data, PREFIX_GREEN, 0) > 0)
      digitalWrite(LED_GREEN, isLEDOn[0] - '0');
    else if(srcInitPrefix(data, PREFIX_BLUE, 0) > 0)
      digitalWrite(LED_BLUE, isLEDOn[0] - '0');
    else if(srcInitPrefix(data, PREFIX_EFFECT, 0) > 0)
      activeEffectLeds = isLEDOn[0] - '0';
  }
}

void controlLED(uint8_t * data) {
    onOffLED(data, PREFIX_RED);
    onOffLED(data, PREFIX_GREEN);
    onOffLED(data, PREFIX_BLUE);
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
    case WStype_TEXT: {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      
      if(isConfigNetwork(payload))
        changeMode(payload, length);
      else if(isControlLED(payload))
        controlLED(payload);
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
  initWiFi("Controle Cubo de LED", "SimoesDS", WIFI_AP);
  //initWiFi("Minha Rede_2.4GHz", "??", WIFI_STA);
  //initWSConfigWiFi();
  initWSCuboLED();
  initWebServer();
  //listDir("/");
}

void loop() {
  if(!isSetting){
    server.handleClient();
    webSocket.loop();

    if(activeEffectLeds) {
      effectLEDs();
      // Serial.printf("Effeito\n");
    }
  }
}

void initWebServer() {
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("HTTP server started");
}