#include <pmax.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

//////////////////// IMPORTANT DEFINES, ADJUST TO YOUR NEEDS //////////////////////
//////////////////// COMMENT DEFINE OUT to disable a feature  /////////////////////
//Telnet allows to see debug logs via network, it allso allows CONTROL of the system if PM_ALLOW_CONTROL is defined
#define PM_ENABLE_TELNET_ACCESS

//This enables control over your system, when commented out, alarm is 'read only' (esp will read the state, but will never ardm/disarm)
#define PM_ALLOW_CONTROL

//This enables flashing of ESP via OTA (WiFI)
#define PM_ENABLE_OTA_UPDATES

//This enables ESP to send a multicast UDP packet on LAN with notifications
//Any device on your LAN can register to listen for those messages, and will get a status of the alarm
#define PM_ENABLE_LAN_BROADCAST

//Those settings control where to send lan broadcast, clients need to use the same value. Note IP specified here has nothing to do with ESP IP, it's only a destination for multicast
//Read more about milticast here: http://en.wikipedia.org/wiki/IP_multicast
IPAddress PM_LAN_BROADCAST_IP(224, 192, 32, 12);
#define   PM_LAN_BROADCAST_PORT  23127

//Specify user friendly names for your zones
const char * const cfg_zones[] = {  "system",  "front door", "hall", "living room",  "kitchen", 
                                    "study",  "upstairs", "conservatory", "garage", "back door", "garage2"};

//Specify Pin for your PowerMax alarm
//If your pin is 1234, you need to return 0x1234 (this is strange, as 0x makes it hex, but the only way it works)
#define ALARM_USER_PIN 0x1234; //IZIZTODO

//Specify your WIFI settings:
#define WIFI_SSID "MyNet"
#define WIFI_PASS "MyPassword"

//////////////////////////////////////////////////////////////////////////////////

PowerMaxAlarm pm;
ESP8266WebServer server(80);

int telnetDbgLevel = LOG_NO_FILTER; //by default only NO FILTER messages are logged to telnet clients 

#ifdef PM_ENABLE_LAN_BROADCAST
WiFiUDP udp;
unsigned int packetCnt = 0;
#endif

#ifdef PM_ENABLE_TELNET_ACCESS
WiFiServer telnetServer(23); //telnet server
WiFiClient telnetClient;
#endif

#define PRINTF_BUF 512 // define the tmp buffer size (change if desired)
void LOG(const char *format, ...)
{
  char buf[PRINTF_BUF];
  va_list ap;
  
  va_start(ap, format);
  vsnprintf(buf, sizeof(buf), format, ap);
#ifdef PM_ENABLE_TELNET_ACCESS  
  if(telnetClient.connected())
  {
    telnetClient.write_P(buf, strlen(buf));
  }
#endif  
  va_end(ap);
}

void handleRoot() {
  unsigned long days = 0, hours = 0, minutes = 0;
  unsigned long val = os_getCurrentTimeSec();
  
  days = val / (3600*24);
  val -= days * (3600*24);
  
  hours = val / 3600;
  val -= hours * 3600;
  
  minutes = val / 60;
  val -= minutes*60;

  char szTmp[PRINTF_BUF];
  sprintf(szTmp, "hello from esp8266: Uptime: %02d:%02d:%02d.%02d", (int)days, (int)hours, (int)minutes, (int)val);  
  server.send(200, "text/plain", szTmp);
}

//writes to webpage without storing large buffers, only small buffer is used to improve performance
class WebOutput : public IOutput
{
  WiFiClient* c;
  char buffer[PRINTF_BUF+1];
  int bufferLen;
  
public:
  WebOutput(WiFiClient* a_c)
  {
    c = a_c;
    bufferLen = 0;
    memset(buffer, 0, sizeof(buffer));
  }
  
  void write(const char* str)
  {
    int len = strlen(str);
    if(len < (PRINTF_BUF/2))
    {
      if(bufferLen+len < PRINTF_BUF)
      {
        strcat(buffer, str);
        bufferLen += len;
        return;
      }
    }
    
    flush();
    c->write(str, len);
        
    yield();
  }

  void flush()
  {
    if(bufferLen)
    {
      c->write_P(buffer, bufferLen);
      buffer[0] = 0;
      bufferLen = 0;
    }   
  }
};

void handleStatus() {
  
  WiFiClient client = server.client();
  
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Connection: close\r\n");
  client.print("\r\n");

  WebOutput out(&client);
  pm.DumpToJson(&out);
  out.flush();

  client.stop();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){

  Serial.begin(9600); //connect to PowerMax

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/status", handleStatus);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);
  server.begin();

#ifdef PM_ENABLE_OTA_UPDATES
  ArduinoOTA.begin();
#endif

#ifdef PM_ENABLE_TELNET_ACCESS
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  pm.Init();
}

