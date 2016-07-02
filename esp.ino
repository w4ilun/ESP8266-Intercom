#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

/** Pin Mappings for NodeMCU **
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*******************************/

const char*   timeServer  = "www.timeapi.org";
const String  timeZoneUrl = "/est/in+one+hour";
const String  startTime   = "20:00";  //buzzer off time
const String  endTime     = "11:00"; //buzzer on time
const uint8_t doorRelay   = D0;
const uint8_t buzzerRelay = D1;
      uint8_t doorState   = 1;  //should be off by default, relay normally open
      uint8_t buzzerState = 1; //should be on by default, relay normally closed
const long pollInterval   = 30000;
unsigned long previousMillis = 0;

std::unique_ptr<ESP8266WebServer> server;

void setup(){
  pinMode(doorRelay, OUTPUT);
  pinMode(buzzerRelay, OUTPUT);
  digitalWrite(doorRelay, doorState);
  digitalWrite(buzzerRelay, buzzerState);
  
  Serial.begin(115200);
  
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Connected!");

  if (!MDNS.begin("esp8266")){
    Serial.println("Error setting up MDNS responder!");
    while(1){ 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  server->on("/door", HTTP_GET, getDoorState);
  server->on("/buzzer", HTTP_GET, getBuzzerState);
  server->on("/door", HTTP_POST, toggleDoor);
  server->on("/buzzer", HTTP_POST, toggleBuzzer);
  
  server->begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());

  MDNS.addService("http", "tcp", 80);
}

void loop(){
  server->handleClient();
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= pollInterval){
    previousMillis = currentMillis;
    Serial.println("Checking Time...");
    String currentTime = getTime();
    if(currentTime == startTime){
      digitalWrite(buzzerRelay, 1);
      buzzerState = 1;
    }
    if(currentTime == endTime){
      digitalWrite(buzzerRelay, 0);
      buzzerState = 0;
    }
    Serial.println("Current Time is: " + currentTime);
    Serial.println("Start Time is: " + startTime);
    Serial.println("Stop Time is: " + endTime);
  }
}

void getDoorState(){
  server->send(200, "text/plain", (doorState == 0) ? "ON" : "OFF");
}

void getBuzzerState(){
  server->send(200, "text/plain", (buzzerState == 1) ? "ON" : "OFF");
}

void toggleDoor(){
  doorState = 0;
  digitalWrite(doorRelay, doorState);
  delay(2000);
  doorState = 1;
  digitalWrite(doorRelay, doorState);
  server->send(200, "text/plain", (doorState == 1) ? "ON" : "OFF");
}

void toggleBuzzer(){
  buzzerState = 1 - buzzerState;
  digitalWrite(buzzerRelay, buzzerState);
  server->send(200, "text/plain", (buzzerState == 1) ? "ON" : "OFF");
}

String getTime(){
  WiFiClient client;
  if (!client.connect(timeServer, 80)){
    Serial.println("Connection Failed!");
    digitalWrite(doorRelay, 1);
    digitalWrite(buzzerRelay, 1);
  }

  client.print(String("GET ") + timeZoneUrl + " HTTP/1.1\r\n" +
               "Host: " + timeServer + "\r\n" + 
               "User_Agent: ESP8266 \r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);
  String line;
  while(client.available()){
    line = client.readStringUntil('\r');
  }
  return line.substring(12,17); //HH:MM
}
