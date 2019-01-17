/*
 Copyright (C) 2018 M. Bąk <marcin.bak287@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

 /**
 * @file WeatheReport_lib.h
 
 * Functions declaration for WeatheReport project
 */
 
#ifndef WeatheReport_lib_h
#define WeatheReport_lib_h

// Adding important libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>  
#include <DHT.h>
#include <RF24.h>
#include <String.h>


// constants definitions
#define ERR           -1

/**
 * Package structure contains data to transmit or recieve
 */
struct package                          
{
  float temperature = -100;
  int humidity = -100;
  int insolation = -100;
  int isRain = -100; 
};





typedef struct package Package;

/**
 * Read is raining from the analog sensor
 *
 * @param analogPin analog pin to analog rain sensor 
 * @param waterLevel value of top dry level: 0-100 
 *
 * @return 1 value if water level on sensor is below @param waterLevel
 * @return 0 value if water level on sensor is above @param waterLevel
 * @return -1 error with parameters or read procedure
 */
int readIsRainingA(const int analogPin, const int waterLevel);

/**
 * Read is raining from the digital sensor
 *
 * @param digitalPin analog pin to analog rain sensor 
 *
 * @return 1 value if water level on sensor is below @param waterLevel
 * @return 0 value if water level on sensor is above @param waterLevel
 */
int readIsRainingD(const int digitalPin);

/**
 * Read insolation from @param PIN_FOTRES, map to the percent value
 *
 * @return percent value of insolation
 */
int readInsolation(const int analogPin);

/**
 * Read a temperature in Celcius degrees from sensor
 * 
 * @param sensors Pointer object of DallasTemperature class, which is in DallasTemperature.h library
 * @param devAddress Pointer to device address
 *
 * @return temperature value
 */
float readTemp(const DallasTemperature * sensors, const DeviceAddress * devAddress);


/**
 * read Humidity from the sensor
 *
 * @param dhtSensor Pointer to the object of DHT class
 * @return humidity percent value
 */
int readHumidity(const DHT * dhtSensor);

/**
 * Set radio communication for transmitter
 *
 * Begin operation of the chip
 *
 * Set RF communication channel
 * Set Power Amplifier (PA) level to one of four levels.
 * Relative mnemonics have been used to allow for future PA level
 * changes. According to 6.5 of the nRF24L01+ specification sheet,
 * they translate to: RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm,
 * RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
 *
 * Set the transmission data rate
 * @warning setting RF24_250KBPS will fail for non-plus units
 *
 * Open a pipe for writing
 *
 * Only one pipe can be open at once, but you can change the pipe
 * you'll listen to.  Do not call this while actively listening.
 * Remember to stopListening() first.
 *
 * Addresses are 40-bit hex values, e.g.:
 *
 * @code
 *   openWritingPipe(0xF0F0F0F0F0);
 * @endcode
 *
 * @param radio Pointer to the object of RF24 class
 * @param channel Which RF channel to communicate on, 0-127
 * @param PALevel Desired PA level.
 * @param DataRate RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
 * @param address The 40-bit address of the pipe to open.  This can be
 * any value whatsoever, as long as you are the only one writing to it
 * and only one other radio is listening to it.  Coordinate these pipe
 * addresses amongst nodes on the network.
 *
 *
 */
void radioTransmitterInit(RF24 * radio, const uint8_t channel, const rf24_pa_dbm_e PALevel, const rf24_datarate_e DataRate, const uint64_t address);

 /**
 * Send data to the reciever 
 *
 * Write to the open writing pipe
 *
 * Be sure to call radioInit() first to set the destination
 * of where to write to.
 *
 * This blocks until the message is successfully acknowledged by
 * the receiver or the timeout/retransmit maxima are reached.  In
 * the current configuration, the max delay here is 60ms.
 *
 * @param radio Pointer to the object of RF24 class
 * @param buf Pointer to the data to be sent
 * @param len Number of bytes to be sent
 *
 * @return True if the payload was delivered successfully false if not
 * for multicast payloads, true only means it was transmitted.
 */
bool radioSend(RF24 * radio, const void* buf, uint8_t len);

/**
 *
 * Open a pipe for reading
 *
 * Up to 6 pipes can be open for reading at once.  Open all the
 * reading pipes, and then call startListening().
 *
 * Begin operation of the chip
 *
 * Set RF communication channel
 * Set Power Amplifier (PA) level to one of four levels.
 * Relative mnemonics have been used to allow for future PA level
 * changes. According to 6.5 of the nRF24L01+ specification sheet,
 * they translate to: RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm,
 * RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
 *
 * Set the transmission data rate
 * @warning setting RF24_250KBPS will fail for non-plus units
 * @see openWritingPipe
 *
 * @warning Pipes 1-5 should share the first 32 bits.
 * Only the least significant byte should be unique, e.g.
 * @code
 *   openReadingPipe(1,0xF0F0F0F0AA);
 *   openReadingPipe(2,0xF0F0F0F066);
 * @endcode
 *
 * @warning Pipe 0 is also used by the writing pipe.  So if you open
 * pipe 0 for reading, and then startListening(), it will overwrite the
 * writing pipe.  Ergo, do an openWritingPipe() again before write().
 *
 * @warning Pipe 0 is also used as the multicast address pipe. Pipe 1
 * is the unicast pipe address.
 *
 * @todo Enforce the restriction that pipes 1-5 must share the top 32 bits
 *
 * @param radio Pointer to the object of RF24 class
 * @param channel Which RF channel to communicate on, 0-127
 * @param PALevel Desired PA level.
 * @param DataRate RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
 * @param number Which pipe# to open, 0-5.
 * @param address The 40-bit address of the pipe to open.
 */
void radioRecieverInit(RF24 * radio, const uint8_t channel, const rf24_pa_dbm_e PALevel, const rf24_datarate_e DataRate, uint8_t number, const uint64_t address);

/**
 * Read recieved data
 * 
 * Read the payload
 *
 * Return the last payload received
 *
 * The size of data read is the fixed payload size, see getPayloadSize()
 *
 * @note I specifically chose 'void*' as a data type to make it easier
 * for beginners to use.  No casting needed.
 *
 * @param radio Pointer to the object of RF24 class
 * @param buf Pointer to a buffer where the data should be written
 * @param len Maximum number of bytes to read into the buffer
 *
 * @return True if the payload was delivered successfully false if not
 */
bool radioRecieve(RF24 * radio, const void* buf, uint8_t len);


/**
 * Read the pressure from BMP280 sensor
 * Assign pressure value calculated to hectoPascals to the @param hpascal
 *
 * @param sensor Pointer to the ocject of BMP280 class
 * @param hpascal Reference to the hpascal value
 */
//void readPressure(const BMP280 * sensor, uint32_t & hpascal)

/**
 * Specify the weather base on data from the transmitter sensor
 * Polish language
 */
char * readWeatherPL(const package data);

/**
 * Specify the weather base on data from the transmitter sensor
 * English language
 */
String readWeatherEN(const package data);

#endif