#ifdef PM_ENABLE_TELNET_ACCESS
void handleNewTelnetClients()
{
  if(telnetServer.hasClient())
  {
    if(telnetClient.connected())
    {
      //no free/disconnected spot so reject
      WiFiClient newClient = telnetServer.available();
      newClient.stop();
    }
    else
    {
      telnetClient = telnetServer.available();
      LOG("Connected to %s, type '?' for help.\r\n", WIFI_SSID); 
    }
  }
}

void handleTelnetRequests(PowerMaxAlarm* pm) {
  char c; 
  if ( telnetClient.connected() && telnetClient.available() )  {
    c = telnetClient.read();

#ifdef PM_ALLOW_CONTROL
    if ( c == 'h' ) {
      DEBUG(LOG_NOTICE,"Arming home");
      pm->sendCommand(Pmax_ARMHOME);
    }
    else if ( c == 'd' ) {
      DEBUG(LOG_NOTICE,"Disarm");
      pm->sendCommand(Pmax_DISARM);
    }  
    else if ( c == 'a' ) {
      DEBUG(LOG_NOTICE,"Arming away");
      pm->sendCommand(Pmax_ARMAWAY);
    }
#endif
 
    if ( c == 'g' ) {
      DEBUG(LOG_NOTICE,"Get Event log");
      pm->sendCommand(Pmax_GETEVENTLOG);
    }
    else if ( c == 't' ) {
      DEBUG(LOG_NOTICE,"try re-enroll");
      pm->sendCommand(Pmax_REENROLL);
    }
    else if ( c == 'v' ) {
      DEBUG(LOG_NOTICE,"getting versions string");
      pm->sendCommand(Pmax_GETVERSION);
    }
    else if ( c == 'r' ) {
      DEBUG(LOG_NOTICE,"Request Status Update");
      pm->sendCommand(Pmax_REQSTATUS);
    }
    else if ( c == 'j' ) {
        ConsoleOutput out;
        pm->DumpToJson(&out);
    }
    else if ( c == 'c' ) {
        DEBUG(LOG_NOTICE,"Exiting...");
        telnetClient.stop();
    }    
    else if ( c == 'p' ) {
      telnetDbgLevel = LOG_DEBUG;
      DEBUG(LOG_NOTICE,"Debug Logs enabled type 'P' (capital) to disable");
    }    
    else if ( c == 'P' ) {
      telnetDbgLevel = LOG_NO_FILTER;
      DEBUG(LOG_NO_FILTER,"Debug Logs disabled");
    }       
    else if ( c == '?' )
    {
        DEBUG(LOG_NO_FILTER,"Allowed commands:");
        DEBUG(LOG_NO_FILTER,"\t c - exit");
        DEBUG(LOG_NO_FILTER,"\t p - output debug messages");
        DEBUG(LOG_NO_FILTER,"\t P - stop outputing debug messages");
#ifdef PM_ALLOW_CONTROL        
        DEBUG(LOG_NO_FILTER,"\t h - Arm Home");
        DEBUG(LOG_NO_FILTER,"\t d - Disarm");
        DEBUG(LOG_NO_FILTER,"\t a - Arm Away");
#endif        
        DEBUG(LOG_NO_FILTER,"\t g - Get Event Log");
        DEBUG(LOG_NO_FILTER,"\t t - Re-Enroll");
        DEBUG(LOG_NO_FILTER,"\t v - Get Version String");
        DEBUG(LOG_NO_FILTER,"\t r - Request Status Update");
        DEBUG(LOG_NO_FILTER,"\t j - Dump Application Status to JSON");  
    }
  }
}
#endif

#ifdef PM_ENABLE_LAN_BROADCAST
void broadcastPacketOnLan(const PlinkBuffer* commandBuffer, bool packetOk)
{
  udp.beginPacketMulticast(PM_LAN_BROADCAST_IP, PM_LAN_BROADCAST_PORT, WiFi.localIP());
  udp.write("{ data: [");
  for(int ix=0; ix<commandBuffer->size; ix++)
  {
    char szDigit[20];
    //sprintf(szDigit, "%d", commandBuffer->buffer[ix]);
    sprintf(szDigit, "%02x", commandBuffer->buffer[ix]); //IZIZTODO: temp

    if(ix+1 != commandBuffer->size)
    {
      strcat(szDigit, ", ");
    }

    udp.write(szDigit);
  }
  
  udp.write("], ok: ");
  if(packetOk)
  {
    udp.write("true");
  }
  else
  {
    udp.write("false");
  }

  char szTmp[50];
  sprintf(szTmp, ", seq: %u }", packetCnt++);
  udp.write(szTmp);
  
  udp.endPacket();
}
#endif

