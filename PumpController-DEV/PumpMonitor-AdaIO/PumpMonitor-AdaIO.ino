#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include "secrets.h"
#include "blynk.h"
#include "thingspeakconfig.h"
int status = WL_IDLE_STATUS;
const int offset = 108; // zero pressure adjust
float temp;
char ssid[] = WIFI_SSID;        // your network SSID (name)
char pass[] = WIFI_PASS;    // your network password (use for WPA, or use as key for WEP)
#define ONE_WIRE_BUS 2
#define REDLED 13
#define GREENLED 12
#define USE_AIRLIFT
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
WiFiClient client;
//#include <AdafruitIO_WiFi.h>
#include "adaio.h"


void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  lcd.init();
  lcd.init();
  lcd.backlight();
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);

  while (status != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
     Serial.print("Connecting to Adafruit IO");
   aio.connect();  // connect to Adafruit IO service
   while(aio.status() < AIO_CONNECTED) {
      Serial.print(".");
      delay(1000);  // wait 1 second between checks
   }
   Serial.println();
   Serial.println(aio.statusText());
   
  lcd.clear();
  printCurrentNet();
  printWifiData();
  Blynk.begin(auth, ssid, pass);
  ThingSpeak.begin(client);
  delay(5000);
}

void setled(void) {

}

void loop(void)
{
  aio.run(); 
  int pPressureVal = analogRead(A0);
  int f1PressureVal = analogRead(A1);
  int f2PressureVal = analogRead(A2);
  int f3PressureVal = analogRead(A3);
 
  float pvoltage = ((pPressureVal - offset) * 5.0) / 1024.0;
  float ppressure_psi = ((float)pvoltage) * 25.0;
  delay(500);

  float f1voltage = ((f1PressureVal - offset) * 5.0) / 1024.0;
  float f1pressure_psi = ((float)f1voltage) * 25.0;
  delay(500);

    float f2voltage = ((f2PressureVal - offset) * 5.0) / 1024.0;
  float f2pressure_psi = ((float)f2voltage) * 25.0;
  delay(500);

   float f3voltage = ((f3PressureVal - offset) * 5.0) / 1024.0;
  float f3pressure_psi = ((float)f3voltage) * 25.0;
  delay(500);

  sensors.requestTemperatures();               //Get Temps
  int pumptempF = (DallasTemperature::toFahrenheit(sensors.getTempC(pumpTherm)));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PumpPSI:"));
  lcd.setCursor(10, 0);
  lcd.print(ppressure_psi, 0);
  lcd.setCursor(0, 1);
  lcd.print(F("Filtered PSI:"));
  lcd.setCursor(0, 2);
  lcd.print(F("F1: "));
  lcd.print(f1pressure_psi, 0);
  lcd.setCursor(8, 2);
  lcd.print(F("F2: "));
  lcd.print(f2pressure_psi, 0);
  lcd.setCursor(16, 2);
  lcd.print(F("F2: "));
  lcd.print(f2pressure_psi, 0);
  lcd.setCursor(0, 3);
  lcd.print(F("PumpTemp:"));
  lcd.setCursor(10, 3);
  lcd.print(pumptempF);
  lcd.print(F(" F"));
  Serial.print("Pump Pressure: ");
  Serial.print(ppressure_psi, 0);
  Serial.println(" psi");
  Serial.print("Filt1 Pressure: ");
  Serial.print(String(f1pressure_psi, 0));
  Serial.print(" " );
  Serial.print("Filt2 Pressure: ");
  Serial.print(String(f2pressure_psi, 0));
  Serial.print(" ");
  Serial.print("Filt3 Pressure: ");
  Serial.print(String(f3pressure_psi, 0));
  Serial.println(" psi");
  Serial.println(" ");
  Serial.print("Pump Temp:  ");
  Serial.print(pumptempF);
  Serial.print("\n\r");
  Serial.println();
  
  if ((ppressure_psi > 20) && (pumptempF < 120)) {
    digitalWrite(GREENLED, HIGH); ;
  }
  Blynk.virtualWrite(V0, ppressure_psi, 0);
  Blynk.virtualWrite(V1, f1pressure_psi, 0);
  Blynk.virtualWrite(V2, f2pressure_psi, 0);
  Blynk.virtualWrite(V3, f3pressure_psi, 0);
  Blynk.virtualWrite(V4, pumptempF);
  if (ppressure_psi < 20) {
    //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Pressure is below 20PSI");
    //    Blynk.notify("Pump Pressure Low");
    digitalWrite(GREENLED, LOW);
    digitalWrite(REDLED, HIGH);
  }
  //  if (fpressure_psi < 20) {
  //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Filtered Pressure is below 20PSI");
  //    Blynk.notify("Filtered Pressure Low");
  //  }
  if (pumptempF > 120) {
    //    Blynk.email("toddgrady@gmail.com", "Pump Alarm", "Pump Temperature is above 120F. Check immediately");
    //    Blynk.notify("Pump Temp High");
    digitalWrite(GREENLED, LOW);
    digitalWrite(REDLED, HIGH);
  }
  ThingSpeak.setField(1, (String(ppressure_psi, 0)));   // set the fields with the values
  ThingSpeak.setField(2, (String(f1pressure_psi, 0)));
  ThingSpeak.setField(3, (String(f2pressure_psi, 0)));   
  ThingSpeak.setField(4, (String(f3pressure_psi, 0)));
  ThingSpeak.setField(5, (String(pumptempF, 0)));
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

ppressurefeed->save(ppressure_psi);
f1pressurefeed->save(f1pressure_psi);
f2pressurefeed->save(f2pressure_psi);
f3pressurefeed->save(f3pressure_psi);
ptempfeed->save(pumptempF);

  delay(13000);
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.setCursor(4, 1);
  lcd.print(ip);
  Serial.println();
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  lcd.setCursor(0, 0);
  lcd.print("SSID: ");
  lcd.setCursor(7, 0);
  lcd.print(WiFi.SSID());
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  lcd.setCursor(0, 2);
  lcd.print("RSSI: ");
  lcd.setCursor(7, 2);
  lcd.print(rssi);
}
