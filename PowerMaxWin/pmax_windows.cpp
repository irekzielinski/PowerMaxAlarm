#include "pmax.h"
#include <string.h>
#include <windows.h>
#include <time.h> /* POSIX terminal control definitions */
#include <stdio.h>

//This file contains OS specific implementation for Windows
//If you build for other platrorms (like Linux, don't include this file, but provide your own)

#ifdef USE_SERIAL
static HANDLE g_hCommPort = INVALID_HANDLE_VALUE;
#endif

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

#ifdef USE_SERIAL
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
#else
//CONNECT TO REMOTE HOST (CLIENT APPLICATION)
//Include the needed header files.
//Don't forget to link libws2_32.a to your program as well
#include <winsock.h>
SOCKET s = 0; //Socket handle

HANDLE hLogFile = INVALID_HANDLE_VALUE;

bool readUntilNewLine(char* buffer, int buffSize)
{
    while( buffSize > 0 ){
        int read = recv(s, buffer, 1, 0);
        if ( read == 1 )
        {
            if(buffer[0] == '\n')
            {
                return true;
            }

            buffer++;
            buffSize--;
        }
        else
        {
            return false;
        }
    } 

    return false;
}

void LogBuffer(const char* prefix, const unsigned char* buffer, int bufferLen)
{
    SYSTEMTIME tm;
    GetLocalTime(&tm);

    char szTmp[60] = "";
    sprintf_s(szTmp, 60, "\r\n%s [%02d:%02d:%02d]", prefix, tm.wHour, tm.wMinute, tm.wSecond);
    DWORD dwWritten;
    WriteFile(hLogFile, szTmp, strlen(szTmp), &dwWritten, NULL); 
    for(int ix=0; ix<bufferLen; ix++)
    {
        if(ix > 0)
        {
            sprintf_s(szTmp, 60, ",0x%02X", (unsigned char)buffer[ix]);
        }
        else
        {
            sprintf_s(szTmp, 60, "0x%02X", (unsigned char)buffer[ix]);
        }
        WriteFile(hLogFile, szTmp, strlen(szTmp), &dwWritten, NULL); 
    }

    if(bufferLen > 1)
    {
        char szId[100] = "";
        if(buffer[1] == 0x02)
        {
            strcpy_s(szId, sizeof(szId), " [ACK]");
        }
        else if(buffer[1] == 0x08)
        {
            strcpy_s(szId, sizeof(szId), " [ACCESS DENIED]");
        }
        else if(buffer[1] == 0x06)
        {
            strcpy_s(szId, sizeof(szId), " [TIME OUT]");
        }
        else if(buffer[1] == 0x0B)
        {
            strcpy_s(szId, sizeof(szId), " [STOP]");
        }
        else if(buffer[1] == 0xAB && buffer[2] == 0x0A && buffer[4] == 0x01)
        {
            strcpy_s(szId, sizeof(szId), " [INIT]");
        }
        else if(buffer[1] == 0xAB && buffer[2] == 0x0A && buffer[4] == 0x00)
        {
            strcpy_s(szId, sizeof(szId), " [ENROL REQ]");
        }
        else if(buffer[1] == 0xAB && buffer[2] == 0x03)
        {
            strcpy_s(szId, sizeof(szId), " [PING]");
        }
        else if(buffer[1] == 0x3E && buffer[4] == 0x20)
        {
            strcpy_s(szId, sizeof(szId), " [Pmax_DL_PANELFW]");
        }
        else if(buffer[1] == 0x0A)
        {
            strcpy_s(szId, sizeof(szId), " [Pmax_DL_GET]");
        }
        else if(buffer[1] == 0x3E && buffer[4] == 0x08)
        {
            strcpy_s(szId, sizeof(szId), " [Pmax_DL_SERIAL]");
        }
        else if(buffer[1] == 0x3E && buffer[4] == 0x00)
        {
            strcpy_s(szId, sizeof(szId), " [Pmax_DL_ZONESTR]");
        }
        else if(buffer[1] == 0xAB && buffer[2] == 0x06)
        {
            strcpy_s(szId, sizeof(szId), " [RESTORE]");
        }
        else if(buffer[1] == 0xA0)
        {
            strcpy_s(szId, sizeof(szId), " [EVENT LOG]");
        }
        else if(buffer[1] == 0xA1 && buffer[4] == 0x00)
        {
            strcpy_s(szId, sizeof(szId), " [DISARM]");
        }
        else if(buffer[1] == 0xA1 && buffer[4] == 0x04)
        {
            strcpy_s(szId, sizeof(szId), " [ARM HOME]");
        }
        else if(buffer[1] == 0xA1 && buffer[4] == 0x05)
        {
            strcpy_s(szId, sizeof(szId), " [ARM AWAY]");
        }
        else if(buffer[1] == 0xA5 && buffer[3] == 0x02)
        {
            strcpy_s(szId, sizeof(szId), " [Status Update Zone Battery]");
        }
        else if(buffer[1] == 0xA5 && buffer[3] == 0x03)
        {
            strcpy_s(szId, sizeof(szId), " [Status Update Zone tamper]");
        }
        else if(buffer[1] == 0xA5 && buffer[3] == 0x04)
        {
            strcpy_s(szId, sizeof(szId), " [Status Update Panel]");
        }
        else if(buffer[1] == 0xA5 && buffer[3] == 0x06)
        {
            strcpy_s(szId, sizeof(szId), " [Status Update Zone Bypassed]");
        }
        else if(buffer[1] == 0x24)
        {
            strcpy_s(szId, sizeof(szId), " [DLOAD START]");
        }
        else if(buffer[1] == 0x0F)
        {
            strcpy_s(szId, sizeof(szId), " [DLOAD EXIT]");
        }
        else if(buffer[1] == 0x3F)
        {
            strcpy_s(szId, sizeof(szId), " [DLOAD INFO(DATA)]");
        }

        if(strlen(szId))
        {
            DWORD dwWritten;
            WriteFile(hLogFile, szId, strlen(szId), &dwWritten, NULL); 
        }
    }
}

