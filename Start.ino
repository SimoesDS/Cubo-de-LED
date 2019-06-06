#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>

#define LED_RED 15
#define LED_GREEN 12
#define LED_BLUE 13
#define LED_ON_ESP 2

#define SWITCH 4

#define LDR A0

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void initLEDs(){
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
}

void initSerial() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Start...");  
}

void testAllLeds(int time) {
  char buffer[100];
  int leds[3] = {LED_RED, LED_GREEN, LED_BLUE};
  int lengthVetLEDS = sizeof(leds)/sizeof(leds[0]);

  for(int on_off = 0; on_off < lengthVetLEDS + 1; on_off++){
    for(int led = 0; led < lengthVetLEDS; led++){
      if ((1 << on_off) == (1 << led))
        digitalWrite(leds[led], 1);
      else
        digitalWrite(leds[led], 0);
    }
    delay(time);
  }
}

void pageRoot() {
  File fileHootPage = SPIFFS.open("/inicial.html", "r");
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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // num: numero do cliente
  // WStype_t: tipo de conexão, pode ser de texto, binaria, messagem de desconectado
  // payload: dados que estão recebendo
  // length é o tamenho da pagina
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
      
      switch (payload[0]) {
        case 'B':
          digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
          break;
        case 'R':
          digitalWrite(LED_RED, !digitalRead(LED_RED));
          break;
        case 'G':
          digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
          break;
        case 'T':
            testAllLeds(500);
          break;        
        default:
          break;
      }
    }
  }
}

void initWebServer() {
  server.on("/", pageRoot);
  server.serveStatic("/inicialjs.js", SPIFFS, "/inicialjs.js");

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);  
}

void initWiFi(char * ssid, char * password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  initLEDs();
  initSerial();
  if (!SPIFFS.begin()) return;
  initWiFi("XT1034", "SimoesDS");
  initWebServer();
}

void loop() {
  server.handleClient();
  webSocket.loop();
}