Sensing In Motion
===============

Environmental awareness is a big topic these days and more and more DIY projects involving environmental sensing are appearing.

"Sensing In Motion" is a project with the goal of creating an environmental sensing structure/station that reads several parameters, as well as its GPS location, and sends them to a remote server.
This station can also be plugged on any vehicle (during the course of this project we chose a quadcopter).


![1](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/system1.png)
![4](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/system4.jpg)

## Hardware involved
### Sensing module
* **Processing unit:** Arduino Uno R3
* **Communication module:** Arduino GSM/GPRS Shield

##### Sensors used
* **Humidity/Temperature sensor:** [DHT11](http://bit.ly/1rroTiP)
* **Particulate/Dust sensor:** [Shinyei PPD42NS](http://www.sca-shinyei.com/pdf/PPD42NS.pdf)

### UV module
- Parrot's AR Drone 2.0

##### GPS device
- Parrot's Flight Recorder

## How does it work?
The sensors we're using are connected to Arduino and samples are taken from time to time. Those samples are sent to a remote server (Thingspeak was the chosen one) with HTTP Posts using a GPRS communication.

A serial communication is estabilished between the Arduino and the AR Drone 2.0 in order to receive GPS position information to tag each sample taken. The developed protocol is illustrated [here](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/gps%20protocol.png).

The packages containing GPS coordinates have the following syntax:
> /[latitude],[longitude]!

Command part  | Meaning
------------- | -------------
"/"           | command starter
[latitude]    | latitude value expressed in the decimal degree representation (includes “-” in the beginning if it’s negative and “.” to separate the integer part from the decimal part)
[longitude]   | longitude value expressed in the decimal degree representation (in the same format as the latitude)
"!" | command finalizer

## Instalation

#### Wiring

<p align="center">
  <img src="https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/system2.jpg" alt="System2"/>
  <img src="https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/system3.png" alt="System3"/>
</p>


Make sure you follow [this tutorial](https://gist.github.com/maxogden/4152815) to enable the serial communication between the Arduino and the Drone, as well as installing Node.js on the Drone. A most recent version of the AR Drone 2.0 does not have the 10 pin set described in the tutorial, so check the following [schematic](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/6%20pin%20set.png) (check [this](http://forum.parrot.com/ardrone/en/viewtopic.php?id=8148) for more information).

A Logic Level Shifter will be needed to avoid ruining the Drone's serial pins. We used [this one](https://www.sparkfun.com/products/12009). Check [mirumod's schematic](http://mirumod.tk/hw/arduino_nano/MIRUMODNANO019GPSG_new.jpg) to know how to do the wiring.

A complete wiring diagram of our system (including the sensors) is available [here](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/System%20schematic.png). Since there was a need for multiple GND and 5V pins, we used a PCB and soldered 2 pin sets which are connected to Arduino.

For powering the whole thing we bifurcated the communication between the quadcopter and the battery to also power the Arduino through its barrel jack.

#### Arduino Configuration
Note: To avoid conflicts between the Arduino GSM Library and Software Serial, you should comment several code lines of both libraries ([check this](http://purposefulscience.blogspot.pt/2013/06/arduino-gsm-shield-tips.html)).
Make sure you comment the following lines in SoftwareSerial.cpp:

```c
/*
#if defined(PCINT2_vect)
ISR(PCINT2_vect)
{
  SoftwareSerial::handle_interrupt();
}
#endif
*/
```

and the following in GSM3SoftSerial.cpp:

```cpp
/*
#if defined(PCINT0_vect)
ISR(PCINT0_vect)
{
  GSM3SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT1_vect)
ISR(PCINT1_vect)
{
  GSM3SoftSerial::handle_interrupt();
}
#endif
*/
```

##### Main configuration
If you want to change the pins used or enable/disable features, there are several ```#define``` directives you can change/comment, as well as the digital pins for to-drone communication (be careful when choosing these to avoid conflicts with pins already in use with other components)  in [project.ino](https://github.com/MigueelS/sensinginmotion/blob/master/arduino/project.ino):

```cpp
/* Temperature and humidity sensor pin configuration */
#define DHT11PIN A1

SoftwareSerial droneSerial(8, 9); // RX pin, TX pin

/* Dust sensor configuration */
#define DUSTPIN1 11 // Pin connected to the sensor's P1
#define DUSTPIN2 10 // Pin connected to the sensor's P2
#define DUSTSAMPLETIME 15000 // Total sample time (in ms)

#define DRONE // Uncomment to send data to the drone's serial port
#define SEND_SERVER // Uncomment to send data to the thingspeak server
```

There's also the possibility of choosing the enabled sensors, by commenting the desired ```#define``` directives in [SensorData.h](https://github.com/MigueelS/sensinginmotion/blob/master/arduino/SensorData.h):
```cpp
#define TH_SENSOR_ON // Uncomment to activate Temp/Hum sensor
#define DUST_SENSOR_ON // Uncomment to activate Dust sensor
```

Information regarding your SIM card's mobile carrier, as well as the API key of the Thingspeak Server you're using, should be included in [ServerConnection.h](https://github.com/MigueelS/sensinginmotion/blob/master/arduino/ServerConnection.h):
```cpp
#define PINNUMBER "" // Pin number of your SIM Card

// The following directives are related to the GPRS connection
// Please fill with your mobile carrier's correct data
#define GPRS_APN "umts"
#define GPRS_LOGIN ""
#define GPRS_PASSWORD ""

#define THINGSPEAK_API_KEY "6AEGT8V0J3ZS77S8"
```

#### AR Drone Configuration
You'll be needing to install the modules [node-mavlink](https://github.com/omcaree/node-mavlink) and [node-ar-drone](https://github.com/felixge/node-ar-drone) in your computer, by using npm (get the latest version of node-ar-drone!).

Copy the folder "drone" in this repository to a usb drive. Search for the node_modules folder in your computer (in mine it was in C:\Program Files (x86)\nodejs), and copy the folders "mavlink" and "ar-drone" to the drone folder in your usb drive. Plug the drive in the female usb port on top of the drone.

Telnet into the drone: ```telnet 192.168.1.1``` and run the following commands:

```bash
cd /data/video
cp -a usb/drone/. .
mv mavlink node_modules
mv ar-drone node_modules
cd /etc
vi inittab #(comment the line "ttyO3::askfirst:/bin/sh -l" with a '#')
```

## Usage instructions

It is possible to either execute the manual control (manually controling the drone with your keyboard, check the controls [here](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/manual%20controls.png)) or the automatic control (you need to define a 'mission' on QGroundControl and transfer it to the drone).

```bash
cd /data/video
sh init_prg.sh manual.js #(you can choose either manual.js or automatic.js)
sh init_perm.sh #(run this in a different shell)
```

## Future work and development
If you want to continue our project or do something of your own based on it, here are some guidelines for a future development:
- [ ] Upgrade to a better UAV: Our quadcopter is on the edge of what's capable of lifting, so the system is a bit unstable and its life time is quite low (around 4min ~ 4min 15s), so a more powerful drone would be required in order to get a more stable system.
- [ ] Upgrade to a better processing unit: Arduino Uno is great for most projects, but in our case we're using almost all its processing capability (the GSM Library is quite heavy).
- [ ] Improve the Sensing Module by adding more sensors (for example light or gas sensors to detect air pollution).
- [ ] Develop a better communication protocol between the UAV and the Sensing Module and a better navigation algorithm, for example to:
  - [ ] Synchronize the start of the flight with the sucessful aquirence of a stable GPRS connection.
  - [ ] Synchronize the sensing process with the movement process (stop to take a reading, and then continue if the reading is successfully sent).
  - [ ] Enable a fault system between the 2 modules: sending regular packets to know if both modules are operational and program some action if one of the modules stops working.
- [ ] Improve the navigation algorithm, so it could change its path by accessing the readings taken by the Sensing Module.

## Contact us!
Any questions on the project installation or on its components? Feel free to contact us:
- [marcorodrigues](https://github.com/marcorodrigues)
- [MigueelS](https://github.com/MigueelS)
