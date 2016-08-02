# Installation procedure.

* Sketch is using a PMAX library that needs to be at the same location as the directory structure in Github:
    * c:\yourArduinoFolder**\Libraries\PMax**
    * c:\yourArduinoFolder**\PowerMaxEsp8266**

    Learm more about using libraries:
    https://www.arduino.cc/en/Guide/Libraries#toc5

* Make sure that "c:\yourArduinoFolder\" is listed in settings of Arduino IDE as "sketchbook location".
* Update sketch with your WiFi settings:
```
#define WIFI_SSID ""
#define WIFI_PASS ""
```
* Add ESP support to arduino IDE as [described here](http://www.wemos.cc/tutorial/get_started_in_arduino.html).
* Connect the Wemos board to PC using USB and do the first flash.
  Afterwards you will be able to flash via WiFi as well (very useful when board is locked inside the alarm box).

