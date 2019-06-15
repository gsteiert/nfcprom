#include "x_nucleo_nfc04.h"
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

int logs = 0;

WebServer server(80);

const int nfcprom = 0x53;
const int led = 13;

void logClr() {
  Wire.beginTransmission(nfcprom);
  Wire.write(byte(0x01));
  Wire.write(byte(0x00));
  for (int i=0; i < 128; i++){
    Wire.write(byte(0x00));
  }
  Wire.endTransmission();
  delay(200); // 5.5ms * 128/4 
}

void logMsg(char *msg) {
  Wire.beginTransmission(nfcprom);
  Wire.write(byte(0x01));
  Wire.write(byte(logs));
  Wire.write(msg);
  Wire.endTransmission();
  logs += 16;  
  delay(25);  // 5.5ms * 16/4
}

void writeMsg(uint8_t *msg, uint16_t len) {
  Wire.beginTransmission(nfcprom);
  Wire.write(byte(0x00)); // high address byte
  Wire.write(byte(0x00)); // low address byte
  Wire.write(byte(0xE1));
  Wire.write(byte(0x40));
  Wire.write(byte(0x40));
  Wire.write(byte(0x00));
  Wire.write(byte(0x03));
  Wire.write(byte(len));
  Wire.write(msg, len);
  Wire.write(byte(0xFE)); // Terminator TLV
  Wire.write(byte(0xFF));  
  Wire.endTransmission();
  delay(2*len);  // 5.5ms/4 * len
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", "<html><body><h1>Qwiic NFCPROM</h1>Led ON, <p><a href=\"/led/off\">turn off</a></body></html>");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  logMsg("! File Not Found !");
}

void setup(void) {
  int ret = 0;
  Wire.begin();
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);

  Serial.println("");
  logClr();

  String ssid = "";
  String password = "";

  Wire.beginTransmission(nfcprom);
  Wire.write(byte(0x00));
  Wire.write(byte(0xC0));
  Wire.endTransmission(false);
  Wire.requestFrom(nfcprom, 32);
  Serial.print("SSID:  ");
  while(Wire.available()) {
    ssid += (char)Wire.read();
  }
  Serial.println(ssid);

  Wire.beginTransmission(nfcprom);
  Wire.write(byte(0x00));
  Wire.write(byte(0xE0));
  Wire.endTransmission(false);
  Wire.requestFrom(nfcprom, 32);
  Serial.print("PWD:  ");
  while(Wire.available()) {
    ssid += (char)Wire.read();
  }
  Serial.println(password);

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection
  int trys = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    trys +=1;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  IPAddress myIP = WiFi.localIP();
  String IPString = String(myIP[0])+
            "."+String(myIP[1])+
            "."+String(myIP[2])+
            "."+String(myIP[3]);
  Serial.println(IPString);
  char msgBuf[16];
  IPString.toCharArray(msgBuf, 16);
  logMsg(msgBuf);
  String("Trys: " + String(trys)).toCharArray(msgBuf, 16);
  logMsg(msgBuf);

  if(ret != NDEF_OK)
    Serial.println("Error initializing NFCTAG");
  sURI_Info myURI;
  strcpy( myURI.protocol,URI_ID_0x03_STRING );
  strcpy( myURI.URI_Message,IPString.c_str() );
  strcpy( myURI.Information,"" );
  uint8_t ndefBuf[192];
  uint16_t ndefSize=0;
  NDEF_PrepareURIMessage(&myURI, ndefBuf, &ndefSize);
  writeMsg(ndefBuf, ndefSize);

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/led/on", []() {
    digitalWrite(led, 1);
    server.send(200, "text/html", "<html><body><h1>Qwiic NFCPROM</h1>Led ON, <p><a href=\"/led/off\">turn off</a></body></html>");
  });

  server.on("/led/off", []() {
    digitalWrite(led, 0);
    server.send(200, "text/html", "<html><body><h1>Qwiic NFCPROM</h1>Led OFF, <p><a href=\"/led/on\">turn on</a></body></html>");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
