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

/*
'########################################################
' PowerMax/Master send messages
'########################################################

' ### ACK messages, depending on the type, we need to ACK differently ###
Private VMSG_ACK1 As Byte[] = [&H02] 'NONE
Private VMSG_ACK2 As Byte[] = [&H02, &H43] 'NONE



Private VMSG_ARMDISARM As Byte[] = [&HA1, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H43] 'NONE - MasterCode: 4 & 5
Private VMSG_STATUS As Byte[] = [&HA2, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H43] 'A5
Private VMSG_EVENTLOG As Byte[] = [&HA0, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H00, &H43] 'A0 - MasterCode: 4 & 5

' #### PowerMaster message ###
Private VMSG_PMASTER_STAT1 As Byte[] = [&HB0, &H01, &H04, &H06, &H02, &HFF, &H08, &H03, &H00, &H00, &H43] 'B0 STAT1



' ### PowerMax download/config items (some apply to PowerMaster too) ###
' ---
' Pos.  0  1  2  3  4  5  6  7  8  9  A
' E.g. 3E 00 04 20 00 B0 00 00 00 00 00
' 1=Index, 2=Page, 3=Low Length, 4=High Length, 5=Always B0?
' ---
' PowerMaster30:
' Pos.  0  1  2  3  4  5  6  7  8  9  A
' E.g. 3E FF FF 42 1F B0 05 48 01 00 00
' 1=FF 2=FF 3=Low Length, 4=High Length, 5=Always B0, 6=Index, 7=Page
' ---*/
//Private VMSG_DL_PANELFW As Byte[] = [0x3E, 0x00, 0x04, 0x20, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_ZONESTR As Byte[] = [0x3E, 0x00, 0x19, 0x00, 0x02, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_SERIAL As Byte[] = [0x3E, 0x30, 0x04, 0x08, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//'Private VMSG_DL_EVENTLOG As Byte[] = [0x3E, 0xDF, 0x04, 0x28, 0x03, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_TIME As Byte[] = [0x3E, 0xF8, 0x00, 0x20, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private vMSG_DL_COMMDEF As Byte[] = [0x3E, 0x01, 0x01, 0x1E, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_USERPINCODES As Byte[] = [0x3E, 0xFA, 0x01, 0x10, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
#define VMSG_DL_OTHERPINCODES {0x3E, 0x0A, 0x02, 0x0A, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00}
#define VMSG_DL_PHONENRS {0x3E, 0x36, 0x01, 0x20, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00}
//Private VMSG_DL_PGMX10 As Byte[] = [0x3E, 0x14, 0x02, 0xD5, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_PARTITIONS As Byte[] = [0x3E, 0x00, 0x03, 0xF0, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_ZONES As Byte[] = [0x3E, 0x00, 0x09, 0x78, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_KEYFOBS As Byte[] = [0x3E, 0x78, 0x09, 0x40, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_2WKEYPADS As Byte[] = [0x3E, 0x00, 0x0A, 0x08, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_1WKEYPADS As Byte[] = [0x3E, 0x20, 0x0A, 0x40, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_SIRENS As Byte[] = [0x3E, 0x60, 0x0A, 0x08, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_X10NAMES As Byte[] = [0x3E, 0x30, 0x0B, 0x10, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_ZONENAMES As Byte[] = [0x3E, 0x40, 0x0B, 0x1E, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//'Private VMSG_DL_ZONECUSTOM As Byte[] = [0x3E, 0xA0, 0x1A, 0x50, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//
//' ### PowerMaster download/config items ###
//Private VMSG_DL_MASTER_SIRENKEYPADSZONE As Byte[] = [0x3E, 0xE2, 0xB6, 0x10, 0x04, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER_USERPINCODES As Byte[] = [0x3E, 0x98, 0x0A, 0x60, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER_SIRENS As Byte[] = [0x3E, 0xE2, 0xB6, 0x50, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER_KEYPADS As Byte[] = [0x3E, 0x32, 0xB7, 0x40, 0x01, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER_ZONENAMES As Byte[] = [0x3E, 0x60, 0x09, 0x40, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER_ZONES As Byte[] = [0x3E, 0x72, 0xB8, 0x80, 0x02, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER10_EVENTLOG As Byte[] = [0x3E, 0xFF, 0xFF, 0xD2, 0x07, 0xB0, 0x05, 0x48, 0x01, 0x00, 0x00] '0x3F
//Private VMSG_DL_MASTER30_EVENTLOG As Byte[] = [0x3E, 0xFF, 0xFF, 0x42, 0x1F, 0xB0, 0x05, 0x48, 0x01, 0x00, 0x00] '0x3F
//
//Private VMSG_SET_DATETIME As Byte[] = [0x46, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF] '0x46


