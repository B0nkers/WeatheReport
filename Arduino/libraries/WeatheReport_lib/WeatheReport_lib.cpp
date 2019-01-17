/*
 Copyright (C) 2018 M. Bąk <marcin.bak287@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include "WeatheReport_lib.h"

/****************************************************************************/

int readInsolation(const int analogPin)
{
  return analogRead(analogPin)*100.0/1023;     // calculate and return percent value of insolation on the photoresistor                                                                                                      
}

/****************************************************************************/

float readTemp(const DallasTemperature * sensors, const DeviceAddress * devAddress)
{
  sensors->requestTemperatures();              // get the temperature from sensor

  return sensors->getTempC(*devAddress);       // return Temperature value;
}

/****************************************************************************/

int readIsRainingA(const int analogPin, const int waterLevel)
{
  int rain = map(analogRead(analogPin), 1023, 0, 0, 100);  // get the value and map to the percent range: 0 - 100%
  if((rain > 100 || rain < 0) || (waterLevel > 100 || waterLevel < 0))
	  return ERR;
  
  if(rain <= waterLevel)                             		
    return 1;                                				// returned value: 1 - is not raining, 0 - is raining

  return 0;                       
}

/****************************************************************************/

int readIsRainingD(const int digitalPin)
{
  return digitalRead(digitalPin);      // return the value: 1 - is not raining, 0 - is raining                   
}

/****************************************************************************/

void radioTransmitterInit(RF24 * radio, const uint8_t channel, const rf24_pa_dbm_e PALevel, const rf24_datarate_e DataRate, const uint64_t address)
{
  radio->begin();  
  radio->setChannel(channel); 
  radio->setPALevel(PALevel);
  radio->setDataRate( DataRate ) ; 
  radio->openWritingPipe( address );
}

/****************************************************************************/

void radioRecieverInit(RF24 * radio, const uint8_t channel, const rf24_pa_dbm_e PALevel, const rf24_datarate_e DataRate, uint8_t number, const uint64_t address)
{
  radio->begin(); 
  radio->setChannel(channel); 
  radio->setPALevel(PALevel);
  radio->setDataRate( DataRate ) ; 
  radio->openReadingPipe(number, address);
  radio->startListening();
}

/****************************************************************************/

bool radioSend(RF24 * radio, const void * buf, uint8_t len)
{  
  /*if(radio->write(buf, len))
    return true;

   return false;*/
   radio->write( buf, len );
   
   return true;
}

/****************************************************************************/

bool radioRecieve(RF24 * radio, const void* buf, uint8_t len)
{
  if ( radio->available()) 
  {
    while (radio->available())
    {
      radio->read( buf, len );
    }
	return true;
  }
  
  return false;
}

/****************************************************************************/

int readHumidity(const DHT * dhtSensor)
{
  int humidity = dhtSensor->getHumidity();
  static int last_hum;
  // checking if reading was correct
  if (dhtSensor->getStatusString() == "OK") 
  {
      last_hum = humidity;
  }
      
  return last_hum;    // return last measured value
}

/****************************************************************************/

/*void readPressure(const BMP280 * sensor, uint32_t & hpascal)
{
	sensor.getPressure(hpascal);
	hpascal /= 100;
}*/

char * readWeatherPL(const package data)
 {
	if(data.isRain == 1)
	{
	  if(data.insolation > 60)
	  {		  
		  return "Slonecznie";
	  }
	  else if(data.insolation <= 5)
	  {
		  return "Mrocznie";
	  }
	  else
	  {
		  return "Pochmurnie";
	  }
	}
	else
	{
		return "Deszczowo";
	}
 }
 
 /***************************************************************************/
 
 String readWeatherEN(const package data)
 {
	if(data.isRain == 1)
	{
	  if(data.insolation > 60)
	  {		  
		  return "sunny";
	  }
	  else if(data.insolation <= 5)
	  {
		  return "night";
	  }
	  else
	  {
		  return "cloudy";
	  }
	}
	else
	{
		return "rainy";
	}
 }