bool serialHandler(PowerMaxAlarm* pm) {

  bool packetHandled = false;
  
  PlinkBuffer commandBuffer ;
  memset(&commandBuffer, 0, sizeof(commandBuffer));
  
  char oneByte = 0;  
  while (  (os_serialPortRead(&oneByte, 1) == 1)  ) 
  {     
    if (commandBuffer.size<(MAX_BUFFER_SIZE-1))
    {
      *(commandBuffer.size+commandBuffer.buffer) = oneByte;
      commandBuffer.size++;
    
      if(oneByte == 0x0A) //postamble received, let's see if we have full message
      {
        if(PowerMaxAlarm::isBufferOK(&commandBuffer))
        {
          DEBUG(LOG_INFO,"--- new packet %d ----", millis());

#ifdef PM_ENABLE_LAN_BROADCAST
          broadcastPacketOnLan(&commandBuffer, true);
#endif

          packetHandled = true;
          pm->handlePacket(&commandBuffer);
          commandBuffer.size = 0;
          break;
        }
      }
    }
    else
    {
      DEBUG(LOG_WARNING,"Packet too big detected");
    }
  }

  if(commandBuffer.size > 0)
  {
#ifdef PM_ENABLE_LAN_BROADCAST
    broadcastPacketOnLan(&commandBuffer, false);
#endif
    
    packetHandled = true;
    //this will be an invalid packet:
    DEBUG(LOG_WARNING,"Passing invalid packet to packetManager");
    pm->handlePacket(&commandBuffer);
  }

  return packetHandled;
}

void loop(void){
#ifdef PM_ENABLE_OTA_UPDATES
  ArduinoOTA.handle();
#endif

  server.handleClient();

  if(serialHandler(&pm) == false)
  {
    if(pm.getEnrolledZoneCnt() == 0)
    {
      //pm.Init() should enroll, but sometimes this fails, this is another attempt to do it if Init() fails. delay is added not to do it too frequently in case of problems
      delay(1000);
      DEBUG(LOG_WARNING,"No enroll information, trying to re-enroll");
      pm.sendCommand(Pmax_REENROLL);      
    }
  }
  else if(pm.getSecondsFromLastComm() > 120)
  {
      //this should not happen (no comms), to re-establish we re-enroll
      delay(1000);
      DEBUG(LOG_WARNING,"Communication failure - trying to re-enroll");
      pm.sendCommand(Pmax_REENROLL);
  }  

#ifdef PM_ENABLE_TELNET_ACCESS
  handleNewTelnetClients();
  handleTelnetRequests(&pm);
#endif  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This file contains OS specific implementation for ESP8266 used by PowerMax library
//If you build for other platrorms (like Linux or Windows, don't include this file, but provide your own)

int log_console_setlogmask(int mask)
{
  int oldmask = telnetDbgLevel;
  if(mask == 0)
    return oldmask; /* POSIX definition for 0 mask */
  telnetDbgLevel = mask;
  return oldmask;
} 

void os_debugLog(int priority, bool raw, const char *function, int line, const char *format, ...)
{
  if(priority <= telnetDbgLevel)
  {
    char buf[PRINTF_BUF];
    va_list ap;
    
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    
  #ifdef PM_ENABLE_TELNET_ACCESS
    if(telnetClient.connected())
    { 
      telnetClient.write_P(buf, strlen(buf));
      if(raw == false)
      {
        telnetClient.write_P("\r\n", 2);
      }    
    }
  #endif
    
    va_end(ap);
  
    yield();
    }
}

void os_usleep(int microseconds)
{
    delay(microseconds / 1000);
}

unsigned long os_getCurrentTimeSec()
{
  static unsigned int wrapCnt = 0;
  static unsigned long lastVal = 0;
  unsigned long currentVal = millis();

  if(currentVal < lastVal)
  {
    wrapCnt++;
  }

  unsigned long seconds = currentVal/1000;
  
  //millis will wrap each 50 days, as we are interested only in seconds, let's keep the wrap counter
  return (wrapCnt*4294967) + seconds;
}

int os_serialPortRead(void* readBuff, int bytesToRead)
{
    int dwTotalRead = 0;
    while(bytesToRead > 0)
    {
        if(Serial.available() == false)
        {
          delay(50);
          if(Serial.available() == false)
          {
            break;
          }
        }

        *((char*)readBuff) = Serial.read();
        dwTotalRead ++;
        readBuff = ((char*)readBuff) + 1;
        bytesToRead--;
    }

    return dwTotalRead;
}

int os_serialPortWrite(const void* dataToWrite, int bytesToWrite)
{
    Serial.write((const uint8_t*)dataToWrite, bytesToWrite);
    return bytesToWrite;
}

bool os_serialPortClose()
{
    return true;
}

bool os_serialPortInit(const char* portName) {
    return true;
} 

void os_strncat_s(char* dst, int dst_size, const char* src)
{
    strncat(dst, src, dst_size);
}

int os_cfg_getUserCode()
{
  return ALARM_USER_PIN;
}

int os_cfg_getZoneCnt()
{
    return sizeof(cfg_zones)/sizeof(cfg_zones[0]);
}

const char* os_cfg_getZoneName(int idx)
{
  if(idx >= os_cfg_getZoneCnt())
  {
    return "??";
  }

    return cfg_zones[idx];
}

int os_cfg_getPacketTimeout()
{
    return PACKET_TIMEOUT_DEFINED;
}
//////End of OS specific part of PM library/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
