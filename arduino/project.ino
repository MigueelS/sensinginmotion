#include <GSM.h>
#include <SoftwareSerial.h>
#include "ServerConnection.h"

/* Temperature and humidity sensor pin configuration */
#define DHT11PIN A1

SoftwareSerial droneSerial(8, 9); // RX pin, TX pin

/* Dust sensor configuration */
#define DUSTPIN1 11 // Pin connected to the sensor's P1
#define DUSTPIN2 10 // Pin connected to the sensor's P2
#define DUSTSAMPLETIME 15000 // Total sample time (in ms)

//#define DRONE // Uncomment to send data to the drone's serial port
#define SEND_SERVER // Uncomment to send data to the thingspeak server

/* Data structs */
SensorData data;
ServerConnection connection;

// Turns the LED on or off
void blinkLED(boolean on)
{
  int led = 13;
  pinMode(led, OUTPUT);

  if (on) digitalWrite(led, HIGH);
  else digitalWrite(led, LOW);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Initiating");

#ifdef DRONE
  droneSerial.begin(9600);
#endif

#ifdef SEND_SERVER
  if (connection.init())
    blinkLED(true);
#endif

  data.config(DHT11PIN, DUSTPIN1, DUSTPIN2, DUSTSAMPLETIME);
}

void loop()
{ 
  data.read();

  if (data.isDataReady())
  {
    blinkLED(false);

#ifdef DRONE
    data.getGPSLocation(droneSerial);
#endif

#ifdef SEND_SERVER
    if (connection.sendData(data))
      blinkLED(true);
    else {
      blinkLED(true);
      delay(500);
      blinkLED(false);
      delay(250);
      blinkLED(true);
      delay(500);
    }
#endif
  }
}
