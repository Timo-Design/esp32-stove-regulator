//
//    FILE: max31855_ESP32_VSPI.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
// PURPOSE: thermocouple lib demo application
//    DATE: 2021-08-11
//     URL: https://github.com/RobTillaart/MAX31855_RT


// YouTube: https://www.youtube.com/watch?v=1xHdYEeETiM&t=942s


#include <Arduino.h>
#include "MAX31855.h"
#include <map>
#include <string>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>



// read()            timing UNO   timing ESP32 |
//---------------------------------------------
// HWSPI  16000000   ~68  us      ~16 us
// HWSPI   4000000   ~72  us      ~23 us
// HWSPI   1000000   ~100 us      ~51 us
// HWSPI    500000   ~128 us      ~89 us
// SWSPI  bitbang    ~500 us      ~17 us


//
// | HW SPI   |  UNO  |  ESP32  |  ESP32  |
// |          |       |  VSPI   |  HSPI   |
// |:---------|:-----:|:-------:|:-------:|
// | CLOCKPIN |   13  |   18    |   14    |
// | MISO     |   12  |   19    |   12    |
// | MOSI     |   11  |   23    |   13    |  * not used...
// | SELECT   | eg. 4 |    5    |   15    |  * can be others too.

const int selectPin = 5;
const int dataPin   = 19;
const int clockPin  = 18;

const int avgCount = 2;

int averageTemp = 0;

int minTemp = 0;
int maxTemp = 0;

bool tempChanged = false;

void printStatusMessage(int statusCode);

SPIClass * myspi = new SPIClass(VSPI);
MAX31855 thermoCouple(selectPin, myspi);
//  MAX31855 thermoCouple(selectPin, dataPin, clockPin);  //  SW SPI to test

uint32_t start, stop;

// OutPut
const byte led_gpio = 15;

const char deg = (char)223;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("MAX31855_VERSION : ");
  Serial.println(MAX31855_VERSION);
  Serial.println();
  delay(3000);

  myspi->begin();

  thermoCouple.begin();
  thermoCouple.setSPIspeed(1000000);

  // LCD
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();

  //Setup the output
  pinMode(led_gpio, OUTPUT);

}


void loop()
{
  start = micros();
  int status = thermoCouple.read();
  stop = micros();

  if (status == STATUS_NO_COMMUNICATION)
  {
    Serial.println("");
    Serial.println("---------------");
    Serial.println("NO COMMUNICATION");
    Serial.println("---------------");
    Serial.println("");

    lcd.clear();
    lcd.print("NO COMMUNICATION");
  }else{

    Serial.println("------------------------------");
    // Serial.print("time:\t\t");
    // Serial.println(stop - start);
  
    Serial.print("stat:\t\t");
    printStatusMessage(status);
  
    // uint32_t raw = thermoCouple.getRawData();
    // Serial.print("raw:\t\t");

    // uint32_t mask = 0x80000000;
    // for (int i = 0; i < 32; i++)
    // {
    //   if ((i > 0)  && (i % 4 == 0)) Serial.print(" ");
    //   Serial.print((raw & mask) ? 1 : 0);
    //   mask >>= 1;
    // }

  
   // float internal = thermoCouple.getInternal();
   // Serial.print("internal:\t");
   // Serial.println(internal, 3);


  
    float temp = thermoCouple.getTemperature();
    Serial.print("temperature:\t");
    Serial.println(temp, 3);

    int tempC = static_cast<int>(round(temp));

    if(averageTemp == 0){
      // First value
      averageTemp = tempC;
      minTemp = tempC;
      tempChanged = true;
    }else{
      int newAvTemp = (averageTemp * (avgCount - 1) + tempC) / avgCount;

      if(newAvTemp != averageTemp){
        tempChanged = true;
      }
      averageTemp = newAvTemp;
    }

    if (tempC < minTemp) {
      minTemp = tempC;
      tempChanged = true;
    }

    if (tempC > maxTemp) {
      maxTemp = tempC;
      tempChanged = true;
    }

    if(tempChanged){
    lcd.clear();
    delay(300);
   
    lcd.print(averageTemp);
    lcd.print(deg);
    lcd.print("C");

    lcd.setCursor(0,1);
    lcd.print("min:");
    lcd.print(minTemp);
    lcd.print(deg);
    lcd.print("C");
    lcd.print(" max:");
    lcd.print(maxTemp);
    lcd.print(deg);
    lcd.print("C");
  }

    tempChanged = false;
    
  }

  // Speed calculations
  int tempOffset = 30;
  int tempMax = 60;
  int tempMultiply = 2;
  int speed = 0;
  
  speed = (averageTemp - tempOffset) * tempMultiply;
  if (speed < 0){
    speed = 0;
  }
  if(speed > 255){
    speed = 255;
  }

  Serial.print("speed:\t");
    Serial.println(speed);

  analogWrite(led_gpio, speed);

  delay(2000);
}

void printStatusMessage(int statusCode) {
  switch(statusCode) {
      case 0:
          Serial.println ("OK");
          break;
      case 1:
          Serial.println ("Thermocouple open circuit - check wiring");
          break;
      case 2:
          Serial.println ("Thermocouple short to GND - check wiring");
          break;
      case 4:
          Serial.println ("Thermocouple short to VCC - check wiring");
          break;
      case 7:
          Serial.println ("Generic error");
          break;
      case 128:
          Serial.println ("No read done yet - check wiring");
          break;
      case 129:
          Serial.println ("No communication - check wiring");
          break;
      default:
          Serial.println ("Unknown status code");
          break;
  }
}




//  -- END OF FILE --