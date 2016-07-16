// PowerMax.cpp : Defines the entry point for the console application.
//

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <iostream>

#include <windows.h>
#include <conio.h>
#include "pmax.h"

void LogBuffer(const char* prefix, const unsigned char* buffer, int bufferLen);

bool serialHandler(PowerMaxAlarm* pm) {
    
    PlinkBuffer commandBuffer ;
    memset(&commandBuffer, 0, sizeof(commandBuffer));

	char byte = 0;
    while (  (os_pmComPortRead(&byte, 1) == 1)  ) { 
        
        if (commandBuffer.size<(MAX_BUFFER_SIZE-1))
		{
			*(commandBuffer.size+commandBuffer.buffer) = byte;
            commandBuffer.size++;

			if(byte == 0x0A) //postamble received, let's see if we have full message
			{
                if(PowerMaxAlarm::isBufferOK(&commandBuffer))
				{
					SYSTEMTIME time;
					GetLocalTime(&time);
					DEBUG(LOG_INFO,"--- new packet %d:%02d:%02d ----", time.wHour, time.wMinute, time.wSecond);

                    LogBuffer("PM", (const unsigned char*)commandBuffer.buffer, commandBuffer.size);
					pm->handlePacket(&commandBuffer);
					commandBuffer.size = 0;
					return true;
				}
			}
		}
        else
			DEBUG(LOG_WARNING,"Packet too big detected");
    }

	if(commandBuffer.size > 0)
	{
		//this will be an invalid packet:
		DEBUG(LOG_WARNING,"Passing invalid packet to packetManager");
        LogBuffer("P?", (const unsigned char*)commandBuffer.buffer, commandBuffer.size);
		pm->handlePacket(&commandBuffer);
        return true;
	}

    return false;
}
/////////////////////////////////////////

