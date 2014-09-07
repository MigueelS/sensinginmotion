Sensing In Motion
===============

Environmental awareness is a big topic these days and more and more DIY projects involving environmental sensing are appearing.

"Sensing In Motion" is a project wit the goal of creating an environmental sensing structure/station that reads several parameters, as well as its GPS location, and sends them to a remote server.
This station can also be plugged on any vehicle (during the course of this project we chose a quadcopter).

## Hardware involved
### Sensing module
* **Processing unit:** Arduino Uno R3
* **Communication module:** Arduino GSM/GPRS Shield

##### Sensors used
* **Humidity/Temperature sensor:** [DHT11] (http://www.micro4you.com/files/sensor/DHT11.pdf)
* **Particulate/Dust sensor:** [Shinyei PPD42NS] (http://www.sca-shinyei.com/pdf/PPD42NS.pdf)

### UV module
- Parrot's AR Drone 2.0

##### GPS device
- Parrot's Flight Recorder

## How does it work?
The sensors we're using are connected to Arduino and samples are taken from time to time. Those samples are sent to a remote server (Thingspeak was the chosen one) with a HTTP Post using a GPRS communication.

A serial communication is estabilished between the Arduino and the AR Drone 2.0 in order to receive GPS position information to tag to each sample taken. The developed protocol is described [here](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/gps%20protocol.png).

## Instalation

#### Wiring
Make sure you follow [this tutorial](https://gist.github.com/maxogden/4152815) to enable the serial communication between the Arduino and the Drone, as well as installing Node.js on the Drone. A most recent version of the AR Drone 2.0 does not have the 10 pin set described in the tutorial, so check the following [schematic](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/6%20pin%20set.png) (check [this](http://forum.parrot.com/ardrone/en/viewtopic.php?id=8148) for more information).

A Logic Level Shifter will be needed to avoid ruining the Drone's serial pins. We used [this one](https://www.sparkfun.com/products/12009). Check [mirumod's schematic](http://mirumod.tk/hw/arduino_nano/MIRUMODNANO019GPSG_new.jpg) to know how to do the wiring.

A complete wiring diagram of our system (including the sensors) is available [here](https://raw.githubusercontent.com/MigueelS/sensinginmotion/master/images/System%20schematic.png). Since there was a need for multiple GND and 5V pins, we used a PCB and soldered 2 pin sets which are connected to Arduino.

#### Arduino Configuration
TODO ....

#### AR Drone Configuration
We advise you to disable the Drone's serial port console communication, by ... (TODO)
