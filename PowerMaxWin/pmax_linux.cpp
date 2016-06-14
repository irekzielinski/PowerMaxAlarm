#include "pmax.h"
#include <string.h>
#include <time.h> /* POSIX terminal control definitions */

//This file contains OS specific implementation for Linux
//If you build for other platrorms (don't include this file, but provide your own)

/* Private storage for the current mask */
static int log_consolemask; 

//comport file descriptor:
int fd = 0;

int log_console_setlogmask(int mask)
{
  int oldmask = log_consolemask;
  if(mask == 0)
    return oldmask; /* POSIX definition for 0 mask */
  log_consolemask = mask;
  return oldmask;
} 

void log_console(int priority, const char *format, va_list arg)
{
//  va_list arglist;
  const char *loglevel;
//  va_start(arglist, format);

  /* Return on MASKed log priorities */
  if (!(LOG_MASK(priority) & log_consolemask))
    return;

  switch(priority)
  {
  case LOG_EMERG:
    loglevel = "EMERG: ";
    break;
  case LOG_ALERT:
    loglevel = "ALERT: ";
    break;
  case LOG_CRIT:
    loglevel = "CRIT: ";
    break;
  case LOG_ERR:
    loglevel = "ERR: ";
    break;
  case LOG_WARNING:
    loglevel = "WARNING: ";
    break;
  case LOG_NOTICE:
    loglevel = "NOTICE: ";
    break;
  case LOG_INFO:
    loglevel = "INFO: ";
    break;
  case LOG_DEBUG:
    loglevel = "DEBUG: ";
    break;
  default:
    loglevel = "UNKNOWN: ";
    break;
  }

  printf(" %s", loglevel);
  vprintf(format, arg);
  printf("\n");
  fflush(stdout);
  //va_end(arglist);
}

void os_debugLog(int priority, const char *function, int line, const char *format, ...)
{
  va_list arglist;
  va_start(arglist,format);
 
  char expanded_log[1024]="[";
  char expanded_line[6];
  time_t rawtime;
  struct tm * timeinfo;
  char buffertime [80];
 
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  strftime (buffertime,80,"%c",timeinfo);

  strcat(expanded_log,buffertime);
  strcat(expanded_log," ");
  strcat(expanded_log,function);
  strcat(expanded_log,":");
  sprintf(expanded_line,"%04d]",line);
  strcat(expanded_log,expanded_line);
  strcat(expanded_log,format);
 
 
  log_console(priority,expanded_log,arglist);
  va_end(arglist);
}

void os_usleep(int microseconds)
{
    usleep(microseconds);
}

unsigned long os_getCurrentTimeSec()
{
    return (unsigned long) time(NULL);
}

int os_pmComPortRead(void* readBuff, int bytesToRead)
{
    if(fd == 0)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    return read(fd, readBuff, bytesToRead);
}

int os_pmComPortWrite(const void* dataToWrite, int bytesToWrite)
{
    if(fd == 0)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    return write(fd, dataToWrite, bytesToWrite);
}

bool os_pmComPortClose()
{
    if(fd == 0)
    {
        DEBUG(LOG_ERR, "Com port is not opened.");
        return false;
    }

    close(fd);
    fd = 0;

    return true;
}

bool os_pmComPortInit(const char* portName) {
  struct termios options;
   
    fd = open(portName, O_RDWR | O_NOCTTY);
    if (fd != -1) break;
	
	if (fd == -1) {
    DEBUG(LOG_ERR,"open_port: Unable to open serial ports");
    printf("exiting: no serial port available");
    exit(EXIT_FAILURE);/*
    * Could not open the port.
	 */}
	DEBUG(LOG_INFO,"opening %s",portName);
	
  fcntl(fd, F_SETFL, 0);
			
  /*
  * Get the current options for the port...
	*/ 
   tcgetattr(fd, &options);
	/*
 	* Set the baud rates to 9600...
  */
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	
	/*
  * Enable the receiver and set local mode...
	*/ 
  options.c_cflag |= (CLOCAL | CREAD | CS8 );
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
//	options.c_cflag &= ~CRTSCTS ;
    	
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  options.c_iflag &= ~(ICRNL | IXON | IXOFF | IXANY);
    
  options.c_oflag &= ~OPOST;
    
  /*
	* Set the new options for the port...
	*/
	tcsetattr(fd, TCSAFLUSH, &options);
  fcntl(fd, F_SETFL, FNDELAY);
} 

void os_strncat_s(char* dst, int dst_size, const char* src)
{
    strncat(dst, src, dst_size);
    dst[dst_size-1] = '\0';
}

////////////////////////

int os_cfg_getPacketTimeout()
{
    return PACKET_TIMEOUT_DEFINED;
}

/////////////////////////