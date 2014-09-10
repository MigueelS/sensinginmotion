#ifndef SERVERCOM_H
#define SERVERCOM_H

#include "SensorData.h"
#include <GSM.h>

#define PINNUMBER "" // Pin number of your SIM Card

// The following directives are related to the GPRS connection
// Please fill with your mobile carrier's correct data
#define GPRS_APN "umts"
#define GPRS_LOGIN ""
#define GPRS_PASSWORD ""

#define THINGSPEAK_API_KEY "6AEGT8V0J3ZS77S8"

#define SERVER  "api.thingspeak.com"
#define POST_STR_LENGTH 100
#define FLOAT_STR_LENGTH 18
#define floatToString(f_value, precision, str) dtostrf(f_value, 0, precision, str)

class ServerConnection
{

private:
  GSMClient client;
  GPRS gprs;
  GSM gsmAccess;

public:

  // Initiates the server connection
  boolean init()
  {
    while (true)
    {
      delay(5000);
      if (gsmAccess.begin(PINNUMBER) == GSM_READY)
      {
        Serial.println("GSM connected");
        delay(3000);
        if (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)
        {
          Serial.println("GPRS connected");
          break;
        }

        else
          Serial.println("GPRS not connected");
      }

      else
        Serial.println("GSM not connected");
    }

    return true;
  }

  // Erases the GPRS client's buffer
  void emptyClientBuffer()
  {
    while (client.available())
      char c = client.read();

    if (!client.connected())
      client.stop();
  }

  // Waits for a "200 OK" response from the HTTP Post
  boolean waitOkResponse()
  {
    char okStatus[] = "200 OK";
    int k=0, timeout_value=0;

    while(timeout_value < 100)
    { 
      if (client.available())
      {
        while(client.available())
        {
          char char_response = client.read();
          //Serial.print(char_response);

          if(char_response == okStatus[k])
          {
            k++;

            if(k+1 == strlen(okStatus))
            {
              Serial.println("Received 200 OK");
              emptyClientBuffer();
              return true;
            }
          }

          else
            k=0;
        }
      }
      else
      {
        timeout_value++;
        delay(200);
      }
    }

    Serial.println("Didnt receive 200 OK");
    emptyClientBuffer();
    return false;
  }

  // Sends the data to the server, and resets its values after it
  boolean sendData(SensorData &data)
  {
    int attempt_n = 1;

    while (attempt_n < 3)
    {
      emptyClientBuffer();

      if (!client.connected())
      {
        data.resetData();

        char http_post_str[POST_STR_LENGTH]; // char buffer to save the string to the http post
        char buf_aux[FLOAT_STR_LENGTH], buf_aux2[FLOAT_STR_LENGTH + 10]; //auxiliar float -> string
        int str_size = 0;

        str_size = snprintf(http_post_str, POST_STR_LENGTH, "field1=%d", data.getHumidity());

        for(int i=2; i <= 6; i++)
        {
          double value;      

          if (i == 2)
            value = data.getTemperature();
          else if (i == 3)
            value = data.getDustConcentration1();
          else if (i == 4)
            value = data.getDustConcentration2();

          if (i != 5 && i != 6)
          {
            floatToString(value, 2, buf_aux);    
            snprintf(buf_aux2, FLOAT_STR_LENGTH + 10, "&field%d=%s", i, buf_aux);   
          }
          
          else if (i == 5)
            snprintf(buf_aux2, FLOAT_STR_LENGTH + 10, "&field%d=%s", i, data.latitude);

          else
            snprintf(buf_aux2, FLOAT_STR_LENGTH + 10, "&field%d=%s", i, data.longitude);
           

          strcat(http_post_str, buf_aux2);
        }

        Serial.println(http_post_str);

        if (client.connect(SERVER, 80))
        {
          Serial.println("connecting...");

          // HTTP POST
          client.print("POST /update HTTP/1.1\n");
          client.print("Host: api.thingspeak.com\n");
          client.print("Connection: close\n");
          client.print("X-THINGSPEAKAPIKEY: ");
          client.print(THINGSPEAK_API_KEY);
          client.print("\n");
          client.print("Content-Type: application/x-www-form-urlencoded\n");
          client.print("Content-Length: ");
          client.print(strlen(http_post_str));
          client.print("\n\n");
          client.print(http_post_str);

          Serial.println("done...");

          return waitOkResponse();
        }
        else
        {
          attempt_n++;

          // if you couldn't make a connection:
          Serial.print("connection failed at attempt"); 
          Serial.println(attempt_n);
          client.stop();
        }
      }
    }

    return false;
  }
};

#endif
