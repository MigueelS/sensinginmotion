#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "dust.h"
#include "dht11.h"

#define SENSOR_ERROR -1
#define SENSOR_OK 0

#define TH_SENSOR_ON // Uncomment to activate Temp/Hum sensor
#define DUST_SENSOR_ON // Uncomment to activate Dust sensor

#define GPS_STR_SIZE 20

// Class designed for sensor data storage
class SensorData
{

private:
  boolean dhtReady;
  boolean dustReady;
  boolean sensingStarted;

public:
  DHT11 dht; // Temp/Hum data storage
  DustSensor dust; // Dust data storage
  char* latitude; // GPS latitude of the reading
  char* longitude; // GPS longitude of the reading

public:

  SensorData()
  {
    sensingStarted = false;
    latitude = new char[GPS_STR_SIZE];
    latitude[0] = '0';
    latitude[1] = 0;
    
    longitude = new char[GPS_STR_SIZE];
    longitude[0] = '0';
    longitude[1] = 0;
  }

  void config(int dhtPin, int dustPin1, int dustPin2, int dustSampleTime)
  {
    dht.config(dhtPin);
    dust.config(dustPin1, dustPin2, dustSampleTime);
  }

  // Takes a sensor reading
  void read()
  {
#ifdef DUST_SENSOR_ON
    if (dust.read() == ERROR)
      Serial.println("Error reading dust sensor");
#endif

#ifdef TH_SENSOR_ON

#ifdef DUST_SENSOR_ON
    if (dust.isDataReady())
#endif
      if (dht.read() == ERROR)
        Serial.println("Error reading temp/hum sensor");

#endif
  }
  
  // Returns the temperature value (in Celsius)
  int getTemperature()
  { 
    return dht.temperature; 
  }
  
  // Returns the humidity value (in %)
  int getHumidity()
  { 
    return dht.humidity; 
  }
  
  // Returns the particulate concentration (>= 1 um)
  float getDustConcentration1()
  { 
    return dust.concentrationP1; 
  }
  
  // Returns the particulate concentration (>= 2.5 um)
  float getDustConcentration2()
  { 
    return dust.concentrationP2; 
  }
  
  // Resets the data after a reading
  void resetData()
  {
#ifdef TH_SENSOR_ON
    dht.resetData();
#endif

#ifdef DUST_SENSOR_ON
    dust.resetData();
#endif
  }
  
  // Checks if the data is all successfully collected and ready to be sent
  boolean isDataReady()
  {
    boolean dataReady = true;

#ifdef TH_SENSOR_ON
    dataReady &= dht.isDataReady();
#endif

#ifdef DUST_SENSOR_ON
    dataReady &= dust.isDataReady();
#endif

    return dataReady;
  }
  
  // Asks the Drone for GPS location
  // droneSerial: SoftwareSerial object to use in the communication
  // Returns true if the connection was successfull, false otherwise
  boolean getGPSLocation(SoftwareSerial &droneSerial)
  {
    int nAttempt = 0;

    boolean received = false;

    while (nAttempt < 2)
    {
      Serial.println("Sending GPS request...");
      droneSerial.println("*GPS!");
      Serial.println("Sent GPS request...");

      int i = 0, state = 0; // 1 -> getting latitude, 2 -> getting longitude
      char c;

      // Ups, latitude has more precision than expected or data is corrupted?! Try again
      if (i >= GPS_STR_SIZE)
      {
        Serial.println("ALERT!");
        continue;
      }

      int serialTimeout = 0; 

      while (!received && serialTimeout < 50)
      {
        while (droneSerial.available() > 0 && !received)
        { 
          c = droneSerial.read();

          // If it's a digit, . or -, continue to fill the buffer with it
          if ((isdigit(c) || c == '-' || c == '.') && state == 1)
            latitude[i++] = c;

          else if ((isdigit(c) || c == '-' || c == '.') && state == 2)
            longitude[i++] = c;

          // Latitude was obtained, let's obtain the longitude
          else if (c == ',' && state == 1)
          {
            i = 0;
            state = 2;
          }

          // Longitude was obtained, let's end the communication
          else if (c == '!' && state == 2)
          {
            received = true;
            break;
          }

          // The string received was not valid, start again
          else
          {
            memset(latitude, 0, GPS_STR_SIZE);
            memset(longitude, 0, GPS_STR_SIZE);
            i = 0;
            state = 1;
          }
        }

        if (!received)
        {
          delay(100);
          serialTimeout++;
        }
      }

      if (received)
      {
        Serial.println(latitude);
        Serial.println(longitude);

        droneSerial.println("*GPS_ACK!");
        Serial.println("Sent ACK command!");

        return true;
      }

      else
        Serial.println("Error GPS location");

      nAttempt++;
    }

    latitude[0] = '0';
    latitude[1] = 0;
    longitude[0] = '0';
    longitude[1] = 0;

    Serial.println("Giving up..");

    return false;
  }
};

#endif
