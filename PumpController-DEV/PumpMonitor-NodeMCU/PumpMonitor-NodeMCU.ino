//Pin Assignments: LCD=D1-D2, OneWire(temperature)=D5, pumpRelay=D6
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <BlynkSimpleEsp8266.h>
#include "SoftwareSerial.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SimpleTimer.h>;
#include <Wire.h>
#include <TM1650.h>
#include "secrets.h"
TM1650 d;
#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress pumpThermometer;
SoftwareSerial nodemcu(2, 3);
#define BLYNK_PRINT Serial
float  OffSet = 0.347 ;
float A;
float V;
float P;
float PSI;
float tempF;
float tempC;
const int pumpRelay = D6;
int status = WL_IDLE_STATUS;
int lowlimit = 30;
int highlimit = 50;
int power = HIGH;
char PSI_led[4];
char tempF_led[4];
SimpleTimer timer;
WiFiClient client;
WidgetLED pumpled(V6);
WidgetTerminal terminal(V6);

void setup(void)
{
  Serial.begin(115200);
  Wire.begin();
  sensors.begin();
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(pumpThermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(pumpThermometer, 9);
  pinMode(pumpRelay, OUTPUT);
  d.init();
  timer.setInterval(1000, updateBlynk);


  WiFi.mode(WIFI_STA); //Wireless Setup and connect
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("pumpmonitor"); //OTA Setup
  ArduinoOTA.setPassword("datiliot");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.begin();

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1500);
  Blynk.begin(auth, STASSID, STAPSK);  //auth, STASSID, STAPSK defined in secrets.h
  Blynk.syncAll();
}

BLYNK_WRITE(V2) {
  lowlimit = param.asInt(); // assigning incoming value from pin V2 to a variable
  Serial.println(lowlimit);
}

BLYNK_WRITE(V3) {
  highlimit = param.asInt(); // assigning incoming value from pin V3 to a variable
  Serial.println(highlimit);
}

BLYNK_WRITE(V4) {
  power = param.asInt(); // assigning incoming value from pin V4 to a variable
  Serial.println(power);
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  float tempF = (DallasTemperature::toFahrenheit(tempC));
  dtostrf(tempF, 4, 0, tempF_led);
  //d.displayString(tempF_led);
  Serial.print(" Temp F: ");
  Serial.println(tempF);
}

void printPressure() {
  A = analogRead(A0);
  V = A * 3.3 / 1024;     //Sensor output voltage
  P = (V - OffSet) * 400;             //Calculate water pressure
  PSI = ((V - OffSet) * 400) / 6.895;
  dtostrf(PSI, 4, 0, PSI_led);
  d.displayString(PSI_led);
  Serial.print("Voltage:");
  Serial.print(V, 3);
  Serial.println("V");
  Serial.print("Pressure:");
  Serial.print(PSI, 1);
  Serial.println(" psi");
  Serial.println();
}

void pumpcontrol() {
  if ((PSI < lowlimit) && (tempF < 120) && (power == HIGH)) {
    digitalWrite(pumpRelay, HIGH);
    Serial.println("Low Pressure Cut On");
    Blynk.setProperty(V6, "color", "#33ff38");
    pumpled.on();
  }
  if ((PSI > highlimit) && (power == HIGH)) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("High Pressure Cutoff");
    pumpled.off();
  }
  if (tempF > 120) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("High Temp Cutoff");
    Blynk.setProperty(V6, "color", "#D3435C");
    Blynk.notify("High Pump Temperature");
    pumpled.on();
  }
  if (power == LOW) {
    digitalWrite(pumpRelay, LOW);
    Serial.println("Powered Off");
    Blynk.setProperty(V6, "color", "#FFC300");
    pumpled.on();
  }
  if (tempF < 20) {
    Blynk.notify("Low Pump Pressure");
  }
}

void updateBlynk() {
  Serial.println("Updating Blynk");
  Blynk.virtualWrite(V0, PSI);
  Blynk.virtualWrite(V1, tempF_led);
  Blynk.virtualWrite(V7, A);
  Blynk.virtualWrite(V8, V);
}

void loop(void)
{
  ArduinoOTA.handle();
  Blynk.run();
  sensors.requestTemperatures();
  printTemperature(pumpThermometer);
  printPressure();
  pumpcontrol();
  timer.run();
  //delay(100);
}
