#include <Ethernet.h>

//Pin Assignments: pPressurePin=A0 fPressurePin=A2 OneWire=D2 Relay=D3 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <SimpleTimer.h>;
#include "secrets.h"
#define ONE_WIRE_BUS 2
const byte pPressurePin = A0;
const byte fPressurePin = A2;
int poffset = 503; // zero pressure adjust
int foffset = 483; // zero pressure adjust
int lowlimit = 30;
int highlimit = 50;
int power = HIGH;
int status = WL_IDLE_STATUS;
float pumpV;
float filtV;
float ppressure_psi; // final pressure
float fpressure_psi; // final pressure
float tempF;
float tempC;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
WiFiClient client;
SimpleTimer timer;
WidgetLED pumpled(V8);

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  pinMode(pumpRelay, OUTPUT);
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(WIFI_SSID);  //WIFI_SSID, WIFI_PASS defined in secrets.h
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(3000);
  }
  printCurrentNet();
  printWifiData();
  delay(1000);
}

void loop(void)
{
  getPressures();
  getTemps();
  printtoSerial();
  //pumpcontrol();
  timer.run();
}

void getPressures() {
    pumpV = analogRead(pPressurePin) * 5.00 / 1024;
    ppressure_psi = ((pumpV - (poffset / 1000)) * 400) / 6.895;
  delay(100);
  filtV = analogRead(fPressurePin) * 5.00 / 1024;
  fpressure_psi = ((filtV - (foffset / 1000)) * 400) / 6.895;
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
  Serial.print(F("Pump V: "));
  Serial.print(pumpV, 0);
  Serial.println(" V");
  Serial.print(F("Filter Pressure: "));
  Serial.print(fpressure_psi, 0);
  Serial.print(" " );
  Serial.println(F(" "));
  Serial.print(F("Pump Temp:  "));
  Serial.print(tempF, 0);
  Serial.print("\n\r");
  Serial.println();
}


void pumpcontrol() {
  if ((ppressure_psi < lowlimit) && (tempF < 120) && (power == HIGH)) {
    digitalWrite(pumpRelay, HIGH);
    Serial.println("Low Pressure Cut On");
    Blynk.setProperty(V6, "color", "#33ff38");
    pumpled.on();
  }
  if ((ppressure_psi > highlimit) && (power == HIGH)) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("High Pressure Cutoff");
    pumpled.off();
  }
  if (tempF > 120) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("High Temp Cutoff");
    Blynk.setProperty(V6, "color", "#D3435C");
    pumpled.on();
  }
  if (power == LOW) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("Powered Off");
    pumpled.on();
  }
}


void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  Serial.println();
}

void printCurrentNet() {
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.println(rssi);
}
