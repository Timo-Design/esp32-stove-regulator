//
//    FILE: max31855_ESP32_VSPI.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
// PURPOSE: thermocouple lib demo application
//    DATE: 2021-08-11
//     URL: https://github.com/RobTillaart/MAX31855_RT
//

#include <Arduino.h>
#include "MAX31855.h"
#include <map>
#include <string>


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

void printStatusMessage(int statusCode);

SPIClass * myspi = new SPIClass(VSPI);
MAX31855 thermoCouple(selectPin, myspi);
//  MAX31855 thermoCouple(selectPin, dataPin, clockPin);  //  SW SPI to test

uint32_t start, stop;


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
  }else{

    Serial.println();
    Serial.print("time:\t\t");
    Serial.println(stop - start);
  
    Serial.print("stat:\t\t");
    printStatusMessage(status);
  
    uint32_t raw = thermoCouple.getRawData();
    Serial.print("raw:\t\t");

    uint32_t mask = 0x80000000;
    for (int i = 0; i < 32; i++)
    {
      if ((i > 0)  && (i % 4 == 0)) Serial.print(" ");
      Serial.print((raw & mask) ? 1 : 0);
      mask >>= 1;
    }
    Serial.println();
  
    float internal = thermoCouple.getInternal();
    Serial.print("internal:\t");
    Serial.println(internal, 3);
  
    float temp = thermoCouple.getTemperature();
    Serial.print("temperature:\t");
    Serial.println(temp, 3);

  }


  delay(1000);
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