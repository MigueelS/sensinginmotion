/* Grove - Dust Sensor Demo v1.0
 Interface to Shinyei Model PPD42NS Particle Sensor
 Program by Christopher Nafis 
 Written April 2012
 
 http://www.seeedstudio.com/depot/grove-dust-sensor-p-1050.html
 http://www.sca-shinyei.com/pdf/PPD42NS.pdf
 
 Based on the source code of DustDuino too (https://github.com/NodeJournalism/DustDuino).
 */

#ifndef DUST_H
#define DUST_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define OK 0
#define ERROR 1
#define WAITING 2

class DustSensor
{
private:
  unsigned long starttime;
  unsigned long sampletime_ms;
  boolean dataReady, startedSample; // indicates if the data is ready to be sent

  unsigned long sensorPin1, triggerOnP1, triggerOffP1, pulseLengthP1, durationP1;
  float ratioP1;
  int valP1;
  boolean triggerP1;

  unsigned long sensorPin2, triggerOnP2, triggerOffP2, pulseLengthP2, durationP2;
  float ratioP2;
  int valP2;
  boolean triggerP2;

public:
  float concentrationP1, concentrationP2, concentration;

  DustSensor()
  {
    dataReady = false;
    startedSample = false;
    starttime = 0;
    sampletime_ms = 0;

    sensorPin1 = 0;
    sensorPin2 = 0;

    concentrationP1 = 0;
    concentrationP2 = 0;

    valP1 = HIGH;
    triggerP1 = false;

    valP2 = HIGH;
    triggerP2 = false;   
  }

  void config(int usedPin1, int usedPin2, unsigned long sampletime_ms)
  {
    this->sampletime_ms = sampletime_ms;

    sensorPin1 = usedPin1;
    sensorPin2 = usedPin2;

    pinMode(sensorPin1,INPUT);
    pinMode(sensorPin2,INPUT);
  }

  void resetData()
  { 
    dataReady = false; 
  }

  boolean isDataReady()
  { 
    return dataReady; 
  }

  int read()
  {
    if (isDataReady())
      return OK;

    if (!startedSample)
    {
      startedSample = true;
      starttime = millis();
    }

    valP1 = digitalRead(sensorPin1);
    valP2 = digitalRead(sensorPin2);

    if(valP1 == LOW && triggerP1 == false)
    {
      triggerP1 = true;
      triggerOnP1 = micros();
    }

    if (valP1 == HIGH && triggerP1 == true)
    {
      triggerOffP1 = micros();
      pulseLengthP1 = triggerOffP1 - triggerOnP1;
      durationP1 += pulseLengthP1;
      triggerP1 = false;
    }

    if(valP2 == LOW && triggerP2 == false)
    {
      triggerP2 = true;
      triggerOnP2 = micros();
    }

    if (valP2 == HIGH && triggerP2 == true)
    {
      triggerOffP2 = micros();
      pulseLengthP2 = triggerOffP2 - triggerOnP2;
      durationP2 += pulseLengthP2;
      triggerP2 = false;
    }

    if ((millis() - starttime) > sampletime_ms) // if it reaches the sample time
    {
      ratioP1 = durationP1 / (sampletime_ms * 10.0);  // Integer percentage 0=>100
      concentrationP1 = 1.1*pow(ratioP1,3) - 3.8*pow(ratioP1,2) + 520*ratioP1 + 0.62; // using spec sheet curve

      ratioP2 = durationP2 / (sampletime_ms * 10.0);  // Integer percentage 0=>100
      concentrationP2 = 1.1*pow(ratioP2,3) - 3.8*pow(ratioP2,2) + 520*ratioP2 + 0.62; // using spec sheet curve

      startedSample = false;
      durationP1 = 0;
      durationP2 = 0;

      dataReady = true;
      return OK;
    }

    return WAITING;
  }
};

#endif