/*static unsigned char g_readCache[MAX_BUFFER_SIZE];
static int g_cacheSize = 0;

static int readFromCache(void* readBuff, int bytesToRead)
{
    int bytesToCopy = bytesToRead;
    if(bytesToCopy >= g_cacheSize)
    {
        bytesToCopy = g_cacheSize;
    }

    if(bytesToCopy == 0)
    {
        return 0;
    }

    memcpy(readBuff, g_readCache, bytesToCopy);
    memmove(g_readCache, g_readCache+bytesToCopy, g_cacheSize-bytesToCopy);
    g_cacheSize -= bytesToCopy;

    return bytesToCopy;
}

int os_serialPortRead(void* readBuff, int bytesToRead)
{
    if(bytesToRead == 0)
    {
        return 0;
    }

    int totalRead = readFromCache(readBuff, bytesToRead);
    if(totalRead > 0)
    {
        return totalRead;
    }

    g_cacheSize = recv(s, (char*)g_readCache, sizeof(g_readCache), 0);
    if(g_cacheSize < 0)
    {
        g_cacheSize = 0;
    }
    return readFromCache(readBuff, bytesToRead);
}*/

int os_serialPortRead(void* readBuff, int bytesToRead)
{
    int totalRead = 0;
    while( bytesToRead > 0 ){
        int read = recv(s, (char*)readBuff, bytesToRead, 0);
        if ( read > 0 )
        {
            totalRead += read;
            bytesToRead -= read;
            readBuff = ((char*)readBuff) + read;
        }
        else
        {
            break;
        }
    } 

    return totalRead;
}

int os_serialPortWrite(const void* dataToWrite, int bytesToWrite)
{
   LogBuffer("PC", (const unsigned char*)dataToWrite, bytesToWrite);

   int totalSent = 0;
    while( bytesToWrite > 0 ){
        int written = send(s, (char*)dataToWrite, bytesToWrite, 0);
        if ( written > 0 )
        {
            totalSent += written;
            bytesToWrite -= written;
            dataToWrite = ((char*)dataToWrite) + written;
        }
        else
        {
            break;
        }
    } 

    return totalSent;
}

bool os_serialPortClose()
{
    //Close the socket if it exists
    if (s)
        closesocket(s);

    WSACleanup(); //Clean up Winsock
    return true;
}

bool os_serialPortInit(const char* portName) {
 
    hLogFile = CreateFile(L"comms.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    SetFilePointer(hLogFile, 0, 0, FILE_END);

    //IZIZTODO:
    int PortNo = 23;
    const char* IPAddress = "192.168.0.119";

    WSADATA wsadata;
    int error = WSAStartup(MAKEWORD(2,2), &wsadata);

    //Did something happen?
    if (error)
        return false;

    //Did we get the right Winsock version?
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup(); //Clean up Winsock
        return false;
    }

    //Fill out the information needed to initialize a socket…
    SOCKADDR_IN target; //Socket address information

    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons (PortNo); //Port to connect on
    target.sin_addr.s_addr = inet_addr (IPAddress); //Target IP

    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (s == INVALID_SOCKET)
    {
        return false; //Couldn't create the socket
    }  

    //Try connecting...
    if (connect(s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
    {
        return false; //Couldn't connect
    }

    int iTimeout = 500;
    int iResult = setsockopt( s,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        (const char *)&iTimeout,
                        sizeof(iTimeout) );

        iResult = setsockopt( s,
                        SOL_SOCKET,
                        SO_SNDTIMEO,
                        (const char *)&iTimeout,
                        sizeof(iTimeout) );

    // Receive until the peer closes the connection
    char szBuffer[256] = "";
    if(readUntilNewLine(szBuffer, sizeof(szBuffer)) == false)
    {
        return false;
    }

    //enter direct comm mode with PowerMax
    char req = 'D';
    if(send(s, &req, 1, 0) != 1)
    {
        return false;
    }

    //read confirmation:
    if(readUntilNewLine(szBuffer, sizeof(szBuffer)) == false)
    {
        return false;
    }

    return true; //Success, direct pipe is established
} 


#endif


void os_strncat_s(char* dst, int dst_size, const char* src)
{
    strncat_s(dst, dst_size, src, _TRUNCATE);
}

////////////////////////

int os_cfg_getPacketTimeout()
{
    return PACKET_TIMEOUT_DEFINED;
}

/////////////////////////