/**
  WeatheReport project - transmitter
 * Arduino Nano code

 Copyright (C) 2018 M. Bąk <marcin.bak287@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */


#include "WeatheReport_lib.h"

#define PIN_DS18B20   A0
#define PIN_DHT11     A1
#define PIN_FOTRES    A2
#define PIN_RAIN      2
#define PIN_LED       3

#define REFRESH       30000UL



OneWire oneWire(PIN_DS18B20);                                               // connect to pin
DallasTemperature tempSensor(&oneWire);                                     // transfer information to the library
DeviceAddress thermOut = { 0x28, 0xFF, 0x9A, 0x26, 0x58, 0x16, 0x4, 0x42 }; //Device Address
DHT humSensor;                                                              // Humidity sensor initialize
RF24 Radio(7, 8);                                                          // connect radio sensor to pins
                                               
byte addresses[][6] = {"0"};  // radio adress 

Package data;

void saveData();
int sendData(Package * data);

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  radioTransmitterInit(&Radio, 115, RF24_PA_MAX, RF24_250KBPS, addresses);
  tempSensor.begin();           // Init temp sensors
  humSensor.setup(PIN_DHT11);   // Init humidity sensor 
  pinMode(PIN_LED, OUTPUT);
  //digitalWrite(PIN_LED, HIGH);
  
}

void loop() {
  
  sendData(&data);
  
   
  delay(500);
}

int sendData(Package * data)
{
  static unsigned long last_time;
  unsigned long timeNow = millis();
  
  if(fabs(timeNow - last_time) >= REFRESH)
  {
    digitalWrite(PIN_LED, HIGH);

   
    //saveData(data);
    data->humidity = readHumidity(&humSensor);
    data->temperature = readTemp(&tempSensor, &thermOut);
    data->insolation = readInsolation(PIN_FOTRES);
    data->isRain = readIsRainingD(PIN_RAIN);
    radioSend(&Radio, data, sizeof(*data));
    
    last_time = timeNow;
    delay(200);
    digitalWrite(PIN_LED, LOW); 

    return 1;
  }
  return 0;
}


void saveData(Package * data)
{
    data->humidity = readHumidity(&humSensor);
    data->temperature = readTemp(&tempSensor, &thermOut);
    data->insolation = readInsolation(PIN_FOTRES);
    data->isRain = readIsRainingD(PIN_RAIN);
}
