// PowerMax.cpp : Defines the entry point for the console application.
//

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <conio.h>
#include "pmax.h"

void LogBuffer(const char* prefix, const unsigned char* buffer, int bufferLen);

bool serialHandler(PowerMaxAlarm* pm) {
    
    PlinkBuffer commandBuffer ;
    memset(&commandBuffer, 0, sizeof(commandBuffer));

	char byte = 0;
    while (  (os_serialPortRead(&byte, 1) == 1)  ) { 
        
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


int _tmain(int argc, _TCHAR* argv[])
{
    /*{
        PowerMaxAlarm pm;
        pm.IZIZTODO_testMap();
    }
    return 0;*/

    if(os_serialPortInit("COM3") == false)
    {
        return 1;
    }

    PowerMaxAlarm pm;
    pm.init();


	log_console_setlogmask(LOG_INFO);

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

    os_serialPortClose();
    return 0;
}

