#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "secrets.h"
#include "wificonfig.h"
#include "blynk.h"
#include "thingspeakconfig.h"
int pumpV = 0;
int filtV = 1;
const float  pumpOffSet = 0.493 ;
const float  filtOffSet = 0.493 ;
int pumpPressure;
int filterPressure;
//int pumptempC;
//int shedtempC;
float temp;
//int numberOfDevices;
float pumpPressSensor = 0;
float filtPressSensor = 1;
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
DeviceAddress shedTherm = { 0x28, 0xDB, 0xF0, 0x9E, 0x2F, 0x14, 0x01, 0x32 };
WiFiClient client;

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  sensors.setResolution(shedTherm, 10);
  lcd.init();
  lcd.init();
  lcd.backlight();
  analogReference(DEFAULT);
  pinMode(pumpPressSensor, INPUT);
  pinMode(filtPressSensor, INPUT);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
}

void loop(void)
{
  pumpPressure = (((analogRead(pumpPressSensor) * 5.00 / 1024) - pumpOffSet) * 400 / 6.865);         //Calculate Pressure
  delay(1000);
  filterPressure = (((analogRead(filtPressSensor) * 5.00 / 1024) - filtOffSet) * 400 / 6.865);      //Calculate Pressure
  delay(1000);
  sensors.requestTemperatures();               //Get Temps
  int pumptempF = (DallasTemperature::toFahrenheit(sensors.getTempC(pumpTherm)));
  int shedtempF = (DallasTemperature::toFahrenheit(sensors.getTempC(shedTherm)));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PumpumpPressureSI:"));
  lcd.setCursor(10, 0);
  lcd.print(pumpPressure, 1);
  lcd.setCursor(0, 1);
  lcd.print(F("HousePSI:"));
  lcd.setCursor(10, 1);
  lcd.print(filterPressure, 2);
  lcd.setCursor(0, 2);
  lcd.print(F("PumpTemp:"));
  lcd.setCursor(10, 2);
  lcd.print(pumptempF);
  lcd.print(F(" F"));
  lcd.setCursor(0, 3);
  lcd.print(F("ShedTemp:"));
  lcd.setCursor(10, 3);
  lcd.print(shedtempF);
  lcd.print(F(" F"));
  Blynk.virtualWrite(V0, pumpPressure);
  Blynk.virtualWrite(V1, filterPressure);
  Blynk.virtualWrite(V2, pumptempF);
  Blynk.virtualWrite(V3, shedtempF);
  if (pumpPressure < 20) {
    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Pressure is below 20PSI");
    Blynk.notify("Pump Pressure Low");
  }
  if (filterPressure < 20) {
    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Filtered Pressure is below 20PSI");
    Blynk.notify("Filtered Pressure Low");
  }
  if (pumptempF > 120) {
    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Temperature is above 120F. Check immediately");
    Blynk.notify("Pump Temp High");
  }
  if (shedtempF < 32) {
    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Shed Temperature is below 32F. Consider heating.");
    Blynk.notify("Shed Temp Low");
  }
  ThingSpeak.setField(1, (String(pumpPressure, DEC)));   // set the fields with the values
  ThingSpeak.setField(2, (String(filterPressure, DEC)));
  ThingSpeak.setField(3, (String(pumptempF, DEC)));
  ThingSpeak.setField(4, (String(shedtempF, DEC)));
  ThingSpeak.setField(5, (String(pumptempF, DEC)));
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  delay(3000);
}
