#pragma once
#include <stdio.h>

#define  MAX_BUFFER_SIZE 250
#define  PACKET_TIMEOUT_DEFINED 2000
#define  MAX_ZONE_COUNT 31

class PowerMaxAlarm;

enum PmaxCommand
{
    Pmax_ACK = 0,
    Pmax_GETEVENTLOG = 2,
    Pmax_DISARM = 3,
    Pmax_ARMHOME = 4,
    Pmax_ARMAWAY = 5,
    Pmax_ARMAWAY_INSTANT = 6,
    Pmax_REQSTATUS = 7,
    Pmax_ENROLLREPLY = 10,
    Pmax_REENROLL = 11,
    Pmax_GETVERSION = 12
};

enum ZoneEvent
{
    ZE_None,
    ZE_TamperAlarm,
    ZE_TamperRestore,
    ZE_Open,
    ZE_Closed,
    ZE_Violated,
    ZE_PanicAlarm,
    ZE_RFJamming,
    ZE_TamperOpen,
    ZE_CommunicationFailure,
    ZE_LineFailure,
    ZE_Fuse,
    ZE_NotActive,
    ZE_LowBattery,
    ZE_ACFailure,
    ZE_FireAlarm,
    ZE_Emergency,
    ZE_SirenTamper,
    ZE_SirenTamperRestore,
    ZE_SirenLowBattery,
    ZE_SirenACFail
};

enum SystemStatus
{
    SS_Disarm = 0x00,
    SS_Exit_Delay = 0x01,
    SS_Exit_Delay2 = 0x02,
    SS_Entry_Delay = 0x03,
    SS_Armed_Home = 0x04,
    SS_Armed_Away = 0x05,
    SS_User_Test = 0x06,
    SS_Downloading = 0x07,
    SS_Programming = 0x08,
    SS_Installer = 0x09,
    SS_Home_Bypass = 0x0A,
    SS_Away_Bypass = 0x0B,
    SS_Ready = 0x0C,
    SS_Not_Ready = 0x0D
};

//this abstract class is used by DumpToJson API
//it allows to redirect JSON to file, console, www output
class IOutput
{
public:
    virtual void write(const char* str) = 0;
    
    void writeQuotedStr(const char* str);
    void writeJsonTag(const char* name, bool value, bool addComma = true);
    void writeJsonTag(const char* name, int value, bool addComma = true);
    void writeJsonTag(const char* name, const char* value, bool addComma = true, bool quoteValue = true);
};

class ConsoleOutput : public IOutput
{
public:
    void write(const char* str);
};

struct PlinkBuffer {
    unsigned char buffer[MAX_BUFFER_SIZE];
    unsigned char size;
    const char* description;
    void (*action)(PowerMaxAlarm* pm, const struct PlinkBuffer *);
};

struct ZoneState {
    bool lowBattery; //battery needs replacing
    bool tamper;     //someone tampered with the device
    bool doorOpen;   //door is open (either intrusion or not redy to arm)
    bool bypased;    //user temporarly disabled this zone
    bool active;     //commication with one is OK
};

struct Zone {
    unsigned char id;    //this will be used to resolve name form configuration
    bool enrolled;       //PowerMax knows about this zone (it's configured)

    ZoneState stat;      //basic state of the zone

    ZoneEvent lastEvent; //last event recodred for this zone
    unsigned long lastEventTime;

    const char* getName() const;
    void DumpToJson(IOutput* outputStream);
};


class PowerMaxAlarm
{
    //Flags with[*] are one-shot notifications of last event
    //For example when user arms away - bit 6 will be set
    //When system will deliver 'last 10 sec' notification - bit 6 will be cleared

    //bit 0: Ready if set
    //bit 1: [?] Alert in Memory if set
    //bit 2: [?] Trouble if set
    //bit 3: [?] Bypass On if set
    //bit 4: [*] Last 10 seconds of entry or exit delay if set
    //bit 5: [*] Zone event if set 
    //bit 6: [*] Arm, disarm event 
    //bit 7: [?] Alarm event if set
    unsigned char flags;

    //overall system status
    SystemStatus stat;

    //status of all zones (0 is not used, system)
    Zone zone[MAX_ZONE_COUNT];

    //used to detect when PowerMax stops talking to us, that will trigger re-establish comms message
    time_t lastIoTime;

public:
    void Init();
    
    bool sendCommand(PmaxCommand cmd);
    void handlePacket(PlinkBuffer  * commandBuffer);
   
    static bool isBufferOK(const PlinkBuffer* commandBuffer);
    static const char* getZoneName(unsigned char zoneId);

    unsigned int getEnrolledZoneCnt() const;
    unsigned long getSecondsFromLastComm() const;
    void DumpToJson(IOutput* outputStream);

private:
    static void addPin(unsigned char* bufferToSend);

    bool isFlagSet(unsigned char id) const { return (flags & 1<<id) != 0; }
    bool isAlarmEvent() const{  return isFlagSet(7); }
    bool isZoneEvent()  const{  return isFlagSet(5); }

    void Format_SystemStatus(char* tpbuff, int buffSize);

    static bool sendBuffer(const unsigned char * data, int bufferSize);
    static void sendBuffer(struct PlinkBuffer * Buff);

public:
    static void PmaxStatusUpdateZoneBat(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxStatusUpdatePanel(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxStatusUpdateZoneBypassed(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxStatusUpdateZoneTamper(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxStatusChange(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxStatusUpdate(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxEventLog(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxAccessDenied(PowerMaxAlarm* pm, const PlinkBuffer  * Bufff);
    static void PmaxAck(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxPing(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
    static void PmaxEnroll(PowerMaxAlarm* pm, const PlinkBuffer  * Buff);
};

/* This section specifies OS specific functions. */
/* Implementation for Windows (MSVS) is provided */
/* If you compile for other OS provide your own. */
#ifndef LOG_INFO
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
#endif
#define	LOG_NO_FILTER 0 /* message always outputed */
#define DEBUG(x,...) os_debugLog(x, false,__FUNCTION__,__LINE__,__VA_ARGS__);
#define DEBUG_RAW(x,...) os_debugLog(x, true,__FUNCTION__,__LINE__,__VA_ARGS__);
int log_console_setlogmask(int mask);

bool os_serialPortInit(const char* portName);
int  os_serialPortRead(void* writePos, int bytesToRead);
int  os_serialPortWrite(const void* dataToWrite, int bytesToWrite);
bool os_serialPortClose();
void os_usleep(int microseconds);

int os_cfg_getUserCode();
int os_cfg_getZoneCnt();
const char* os_cfg_getZoneName(int idx);
int os_cfg_getPacketTimeout();

void os_debugLog(int priority, bool raw, const char *function, int line,const char *format, ...);
void os_strncat_s(char* dst, int dst_size, const char* src);

unsigned long os_getCurrentTimeSec();