void KeyPressHandling(PowerMaxAlarm* pm) {
  char c; 
  if ( _kbhit() )  {
    c = getchar();
 
    if(pm->isConfigParsed() == false &&
       (c == 'h' || //arm home
        c == 'a' || //arm away
        c == 'd'))  //disarm
    {
        DEBUG(LOG_NOTICE,"EPROM is not download yet (no PIN requred to perform this operation)");
        return;
    }

    if ( c == 'c' ) {
      printf("Exiting...");
      exit(1);
    }
    else if ( c == 'h' ) {
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
    else if ( c == 'g' ) {
      DEBUG(LOG_NOTICE,"Get Event log");
      pm->sendCommand(Pmax_GETEVENTLOG);
    }
    else if ( c == 't' ) {
      DEBUG(LOG_NOTICE,"Restore Comms");
      pm->sendCommand(Pmax_RESTORE);
    }
    else if ( c == 'v' ) {
      DEBUG(LOG_NOTICE,"Exit Download mode");
      pm->sendCommand(Pmax_DL_EXIT);
    }
    else if ( c == '!' ) {
      DEBUG(LOG_NOTICE,"Get zone signal strength");
      pm->sendCommand(Pmax_DL_START);
      pm->sendCommand(Pmax_DL_ZONESIGNAL);
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
    else if ( c == '?' )
    {
        DEBUG(LOG_NO_FILTER,"Allowed commands:");
        DEBUG(LOG_NO_FILTER,"\t c - exit");
        DEBUG(LOG_NO_FILTER,"\t h - Arm Home");
        DEBUG(LOG_NO_FILTER,"\t d - Disarm");
        DEBUG(LOG_NO_FILTER,"\t a - Arm Away");
        DEBUG(LOG_NO_FILTER,"\t g - Get Event Log");
        DEBUG(LOG_NO_FILTER,"\t t - Restore comms");
        DEBUG(LOG_NO_FILTER,"\t v - Exit download mode");
        DEBUG(LOG_NO_FILTER,"\t ! - Get Zone signal strength");
        DEBUG(LOG_NO_FILTER,"\t r - Request Status Update");
        DEBUG(LOG_NO_FILTER,"\t j - Dump Application Status to JSON");
    }
  }
}

void saveMapToFile(const char* name, MemoryMap* map)
{
    HANDLE hDump = CreateFileA(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    for(int ix=0; ix<MAX_PAGES_CNT; ix++)
    {
        unsigned char page[MAX_PAGE_SIZE] = {0};
        if(map->Exist(ix))
        {
            
            map->Read(ix,0,MAX_PAGE_SIZE, page);
        }

        DWORD dw;
        WriteFile(hDump, page, MAX_PAGE_SIZE,&dw, NULL);
    }
    CloseHandle(hDump);
}

void loadMapToFile(const char* name, MemoryMap* map)
{
    HANDLE hDump = CreateFileA(name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    for(int ix=0; ix<MAX_PAGES_CNT; ix++)
    {
        DWORD dw;
        unsigned char page[MAX_PAGE_SIZE] = {0};
        ReadFile(hDump, page, MAX_PAGE_SIZE, &dw, NULL);
        map->Write(ix,0,MAX_PAGE_SIZE, page);
    }
    CloseHandle(hDump);
}

extern bool g_useComPort;

void askUserForSettings(char* port, int portSize, char* localCom0comPort, int localCom0comPortSize)
{
    char tmp[10] = "";
    std::cout << "This application can work in three modes:\r\n";
    std::cout << "Press 1: to connect to alarm directly via COM port\r\n";
    std::cout << "Press 2: to connect to alarm via ESP8266 running PMAX (TCP/IP)\r\n";
    std::cout << "Press 3: to connect to alarm via ESP8266 running PMAX (TCP/IP)\r\n";
    std::cout << "         and re-direct all traffic to local com0com port. This option can\r\n";
    std::cout << "         be used to connect Visonic 'Remote Programmer' to alarm via wifi\r\n";
  
    while(strcmp(tmp, "1") != 0 && strcmp(tmp, "2") != 0 && strcmp(tmp, "3") != 0)
    {
        std::cout << "Press 1 for COM\r\nPress 2 or 3 for TCP/IP\r\n";
        std::cin.getline(tmp,10);
    }

    if(strcmp(tmp, "1") == 0)
    {
        g_useComPort = true;
        std::cout << "Specify COM port name to open, for example: COM3\r\n";
    }
    else if(strcmp(tmp, "2") == 0 || strcmp(tmp, "3") == 0)
    {
        g_useComPort = false;
        std::cout << "Specify IP or host name of ESP8266 (default: 192.168.0.119):\r\n";
    }

    std::cin.getline(port,portSize);

    if(strcmp(tmp, "3") == 0)
    {
        std::cout << "Specify local com0com port name where to re-direct all traffic from ESP8266 (default: COM6):\r\n";
        std::cin.getline(localCom0comPort, localCom0comPortSize);

        //just some default for debugging
        if(strlen(localCom0comPort) == 0)
        {
            strcpy(localCom0comPort, "COM6");
        }
    }

    //just some default for debugging
    if(g_useComPort == false && strlen(port) == 0)
    {
        strcpy(port, "192.168.0.119");
    }

    if(strcmp(tmp, "3") == 0)
    {
        std::cout << "Make sure that " << localCom0comPort << " is a com0com device and has a second connected virtual com port to it.\r\n";
        std::cout << "Start 'Remote Programmer' and connect it to the second com port.\r\n";
    }
    else
    {
        std::cout << "After connection is established and handshake complete press ? for list of commands.\r\n";
    }
}

//NOTE: PowerMaxAlarm class should contain ONLY functionality of Powerlink
//If you want to for example send an SMS on arm/disarm event, don't add it to PowerMaxAlarm
//Instead create a new class that inherits from PowerMaxAlarm, and override required function
class MyPowerMax : public PowerMaxAlarm
{
public:
    virtual void OnStatusChange(const PlinkBuffer  * Buff)
    {
        //call base class implementation first, this will send ACK back and upate internal state.
        PowerMaxAlarm::OnStatusChange(Buff);

        //now our customization:
        switch(Buff->buffer[4])
        {
        case 0x51: //"Arm Home" 
        case 0x53: //"Quick Arm Home"
            //do something...
            break;

        case 0x52: //"Arm Away"
        case 0x54: //"Quick Arm Away"
            //do something...
            break;

        case 0x55: //"Disarm"
            //do someting...
            break;
        }        
    }

    virtual void OnPanelDateTime(unsigned char year, unsigned char month, unsigned char day, unsigned char hour, unsigned char minutes, unsigned char seconds)
    {
        printf("Date/time of panel: %d/%02d/%02d %02d:%02d:%02d\r\n", year+2000, month, day, hour, minutes, seconds);
    }
};

BOOL WriteAllBytes(HANDLE hHandle, const unsigned char* buff, int bytesToWrite, DWORD* totalWritten)
{
    *totalWritten = 0;
    while(bytesToWrite > 0)
    {
        DWORD dwWritten = 0;
        if(WriteFile(hHandle, buff, bytesToWrite, &dwWritten, NULL) == FALSE)
        {
            return FALSE;
        }

        bytesToWrite -= dwWritten;
        buff += dwWritten;
        *totalWritten += dwWritten;

        Sleep(10);
    }

    return TRUE;
}

//This can be used to connect PowerMaster Remote Controler application (provided for free by Visonic) to the alarm via ESP8266
//You need to install Com0com driver first (it's available on the internet).
//Com0com creates a PAIR of connected COM ports (for example COM6 <-> COM7), select one (for example COM6) in this application.
//This will re-direct all traffic from ESP8266 (and alarm) to this port). 
//Then start Visonic software and select other port (COM7) as source. This should allow do download/updad configuration via WiFi.
bool RedirectPowerMaxToComPort(const char* destPort)
{
    HANDLE localComPort = ::CreateFileA(destPort,
        GENERIC_READ|GENERIC_WRITE,//access ( read and write)
        0,                         //(share) 0:cannot share the COM port                        
        0,                         //security  (None)                
        OPEN_EXISTING,             // creation : open_existing
        0,                         // we dont want overlapped operation
        0                          // no templates file for COM port...
        );

    if(localComPort == INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Failed to open com port, Reason: %d.", GetLastError());
        return false;
    }

    DCB config = {0};
    config.DCBlength = sizeof(config);
    if((GetCommState(localComPort, &config) == 0))
    {
        CloseHandle(localComPort);
        DEBUG(LOG_ERR, "Get configuration port has a problem.");
        return false;
    }

    config.BaudRate = CBR_9600;
    config.StopBits = ONESTOPBIT;
    config.Parity   = (BYTE)PARITY_NONE; 
    config.ByteSize = DATABITS_8;
    config.fDtrControl = 0;
    config.fRtsControl = 0;

    if (!SetCommState(localComPort, &config))
    {
        CloseHandle(localComPort);
        DEBUG(LOG_ERR,"Failed to Set Comm State Reason: %d",GetLastError());
        return false;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 10;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 50;
    if (!SetCommTimeouts(localComPort, &timeouts))
    {
        CloseHandle(localComPort);
        DEBUG(LOG_ERR,"Failed to SetTimeouts: %d",GetLastError());
        return false;
    }
    DEBUG(LOG_INFO, "Current Settings\n Baud Rate %d\n Parity %d\n Byte Size %d\n Stop Bits %d", config.BaudRate, config.Parity, config.ByteSize, config.StopBits);
    
    //destination com port is open, let's run redirect loop:
    while(true)
    {
        DWORD dwRead = 0;
        unsigned char buffer[256] = "";
        if(ReadFile(localComPort, buffer, sizeof(buffer), &dwRead, NULL) == FALSE)
        {
            break;
        }

        if(dwRead > 0)
        {
            os_pmComPortWrite(buffer, dwRead); //this will also call LogBuffer command, so all traffic will be saved to local file
        }

        int read = os_pmComPortRead(buffer, sizeof(buffer));
        if(read > 0)
        {
            DWORD dwWritten = 0;
            LogBuffer("PM", buffer, read);
            if(WriteAllBytes(localComPort, buffer, read, &dwWritten) == FALSE)
            {
                break;
            }

            if(read != dwWritten)
            {
                DEBUG(LOG_ERR,"Not all data writen: %d != %d",read, dwWritten);
            }
        }

        Sleep(0);
    }
    
    CloseHandle(localComPort);
    return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
    /*{
        PowerMaxAlarm pm;
        pm.IZIZTODO_testMap();
    }
    return 0;*/

    log_console_setlogmask(LOG_INFO);

    char szPort[512] = "";
    char localCom0comPort[50] = "";
    askUserForSettings(szPort, sizeof(szPort), localCom0comPort, sizeof(localCom0comPort));

    if(os_pmComPortInit(szPort) == false)
    {
        return 1;
    }

    if(strlen(localCom0comPort)) //app serves as link between 'Visonic Remote Programer' -> Com0Com -> 'This App' -> ESP8266 -> Alarm 
    {
        RedirectPowerMaxToComPort(localCom0comPort);
    }
    else //app emulates Power Link hardware ('This App (emulated powerlink) -> ESP8266 (or local COM port) -> Alarm)
    {
        MyPowerMax pm;
        pm.init();

        DWORD dwLastMsg = 0;
        while (true) {
            KeyPressHandling(&pm);
            
            if(serialHandler(&pm) == true)
            {
                dwLastMsg = GetTickCount();
            }
            
            if(GetTickCount() - dwLastMsg > 300)
            {
                pm.sendNextCommand();
            }


		    os_usleep(os_cfg_getPacketTimeout());

            /*if(pm.getSecondsFromLastComm() > 120)
            {
                pm.sendCommand(Pmax_RESTORE);
            }*/
        }  
    }

    os_pmComPortClose();
    return 0;
}

