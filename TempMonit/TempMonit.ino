#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

#define AnalogRead 2000 //период чтения напряжения, ms
#define DrawTime 200 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define ONE_WIRE_BUS 2 // Data wire is plugged into port 2 on the Arduino
#define DHTPIN 3     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);

DeviceAddress Thermometer1 = { 
  0x28, 0xFF, 0x31, 0x67, 0x63, 0x16, 0x03, 0xF2 };  // адрес датчика DS18B20 
DeviceAddress Thermometer2 = { 
  0x28, 0xFF, 0x5B, 0xF2, 0x63, 0x16, 0x04, 0xBD}; 

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool rt = false; //флаг чтения температуры
bool kr = 0; //флаг периода проверки клавиатуры


void DrawMenu () {
 
}

void KeyPad () {
 
}

void ReadAnalog () {
  // put your main code here, to run repeatedly:
  sensors.requestTemperatures();
  float  tempC = sensors.getTempC(Thermometer1);
  float  tempB = sensors.getTempC(Thermometer2);
  Serial.print("temp1 = ");
  Serial.println(tempC);
  Serial.print("temp2 = ");
  Serial.println(tempB);
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  Serial.print("Humidity = ");
  Serial.println(h);
  Serial.print("temp3 = ");
  Serial.println(t);
  Serial.println();  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание
  
  sensors.begin();
  sensors.setResolution(Thermometer1, 10);
  sensors.setResolution(Thermometer2, 10);
  dht.begin();
}

SIGNAL(TIMER0_COMPA_vect) { //прерывание считывающие мощность
  tyme = tyme + 1;
  if (tyme % AnalogRead == 0) { rt = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop()
{
  if ((rt == 1)) { rt = 0; ReadAnalog();}; //в режиме архива измерений не происходит 
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); }; 
}



