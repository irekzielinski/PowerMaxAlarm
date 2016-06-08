// PowerMax.cpp : Defines the entry point for the console application.
//

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <conio.h>
#include "pmax.h"


void serialHandler(PowerMaxAlarm* pm) {
    
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

					pm->handlePacket(&commandBuffer);
					commandBuffer.size = 0;
					break;
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
		pm->handlePacket(&commandBuffer);
	}
}
/////////////////////////////////////////

void KeyPressHandling(PowerMaxAlarm* pm) {
  char c; 
  if ( _kbhit() )  {
    c = getchar();
 
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
    else if ( c == '?' )
    {
        DEBUG(LOG_NO_FILTER,"Allowed commands:");
        DEBUG(LOG_NO_FILTER,"\t c - exit");
        DEBUG(LOG_NO_FILTER,"\t h - Arm Home");
        DEBUG(LOG_NO_FILTER,"\t d - Disarm");
        DEBUG(LOG_NO_FILTER,"\t a - Arm Away");
        DEBUG(LOG_NO_FILTER,"\t g - Get Event Log");
        DEBUG(LOG_NO_FILTER,"\t t - Re-Enroll");
        DEBUG(LOG_NO_FILTER,"\t v - Get Version String");
        DEBUG(LOG_NO_FILTER,"\t r - Request Status Update");
        DEBUG(LOG_NO_FILTER,"\t j - Dump Application Status to JSON");
    }
  }
}


int _tmain(int argc, _TCHAR* argv[])
{
    if(os_serialPortInit("COM3") == false)
    {
        return 1;
    }

    PowerMaxAlarm pm;
    pm.Init();

	log_console_setlogmask(LOG_INFO);

    while (true) {
        KeyPressHandling(&pm);
        serialHandler(&pm);
		os_usleep(os_cfg_getPacketTimeout());

        if(pm.getSecondsFromLastComm() > 120)
        {
            pm.sendCommand(Pmax_REENROLL);
        }
    }  

    os_serialPortClose();
    return 0;
}

