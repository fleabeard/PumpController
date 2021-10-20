#include <Ethernet.h>

//Pin Assignments: pPressurePin=A0 fPressurePin=A2 OneWire=D2 Relay=D3 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <SimpleTimer.h>;
#include <BlynkSimpleWiFiNINA.h>
#include "secrets.h"
#define ONE_WIRE_BUS 2
#define BLYNK_PRINT Serial
const byte pPressurePin = A0;
const byte fPressurePin = A2;
int poffset = 503; // zero pressure adjust
int foffset = 483; // zero pressure adjust
int lowlimit = 30;
int highlimit = 50;
int power = HIGH;
const int pumpRelay = 3;  // the Arduino pin, which connects to the IN pin of relay
int status = WL_IDLE_STATUS;
float pumpV;
float filtV;
float ppressure_psi; // final pressure
float fpressure_psi; // final pressure
float tempF;
float tempC;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
WiFiClient client;
SimpleTimer timer;
WidgetLED pumpled(V8);

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  timer.setInterval(1000, updateBlynk);
  lcd.init();
  lcd.backlight();
  pinMode(pumpRelay, OUTPUT);
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(WIFI_SSID);  //WIFI_SSID, WIFI_PASS defined in secrets.h
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(3000);
  }
  printCurrentNet();
  printWifiData();
  Blynk.begin(auth, WIFI_SSID, WIFI_PASS);  //auth, WIFI_SSID, WIFI_PASS defined in secrets.h
  Blynk.syncAll();
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PSI:"));
  lcd.setCursor(6, 0);
  lcd.print(F("Pump"));
  lcd.setCursor(11, 0);
  lcd.print(F("Filter"));
  lcd.setCursor(0, 2);
  lcd.print(F("PumpTemp:"));
}

BLYNK_WRITE(V3){
  lowlimit = param.asInt(); // assigning incoming value from pin V2 to a variable
  Serial.println(lowlimit);
}

BLYNK_WRITE(V4){
  highlimit = param.asInt(); // assigning incoming value from pin V3 to a variable
  Serial.println(highlimit);
}

BLYNK_WRITE(V5){
  power = param.asInt(); // assigning incoming value from pin V4 to a variable
  Serial.println(power);
}

BLYNK_WRITE(V6){
  poffset = param.asInt(); // assigning incoming value from pin V5 to a variable
  Serial.println(poffset);
}

BLYNK_WRITE(V7){
  poffset = param.asInt(); // assigning incoming value from pin V6 to a variable
  Serial.println(foffset);
}

void loop(void)
{
  getPressures();
  getTemps();
  printtoSerial();
  printtoLCD();
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

void printtoLCD() {
  clearLCDLine(1);
  lcd.setCursor(6, 1);
  lcd.print(ppressure_psi, 0);
  delay(500);
  lcd.setCursor(11, 1);
  lcd.print(fpressure_psi, 0);
  clearLCDLine(3);
  lcd.setCursor(6, 3);
  lcd.print(tempF, 0);
  lcd.print(F(" F"));
}



void clearLCDLine(int line)
{
  lcd.setCursor(0, line);
  for (int n = 0; n < 20; n++) // 20 indicates symbols in line. For 2x16 LCD write - 16
  {
    lcd.print(" ");
  }
}

void updateBlynk() {
  Serial.println("Updating Blynk");
  Blynk.virtualWrite(V0, ppressure_psi);
  Blynk.virtualWrite(V1, fpressure_psi);
  Blynk.virtualWrite(V2, tempF);
  //if (ppressure_psi < 20) {
  //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Pressure is below 20PSI");
  //    Blynk.notify("Pump Pressure Low");
  //}
  //  if (fpressure_psi < 20) {
  //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Filtered Pressure is below 20PSI");
  //    Blynk.notify("Filtered Pressure Low");
  //  }
  //if (tempF > 120) {
  //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Temperature is above 120F. Check immediately");
  //    Blynk.notify("Pump Temp High");
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
    Blynk.setProperty(V6, "color", "#FFC300");
    pumpled.on();
  }
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  lcd.setCursor(0, 1);
  lcd.print(F("IP: "));
  lcd.setCursor(4, 1);
  lcd.print(ip);
  Serial.println();
}

void printCurrentNet() {
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  lcd.setCursor(0, 0);
  lcd.print(F("SSID: "));
  lcd.setCursor(7, 0);
  lcd.print(WiFi.SSID());
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.println(rssi);
  lcd.setCursor(0, 2);
  lcd.print(F("RSSI: "));
  lcd.setCursor(7, 2);
  lcd.print(rssi);
}
