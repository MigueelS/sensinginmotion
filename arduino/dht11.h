// 
//    FILE: dht11.h
// VERSION: 0.3.2
// PURPOSE: DHT11 Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
//     URL: http://arduino.cc/playground/Main/DHT11Lib
//
// HISTORY:
// George Hadjikyriacou - Original version
// see dht.cpp file
// 

#ifndef dht11_h
#define dht11_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DHT11_CHECKSUM_ERROR -1
#define DHT11_TIMEOUT_ERROR -2
#define DHT11LIB_VERSION "0.3.2"

#define OK 0
#define ERROR 1
#define WAITING 2

class DHT11
{
private:
  unsigned long sensorPin;
  boolean dataReady;
  int errorAttempt;

  int readSensor()
  {
    // BUFFER TO RECEIVE
    uint8_t bits[5];
    uint8_t cnt = 7;
    uint8_t idx = 0;

    // EMPTY BUFFER
    for (int i=0; i< 5; i++) bits[i] = 0;

    // REQUEST SAMPLE
    pinMode(sensorPin, OUTPUT);
    digitalWrite(sensorPin, LOW);
    delay(18);
    digitalWrite(sensorPin, HIGH);
    delayMicroseconds(40);
    pinMode(sensorPin, INPUT);

    // ACKNOWLEDGE or TIMEOUT
    unsigned int loopCnt = 10000;
    while(digitalRead(sensorPin) == LOW)
      if (loopCnt-- == 0) return -2;

    loopCnt = 10000;
    while(digitalRead(sensorPin) == HIGH)
      if (loopCnt-- == 0) return -2;

    // READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
    for (int i=0; i<40; i++)
    {
      loopCnt = 10000;
      while(digitalRead(sensorPin) == LOW)
        if (loopCnt-- == 0) return -2;

      unsigned long t = micros();

      loopCnt = 10000;
      while(digitalRead(sensorPin) == HIGH)
        if (loopCnt-- == 0) return -2;

      if ((micros() - t) > 40) bits[idx] |= (1 << cnt);

      if (cnt == 0)   // next byte?
      {
        cnt = 7;    // restart at MSB
        idx++;      // next byte!
      }
      else cnt--;
    }

    // WRITE TO RIGHT VARS
    // as bits[1] and bits[3] are allways zero they are omitted in formulas.
    humidity    = bits[0]; 
    temperature = bits[2]; 

    uint8_t sum = bits[0] + bits[2];  

    if (bits[4] != sum) return -1;

    return 0;
  }

public:
  long humidity, temperature;

  DHT11()
  {
    humidity = 0;
    temperature = 0;

    errorAttempt = 0;
    dataReady = false;
    sensorPin = 0;
  }

  void config(unsigned long usedPin)
  { 
    sensorPin = usedPin; 
  }

  boolean isDataReady()
  { 
    return dataReady; 
  }

  void resetData()
  { 
    dataReady = false; 
  }

  int read()
  {
    if (isDataReady())
      return OK;

    int status = readSensor();

    if (status == DHT11_CHECKSUM_ERROR || status == DHT11_TIMEOUT_ERROR)
    {
      humidity = -1; 
      temperature = -1;
      errorAttempt++;
      delay(1000);

      if (errorAttempt > 5) // Error timeout, upload (-1, -1) to the server
      {
        dataReady = true;
        errorAttempt = 0;
        return OK;
      }

      return ERROR;
    }

    errorAttempt = 0;
    dataReady = true;
    return OK;
  }
};

#endif

