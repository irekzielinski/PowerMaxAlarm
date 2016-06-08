#include "pmax.h"
#include <string.h>
#include <string>

#define IMPEMENT_GET_FUNCTION(tblName)\
const char* GetStr##tblName(int index)\
{\
    int nameCnt = sizeof(tblName)/sizeof(tblName[0]);\
    if(index < nameCnt)\
    {\
        return tblName[index];\
    }\
\
    return "??";\
}

//    {{0xAA,0x12,0x34,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x43},12 ,"Enable bypass"        ,NULL},
//    {{0xAA,0x12,0x34,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Disable bypass"       ,NULL},
bool PowerMaxAlarm::sendCommand(PmaxCommand cmd)
{
    switch(cmd)
    {
    case Pmax_ACK:
        {
            unsigned char buff[] = {0x02,0x43};
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_GETEVENTLOG:
        {
            unsigned char buff[] = {0xA0,0x00,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_DISARM:
        {
            unsigned char buff[] = {0xA1,0x00,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_ARMHOME:
        {
            unsigned char buff[] = {0xA1,0x00,0x00,0x04,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_ARMAWAY:
        {
            unsigned char buff[] = {0xA1,0x00,0x00,0x05,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_ARMAWAY_INSTANT:
        {
            unsigned char buff[] = {0xA1,0x00,0x00,0x14,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_REQSTATUS:
        {
            unsigned char buff[] = {0xA2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43};
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_ENROLLREPLY:
        {
            unsigned char buff[] = {0xAB,0x0A,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43};
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_REENROLL:
        {
            unsigned char buff[] = {0xAB,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43};
            return sendBuffer(buff, sizeof(buff));
        }

    case Pmax_GETVERSION:
        {
            unsigned char buff[] = {0x3E,0x00,0x04,0x36,0x00,0xB0,0x30,0x30,0x33,0x35,0x35};
            return sendBuffer(buff, sizeof(buff));
        }
        
    default:
        return false;
    }
}


//FF means: match anything
struct PlinkBuffer PmaxCommand[] =
{
    {{0x08,0x43                                                  },2  ,"Access denied"              ,&PowerMaxAlarm::PmaxAccessDenied},
    {{0xA0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Event Log"                  ,&PowerMaxAlarm::PmaxEventLog},
    {{0xA5,0xFF,0x02,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Battery" ,&PowerMaxAlarm::PmaxStatusUpdateZoneBat},
    {{0xA5,0xFF,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone tamper"  ,&PowerMaxAlarm::PmaxStatusUpdateZoneTamper},
    {{0xA5,0xFF,0x04,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Panel"        ,&PowerMaxAlarm::PmaxStatusUpdatePanel},
    {{0xA5,0xFF,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Bypassed",&PowerMaxAlarm::PmaxStatusUpdateZoneBypassed},
    {{0xA7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Panel status change"        ,&PowerMaxAlarm::PmaxStatusChange},
    {{0xAB,0x0A,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Enroll request"             ,&PowerMaxAlarm::PmaxEnroll},
    {{0x02,0x43                                                  },2  ,"Acknowledgement"            ,&PowerMaxAlarm::PmaxAck},
	{{0x06                                                       },1  ,"Ping"                       ,&PowerMaxAlarm::PmaxPing} //This might be PM Complete specific, happens every 20seconds
};

const char* PmaxSystemStatus[] = {
    "Disarmed"     ,
    "Exit Delay" ,
    "Exit Delay" ,
    "Entry Delay",
    "Armed Home" ,
    "Armed Away" ,
    "User Test"  ,
    "Downloading",
    "Programming",
    "Installer"  ,
    "Home Bypass",
    "Away Bypass",
    "Ready"      ,
    "Not Ready"
};

const char* SystemStateFlags[] = {
    "Ready",
    "Alert-in-Memory",
    "Trouble",
    "Bypass-On",
    "Last-10-sec-delay",
    "Zone-event",
    "Arm/disarm-event",
    "Alarm-event"
};

const char*  PmaxLogEvents[] = {            
    "None"                                      ,
    "Interior Alarm"                            ,
    "Perimeter Alarm"                           ,
    "Delay Alarm"                               ,
    "24h Silent Alarm"                          ,
    "24h Audible Alarm"                         ,
    "Tamper"                                    ,
    "Control Panel Tamper"                      ,
    "Tamper Alarm"                              ,
    "Tamper Alarm"                              ,
    "Communication Loss"                        ,
    "Panic From Keyfob"                         ,
    "Panic From Control Panel"                  ,
    "Duress"                                    ,
    "Confirm Alarm"                             ,
    "General Trouble"                           ,
    "General Trouble Restore"                   ,
    "Interior Restore"                          ,
    "Perimeter Restore"                         ,
    "Delay Restore"                             ,
    "24h Silent Restore"                        ,
    "24h Audible Restore"                       ,
    "Tamper Restore"                            ,
    "Control Panel Tamper Restore"              ,
    "Tamper Restore"                            ,
    "Tamper Restore"                            ,
    "Communication Restore"                     ,
    "Cancel Alarm"                              ,
    "General Restore"                           ,
    "Trouble Restore"                           ,
    "Not used"                                  ,
    "Recent Close"                              ,
    "Fire"                                      ,
    "Fire Restore"                              ,
    "No Active"                                 ,
    "Emergency"                                 ,
    "No used"                                   ,
    "Disarm Latchkey"                           ,
    "Panic Restore"                             ,
    "Supervision (Inactive)"                    ,
    "Supervision Restore (Active)"              ,
    "Low Battery"                               ,
    "Low Battery Restore"                       ,
    "AC Fail"                                   ,
    "AC Restore"                                ,
    "Control Panel Low Battery"                 ,
    "Control Panel Low Battery Restore"         ,
    "RF Jamming"                                ,
    "RF Jamming Restore"                        ,
    "Communications Failure"                    ,
    "Communications Restore"                    ,
    "Telephone Line Failure"                    ,
    "Telephone Line Restore"                    ,
    "Auto Test"                                 ,
    "Fuse Failure"                              ,
    "Fuse Restore"                              ,
    "Keyfob Low Battery"                        ,
    "Keyfob Low Battery Restore"                ,
    "Engineer Reset"                            ,
    "Battery Disconnect"                        ,
    "1-Way Keypad Low Battery"                  ,
    "1-Way Keypad Low Battery Restore"          ,
    "1-Way Keypad Inactive"                     ,
    "1-Way Keypad Restore Active"               ,
    "Low Battery"                               ,
    "Clean Me"                                  ,
    "Fire Trouble"                              ,
    "Low Battery"                               ,
    "Battery Restore"                           ,
    "AC Fail"                                   ,
    "AC Restore"                                ,
    "Supervision (Inactive)"                    ,
    "Supervision Restore (Active)"              ,
    "Gas Alert"                                 ,
    "Gas Alert Restore"                         ,
    "Gas Trouble"                               ,
    "Gas Trouble Restore"                       ,
    "Flood Alert"                               ,
    "Flood Alert Restore"                       ,
    "X-10 Trouble"                              ,
    "X-10 Trouble Restore"                      ,
    "Arm Home"                                  ,
    "Arm Away"                                  ,
    "Quick Arm Home"                            ,
    "Quick Arm Away"                            ,
    "Disarm"                                    ,
    "Fail To Auto-Arm"                          ,
    "Enter To Test Mode"                        ,
    "Exit From Test Mode"                       ,
    "Force Arm"                                 ,
    "Auto Arm"                                  ,
    "Instant Arm"                               ,
    "Bypass"                                    ,
    "Fail To Arm"                               ,
    "Door Open"                                 ,
    "Communication Established By Control Panel",
    "System Reset"                              ,
    "Installer Programming"                     ,
    "Wrong Password"                            ,
    "Not Sys Event"                             ,
    "Not Sys Event"                             ,
    "Extreme Hot Alert"                         ,
    "Extreme Hot Alert Restore"                 ,
    "Freeze Alert"                              ,
    "Freeze Alert Restore"                      ,
    "Human Cold Alert"                          ,
    "Human Cold Alert Restore"                  ,
    "Human Hot Alert"                           ,
    "Human Hot Alert Restore"                   ,
    "Temperature Sensor Trouble"                ,
    "Temperature Sensor Trouble Restore"        
};



const char*  PmaxZoneEventTypes[] = {
    "None"                 ,
    "Tamper Alarm"         ,
    "Tamper Restore"       ,
    "Open"                 ,
    "Closed"               ,
    "Violated (Motion)"    ,
    "Panic Alarm"          ,
    "RF Jamming"           ,
    "Tamper Open"          ,
    "Communication Failure",
    "Line Failure"         ,
    "Fuse"                 ,
    "Not Active"           ,
    "Low Battery"          ,
    "AC Failure"           ,
    "Fire Alarm"           ,
    "Emergency"            ,
    "Siren Tamper"         ,
    "Siren Tamper Restore" ,
    "Siren Low Battery"    ,
    "Siren AC Fail"
};

const char*  PmaxDefaultZoneNames[] = {
    "System"               ,
    "Zone 1"               ,
    "Zone 2"               ,
    "Zone 3"               ,
    "Zone 4"               ,
    "Zone 5"               ,
    "Zone 6"               ,
    "Zone 7"               ,
    "Zone 8"               ,
    "Zone 9"               ,
    "Zone 10"              ,
    "Zone 11"              ,
    "Zone 12"              ,
    "Zone 13"              ,
    "Zone 14"              ,
    "Zone 15"              ,
    "Zone 16"              ,
    "Zone 17"              ,
    "Zone 18"              ,
    "Zone 19"              ,
    "Zone 20"              ,
    "Zone 21"              ,
    "Zone 22"              ,
    "Zone 23"              ,
    "Zone 24"              ,
    "Zone 25"              ,
    "Zone 26"              ,
    "Zone 27"              ,
    "Zone 28"              ,
    "Zone 29"              ,
    "Zone 30"              ,
    "Keyfob1"              ,
    "Keyfob2"              ,
    "Keyfob3"              ,
    "Keyfob4"              ,
    "Keyfob5"              ,
    "Keyfob6"              ,
    "Keyfob7"              ,
    "Keyfob8"              ,
    "User1"                ,
    "User2"                ,
    "User3"                ,
    "User4"                ,
    "User5"                ,
    "User6"                ,
    "User7"                ,
    "User8"                ,
    "Wireless Commander1"  ,
    "Wireless Commander2"  ,
    "Wireless Commander3"  ,
    "Wireless Commander4"  ,
    "Wireless Commander5"  ,
    "Wireless Commander6"  ,
    "Wireless Commander7"  ,
    "Wireless Commander8"  ,
    "Wireless Siren1"      ,
    "Wireless Siren2"      ,
    "2Way Wireless Keypad1",
    "2Way Wireless Keypad2",
    "2Way Wireless Keypad3",
    "2Way Wireless Keypad4",
    "X10-1"                ,
    "X10-2"                ,
    "X10-3"                ,
    "X10-4"                ,
    "X10-5"                ,
    "X10-6"                ,
    "X10-7"                ,
    "X10-8"                ,
    "X10-9"                ,
    "X10-10"               ,
    "X10-11"               ,
    "X10-12"               ,
    "X10-13"               ,
    "X10-14"               ,
    "X10-15"               ,
    "PGM"                  ,
    "GSM"                  ,
    "Powerlink"            ,
    "Proxy Tag1"           ,
    "Proxy Tag2"           ,
    "Proxy Tag3"           ,
    "Proxy Tag4"           ,
    "Proxy Tag5"           ,
    "Proxy Tag6"           ,
    "Proxy Tag7"           ,
    "Proxy Tag8"
};

IMPEMENT_GET_FUNCTION(PmaxSystemStatus);
IMPEMENT_GET_FUNCTION(SystemStateFlags);
IMPEMENT_GET_FUNCTION(PmaxDefaultZoneNames);
IMPEMENT_GET_FUNCTION(PmaxZoneEventTypes);
IMPEMENT_GET_FUNCTION(PmaxLogEvents);

void PowerMaxAlarm::Init() {

    int count = os_cfg_getZoneCnt();
    printf("I have %d zone:\n", count);
    for (int i = 0; i < count; i++) { 

        DEBUG(LOG_NOTICE,"zone: %i, name: %s",i,os_cfg_getZoneName(i));
    }
    
    flags = 0;
    stat  = SS_Not_Ready;
    lastIoTime = 0;

    for (unsigned char i=1;i<MAX_ZONE_COUNT;i++) //zone 0 is not used (system)
    {
        zone[i].id             = i;
        zone[i].enrolled       = false;
        zone[i].stat.bypased   = false;
        zone[i].stat.doorOpen  = false;
        zone[i].stat.tamper    = false;
        zone[i].stat.active    = false;
        zone[i].stat.lowBattery= false;
        zone[i].lastEvent      = ZE_None;
        zone[i].lastEventTime  = 0;
    }

    PowerMaxAlarm::sendCommand(Pmax_REQSTATUS);

    //IZIZTODO: this seems to be not required on PowerMaxComplete, even more: when issued with PMC: it stops panel updates (A7 messages), to re-enable updates power and battery has to be removed
    os_usleep(20*os_cfg_getPacketTimeout());
    PowerMaxAlarm::sendCommand(Pmax_REENROLL);
}

unsigned int PowerMaxAlarm::getEnrolledZoneCnt() const
{
    unsigned int cnt = 0;
    for (unsigned char i=1;i<MAX_ZONE_COUNT;i++) //zone 0 is not used (system)
    {
        if(zone[i].enrolled)
        {
            cnt++;
        }
    }

    return cnt;
}

unsigned long PowerMaxAlarm::getSecondsFromLastComm() const
{ 
    return (unsigned long)(os_getCurrentTimeSec()-lastIoTime); 
}

void PowerMaxAlarm::addPin(unsigned char* bufferToSend)
{
    int usercode = os_cfg_getUserCode();
    bufferToSend[4]=usercode>>8;
    bufferToSend[5]=usercode & 0x00FF ;
}


void PowerMaxAlarm::PmaxEnroll(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ENROLLREPLY);
    DEBUG(LOG_INFO,"Enrolling.....");
}

void PowerMaxAlarm::PmaxAck(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{

}

void PowerMaxAlarm::PmaxPing(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
	pm->sendCommand(Pmax_ACK);   
    DEBUG(LOG_INFO,"Ping");
}

void PowerMaxAlarm::PmaxAccessDenied(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    DEBUG(LOG_INFO,"Access denied");
}

void PowerMaxAlarm::PmaxEventLog(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);   

    const unsigned char zoneId = Buff->buffer[9];
    const char* tpzone = pm->getZoneName(zoneId);

    char logline[MAX_BUFFER_SIZE] = "";
    sprintf(logline,"event number:%d/%d at %d:%d:%d %d/%d/%d %s:%s",
        Buff->buffer[2],Buff->buffer[1],  // event number amoung number
        Buff->buffer[5],Buff->buffer[4],Buff->buffer[3],  //hh:mm:ss
        Buff->buffer[6],Buff->buffer[7],2000+Buff->buffer[8], //day/month/year
        tpzone,       //zone
        GetStrPmaxLogEvents(Buff->buffer[10])
    );  
    DEBUG(LOG_NOTICE,"event log:%s",logline);
}

void PowerMaxAlarm::PmaxStatusUpdate(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);   
    DEBUG(LOG_INFO,"pmax status update")   ;
}

void PowerMaxAlarm::PmaxStatusChange(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);
    unsigned char zoneID = Buff->buffer[3];
    DEBUG(LOG_INFO,"PmaxStatusChange %s %s",pm->getZoneName(zoneID),GetStrPmaxLogEvents(Buff->buffer[4])); 
}

void PowerMaxAlarm::PmaxStatusUpdateZoneTamper(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);
    DEBUG(LOG_INFO,"Status Update : Zone active/tampered");

    const unsigned char * ZoneBuffer = Buff->buffer+3;
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset)
        { 
            DEBUG(LOG_INFO,"Zone %d is NOT active",i ); 
            pm->zone[i].stat.active=false;
        }
        else
        {
            pm->zone[i].stat.active=true;
        }
    }

    ZoneBuffer=Buff->buffer+7;  
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset)
        {
            DEBUG(LOG_INFO,"Zone %d is tampered",i );
            pm->zone[i].stat.tamper=true;
        }
        else
        {
            pm->zone[i].stat.tamper=false;    
        }      
    } 
}

void PowerMaxAlarm::PmaxStatusUpdateZoneBypassed(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{  
    pm->sendCommand(Pmax_ACK);
    DEBUG(LOG_INFO,"Status Update : Zone Enrolled/Bypassed");

    const unsigned char * ZoneBuffer = Buff->buffer+3;
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset)
        {
            DEBUG(LOG_INFO,"Zone %d is enrolled",i );
            pm->zone[i].enrolled = true;
        }
        else
        {
            pm->zone[i].enrolled = false;
        }
    }

    ZoneBuffer=Buff->buffer+7;  
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset) {
            DEBUG(LOG_INFO,"Zone %d is bypassed",i );
            pm->zone[i].stat.bypased=true;
        }
        else {
            pm->zone[i].stat.bypased=false;
        }       
    }
}

//0xA5,XX,0x04
void PowerMaxAlarm::PmaxStatusUpdatePanel(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)    
{
    pm->sendCommand(Pmax_ACK);

    pm->stat  = (SystemStatus)Buff->buffer[3];
    pm->flags = Buff->buffer[4];

    {
        char tpbuff[MAX_BUFFER_SIZE] = "";  //IZIZTODO: remove this, output directly to console
        sprintf(tpbuff,"System status: %s, Flags :", GetStrPmaxSystemStatus(pm->stat));    
        for (int i=0;i<8;i++) //loop trough all 8 bits
        {
            if (pm->flags & 1<<i) {
                os_strncat_s(tpbuff, MAX_BUFFER_SIZE, " ");
                os_strncat_s(tpbuff, MAX_BUFFER_SIZE, GetStrSystemStateFlags(i));
            }      
        }
        DEBUG(LOG_INFO,"%s", tpbuff);
    }

    // if system state flag says it is a zone event (bit 5 of system flag)  
    if (pm->isZoneEvent()) {
        const unsigned char zoneId = Buff->buffer[5];
        ZoneEvent eventType = (ZoneEvent)Buff->buffer[6];

        //this information is delivered also by periodic updates, but here we get instant update:
        if  ( zoneId<MAX_ZONE_COUNT )
        {
            DEBUG(LOG_INFO,"Zone-%d-event, (%s) %s", zoneId, pm->getZoneName(zoneId), GetStrPmaxZoneEventTypes(Buff->buffer[6]));
            pm->zone[zoneId].lastEvent = eventType;
            pm->zone[zoneId].lastEventTime = os_getCurrentTimeSec();

            switch(eventType)
            {
            case ZE_NotActive:
                pm->zone[zoneId].stat.active = false;
                break;

            case ZE_Open:
                pm->zone[zoneId].stat.doorOpen = true;
                break;

            case ZE_Closed:
                pm->zone[zoneId].stat.doorOpen = false;
                break;

            case ZE_LowBattery:
            case ZE_SirenLowBattery:
                pm->zone[zoneId].stat.lowBattery = true;
                break;

            case ZE_TamperAlarm:
            case ZE_TamperOpen:
            case ZE_SirenTamper:
                pm->zone[zoneId].stat.tamper = true;
                break;

            case ZE_TamperRestore:
            case ZE_SirenTamperRestore:
                pm->zone[zoneId].stat.tamper = false;
                break;

            default:
                break;
            }
        }
    }
}


void PowerMaxAlarm::PmaxStatusUpdateZoneBat(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);
    DEBUG(LOG_INFO,"Status Update : Zone state/Battery");

    const unsigned char * ZoneBuffer = Buff->buffer+3;
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset) {
            DEBUG(LOG_INFO,"Zone %d is open",i );
            pm->zone[i].stat.doorOpen=true;      
        }
        else {
            pm->zone[i].stat.doorOpen=false;
        }     
    }

    ZoneBuffer=Buff->buffer+7;  
    for (int i=1;i<MAX_ZONE_COUNT;i++)
    {
        int byte=(i-1)/8;
        int offset=(i-1)%8;
        if (ZoneBuffer[byte] & 1<<offset) {
            DEBUG(LOG_INFO,"Zone %d battery is low",i );
            pm->zone[i].stat.lowBattery=true;
        }
        else  {
            pm->zone[i].stat.lowBattery=false;
        }             
    }
} 


void logBuffer(int priority,struct PlinkBuffer * Buff)  {
    unsigned short i;
    char printBuffer[MAX_BUFFER_SIZE*3+3];
    char *bufptr;      /* Current char in buffer */
    bufptr=printBuffer;
    for (i=0;i<(Buff->size);i++) {
        sprintf(bufptr,"%02X ",Buff->buffer[i]);
        bufptr=bufptr+3;
    }  
    DEBUG(LOG_DEBUG,"BufferSize: %d" ,Buff->size);
    DEBUG(priority,"Buffer: %s", printBuffer);  
}

unsigned char calculChecksum(const unsigned char* data, int dataSize) {
    unsigned short checksum = 0xFFFF;
    for (int i=0;i<dataSize;i++)
        checksum=checksum - data[i];
    checksum=checksum%0xFF;
    DEBUG(LOG_DEBUG,"checksum: %04X",checksum);
    return (unsigned char) checksum;
} 

bool PowerMaxAlarm::sendBuffer(const unsigned char * data, int bufferSize)
{
    DEBUG(LOG_DEBUG,"Sending the following buffer to serial TTY");  
    //logBuffer(LOG_DEBUG,Buff); //IZIZTODO

    if(bufferSize >= MAX_BUFFER_SIZE-2) //should never happen, but this will prevent any buffer overflows to writeBuffer.buffer
    {
        DEBUG(LOG_ERR,"Too long buffer: %d", bufferSize);  
        return false;
    }

    PlinkBuffer writeBuffer;
    writeBuffer.buffer[0]=0x0D;
    for (int i=0;i<(bufferSize);i++)
    {
        writeBuffer.buffer[i+1]=data[i];
    }
    writeBuffer.buffer[bufferSize+1]=calculChecksum(data, bufferSize);
    writeBuffer.buffer[2+bufferSize]=0x0A;
    writeBuffer.size=bufferSize+3;

    const int bytesWritten = os_serialPortWrite(writeBuffer.buffer,bufferSize+3);
    if(bytesWritten == bufferSize+3)
    {
        DEBUG(LOG_DEBUG,"serial write OK"); 
        return true;
    }
    else
    {
        DEBUG(LOG_ERR,"serial write failed, bytes transfered: %d, expected: %d",bytesWritten, bufferSize+3); 	
        return false;
    }
}

void PowerMaxAlarm::sendBuffer(struct PlinkBuffer * Buff)  {
    sendBuffer(Buff->buffer, Buff->size);
}

bool deFormatBuffer(struct PlinkBuffer  * Buff) {
    int i;
    unsigned char checkedChecksum,checksum;
    checksum=Buff->buffer[Buff->size-2];

    for (i=0;i<Buff->size;i++)
        Buff->buffer[i]=Buff->buffer[i+1]; 

    Buff->size=Buff->size-3;
    checkedChecksum=calculChecksum(Buff->buffer, Buff->size);
    if (checksum==checkedChecksum)  {
        DEBUG(LOG_DEBUG,"checksum OK");
        return true;
    } 
    else  {
        DEBUG(LOG_ERR,"checksum NOK calculated:%04X in packet:%04X",checkedChecksum,checksum);
        return false;
    }  
}  

// compare two buffer, 0xff are used as jocker char
bool findCommand(struct PlinkBuffer  * Buff,struct PlinkBuffer  * BuffCommand)  {
    int i=0;
    if (Buff->size!=BuffCommand->size)  return false;
    for (i=0;i<Buff->size;i++)  {
        if ((Buff->buffer[i] != BuffCommand->buffer[i]) && (BuffCommand->buffer[i] != 0xFF )) return false;
    }
    return true;   
}


bool PowerMaxAlarm::isBufferOK(const PlinkBuffer* commandBuffer)
{
	const int old = log_console_setlogmask(0);
	PlinkBuffer commandBufferTmp ;
	memcpy(&commandBufferTmp, commandBuffer, sizeof(PlinkBuffer));
	bool ok = deFormatBuffer(&commandBufferTmp);

	log_console_setlogmask(old);
	return ok;
}

void PowerMaxAlarm::handlePacket(PlinkBuffer* commandBuffer) {

    if (deFormatBuffer(commandBuffer)) {
        DEBUG(LOG_DEBUG,"Packet received");
        lastIoTime = os_getCurrentTimeSec();

        logBuffer(LOG_DEBUG,commandBuffer);         
        int cmd_not_recognized=1;
        
        const int cmdCnt = (sizeof(PmaxCommand)/sizeof(PmaxCommand[0]));
        for (int i=0;i<cmdCnt;i++)  {
            if (findCommand(commandBuffer,&PmaxCommand[i]))  {
				DEBUG(LOG_INFO, "Command found: '%s'", PmaxCommand[i].description);
                PmaxCommand[i].action(this, commandBuffer);
                cmd_not_recognized=0;
                break;
            }   
        }  
        if ( cmd_not_recognized==1 )  {
            DEBUG(LOG_INFO,"Packet not recognized");
            logBuffer(LOG_INFO,commandBuffer);
            sendCommand(Pmax_ACK);   
        }                  
    }                                                         
    else  {
        DEBUG(LOG_ERR,"Packet not correctly formated");
        logBuffer(LOG_ERR,commandBuffer);
    }              
    //command has been treated, reset the commandbuffer
    commandBuffer->size=0;                    
    //reset End Of Packet to listen for a new packet       

    DEBUG(LOG_DEBUG,"End of packet treatment");
}

const char* PowerMaxAlarm::getZoneName(unsigned char zoneId)
{
    int count = os_cfg_getZoneCnt();
    if(zoneId < count)
    {
        return os_cfg_getZoneName(zoneId);
    }
    
    return GetStrPmaxDefaultZoneNames(zoneId);
}

const char* Zone::getName() const
{
    return PowerMaxAlarm::getZoneName(id);
}

void ConsoleOutput::write(const char* str)
{
    DEBUG_RAW(LOG_NO_FILTER, "%s", str);
}

void PowerMaxAlarm::DumpToJson(IOutput* outputStream)
{
    outputStream->write("{");
    {
        outputStream->writeJsonTag("stat", stat);
        outputStream->writeJsonTag("stat_str", GetStrPmaxSystemStatus(stat));
        outputStream->writeJsonTag("lastCom", (int)getSecondsFromLastComm());
        
        outputStream->writeJsonTag("flags", flags);
        outputStream->writeJsonTag("flags.ready", isFlagSet(0));
        outputStream->writeJsonTag("flags.alertInMemory", isFlagSet(1));
        outputStream->writeJsonTag("flags.trouble", isFlagSet(2));
        outputStream->writeJsonTag("flags.bypasOn", isFlagSet(3));
        outputStream->writeJsonTag("flags.last10sec", isFlagSet(4));
        outputStream->writeJsonTag("flags.zoneEvent", isFlagSet(5));
        outputStream->writeJsonTag("flags.armDisarmEvent", isFlagSet(6));
        outputStream->writeJsonTag("flags.alarm", isFlagSet(7));

        outputStream->write("\"enroled_zones\":[");
        {
            int addedCnt=0;
            for(int ix=1; ix<MAX_ZONE_COUNT; ix++)
            {
                if(zone[ix].enrolled)
                {
                    if(addedCnt >0)
                    {
                        outputStream->write(",\r\n");
                    }

                    addedCnt++;
                    zone[ix].DumpToJson(outputStream);
                }
            }
        }
        outputStream->write("]");

    }
    outputStream->write("}");
}

void Zone::DumpToJson(IOutput* outputStream)
{
    outputStream->write("{");
    {
        outputStream->writeJsonTag("id", id);
        outputStream->writeJsonTag("id_str", getName());
        
        if(lastEventTime > 0)
        {
            outputStream->writeJsonTag("lastEvent", lastEvent);
            outputStream->writeJsonTag("lastEventAge", (int)(os_getCurrentTimeSec()-lastEventTime));
        }

        outputStream->writeJsonTag("stat.doorOpen", stat.doorOpen);
        outputStream->writeJsonTag("stat.bypased", stat.bypased);
        outputStream->writeJsonTag("stat.lowBattery", stat.lowBattery);
        outputStream->writeJsonTag("stat.active", stat.active);
        outputStream->writeJsonTag("stat.tamper", stat.tamper, false);
    }
    outputStream->write("}");
}

void IOutput::writeQuotedStr(const char* str)
{
    write("\"");
    write(str); //todo: escape
    write("\"");
}

void IOutput::writeJsonTag(const char* name, bool value, bool addComma)
{
    writeJsonTag(name, value ? "true" : "false", addComma, false);
}

void IOutput::writeJsonTag(const char* name, int value, bool addComma)
{
    char szTmp[20];
    sprintf(szTmp, "%d", value);
    writeJsonTag(name, szTmp, addComma, false);
}

void IOutput::writeJsonTag(const char* name, const char* value, bool addComma, bool quoteValue)
{
    writeQuotedStr(name);
    write(":");
    if(quoteValue)
    {
        writeQuotedStr(value);
    }
    else
    {
        write(value);
    }

    if(addComma)
    {
        write(",\r\n");
    }
}