#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <DS3231.h>
#include <EEPROM.h>

#define AnalogRead 1000 //период проверки обновлений значений температуры, ms
#define DrawTime 2000 //период обновления экрана, ms
#define KeyReadTime 20 //период проверки нажатия клавиатуры

#define ONE_WIRE_BUS 2 // Data wire is plugged into port 2 on the Arduino
#define DHTPIN 3     // what digital pin we're connected to
#define BattaryPin A3 //пин, принимающий сигнал от батареи

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define MaxBatteryVoltage 4.2 //Напряжение полностью заряженной батареи (значение на ацп)
#define MinBatteryVoltage 2.9 //Напряжение полностью разряженной батареи (значение на ацп)
#define t1Hour 20 //час отправки значений, ms
#define t1Minute 10 //минута отправки значений, ms

//19 ячейка EEPROM - хранит порядковый номер последней записи в архив

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);
DS3231 Clock;
DeviceAddress Thermometer1 = { 
  0x28, 0xFF, 0x31, 0x67, 0x63, 0x16, 0x03, 0xF2 };  // адрес датчика DS18B20 
DeviceAddress Thermometer2 = { 
  0x28, 0xFF, 0x5B, 0xF2, 0x63, 0x16, 0x04, 0xBD}; 

volatile int tyme = 0; //переменная прерываний
bool dt = false; //флаг обновления экрана
bool rt = false; //флаг чтения температуры
bool kr = 0; //флаг периода проверки клавиатуры

//clock variable
byte year, month, date, doW, hour, minute, second;
bool Century=false;
bool h12;
bool PM, g;

bool timerCompleted = 0;
float temp1, temp2, temp3, humidity;
byte adress; //порядковый номер записей в архив
//---------------- ФУНКЦИИ ----------------
void writeToArchive (byte adress1, float value1, float value2, float value3, float value4) {
  if (adress1 > 83) return;
  
  int adr2 = 20+adress1*12;
  int intValue1 = round(value1*100);
  int intValue2 = round(value2*100);
  int intValue3 = round(value3*100);
  int intValue4 = round(value4*100);
  
  EEPROM.write(adr2, intValue1);
  EEPROM.write(adr2+1, intValue1>>8); 
  EEPROM.write(adr2+2, intValue2);
  EEPROM.write(adr2+3, intValue2>>8); 
  EEPROM.write(adr2+4, intValue3);
  EEPROM.write(adr2+5, intValue3>>8); 
  EEPROM.write(adr2+6, intValue4);
  EEPROM.write(adr2+7, intValue4>>8); 
  EEPROM.write(adr2+8, (year<<2) | (month>>2));
  EEPROM.write(adr2+9, ((month<<6) | date));  
  EEPROM.write(adr2+10, hour);
  EEPROM.write(adr2+11, minute); 
  EEPROM.write(19, adress1);
}


//функция возвратит 1 только при первом совпадении времени таймера и текущего времени
bool checkTimer() {
  if (hour == t1Hour && minute == t1Minute) {
    if (!timerCompleted) {
      timerCompleted = 1;
      return 1;  
    } else {
      return 0; 
    }
  } else {
    timerCompleted = 0;
    return 0; 
  }
}

byte getBatteryLevel() {
  byte batLevelPercent = analogRead(BattaryPin)/(MaxBatteryVoltage/5*1024)*100;
  if (batLevelPercent >= 75) return 4;
  if (batLevelPercent >= 50) return 3;
  if (batLevelPercent >= 25) return 2;
  if (batLevelPercent > 5)  return 1;
  if (batLevelPercent >= 0) return 0;
}

void getTime() {
  year=Clock.getYear();
  month=Clock.getMonth(Century);
  date=Clock.getDate();
  hour=Clock.getHour(h12,PM); 
  minute=Clock.getMinute();  
  second=Clock.getSecond();
}

void updateTempValue() {
  sensors.requestTemperatures();
  temp1 = sensors.getTempC(Thermometer1);
  temp2 = sensors.getTempC(Thermometer2);
  temp3 = dht.readHumidity();
  humidity = dht.readTemperature();
  
  Serial.print("temp1 = ");
  Serial.println(temp1);
  Serial.print("temp2 = ");
  Serial.println(temp2);
  Serial.print("Humidity = ");
  Serial.println(humidity);
  Serial.print("temp3 = ");
  Serial.println(temp3);
  Serial.println();  
}

//---------------- System ФУНКЦИИ ----------------
void DrawMenu () {
  getTime();
  updateTempValue();
  if (checkTimer()) {
    writeToArchive(adress, temp1, temp2, temp3, humidity);
    adress++;  
  }
  
}

void KeyPad () {
 
}

void ReadAnalog () {
  
}

void setup() {
  Serial.begin(9600);
  OCR0A = 0xAF; //прерывание
  TIMSK0 |= _BV(OCIE0A); //прерывание
  
  sensors.begin();
  sensors.setResolution(Thermometer1, 10);
  sensors.setResolution(Thermometer2, 10);
  dht.begin();

  adress=EEPROM.read(19); //последняя записанная ячейка
}

//прерывание, формирует мини операционную систему (system)
SIGNAL(TIMER0_COMPA_vect) { 
  tyme = tyme + 1;
  if (tyme % AnalogRead == 0) { rt = 1; };
  if (tyme % DrawTime == 0) { dt = 1; };
  if (tyme % KeyReadTime == 0) { kr = 1; };
}

void loop() {
  if ((rt == 1)) { rt = 0; ReadAnalog();}; //в режиме архива измерений не происходит 
  if (dt == 1) { dt = 0; DrawMenu(); };
  if (kr == 1) { kr = 0; KeyPad(); }; 
}



