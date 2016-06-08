#include "pmax.h"
#include <string.h>
#include <windows.h>
#include <time.h> /* POSIX terminal control definitions */

//This file contains OS specific implementation for Windows
//If you build for other platrorms (like Linux, don't include this file, but provide your own)

static HANDLE g_hCommPort = INVALID_HANDLE_VALUE;

/* Private storage for the current mask */
static int log_consolemask = 0xFFFF; 

int log_console_setlogmask(int mask)
{
  int oldmask = log_consolemask;
  if(mask == 0)
    return oldmask; /* POSIX definition for 0 mask */
  log_consolemask = mask;
  return oldmask;
} 

void os_debugLog(int priority, bool raw, const char *function, int line, const char *format, ...)
{
    char buf[1024]; 

    va_list ap;
    va_start(ap, format);
    _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format, ap);
    va_end(ap);

	if(priority <= log_consolemask)
	{
        if(raw == false)
        {
		    printf("[%d][%s @ %d] %s\n", priority, function, line, buf);
        }
        else
        {
            printf("%s", buf);
        }

		fflush(stdout); 
	}
}

void os_usleep(int microseconds)
{
    Sleep(microseconds / 1000);
}

unsigned long os_getCurrentTimeSec()
{
    return (unsigned long) time(NULL);
}

int os_serialPortRead(void* readBuff, int bytesToRead)
{
    if(g_hCommPort == INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    DWORD dwTotalRead = 0;
    while(bytesToRead > 0)
    {
        DWORD dwRead = 0;
        if(ReadFile(g_hCommPort, readBuff, bytesToRead, &dwRead, NULL) == FALSE || dwRead == 0)
        {
            break;
        }

        dwTotalRead += dwRead;
        readBuff = ((char*)readBuff) + dwRead;
        bytesToRead -= dwRead;
    }

    return dwTotalRead;
}

int os_serialPortWrite(const void* dataToWrite, int bytesToWrite)
{
    if(g_hCommPort == INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    DWORD dwTotalWritten = 0;
    while(bytesToWrite > 0)
    {
        DWORD dwWritten = 0;
        if(WriteFile(g_hCommPort, dataToWrite, bytesToWrite, &dwWritten, NULL) == FALSE || dwWritten == 0)
        {
            break;
        }

        dwTotalWritten += dwWritten;
        dataToWrite = ((char*)dataToWrite) + dwWritten;
        bytesToWrite -= dwWritten;
    }

    return dwTotalWritten;
}

bool os_serialPortClose()
{
    if(g_hCommPort == INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    CloseHandle(g_hCommPort);
    g_hCommPort = INVALID_HANDLE_VALUE;

    return true;
}

bool os_serialPortInit(const char* portName) {
    if(g_hCommPort != INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Com port is already opened.");
        return false;
    }

    g_hCommPort = ::CreateFileA(portName,
        GENERIC_READ|GENERIC_WRITE,//access ( read and write)
        0,                         //(share) 0:cannot share the COM port                        
        0,                         //security  (None)                
        OPEN_EXISTING,             // creation : open_existing
        0,                         // we dont want overlapped operation
        0                          // no templates file for COM port...
        );

    if(g_hCommPort == INVALID_HANDLE_VALUE)
    {
        DEBUG(LOG_ERR, "Failed to open com port, Reason: %d.", GetLastError());
        return false;
    }

    DCB config = {0};
    config.DCBlength = sizeof(config);


    if((GetCommState(g_hCommPort, &config) == 0))
    {
        DEBUG(LOG_ERR, "Get configuration port has a problem.");
        return false;
    }

    config.BaudRate = CBR_9600;
    config.StopBits = ONESTOPBIT;
    config.Parity   = (BYTE)PARITY_NONE; 
    config.ByteSize = DATABITS_8;
    config.fDtrControl = 0;
    config.fRtsControl = 0;

    if (!SetCommState(g_hCommPort, &config))
    {
        DEBUG(LOG_ERR,"Failed to Set Comm State Reason: %d",GetLastError());
        return false;
    }

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 200;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = 500;
    if (!SetCommTimeouts(g_hCommPort, &timeouts))
    {
        DEBUG(LOG_ERR,"Failed to SetTimeouts: %d",GetLastError());
        return false;
    }

    DEBUG(LOG_INFO, "Current Settings\n Baud Rate %d\n Parity %d\n Byte Size %d\n Stop Bits %d", config.BaudRate, config.Parity, config.ByteSize, config.StopBits);
    return true;
} 

void os_strncat_s(char* dst, int dst_size, const char* src)
{
    strncat_s(dst, dst_size, src, _TRUNCATE);
}

////////////////////////
int os_cfg_getUserCode()
{
	//if your pin is 1234, you need to return 0x1234 (this is strange, as 0x makes it hex, but the only way it works)
    return 0x1234; //IZIZTODO
}

const char * const cfg_zones[] = {  "system",  "front door", "hall", "living room",  "kitchen", 
                                    "study",  "upstairs", "conservatory", "garage", "back door", "garage2"};

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

/////////////////////////