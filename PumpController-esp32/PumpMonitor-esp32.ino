//Pin Assignments: LCD=D1-D2, OneWire=35, pumpRelay=34  ppressure=36 fpressure=39
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_PCF8574.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SimpleTimer.h>;
#include <BlynkSimpleEsp32.h>
#include "secrets.h"
//#include "SoftwareSerial.h"
#define BLYNK_PRINT Serial
//SoftwareSerial nodemcu(2, 3);
int poffset = 300; // zero pressure adjust
int foffset = 300; // zero pressure adjust
int lowlimit = 30;
int highlimit = 50;
int power = HIGH;
const int pumpRelay = 34;  // the Arduino pin, which connects to the IN pin of relay
#define ONE_WIRE_BUS 35
int status = WL_IDLE_STATUS;
float pumpV;
float filtV;
float ppressure_psi; // final pressure
float fpressure_psi; // final pressure
float tempF;
float tempC;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm;
WidgetLED blynkled(V3);
WiFiClient client;
SimpleTimer timer;

void setup(void)
{
  Serial.begin(115200);
  Serial.println("\nConnecting");

  wifiMulti.addAP(ssid, password);

  for (int loops = 10; loops > 0; loops--) {
    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("WiFi connected ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      break;
    }
    else {
      Serial.println(loops);
      delay(1000);
    }
  }
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi connect failed");
    delay(1000);
    ESP.restart();
  }

 
  Serial.println("Ready");
  Serial.printf("IP address: ");
  Serial.println(WiFi.localIP());
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  timer.setInterval(1000, updateBlynk);
  Wire.begin();
  Wire.beginTransmission(0x27);
  lcd.begin(20, 4);
  lcd.setBacklight(255);
  pinMode(pumpRelay, OUTPUT);
  pinMode(S0, OUTPUT);                      /* Define digital signal pin as output to the Multiplexer pin SO */
  pinMode(SIG, INPUT);                      /* Define analog signal pin as input or receiver from the Multiplexer pin SIG */
  Blynk.begin(auth, STASSID, STAPSK);  //auth, STASSID, STAPSK defined in secrets.h
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

BLYNK_WRITE(V5) {
  poffset = param.asInt(); // assigning incoming value from pin V5 to a variable
  Serial.println(poffset);
}

BLYNK_WRITE(V6) {
  foffset = param.asInt(); // assigning incoming value from pin V6 to a variable
  Serial.println(foffset);
}

void loop(void)
{
  ArduinoOTA.handle();
  getPressures();
  getTemps();
  printtoSerial();
  printtoLCD();
  pumpcontrol();
  timer.run();
}

void getPressures() {
  pumpV = analogRead(36) * 3.30 / 1024;     //Sensor output voltage
  ppressure_psi = (pumpV - (poffset / 1000)) * 400;
  delay(100);
  filtV = analogRead(39) * 3.30 / 1024;     //Sensor output voltage
  fpressure_psi = (filtV - (foffset / 1000)) * 400;
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

void printtoLCD() {
  clearLCDLine(1);
  lcd.setCursor(6, 1);
  lcd.print(ppressure_psi, 0);
  lcd.setCursor(11, 1);
  //lcd.print(fpressure_psi, 0);
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
  Serial.printf("Updating Blynk");
  Blynk.virtualWrite(V0, ppressure_psi);
  // Blynk.virtualWrite(V1, fpressure_psi);
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
  Serial.printf("\n\r");
  Serial.println();
}

void pumpcontrol() {
  if (ppressure_psi < 40 && (tempF < 120)) {
    digitalWrite(pumpRelay, HIGH);
    Serial.print("cut-in");
    blynkled.on();
  }
  if (ppressure_psi > 50) {
    digitalWrite(pumpRelay, LOW);
    Serial.print("cut-out");
    blynkled.off();
  }
  if (tempF > 120) {
    digitalWrite(pumpRelay, LOW);
    Serial.print("High Temp Cutoff");
    blynkled.off();
  }
  //  if (digitalRead(PUMPBUTTON) == LOW) {
  //    digitalWrite(pumpRelay, HIGH);
  //    Serial.print("Pump Manual");
}
