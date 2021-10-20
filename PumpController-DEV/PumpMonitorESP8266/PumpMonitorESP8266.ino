#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ThingSpeak.h>
#include <WiFiNINA_Generic.h>
#include <SPI.h>
#include "secrets.h"
#define ONE_WIRE_BUS 2
const float  pumpOffSet = 0.493 ;
const float  filtOffSet = 0.493 ;
int pumpV = 0;
int filtV = 1;
int pumppressure;
int filtpressure;
int pumptempF;
int shedtempF;
int status = WL_IDLE_STATUS;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
DeviceAddress shedTherm = { 0x28, 0xDB, 0xF0, 0x9E, 0x2F, 0x14, 0x01, 0x32 };
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char channelid = SECRET_CH_ID;
char apikey = SECRET_WRITE_KEY;
WiFiClient client;

void setup(void)
{
  Serial.begin(115200);
  //set up temp sensors
  sensors.begin();
  sensors.setResolution(pumpTherm, 10);
  sensors.setResolution(shedTherm, 10);

  //initialize LCD
  lcd.init();
  lcd.backlight();

  //set up analog pressure sensors
  analogReference(DEFAULT);
  pinMode(pumpV, INPUT);
  pinMode(filtV, INPUT);

  //initialize ThingSpeak
  ThingSpeak.begin(client);


  //initialize Wifi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  //check Wifi firmware
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

void loop(void)
{
  //get sensor readings
  sensors.requestTemperatures();
  pumppressure = ((analogRead(0)-102.4)*100)/(921.6/102.4);
  delay(5000);
  filtpressure = ((analogRead(1)-102.4)*100)/(921.6/102.4);
  int pumptempF = (DallasTemperature::toFahrenheit(sensors.getTempC(pumpTherm)));
  int shedtempF = (DallasTemperature::toFahrenheit(sensors.getTempC(shedTherm)));
  lcd.clear();
  
  //print pressures
  lcd.setCursor(0, 0);
  lcd.print(F("PumpPress:  "));
  lcd.print(pumppressure, 1);
  lcd.print(F(" PSI"));

  lcd.setCursor(0, 1);
  lcd.print(F("HousePress: "));
  lcd.print(filtpressure, 2);
  lcd.print(F(" PSI"));

  //print Temperatures
  lcd.setCursor(0, 2);
  lcd.print(F("PumpTemp:   "));
  lcd.print(pumptempF);
  lcd.print(F(" F"));

  lcd.setCursor(0, 3);
  lcd.print(F("ShedTemp:   "));
  lcd.print(shedtempF);
  lcd.print(F(" F"));

  //send to Thingspeak
  ThingSpeak.setField(1, (String(pumppressure, DEC)));   // set the fields with the values
  ThingSpeak.setField(2, (String(filtpressure, DEC)));
  ThingSpeak.setField(4, (String(shedtempF, DEC)));
  ThingSpeak.setField(5, (String(pumptempF, DEC)));
  ThingSpeak.writeFields(channelid, apikey);

//print to Serial for debug (comment out to save code)
  Serial.print(F("PumpPress:  "));
  Serial.print(pumppressure, 1);
  Serial.print(" PSI");
  Serial.print(F("HousePress:  "));
  Serial.print(filtpressure, 1);
  Serial.print(" PSI");
  Serial.print(F("PumpTemp:  "));
  Serial.print(pumptempF);
  Serial.print(" F");
  Serial.print(F("ShedTemp:  "));
  Serial.print(shedtempF);
  Serial.print(" F");

  delay(10000);
}

void printWifiStatus() 
{
  long rssi = WiFi.RSSI();
  IPAddress ip = WiFi.localIP(); 

  // print the SSID of the network you're attached to:
  lcd.setCursor(0, 0);
  lcd.print("SSID: ");
  lcd.print(WiFi.SSID());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.print(ip);
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  lcd.setCursor(0, 3);
  lcd.print("RSSI: ");
  lcd.print(rssi);
  lcd.print(" dBm");  
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