//    {{0xAA,0x12,0x34,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x43},12 ,"Enable bypass"        ,NULL},
//    {{0xAA,0x12,0x34,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Disable bypass"       ,NULL},
bool PowerMaxAlarm::sendCommand(PmaxCommand cmd)
{
    switch(cmd)
    {
    case Pmax_ACK:
        {
            if(m_ackTypeForLastMsg == ACK_1)
            {
                unsigned char buff[] = {0x02};
                return sendBuffer(buff, sizeof(buff));
            }
            else
            {
                unsigned char buff[] = {0x02,0x43};
                return sendBuffer(buff, sizeof(buff));
            }
        }

    case Pmax_GETEVENTLOG:
        {
            unsigned char buff[] = {0xA0,0x00,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return QueueCommand(buff, sizeof(buff), "Pmax_GETEVENTLOG", 0xA0, "PIN:MasterCode:4");
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

    /*
        The enrollment process
        The (emulated) Powerlink needs a pin code in order to:
        - Get some information from the Powermax
        - Use the bypass command
        - Use the disarm and arm commands
        During the enrollment process the Powerlink will create a pin and register this pin at the Powermax. The advantage is that none of the already pins is required.
        On the Powermax+ and Powermax Pro the (emulated) Powerlink can be enrolled via the installer menu, on the Powermax Complete the Powerlink will be registered automatically.

        When the (emulated) Powerlink is connected and the 'Install Powerlink' option is selected from the installer menu the Powermax sends the following message:
        CODE: SELECT ALL
        0xAB 0x0A 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x43

        The (emulated) Powerlink should respond with the following message:
        CODE: SELECT ALL
        0xAB 0x0A 0x00 0x00 <pin1> <pin2> 0x00 0x00 0x00 0x00 0x00 0x43

        where <pin1> and <pin2> are the digits of the pin code that needs to be registered in order to be used by the Powerlink. When the enrollment process is completed successfully, a beep will be sounded.
        Note: When a new Powerlink module needs to be registered while there is already a Powerlink registered, the previous registered one needs to be uninstalled. You can do so by selecting 'Install Powerlink' from the installer menu en then press the disarm button.

        On PowerMax Complete: trigger enrolment by asking to start download, this will fail on access denied, and PM will ask for enrolment.
    */
    case Pmax_ENROLLREPLY:
        {
            m_bDownloadMode = false;

            unsigned char buff[] = {0xAB,0x0A,0x00,0x00,0x12,0x34,0x00,0x00,0x00,0x00,0x00,0x43}; addPin(buff);
            return QueueCommand(buff, sizeof(buff), "Pmax_ENROLLREPLY");
        }

    case Pmax_RESTORE:
        {
            unsigned char buff[] = {0xAB,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43};
            return QueueCommand(buff, sizeof(buff), "Pmax_RESTORE");
        }

    case Pmax_INIT:
        {
            unsigned char buff[] = {0xAB,0x0A,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43};
            return QueueCommand(buff, sizeof(buff), "Pmax_INIT");
        }

    // ### PowerMax download/config items (some apply to PowerMaster too) ###
    // ---
    // Pos.  0  1  2  3  4  5  6  7  8  9  A
    // E.g. 3E 00 04 20 00 B0 00 00 00 00 00
    // 1=Index, 2=Page, 3=Low Length, 4=High Length, 5=Always B0?
    // ---
    // PowerMaster30:
    // Pos.  0  1  2  3  4  5  6  7  8  9  A
    // E.g. 3E FF FF 42 1F B0 05 48 01 00 00
    // 1=FF 2=FF 3=Low Length, 4=High Length, 5=Always B0, 6=Index, 7=Page
    // ---
    case Pmax_DL_PANELFW:
        {
            unsigned char buff[] = {0x3E, 0x00, 0x04, 0x20, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00};
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_PANELFW", 0x3F);
        }

    case Pmax_DL_SERIAL:
        {
            unsigned char buff[] = {0x3E, 0x30, 0x04, 0x08, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00};
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_SERIAL", 0x3F);
        }

   case Pmax_DL_ZONESTR:
        {
            unsigned char buff[] = {0x3E, 0x00, 0x19, 0x00, 0x02, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00};
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_ZONESTR", 0x3F);
        }

   case Pmax_DL_GET:
        {
            unsigned char buff[] = {0x0A};
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_GET", 0x33);
        }

    case Pmax_DL_START: //start download (0x3C - DownloadCode: 3 & 4)
        {
            unsigned char buff[] = {0x24,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; addPin(buff, 3);
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_START", 0x3C, "PIN:DownloadCode:3");

            if(m_bDownloadMode == false)
            {
                m_bDownloadMode = true;
            }
            else
            {
                DEBUG(LOG_WARNING,"Already in Download Mode?");
            }
        }

    case Pmax_DL_EXIT: //stop download
        {
            unsigned char buff[] = {0x0F};
            return QueueCommand(buff, sizeof(buff), "Pmax_DL_EXIT");

            if(m_bDownloadMode)
            {
                m_bDownloadMode = false;
            }
            else
            {
                DEBUG(LOG_WARNING,"Not in Download Mode?");
            }
        }
        
    default:
        return false;
    }
}


//FF means: match anything
struct PlinkCommand PmaxCommand[] =
{
    {{0x08                                                       },1  ,"Access denied"              ,&PowerMaxAlarm::PmaxAccessDenied},
    {{0x08,0x43                                                  },2  ,"Access denied 2"            ,&PowerMaxAlarm::PmaxAccessDenied},
    {{0xA0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Event Log"                  ,&PowerMaxAlarm::PmaxEventLog},
    {{0xA5,0xFF,0x02,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Battery" ,&PowerMaxAlarm::PmaxStatusUpdateZoneBat},
    {{0xA5,0xFF,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone tamper"  ,&PowerMaxAlarm::PmaxStatusUpdateZoneTamper},
    {{0xA5,0xFF,0x04,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Panel"        ,&PowerMaxAlarm::PmaxStatusUpdatePanel},
    {{0xA5,0xFF,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Status Update Zone Bypassed",&PowerMaxAlarm::PmaxStatusUpdateZoneBypassed},
    {{0xA7,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Panel status change"        ,&PowerMaxAlarm::PmaxStatusChange},
    {{0xAB,0x0A,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x43},12 ,"Enroll request"             ,&PowerMaxAlarm::PmaxEnroll},
    {{0xAB,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x43},12 ,"Ping"                       ,&PowerMaxAlarm::PmaxPing},
    {{0x3C,0xFD,0x0A,0x00,0x00,0x0E,0x05,0x01,0x00,0x00,0x00     },11, "Panel Info"                 ,&PowerMaxAlarm::PmaxPanelInfo},
    {{0x3F,0xFF,0xFF,0xFF                                        },-4, "Download Info"              ,&PowerMaxAlarm::PmaxDownloadInfo}, //-4 means: size>=4, len can differ
    {{0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF     },11, "Download Settings"          ,&PowerMaxAlarm::PmaxDownloadSettings},
    {{0x02,0x43                                                  },2  ,"Acknowledgement"            ,&PowerMaxAlarm::PmaxAck},
    {{0x02,                                                      },1  ,"Acknowledgement 2"          ,&PowerMaxAlarm::PmaxAck},
	{{0x06                                                       },1  ,"Time Out"                   ,&PowerMaxAlarm::PmaxTimeOut},
    {{0x0B                                                       },1  ,"Stop (Dload Complete)"      ,&PowerMaxAlarm::PmaxStop} 
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

const char*  PmaxPanelType[] = {
    "PowerMax"               ,
    "PowerMax+"              ,
    "PowerMax Pro"           ,
    "PowerMax Complete"      ,
    "PowerMax Pro Part"      ,
    "PowerMax Complete Part" ,
    "PowerMax Express"       ,
    "PowerMaster10"          ,
    "PowerMaster30"              
};

IMPEMENT_GET_FUNCTION(PmaxSystemStatus);
IMPEMENT_GET_FUNCTION(SystemStateFlags);
IMPEMENT_GET_FUNCTION(PmaxDefaultZoneNames);
IMPEMENT_GET_FUNCTION(PmaxZoneEventTypes);
IMPEMENT_GET_FUNCTION(PmaxLogEvents);
IMPEMENT_GET_FUNCTION(PmaxPanelType);

void PowerMaxAlarm::Init() {

    memset(&m_lastSentCommand, 0, sizeof(m_lastSentCommand));
    m_bEnrolCompleted = false;
    m_bDownloadMode = false;
    m_iPanelType = -1;
    m_iModelType = 0;
    m_bPowerMaster = false;
    m_ackTypeForLastMsg = ACK_1;

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

    //if PM is in dowload mode, then it would interfere with out init sequence
    //so let's start with DL exit command
    PowerMaxAlarm::sendCommand(Pmax_DL_EXIT); 

    PowerMaxAlarm::sendCommand(Pmax_INIT);

    //Send the download command, this should initiate the communication
    //First DL_START request is likely to fail on access denied, and PM should request the enrolment
    PowerMaxAlarm::sendCommand(Pmax_DL_START);
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

void PowerMaxAlarm::addPin(unsigned char* bufferToSend, int pos)
{
    int usercode = os_cfg_getUserCode();
    bufferToSend[pos]=usercode>>8;
    bufferToSend[pos+1]=usercode & 0x00FF ;
}

void PowerMaxAlarm::PmaxEnroll(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    //Remove anything else from the queue, we need to restart
    pm->clearQueue();
    pm->sendCommand(Pmax_ENROLLREPLY);
    DEBUG(LOG_INFO,"Enrolling.....");

    //if we're doing enroll, most likely first call to download failed, re-try now, after enrolment is done:
    pm->sendCommand(Pmax_DL_START);
}

//Should be called when panel is enrolled and entered download mode successfully
void PowerMaxAlarm::PowerLinkEnrolled()
{
    m_bEnrolCompleted = true;

   //Request the panel FW
   sendCommand(Pmax_DL_PANELFW);

   //Request serial & type (not always sent by default)
   sendCommand(Pmax_DL_SERIAL);

   //Read the names of the zones
   //IZIZTODO: PM responds with truncated packets (at least on windows)
   //sendCommand(Pmax_DL_ZONESTR);

   //' Retrieve extra info if this is a PowerMaster
   //If $bPowerMaster Then 
   //  SendMsg_DL_MASTER_SIRENKEYPADSZONE()

   //  ' Only request eventlog in debug mode
   //  If $bDebug Then
   //    Select $iPanelType
   //        Case &H07 ' PowerMaster10
   //          SendMsg_DL_MASTER10_EVENTLOG()
   //        Case &H08 ' PowerMaster30
   //          SendMsg_DL_MASTER30_EVENTLOG()
   //      End Select
   //    Endif
   //Endif

   //Request all other relevant information
   sendCommand(Pmax_DL_GET);

   //' We are done, exit download mode
   sendCommand(Pmax_DL_EXIT);

   //' auto-sync date/time
   //SendMsg_SetDateTime()

    //Lets request the eventlogs
    if(m_bPowerMaster == false)
    {
        //IZIZTOOD: uncomment:
        //pm->sendCommand(Pmax_GETEVENTLOG);
    }
}

//Direct message after we do a download start. Contains the paneltype information
void PowerMaxAlarm::PmaxPanelInfo(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    // The panel information is in 5 & 6
    // 6=PanelType e.g. PowerMax, PowerMaster
    // 5=Sub model type of the panel - just informational
    pm->m_iPanelType = Buff->buffer[6];
    pm->m_iModelType = Buff->buffer[5];
    pm->m_bPowerMaster =  (pm->m_iPanelType >= 7);
    pm->sendCommand(Pmax_ACK);

    DEBUG(LOG_INFO,"Received Panel Info. PanelType: %s, Model=%d (0x%X)", GetStrPmaxPanelType(pm->m_iPanelType),  pm->m_iModelType, (pm->m_iPanelType * 0x100 + pm->m_iModelType));

    //We got a first response, now we can continue enrollment the PowerMax/Master PowerLink
    pm->PowerLinkEnrolled();
}

int PowerMaxAlarm::ReadMemoryMap(const unsigned char* sData, unsigned char* buffOut, int buffOutSize)
{
    //The aMsg is in the regular download format, ignore the SubType and only use page, index and length
    //NOTE: Length can be more then &HFF bytes, in such we got multiple 3F responses with a "real" download
    int iPage = sData[2];
    int iIndex = sData[1];
    int iLength = (sData[4] * 0x100) + sData[3];

    if(iLength > buffOutSize)
    {
        DEBUG(LOG_ERR, "ReadMemoryMap, buffer too small, needed: %d, got: %d", iLength, buffOutSize);
        return 0;
    }

    MemoryMap* pMap = &m_mapMain;
    if(iPage == 0xFF && iIndex == 0xFF)
    {
        pMap = &m_mapExtended;
        
        //Overrule page/index/data if we have an extended message
        iPage = sData[7];
        iIndex = sData[6];
    }

    return pMap->Read(iPage, iIndex, iLength, buffOut);
}


//Write the 3C and 3F information into our own memory map structure. This contains all the
//information of the PowerMax/Master and will later be processed in ReadSettings
void PowerMaxAlarm::WriteMemoryMap(int iPage, int iIndex, const unsigned char* sData, int sDataLen)
{
    MemoryMap* pMap = &m_mapMain;
    if(iPage == 0xFF && iIndex == 0xFF)
    {
        pMap = &m_mapExtended;
        
        //Overrule page/index/data if we have an extended message
        iPage = sData[1];
        iIndex = sData[0];

        //Remove page/index and other 2 bytes, unknown usage
        sData += 4;
        sDataLen -= 4;
    }

    int bytesWritten = pMap->Write(iPage, iIndex, sDataLen, sData);
    if(bytesWritten != sDataLen)
    {
        DEBUG(LOG_ERR, "Failed to write to memory, page: %d, index: %d, len: %d, got: %d", iPage, iIndex, sDataLen, bytesWritten);
    }
}

//MsgType=3F - Download information
//Multiple 3F can follow eachother, if we request more then 0xFF bytes
void PowerMaxAlarm::PmaxDownloadInfo(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);

    //Format is normally: <MsgType> <index> <page> <length> <data ...>
    //If the <index> <page> = FF, then it is an additional PowerMaster MemoryMap
    int iIndex  = Buff->buffer[1];
    int iPage   = Buff->buffer[2];
    int iLength = Buff->buffer[3];


    if(iLength != Buff->size-4)
    {
        DEBUG(LOG_WARNING,"Received Download Data with invalid len indication: %d, got: %d", iLength, Buff->size-4);
    }

    //Write to memory map structure, but remove the first 4 bytes (3F/index/page/length) from the data
    pm->WriteMemoryMap(iPage, iIndex, Buff->buffer+4, Buff->size-4);
}

//MsgType=33 - Settings
// Message send after a DL_GET. We will store the information in an internal array/collection
void PowerMaxAlarm::PmaxDownloadSettings(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);

    //Format is: <MsgType> <index> <page> <data 8x bytes>
    int iIndex  = Buff->buffer[1];
    int iPage   = Buff->buffer[2];

    //Write to memory map structure, but remove the first 3 bytes from the data
    pm->WriteMemoryMap(iPage, iIndex, Buff->buffer+3, Buff->size-3);
}

//This one happens after all items requested by DLOAD_Get has been sent by PM
void PowerMaxAlarm::PmaxStop(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->sendCommand(Pmax_ACK);   
    DEBUG(LOG_INFO,"Stop (Dload complete)"); 
}

void PowerMaxAlarm::StartKeepAliveTimer()
{
    //IZIZTODO:
}

void PowerMaxAlarm::StopKeepAliveTimer()
{
    //IZIZTODO:
}

void PowerMaxAlarm::PmaxPing(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    pm->m_bDownloadMode = false; //ping never happens in download mode
    pm->sendCommand(Pmax_ACK);
    DEBUG(LOG_INFO,"Ping.....");

    //re-starting keep alive timer
    pm->StartKeepAliveTimer();
}

void PowerMaxAlarm::ProcessSettings()
{
    unsigned char tmpBuff[MAX_BUFFER_SIZE] = {0};

    {
        //Retrieve the installer and powerlink pincodes - they are known/visible
        unsigned char msg[] = VMSG_DL_OTHERPINCODES;
        int readCnt = ReadMemoryMap(msg, tmpBuff, sizeof(tmpBuff));

        DEBUG(LOG_INFO, "installer pin: %X%X", tmpBuff[0], tmpBuff[1]);
        DEBUG(LOG_INFO, "masterinstaller pin: %X%X", tmpBuff[2], tmpBuff[3]);
        DEBUG(LOG_INFO, "powerlink pin: %X%X", tmpBuff[8], tmpBuff[9]);
        //$cConfig["pincode"]["installer"] = ByteToHex(sData.Copy(0, 2))
        //$cConfig["pincode"]["masterinstaller"] = ByteToHex(sData.Copy(2, 2))
        //$cConfig["pincode"]["powerlink"] = ByteToHex(sData.Copy(8, 2))
    }

    {
        unsigned char msg[] = VMSG_DL_PHONENRS;
        int readCnt = ReadMemoryMap(msg, tmpBuff, sizeof(tmpBuff));
        for(int iCnt=1; iCnt<=4; iCnt++)
        {
            for(int jCnt=0; jCnt<=7; jCnt++)
            {
                if(tmpBuff[8 * (iCnt - 1) + jCnt] != 0xFF)
                {
                    unsigned char p = tmpBuff[8 * (iCnt - 1) + jCnt];
                    p++;
                }
            }
        }
    }
}

void PowerMaxAlarm::PmaxAck(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    if(pm->m_lastSentCommand.size == 1 &&
       pm->m_lastSentCommand.buffer[0] == 0x0F) //Pmax_DL_EXIT
    {
        //we got an ack for exit from dload mode:
        pm->m_bDownloadMode = false;

        //this will be false for the first Pmax_DL_EXIT that is called from Init()
        if(pm->m_bEnrolCompleted)
        {
            pm->ProcessSettings();

            //after download is complete, we call restore - this will get other important settings, and make sure panel is happy with comms
            pm->sendCommand(Pmax_RESTORE);
        }

        //re-starting keep alive timer
        pm->StartKeepAliveTimer();
    }
}

//Timeout message from the PM, most likely we are/were in download mode
void PowerMaxAlarm::PmaxTimeOut(PowerMaxAlarm* pm, const PlinkBuffer  * Buff)
{
    if(pm->m_bDownloadMode)
    {
        pm->sendCommand(Pmax_DL_EXIT); 
    }
    else
    {
        pm->sendCommand(Pmax_ACK);
    }

    DEBUG(LOG_INFO,"Time Out"); 
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

void  PowerMaxAlarm::SendNextCommand()
{
    if(m_sendQueue.isEmpty())
    {
        return;
    }

    os_usleep(50*1000); //to prevent messages going too quickly to PM

    PmQueueItem item = m_sendQueue.pop();

    sendBuffer(item.buffer, item.bufferLen);
}

//buffer is coppied, description and options need to be in pernament addressess (not something from stack)
bool PowerMaxAlarm::QueueCommand(const unsigned char* buffer, int bufferLen, const char* description, unsigned char expectedRepply, const char* options)
{
    if(m_sendQueue.isFull())
    {
        DEBUG(LOG_CRIT,"Send queue is full, dropping packet: %s", description);  
        return false;
    }

    if(bufferLen > MAX_SEND_BUFFER_SIZE)
    {
        DEBUG(LOG_CRIT,"Buffer to send too long: %d", bufferLen);  
        return false;
    }

    PmQueueItem item;
    memcpy(item.buffer, buffer, bufferLen);
    item.bufferLen = bufferLen;
    item.description = description;
    item.expectedRepply = expectedRepply;
    item.options = options;

    m_sendQueue.push(item);
    return true;
}

bool PowerMaxAlarm::sendBuffer(const unsigned char * data, int bufferSize)
{
    DEBUG(LOG_DEBUG,"Sending the following buffer to serial TTY");  
    //logBuffer(LOG_DEBUG,Buff); //IZIZTODO

    if(bufferSize >= MAX_BUFFER_SIZE-2 || 
       bufferSize > MAX_SEND_BUFFER_SIZE) //should never happen, but this will prevent any buffer overflows to writeBuffer.buffer
    {
        DEBUG(LOG_ERR,"Too long buffer: %d", bufferSize);  
        return false;
    }

    memcpy(m_lastSentCommand.buffer, data, bufferSize);
    m_lastSentCommand.size = bufferSize;

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
bool findCommand(const PlinkBuffer  * Buff, const PlinkCommand  * BuffCommand)  {
    
    if(BuffCommand->size < 0)
    {
        //ignore len, match begining only
        if(Buff->size < (-BuffCommand->size))
        {
            return false;
        }

        for (int i=0;i<(-BuffCommand->size);i++)  
        {
            if ((Buff->buffer[i] != BuffCommand->buffer[i]) && (BuffCommand->buffer[i] != 0xFF )) return false;
        }
    }
    else
    {
        if (Buff->size!=BuffCommand->size)
        {
            return false;
        }

        for (int i=0;i<Buff->size;i++)  
        {
            if ((Buff->buffer[i] != BuffCommand->buffer[i]) && (BuffCommand->buffer[i] != 0xFF )) return false;
        }
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

PmAckType PowerMaxAlarm::calculateAckType(const unsigned char* deformattedBuffer, int bufferLen)
{
    //Depending on the msgtype and/or last byte, we will be sending type-1 or 2 ACK
    if(bufferLen > 1)
    { 
        if(deformattedBuffer[0] >= 0x80 || (deformattedBuffer[0] < 0x10 && deformattedBuffer[bufferLen-1] == 0x43))
        {
            return ACK_2;
        }
    }

    return ACK_1;
}

void PowerMaxAlarm::handlePacket(PlinkBuffer* commandBuffer) {

    m_ackTypeForLastMsg = ACK_1;

    if (deFormatBuffer(commandBuffer)) {
        DEBUG(LOG_DEBUG,"Packet received");
        lastIoTime = os_getCurrentTimeSec();

        m_ackTypeForLastMsg = calculateAckType(commandBuffer->buffer, commandBuffer->size);

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

        //IZIZTODO:
        //should we send ack?
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