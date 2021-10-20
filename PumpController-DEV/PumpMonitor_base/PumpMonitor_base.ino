#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

int pumpV = 0;
int filtV = 1;
const float  pumpOffSet = 0.493 ;
const float  filtOffSet = 0.493 ;
float V;
int P;
float V1;
int P1;
int numberOfDevices;
float pumpVoltage = 0;
float filtVoltage = 0;
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
DeviceAddress pumpTherm = { 0x28, 0x9C, 0x83, 0x47, 0x2F, 0x14, 0x01, 0x73 };
DeviceAddress shedTherm = { 0x28, 0xDB, 0xF0, 0x9E, 0x2F, 0x14, 0x01, 0x32 };
const char mySSID[] = "iot";
const char myPSK[] = "h0bg0blin$";

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
  pinMode(pumpV, INPUT);
  pinMode(filtV, INPUT);
  {
    Serial.println("Monitoring Pump Systems");
    lcd.setCursor(4, 0); 
    lcd.print("Monitoring");
    lcd.setCursor(8, 1); 
    lcd.print("Pump");
    lcd.setCursor(5, 2); 
    lcd.print("Systems");
    delay(1000);

  }
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC > 52.00) {
    Serial.print("HIGH TEMP  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.print(" F");
    lcd.print("HIGH ");
    lcd.print((DallasTemperature::toFahrenheit(tempC)),0);
    lcd.print(" F");
  } else {
    if (tempC < 2.00) {
      Serial.print("LOW TEMP  ");
      Serial.print(DallasTemperature::toFahrenheit(tempC));
      Serial.print(" F");
      lcd.print("LOW ");
      lcd.print((DallasTemperature::toFahrenheit(tempC)),0);
      lcd.print(" F");
    } else {
      Serial.print(DallasTemperature::toFahrenheit(tempC));
      Serial.print(" F");
      lcd.print((DallasTemperature::toFahrenheit(tempC)),0); 
      lcd.print(" F");
    }
  }
}


void loop(void)
{
  sensors.requestTemperatures();               //Get Temps
  pumpVoltage = analogRead(pumpV);
  V = analogRead(0) * 5.00 / 1024;             //Get Pressure Voltage
  P = ((V - pumpOffSet) * 400 / 6.865);         //Calculate Pressure
  delay(1500);
  filtVoltage = analogRead(filtV);
  V1 = analogRead(0) * 5.00 / 1024;           //Get Voltage
  P1 = ((V1 - filtOffSet) * 400 / 6.865);      //Calculate Pressure
  delay(1500);
  lcd.clear();
  Serial.print("Pump: ");   //Print Pressures
  Serial.print(V, 3);
  Serial.print("V ");
  Serial.print(P, 2);
  Serial.println(" PSI");
  lcd.setCursor(0, 0);                         
  lcd.print("PumpPSI:");
  lcd.setCursor(10, 0);
  lcd.print(P, 1);
  Serial.print("Filt: ");
  Serial.print(V1, 3);
  Serial.print("V ");
  Serial.print(P1, 2);
  Serial.println(" PSI");
  lcd.setCursor(0, 1);
  lcd.print("FiltPSI:");
  lcd.setCursor(10, 1);
  lcd.print(P1, 1);
  Serial.println();
  lcd.setCursor(0, 2);                      //Print Temparatures  
  lcd.print("PumpTemp:");
  lcd.setCursor(10, 2);
  Serial.print("Pump Temp:  ");    
  printTemperature(pumpTherm);
  Serial.print("\n\r");
  lcd.setCursor(0, 3);
  lcd.print("ShedTemp:");
  lcd.setCursor(10, 3);
  Serial.print("Shed Temp:  ");
  printTemperature(shedTherm);
  Serial.print("\n\r");
  Serial.println();
  Serial.println();
  delay(5000);
}



