//Pin Assignments: LCD=D1-D2, OneWire=D5, pumpRelay=D6, MUX=D7
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include "secrets.h"
int poffset = 300; // zero pressure adjust
int foffset = 300; // zero pressure adjust
int lowlimit = 30;
int highlimit = 50;
int power = HIGH;
const int pumpRelay = D6;  // the Arduino pin, which connects to the IN pin of relay
#define ONE_WIRE_BUS D5 
int status = WL_IDLE_STATUS;
float pumpV;
float filtV;
float ppressure_psi; // final pressure
float fpressure_psi; // final pressure
float tempF;
float tempC;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress pumpTherm;
WiFiClient client;

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

   ArduinoOTA.setHostname("pumpmonitor");


  ArduinoOTA.setPassword("datiliot");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.printf("IP address: ");
  Serial.println(WiFi.localIP());
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  Wire.begin(); 
  Wire.beginTransmission(0x27);
}


void loop(void)
{
  ArduinoOTA.handle();
  getPressures();
  getTemps();
  printtoSerial();
}

void getPressures() {
  pumpV = analogRead(0) * 3.30 / 1024;     //Sensor output voltage
  ppressure_psi = (pumpV - (poffset/1000)) * 400; 
  delay(100);
  filtV = analogRead(0) * 3.30 / 1024;     //Sensor output voltage
  fpressure_psi = (filtV - (foffset/1000)) * 400; 
  delay(100);
}

void getTemps() {
  sensors.requestTemperatures();
  tempF = (DallasTemperature::toFahrenheit(sensors.getTempC(pumpTherm)));
}

void printtoSerial() {
  Serial.print(F("Pump Pressure: "));
  Serial.print(ppressure_psi, 0);
  Serial.println(" psi");
  Serial.print(F("Filter Pressure: "));
 // Serial.print(fpressure_psi, 0);
  Serial.printf(" " );
  Serial.println(F(" "));
  Serial.print(F("Pump Temp:  "));
  Serial.print(tempF, 0);
  Serial.printf("\n\r");
  Serial.println();
}

