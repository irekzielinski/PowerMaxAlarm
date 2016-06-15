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
//#define PM_ENABLE_LAN_BROADCAST

//Those settings control where to send lan broadcast, clients need to use the same value. Note IP specified here has nothing to do with ESP IP, it's only a destination for multicast
//Read more about milticast here: http://en.wikipedia.org/wiki/IP_multicast
IPAddress PM_LAN_BROADCAST_IP(224, 192, 32, 12);
#define   PM_LAN_BROADCAST_PORT  23127

//Specify your WIFI settings:
#define WIFI_SSID "IreksNetUnit8"
#define WIFI_PASS "??"

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

  char szTmp[PRINTF_BUF*2];
  sprintf(szTmp, "<html>Hello from esp8266: Uptime: %02d:%02d:%02d.%02d, free heap: %u<br><a href='status'>Alarm Status</a></html>", (int)days, (int)hours, (int)minutes, (int)val, ESP.getFreeHeap());  
  server.send(200, "text/html", szTmp);
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
  pm.dumpToJson(&out);
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

  pm.init();
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

void runDirectRelayLoop()
{
  while(telnetClient.connected())
  { 
    bool wait = true;
    
    //we want to read/write in bulk as it's much faster than read one byte -> write one byte (this is known to create problems with PowerMax)
    unsigned char buffer[256];
    int readCnt = 0;
    while(readCnt < 256)
    {
      if(Serial.available())
      {
        buffer[readCnt] = Serial.read();
        readCnt++;
        wait = false;
      }
      else
      {
        break;
      }
    }
    
    if(readCnt > 0)
    {
      telnetClient.write_P( (char*)buffer, readCnt );
    }

    if(telnetClient.available())
    {
      Serial.write( telnetClient.read() );
      wait = false;
    }

    if(wait)
    {
      delay(10);
    }
  } 
}

void handleTelnetRequests(PowerMaxAlarm* pm) {
  char c; 
  if ( telnetClient.connected() && telnetClient.available() )  {
    c = telnetClient.read();

    if(pm->isConfigParsed() == false &&
       (c == 'h' || //arm home
        c == 'a' || //arm away
        c == 'd'))  //disarm
    {
        //DEBUG(LOG_NOTICE,"EPROM is not download yet (no PIN requred to perform this operation)");
        return;
    }

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
    else if ( c == 'D' ) {
      DEBUG(LOG_NO_FILTER,"Direct relay enabled, disconnect to stop...");
      runDirectRelayLoop();
    }
#endif
 
    if ( c == 'g' ) {
      DEBUG(LOG_NOTICE,"Get Event log");
      pm->sendCommand(Pmax_GETEVENTLOG);
    }
    else if ( c == 't' ) {
      DEBUG(LOG_NOTICE,"Restore comms");
      pm->sendCommand(Pmax_RESTORE);
    }
    else if ( c == 'v' ) {
      DEBUG(LOG_NOTICE,"Exit Download mode");
      pm->sendCommand(Pmax_DL_EXIT);
    }
    else if ( c == 'r' ) {
      DEBUG(LOG_NOTICE,"Request Status Update");
      pm->sendCommand(Pmax_REQSTATUS);
    }
    else if ( c == 'j' ) {
        ConsoleOutput out;
        pm->dumpToJson(&out);
    }
    else if ( c == 'c' ) {
        DEBUG(LOG_NOTICE,"Exiting...");
        telnetClient.stop();
    }    
    else if( c == 'C' ) {
      DEBUG(LOG_NOTICE,"Reseting...");
      ESP.reset();
    }    
    else if ( c == 'p' ) {
      telnetDbgLevel = LOG_DEBUG;
      DEBUG(LOG_NOTICE,"Debug Logs enabled type 'P' (capital) to disable");
    }    
    else if ( c == 'P' ) {
      telnetDbgLevel = LOG_NO_FILTER;
      DEBUG(LOG_NO_FILTER,"Debug Logs disabled");
    }       
    else if ( c == 'H' ) {
      DEBUG(LOG_NO_FILTER,"Free Heap: %u", ESP.getFreeHeap());
    }  
    else if ( c == '?' )
    {
        DEBUG(LOG_NO_FILTER,"Allowed commands:");
        DEBUG(LOG_NO_FILTER,"\t c - exit");
        DEBUG(LOG_NO_FILTER,"\t C - reset device");        
        DEBUG(LOG_NO_FILTER,"\t p - output debug messages");
        DEBUG(LOG_NO_FILTER,"\t P - stop outputing debug messages");
#ifdef PM_ALLOW_CONTROL        
        DEBUG(LOG_NO_FILTER,"\t h - Arm Home");
        DEBUG(LOG_NO_FILTER,"\t d - Disarm");
        DEBUG(LOG_NO_FILTER,"\t a - Arm Away");
        DEBUG(LOG_NO_FILTER,"\t D - Direct mode (relay all bytes from client to PMC and back with no processing, close connection to exit");  
#endif        
        DEBUG(LOG_NO_FILTER,"\t g - Get Event Log");
        DEBUG(LOG_NO_FILTER,"\t t - Restore Comms");
        DEBUG(LOG_NO_FILTER,"\t v - Exit download mode");
        DEBUG(LOG_NO_FILTER,"\t r - Request Status Update");
        DEBUG(LOG_NO_FILTER,"\t j - Dump Application Status to JSON");  
        DEBUG(LOG_NO_FILTER,"\t H - Get free heap");  

        
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
  while (  (os_pmComPortRead(&oneByte, 1) == 1)  ) 
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

  static unsigned long lastMsg = 0;
  if(serialHandler(&pm) == true)
  {
    lastMsg = millis();
  }

  if(millis() - lastMsg > 300 || millis() < lastMsg)
  {
    pm.sendNextCommand();
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

int os_pmComPortRead(void* readBuff, int bytesToRead)
{
    int dwTotalRead = 0;
    while(bytesToRead > 0)
    {
        for(int ix=0; ix<10; ix++)
        {
          if(Serial.available())
          {
            break;
          }
          delay(5);
        }
        
        if(Serial.available() == false)
        {
            break;
        }

        *((char*)readBuff) = Serial.read();
        dwTotalRead ++;
        readBuff = ((char*)readBuff) + 1;
        bytesToRead--;
    }

    return dwTotalRead;
}

int os_pmComPortWrite(const void* dataToWrite, int bytesToWrite)
{
    Serial.write((const uint8_t*)dataToWrite, bytesToWrite);
    return bytesToWrite;
}

bool os_pmComPortClose()
{
    return true;
}

bool os_pmComPortInit(const char* portName) {
    return true;
} 

void os_strncat_s(char* dst, int dst_size, const char* src)
{
    strncat(dst, src, dst_size);
}

int os_cfg_getPacketTimeout()
{
    return PACKET_TIMEOUT_DEFINED;
}
//////End of OS specific part of PM library/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
