# PowerMaxAlarm

## Generic C++ library for Visonic PowerMax alarms, with number of projects that use it.
## Monitor and control your alarm (via wired or WiFi link).

This project is separate into following parts:
* [Generic C++ library](#generic-c-library-pmax) (called PMAX) that you can use to interface with PowerMax alarms.
  Allows decoding packets, handling all communication, reading settings from device, arm/disarm, etc.
  
* [Windows command line application](#windows-command-line-application).
  It uses PMAX library to communicate with PM alarm.
  Communication can be done using:
    * Serial interface
    * TCP/IP (by connecting to ESP8266 running PMAX in packet relay mode)
    
* [ESP8266 sketch](#esp8266-sketch).
  ESP8266 is a cheap (around 4£) microcontroller with Wi-Fi capability.
  It's very easy to connect it into the PM and can be hidden inside the alarm.
  ESP8266 can work in two modes:
    * Stand alone: where it uses PMAX library to communicate with PM alarm, and exposes web and telnet interface.
    * Packet relay mode: where ESP serves as a transparent, wireless communication link, this removes a need for USB->Serial adapters and wires.
                         Packet relay mode is used by Windows command line app to connect to PM alarm wirelessly.

***
## Generic C++ library (PMAX).
Implements everything needed to communicate with PowerMax alarm in portable C++ class (no references to other libraries).
If you want to extend the functionality (for example add MQTT support, inherit from this class and override functions you need). 
This library is used by Windows command line app and ESP8266 microcontroller.

***
## Windows command line application.
This application allows you to connect to PM alarm, monitor and control it via command line.
There are two ways to establish connection:

* Wired:
  Use 3.3V USB to serial interface and connect it to RS232 port in your alarm.
  NOTE: DO NOT USE 5V usb to serial - as this will kill the alarm.
  Serial interface can be bought cheaply on ebay for around 3£, when connected to the PC, it appears as COM port.
  
* Wireless (WiFi):
  Connect to ESP8266 via WiFi (this will put ESP into transparent packet relay mode).
  

***
## ESP8266 sketch.
ESP8266 is a very inexpensive way to add WiFi capability to your PM alarm.
In simplest scenario **you don't need ANYTHING else**, no PC, no raspberry PI needed, nothing.

ESP8266 communicates with your alarm via PMAX library, exposes simple Web and telnet interfaces on your WiFi network.
ESP8266 allows also for other apps to connect directly to the alarm (works as a wireless packet relay).
You can modify ESP8266 code according to your needs (maybe push data to external website, etc).

For this project I recommend to use WeMos D1 board (full size, not mini):
* D1 operates on 3.3V which is compatible with PM RS232 interface.
* D1 has a DC jack that can accept 12V power that is available inside PM alarms .
* Size of the board allows to fit it inside PM alarm (tested with PowerMax Complete)
* D1 allows Over-The-Air (WiFi) firmware updates, so you don't need to open the alarm to update the firmware.
    
Getting started:
* Purchase [WeMos D1 board](http://www.wemos.cc/Products/d1_r2.html) (both revision 1 and 2 are OK)
* Connect it first to the PC, and flash PowerMaxEsp8266 project using [Arduino IDE](http://www.wemos.cc/tutorial/get_started_in_arduino.html). Make sure to update your WiFi details on top of the sketch.
* Connect WeMos to PowerMax RS232 port:
  ![Wemos pinout](https://github.com/irekzielinski/PowerMaxAlarm/blob/master/doc/WemosPowerMax.png?raw=true)
* Connect 12V power from alarm to DC input in WeMos D1 (ground to the outside, 12V+ to the centre pin).
* On PowerMax Complete that's all you need to do, on other board you might need to perform manual powerlink enrolment
* Find IP address of your ESP.
* Connect to this IP with telnet or web browser.


***
## Credits:
* Various posts on: http://www.domoticaforum.eu/viewforum.php?f=68
* Implementation of PMAX library directly based on this file form DomotiGa3 project: https://github.com/DomotiGa/DomotiGa/blob/master/DomotiGa3/.src/CVisonic.class
* This project: http://code.mios.com/trac/mios_visonic-powermax/browser
* John Chan: for sharing his own ESP8266 code
