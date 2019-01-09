/**
  WeatheReport project
 * Arduino Uno code

 Copyright (C) 2018 M. Bąk <marcin.bak287@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "WeatheReport_lib.h"
#include "i2c_BMP280.h"
#include <SPI.h>
#include <Ethernet.h>


#define PIN_DS18B20   A1
#define PIN_DHT11     A0
#define LCD_ADDR      0x27 

#define PRESSKOR      67
#define REFRESH       15000UL
#define SEK           1000UL


static const unsigned long MIN = 60*SEK;  
String pdata;

// declerating numbers of signs
static const int deg = 0;             // degree sign
static const int upArr = 1;           // up Arrow
static const int doArr = 2;           // down Arrow
static const int stArr = 3;           // straight Arrow
static const int netSignal = 4;       // net signal sign
static const int NET1 = 5;            // net signal sign
static const int NET2 = 6;            // net signal sign
static const int ERRSIG = 7;          // connection error sign

/**
 * Inititate signs for lcd display
 * 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 */
void signsInitiate(LiquidCrystal_I2C * lcd_screen);

/**
 * Compare temperature values and displays tendence on the screen by the arrow signs
 * 
 * @param ptemp previous temperature value 
 * @param temp current temperature value 
 * @param eps minimal difference beetwen values
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 * @param posX horizontal position on the screen
 * @param posY vertical position on the screen
 */
void comapareTemp(float ptemp, float temp, float eps, const LiquidCrystal_I2C * lcd_screen, const int posX, const int posY);

/**
 * Compare integer values and displays tendence on the screen by the arrow signs
 * 
 * @param pVal previous temperature value 
 * @param Val current temperature value 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 * @param posX horizontal position on the screen
 * @param posY vertical position on the screen
 */
void comapareValues(int pVal, int Val, const LiquidCrystal_I2C * lcd_screen, const int posX, const int posY);

/**
 * Initiate lcd screen, sets meassure types and units
 * 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 */
void displayInit(const LiquidCrystal_I2C * lcd);

/**
 * Get data from internal sensors
 * Sets values on the lcd screen
 * 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 */
void displayData(const LiquidCrystal_I2C * lcd_screen);

/**
 * Recieve meassured data from the transmitter
 * Compares current values with previous meassured
 * Displays recieved data on the screen
 * 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object
 */
int recievedData(const LiquidCrystal_I2C * lcd_screen);

/**
 * Send data to database 
 * 
 * @return ERR connection failed
 */
int SendToDatabase();

/**
 * Display recieved data from transmitter
 * 
 * @param lcd_screen Pointer to the LiquidCrystal_I2C object 
 */
void displayRecievedData(const LiquidCrystal_I2C * lcd_screen);

/**
 * if connection with transmitter succed set blinking Colon on the screen 
 * else set blinking x if connecion failed
 */
void blinkColon();

/*
 * display connection status with database
 */
int dispStatusConnection();


OneWire oneWire(PIN_DS18B20);                                              // connect to pin       
DallasTemperature tempSensor(&oneWire);                                    // transfer information to the library
DeviceAddress thermIn = { 0x28, 0xFF, 0xD2, 0xB2, 0x57, 0x16, 0x4, 0x25 }; //Device Address

DHT humSensor;                         // Humidity sensor initialize
LiquidCrystal_I2C lcd(LCD_ADDR,20,4);  // set the LCD address to LCD_ADDR for a 20 chars and 4 line display

RF24 Radio(7, 8);                      // connect radio sensor to pins
BMP280 bmp280;                         // initialize pressure sensor object
                                               
byte addresses[][6] = {"0"};           // radio adress 


Package data;

int resume_disp;                      // global variable for resume display information about data from transmitter
int resume_connection;                // global variable for resume connection with database

byte mac[] = { 0x20, 0xD3, 0x90, 0x57, 0x49, 0xD2 };  // device mac address
byte ip[] = {192, 168, 0, 12 };    //Enter the IP of ethernet shield
char serv[] = "www.mysql.cba.pl" ; //Enter the IPv4 address
EthernetClient cliente;            //Ethernet client object

// Global variables used to store information about previous data
static float tempG;
static int humG;
static int pressG;


