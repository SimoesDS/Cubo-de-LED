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


void initSerial() {
  Serial.begin(115200);
  delay(10);
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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // num: numero do cliente
  // WStype_t: tipo de conexão, pode ser de texto, binaria, messagem de desconectado
  // payload: dados que estão recebendo
  // length é o tamenho da pagina
  switch(type){
    case WStype_CONNECTED:
    {
      Serial.printf("Alguem se conectou!!");
      //IPAddress ip = webSocket.remoteIP(num);
      //Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      //webSocket.sendTXT(num, "Connected");
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
            //testAllLeds(500);
          break;        
        default:
          break;
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
  //webSocket.onEvent(webSocketEvent);

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
  //webSocket.loop();
}