byte degree[8] = {
  0b01100,
  0b10010,
  0b10010,
  0b01100,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte net[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111
};

byte errsig[8] = {
  0b10100,
  0b01000,
  0b10100,
  0b00001,
  0b00011,
  0b00111,
  0b01111,
  0b11111
};

byte net1[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01000,
  0b11000
};

byte net2[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00010,
  0b00110,
  0b01110,
  0b11110
};

byte ok[8] = {
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b10100,
  0b01000,
  0b00000,
  0b00000
};

byte upArrow[8] = {
  0b00111,
  0b00011,
  0b00101,
  0b01000,
  0b10000,
  0b00000,
  0b00000,
  0b00000
};

byte downArrow[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b01000,
  0b00101,
  0b00011,
  0b00111
};

byte straightArrow[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b00000
};


void setup() {

  Ethernet.begin(mac, ip);        //ETHERNET
  radioRecieverInit(&Radio, 115, RF24_PA_MAX, RF24_250KBPS, 1, addresses);
  tempSensor.begin();           // Init temp sensors
  humSensor.setup(PIN_DHT11);   // Init humidity sensor 
  displayInit(&lcd);
    
  bmp280.initialize();
  //onetime-measure:
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();

  
}

void loop() {
  blinkColon();
  dispStatusConnection();
  recievedData(&lcd);
  displayData(&lcd);
  SendToDatabase();
  
  delay(500);
}


void signsInitiate(const LiquidCrystal_I2C * lcd_screen)
{
  // creating signs on lcd
  lcd_screen->createChar(deg, degree);
  lcd_screen->createChar(upArr, upArrow);
  lcd_screen->createChar(doArr, downArrow);
  lcd_screen->createChar(stArr, straightArrow);
  lcd_screen->createChar(netSignal, net);
  lcd_screen->createChar(NET1, net1);
  lcd_screen->createChar(NET2, net2);
  lcd_screen->createChar(ERRSIG, errsig);
}

void displayInit(const LiquidCrystal_I2C * lcd_screen)
{
  signsInitiate(lcd_screen);

  lcd_screen->init(); // initialize the lcd 
  lcd_screen->backlight();
  lcd_screen->clear();
  
  lcd_screen->setCursor(4,1);
  lcd_screen->print("WeatheReport");
  lcd_screen->setCursor(7,2);
  lcd_screen->print("v5.00");

  delay(3000);
  lcd_screen->clear();

  lcd_screen->setCursor(0,1);
  lcd_screen->print("Tw   ");
  lcd_screen->print("  ");
  lcd_screen->write((uint8_t)deg);
  lcd_screen->print("C");
  lcd_screen->setCursor(0,2);
  lcd_screen->print("Ww   ");
  lcd_screen->print(" %");
  lcd_screen->setCursor(0,3);
  lcd_screen->print("C     ");
  lcd_screen->print("hPa");
  displayData(lcd_screen);

////////////////////////////////////////

  signsInitiate(lcd_screen);
  lcd_screen->setCursor(0, 0);
  lcd_screen->print("Pog: ");
  lcd_screen->setCursor(19, 0);
  lcd_screen->write((uint8_t)netSignal);
  lcd_screen->setCursor(10,1);
  lcd_screen->print("Tz   ");
  lcd_screen->print("  ");
  lcd_screen->write((uint8_t)deg);
  lcd_screen->print("C");
  lcd_screen->setCursor(10 ,2);
  lcd_screen->print("Wz   ");
  lcd_screen->print(" %");
  lcd_screen->setCursor(10 ,3);
  lcd_screen->print("Nz   ");
  lcd_screen->print(" %");
  
}


void displayData(const LiquidCrystal_I2C * lcd_screen)
{
  static unsigned long last_time;
  unsigned long timeNow = millis();
  
  if(fabs(timeNow - last_time) >= REFRESH)
  {
    static int prev_humIN;
    static float prev_tempIN;
    static float prev_pressure;

    
    float eps = 0.1;

    int humIN = readHumidity(&humSensor);
    float tempIN = readTemp(&tempSensor, &thermIn);
    
    float pascal;
    bmp280.awaitMeasurement();
    bmp280.getPressure(pascal);
    int pressure = pascal / 100 + PRESSKOR;

    comapareTemp(prev_tempIN, tempIN, eps, lcd_screen, 9, 1);
    if(fabs(prev_tempIN - tempIN) >= eps)
    {
      lcd_screen->setCursor(3, 1);
      lcd_screen->print("   ");
      lcd_screen->setCursor(3, 1);
      lcd_screen->print(tempIN, 1);
      prev_tempIN = tempIN;
    }

    comapareValues(prev_humIN, humIN, lcd_screen, 9, 2);
    if(prev_humIN != humIN)
    {
      lcd_screen->setCursor(3, 2);
      lcd_screen->print("   ");
      lcd_screen->setCursor(3, 2);
      lcd_screen->print(humIN);
      prev_humIN = humIN;
    }  
    comapareValues(prev_pressure, pressure, lcd_screen, 9, 3);
    if(prev_pressure != pressure)
    {
      lcd_screen->setCursor(1, 3);
      lcd_screen->print("     ");
      lcd_screen->setCursor(2, 3);
      lcd_screen->print(pressure);
      prev_pressure = pressure;
      
    } 
    tempG = tempIN;
    humG = humIN;
    pressG = pressure;
    last_time = timeNow;
  }
}

void displayRecievedData(const LiquidCrystal_I2C * lcd_screen)
{

    static Package prev_data;
    float eps = 0.1;

    if(resume_disp == 1)
    {
      Package newData;
      prev_data = newData;
    }
    comapareTemp(prev_data.temperature, data.temperature, eps, lcd_screen, 19, 1);
    if(fabs(prev_data.temperature - data.temperature) >= eps)
    {
      lcd_screen->setCursor(12,1);
      lcd_screen->print("     ");
      lcd_screen->setCursor(13,1);
      lcd_screen->print(data.temperature, 1);
      prev_data.temperature = data.temperature;
    }

    comapareValues(prev_data.humidity, data.humidity, lcd_screen, 19, 2);
    if(prev_data.humidity != data.humidity)
    {
      lcd_screen->setCursor(12,2);
      lcd_screen->print("    ");
      lcd_screen->setCursor(13,2);
      lcd_screen->print(data.humidity);
      prev_data.humidity = data.humidity;
    }  

    if(fabs(prev_data.insolation - data.insolation) >= 5)
    {
      lcd_screen->setCursor(12,3);
      lcd_screen->print("    ");
      lcd_screen->setCursor(13,3);
      lcd_screen->print(data.insolation);
    }  

    comapareValues(prev_data.insolation, data.insolation, lcd_screen, 19, 3);
    if(prev_data.isRain != data.isRain || fabs(prev_data.insolation - data.insolation) >= 5)
    {
      lcd_screen->setCursor(5,0);
      lcd_screen->print("            ");
      
      lcd_screen->setCursor(5,0);
      lcd_screen->print(readWeatherPL(data));
      
      prev_data.insolation = data.insolation;
      prev_data.isRain = data.isRain;
    }  
    
    
 
}

void blinkColon()
{
  unsigned long timeNow = millis();
  static unsigned long last_c;
  if(resume_disp != 0)
  {
    lcd.setCursor(3, 0);
    lcd.print(" ");
    if(fabs(timeNow - last_c) >= SEK)
    {
      lcd.setCursor(3, 0);
      lcd.print(":");
      last_c = timeNow;
    }
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print(" ");
    if(fabs(timeNow - last_c) >= SEK)
    {
      lcd.setCursor(3, 0);
      lcd.print("x");
      last_c = timeNow;
    }
  }
  
}

int dispStatusConnection()
{
  unsigned long timeNow = millis();
  static unsigned long last_c;
  static int i;
  static bool errconnect;
  if(fabs(timeNow - last_c) >= SEK)
  {
    if(resume_connection)
    {
        if(i == 0)
        {
          lcd.setCursor(19, 0);
          lcd.print(" ");
          lcd.setCursor(19, 0);
          lcd.write((uint8_t)NET1);

          i++;
          last_c = timeNow;
          return 1;
        }
        else if(i == 1)
        {
          lcd.setCursor(19, 0);
          lcd.write((uint8_t)NET2);

          i++;
          last_c = timeNow;
          return 1;
        }
        else
        {
          lcd.setCursor(19, 0);
          lcd.write((uint8_t)netSignal);

          i = 0;
          last_c = timeNow;
          return 1;
        }
    }
    else
    {
          if(errconnect)
          {
              lcd.setCursor(19, 0);
              lcd.write((uint8_t)ERRSIG);
              last_c = timeNow;
              errconnect = !errconnect;
          }
          else
          {
            lcd.setCursor(19, 0);
            lcd.write((uint8_t)netSignal);
            last_c = timeNow;
            errconnect = !errconnect;
          }
      
          return ERR;  
    }
    
  }
  return 0;
}




int recievedData(const LiquidCrystal_I2C * lcd_screen)
{
  static unsigned long last_c;
  unsigned long timeNow = millis();
  
  
  if(radioRecieve(&Radio, &data, sizeof(data)))
  {

    if(resume_disp == 0)
    {
      resume_disp = 1;  
    }

    displayRecievedData(lcd_screen);
    last_c = timeNow;
    
    resume_disp = 2;
    return 1;
  }
  if(fabs(timeNow - last_c) >= MIN)
  { 
     resume_disp = 0;
     last_c = timeNow;

     return -1;
  }

  return 0;  
}

void comapareTemp(float ptemp, float temp, float eps, const LiquidCrystal_I2C * lcd_screen, const int posX, const int posY)
{
  lcd_screen->setCursor(posX, posY);
  lcd_screen->print(" ");
  lcd_screen->setCursor(posX, posY);
  if(ptemp - temp > eps)
  {
    lcd_screen->write((uint8_t)doArr);  
  }
  else if(temp - ptemp > eps)
  {
    lcd_screen->write((uint8_t)upArr);  
  }
  else
  {
    lcd_screen->write((uint8_t)stArr);
  }
}
  
void comapareValues(int pVal, int Val, const LiquidCrystal_I2C * lcd_screen, const int posX, const int posY)
{
  lcd_screen->setCursor(posX, posY);
  lcd_screen->print(" ");
  lcd_screen->setCursor(posX, posY);
  if(pVal > Val)
  {
    lcd_screen->write((uint8_t)doArr);  
  }
  else if(pVal < Val)
  {
    lcd_screen->write((uint8_t)upArr);  
  }
  else
  {
    lcd_screen->write((uint8_t)stArr);
  }
}


int SendToDatabase()
{
  unsigned long long cur_time = millis();
  static unsigned long long last_check;
  if(fabs(cur_time - last_check) >= 5*MIN)
  {
    if (cliente.connect("www.weathereport.cba.pl", 80)) 
    { //Connecting at the IP address and port we saved before
      if(resume_disp == 1 || resume_disp == 2)
      {
  
        cliente.print("GET /wr_data.php?"); //Connecting and Sending values to database
        cliente.print("temperature=");
        cliente.print(tempG, 1);
        cliente.print("&humidity=");
        cliente.print(humG);
        cliente.print("&pressure=");
        cliente.print(pressG);
      
        cliente.print("&temperature_out=");
        cliente.print(data.temperature, 1);
        cliente.print("&humidity_out=");
        cliente.print(data.humidity);
        cliente.print("&insolation=");
        cliente.print(data.insolation);
        cliente.print("&weather=");
        cliente.print(readWeatherEN(data));
        cliente.println( " HTTP/1.1");
        cliente.println( "Host: www.weathereport.cba.pl" );
        cliente.println( "Connection: close" );
        cliente.println();
    
      }
      else
      {

        cliente.print("GET /wr_data.php?"); //Connecting and Sending values to database
        cliente.print("temperature=");
        cliente.print(tempG, 1);
        cliente.print("&humidity=");
        cliente.print(humG);
        cliente.print("&pressure=");
        cliente.print(pressG);
        cliente.println( " HTTP/1.1");
        cliente.println( "Host: www.weathereport.cba.pl" );
        cliente.println( "Connection: close" );
        cliente.println();
        
      }
  
      last_check = cur_time;
      cliente.stop(); //Closing the connection
      resume_connection = 1;
      return 1;
    }
    else {
      // if you didn't get a connection to the server:
      Serial.println("disconnected");
      last_check += 5*SEK;
      resume_connection = 0;
      return ERR;
    }
  }
  return 0;
}
