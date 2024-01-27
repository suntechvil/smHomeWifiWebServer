
#ifndef _SMART_DOOR_WIFI_WEB_SERVER_H
#define _SMART_DOOR_WIFI_WEB_SERVER_H

//==========================================================================================
#define SERVER_ID_MAIN_DOOR          0
#define SERVER_ID_SIDE_DOOR          1
#define SERVER_ID_SIDE_LAB           6
#define SERVER_ID_SIDE_PROTO_BOX     7

//=====
// define target control server with which this wifi server work together.
#define SERVER_ID                    SERVER_ID_SIDE_PROTO_BOX


#define EN_SOFT_AP
//#define REDIRECT_TO_ROOT_WHEN_NOT_FOUND_HANDLE
//#define USE_SMARTHOME_DNS_SERVER
//#define USE_DEFAULT_INDEX_PAGE
//#define USE_DOOR_FOLDER_AS_DOOR_CTRL_PAGE
#define USE_DOOR_CONTROL_PAGE_DIRECT_ACCESS
//#define USE_PING

// Define following preprocessor if you want to enable OTA f/w update.
//#define USE_OTA

#ifdef EN_SOFT_AP
# if SERVER_ID != SERVER_ID_SIDE_PROTO_BOX
    //2018.8.6: Enable it if the target is not the proto type which is used only during development.
#   define SOFT_AP_USE_PASSWORD
# endif
#endif
//==========================================================================================
//==> 2018.6.24: Following included files are for OTA operation. The Arduino IDE seems to include all associated libraries.
//               For information about this OTA, refers to the website: https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/readme.rst#id1
#include <ESP8266WiFi.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ArduinoJson.h>
#include "FS.h"
#include <lwip/etharp.h>

#ifdef USE_SMARTHOME_DNS_SERVER
# include <DNSServer.h>
# include <ESP8266mDNS.h>
#endif

# ifdef USE_OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
# endif
//<==

extern "C"
{
# include <lwip/icmp.h> // needed for icmp packet definitions
}

#ifdef USE_PING
//# include <ESP8266Ping.h>
# include <Pinger.h>
//# include <PingerResponse.h>
#endif

//==========================================================================================
// MY_WIFI_AP:
// 1 : [YOUR HOME ROUTER SSID]
// 2 : Toronto Library
// 3 : Second Cup
// 4 : Timhortons
// 5 : gGalaxy
// 6 : RioCanYSC_Guest
// 7 : LaonMakers
// 8 : select by GPIO pins.
// 0xFF : All of above 
#define DFT_HOME__WIFI_IX               1
#define TPL__WIFI_IX                    2
#define SECONDCUP__WIFI_IX              3
#define TIMHORTONS__WIFI_IX             4
#define G_GALAXY__WIFI_IX               5
#define LAON__WIFI_IX                   6
#define RIOCANYSC__WIFI_IX              7
#define SEL_GPIO__WIFI_IX               8
#define ALL_SSID__WIFI_IX               0xFF

#define MY_WIFI_AP                      DFT_HOME__WIFI_IX //8 //6 //4//6 // 0xFF //2 //5

//#define USE_MULTI_WIFI_CONNECT

#define NOF_WIFI_SSID                   5 //6
#define NOF_WIFI_SSID_PW_IN_EACH_GP     2
#define NOF_CHAR_WIFI_SSID              25 //30
#define NOF_WAIT_FOR_WIFI_CONNECT       10

#if MY_WIFI_AP == ALL_SSID__WIFI_IX //0xFF
# ifdef USE_MULTI_WIFI_CONNECT
#   include <ESP8266WiFiMulti.h>
# endif
#endif

#define HOST_NAME__SETUP                0
#define WIFI_MODE__SETUP                1
#define WIFI_CONFIG__SETUP              2


#ifdef VS_PRJ
# include <cstdint>
//# include "../SmartDoorWifiWebServer_hal_lib/hardware/esp8266com/esp8266/cores/esp8266/StreamString.h"
# include "../SmartDoorWifiWebServer_hal_lib/hardware/esp8266com/esp8266/cores/esp8266/WString.h"
#endif
//==========================================================================================
// compiler options:
#define EN_UART_RX
//==========================================================================================

#define LAB_DOOR_CLIENT             0
#define MAIN_DOOR_CLIENT            1
#define SIDE_DOOR_CLIENT            2

#define NOF_SMART_DOOR_CLIENT       2
#define SMART_DOOR_TYPE             MAIN_DOOR_CLIENT

//#define COOKIE_INVALID_USER_ID          "0"
//#define COOKIE_VALID_USER_ID            "1"
//#define COOKIE_VALID_ADMIN_ID           "7000"
//#define COOKIE_VALID_SUPER_USER_I_ID    "8000"
//#define COOKIE_VALID_SUPER_USER_II_ID   "C000"

#define INVALID_USER                0
#define VALID_USER                  1
#define VALID_ADMIN_USER            0x7000 // it is considered as guest during the development phase.
#define VALID_SUPER_USER_I          0x8000
#define VALID_SUPER_USER_II         0xC000

#define MASK_VALID_USER             0x7FFF
#define MASK_VALID_SUPER_USER_I     0xBFFF

#define CMD_LENGTH                  10


#define DFT_NOF_ADMIN_MAC           2
#define NOF_MAC_BYTE                6

#define DFT_ADMIN_GAL_NOTE_MAC      {0xf8,0xd0,0xbd,0xfd,0xfe,0xac}
#define DFT_ADMIN_GAL_2_MAC         {0x9c,0x02,0x98,0x46,0x04,0xef}


// Refers to this site to learn about more details of GPIO control: https://tttapa.github.io/ESP8266/Chap04%20-%20Microcontroller.html
#define SMART_DOOR_CTRL_PIN         5   // GPIO5

#define AP_SELECT_PIN               16  // GPIO16 // Access Point Selection. It is temporariy use before it is installed the smart door main board. GPIO16 has configurable internal pull-down resistor.
#define STATUS_LED_BLUE             14  // GPIO14 // Access Point Selection Pin 2
#define STATUS_LED_GREEN            12  // GPIO12
#define STATUS_LED_RED              13  // GPIO13

//#define STATUS_LED_RED_PIN          9   // GPIO9  // 2020.09.15: It seems not to be supported (available) by the ESP8266.
//#define STATUS_LED_GREEN_PIN        10  // GPIO10 // 2020.09.15: It seems not to be supported (available) by the ESP8266.

#define LOCK_SMART_DOOR             LOW
#define UNLOCK_SMART_DOOR           HIGH
#define MY_LED_ON                   LOW
#define MY_LED_OFF                  HIGH

#define INIT__ST                   0  // shared with system.hpp (Laon Control Server).
#define IDLE__ST                   1  // shared with system.hpp (Laon Control Server).
#define NOT_IN_SERVICE__ST         0xFF // shared with system.hpp (Laon Control Server). It is for the WiFi Server.

//#define DBG_PRINT_INDEX_HTML

//==> UART =======================================================================
// shared with both usettings.js and command.hpp; this number must be equal to the value in variable MAX_NOF_USERS_PER_ID in uSettings.html.
#define MAX_NOF_USERS_PER_ID        1 //Apr. 18, 2020: decreased to 1. //Mar. 7, 2020: decreased from 3 to 2.  // max number of users with the same ID including blocked users. This number limits the number user to disply on usettings.html when 'Read User' button is clicked;

//#define NOF_RX_BUF_ARRAY            2
//#define NOF_BYTE_ONE_RX_BUF         55

#define FOB_USR_BYTE_4_UPDATE       12  // The number of bytes per one Fob user record which are used when you request updating fob user record.
#define WIFI_USR_UPDATE_CMD_N_ARG_HEADER_BYTE 3 // It includes the command (Save Wifi User) and two args for the number of user record and the user ID.
#define WIFI_USR_BYTE_4_UPDATE      33 // The number of bytes per one Wifi user record which are used when you request updating Wifi user record. It includes 2 pair of host and mac.
// The size of both header and footer, so it excludes the command itself; 1 byte footer is the CRC.
#define CMD_HEADER_FOOTER_SIZE      5  // shared with control server.
#define SIZE_ONE_USER_RECORED       32 // it is shared by the control server (CS) and nodeJs.
#define NOF_MAX_USR_REC_2_UPDATE    (SIZE_ONE_USER_RECORED * MAX_NOF_USERS_PER_ID)
#define NOF_MAX_ACCESS_LOG          10 // shared with Control Server and nodeJs.
#define SOF_ONE_RAW_ACCESS_LOG_REC  4 // shared with Control Server and nodeJs.
#define SOF_ONE_ACCESS_LOG_AS_ARG   (SOF_ONE_RAW_ACCESS_LOG_REC + 1) // shared with both the Wifi Server and nodeJs project.
#define SOF_MAX_ACCESS_LOG_AS_ARG   (SOF_ONE_ACCESS_LOG_AS_ARG * NOF_MAX_ACCESS_LOG)  // shared with Control Server and nodeJs.

//Oct. 26, 2020: NOF_MAX_ACCESS_LOG is from the increased 1 byte for each time stamp.
//              '+2' is to add both log type and the number of return log.
#define NOF_MAX_BYTE_4_ACCESS_LOG_RSP  (SOF_MAX_ACCESS_LOG_AS_ARG + 2) // shared with Control Server and nodeJs.

// Mar. 7, 2020: I have replaced following definition with bigger size for Wifi user recode to update;
//               2 pair of host and mac alon are 2 * ( 1 + 6) = 14 bytes. It is actually greater than Fob user record.
//#define MAX_CMD_BUFFER_ARG_SIZE     ((5 + (MAX_NOF_USERS_PER_ID * FOB_USR_BYTE_4_UPDATE)) + 1) // the last number 1 is for the buffer.
#define MAX_CMD_BUFFER_ARG_SIZE     ((WIFI_USR_UPDATE_CMD_N_ARG_HEADER_BYTE + (MAX_NOF_USERS_PER_ID * WIFI_USR_BYTE_4_UPDATE))) // shared with Control Server and nodeJs.

#if NOF_MAX_BYTE_4_ACCESS_LOG_RSP > ( NOF_MAX_USR_REC_2_UPDATE + 2)
# define MAX_RSP_BUFFER_ARG_SIZE    NOF_MAX_BYTE_4_ACCESS_LOG_RSP // shared with Control Server and nodeJs.
#else
# define MAX_RSP_BUFFER_ARG_SIZE    (NOF_MAX_USR_REC_2_UPDATE + 2) // shared with Control Server and nodeJs.
#endif


#define MAX_CMD_BUFFER_SIZE         (CMD_HEADER_FOOTER_SIZE + MAX_CMD_BUFFER_ARG_SIZE + 1) // shared with Control Server and nodeJs. // the '+ 1' is for the command. //Jul. 13, 2019: replaced 25.
#define NOF_BYTE_RX_CMD_BUF         (CMD_HEADER_FOOTER_SIZE + MAX_RSP_BUFFER_ARG_SIZE + 1) // shared with Control Server and nodeJs. //Mar. 7, 2020: 72. The '+ 1' is for the command.


#define RX_DONE_IND_BUF_0__SERVER   0
#define RX_DONE_IND_BUF_1__SERVER   1
#define RX_NOT_DONE_IND__SERVER     0xFF


//The command and response which are exchanged between the server (Atmeg128) and client (ESP-8266)
// consists of two predefined header + command length + command itself with params. + one byte crc.
// The example of command and response is as follows:
//  - command:  '`', '>', 0x08, 0xF7, 0x01 0x00 0x00, CRC (one byte) // The fifth value 0x01 is the command code and following two values are parameters in this example.
//  - response: '`', '<', 0x07, 0xF8, 0x01 0x00, CRC (one byte) // The fifth value 0x01 is the command code and following one byte is ACK in this example.
//        or    '`', '<', 0x07, 0xF8, 0x01 0xFF, CRC (one byte) // The fifth value 0x01 is the command code and following one byte is NAK/error code in this example.
// More data can follow either ACK or NAK value to details about the result.



#define CMD_HEADER_1_CH             '`'
#define CMD_HEADER_2_CH             '>'
#define CMD_RSP_HEADER_1_CH         CMD_HEADER_1_CH
#define CMD_RSP_HEADER_2_CH         '<'

#define CMD_HEADER_1_IDX            0
#define CMD_HEADER_2_IDX            1
#define CMD_MSG_LEN_IDX             2    // Index to command message element which contains command length which includes everything related to the command; headers, command, args, one byte crc.
#define CMD_MSG_LEN_INV_IDX         3    // Index to command message element which contains one's complement of the command length
#define CMD_COMMAND_IDX             4
#define CMD_ARG_1_IDX               5
//#define CMD_RSP_COMMAND_IDX         5
#define CMD_RSP_ACK_NAK_IDX         5 //6

// Commands ///////////////////////////////////
// All these values must be shared with Smart Home Server Sub-system.
#define CMD_LOCK_DOOR               1
#define CMD_LOCK_DOOR_SERVER        2
#define CMD_UNLOCK_DOOR             3
#define CMD_UNLOCK_DOOR_SERVER      4
#define CMD_UPDATE_DOOR_STATUS      5

#define CMD_LIGHT_OFF               0x10
#define CMD_LIGHT_ON                0x11
#define CMD_UPDATE_LIGHT_STATUS     0x12

#define CMD_POLL                    0x20
#define CMD_ALERT_STATUS_CHANGE     0x21 // when there is a status change in either Control Server or WiFi Web Server.

#define CMD_GET_ACCESS_LOG          0x80 // 4 bytes: userStatus, day, hour, minute
#define CMD_EN_DISABLE_LOG          0x81
#define CMD_GET_CONFIG              0x82
#define CMD_SET_CONFIG              0x83
//#define CMD_READ_USER_ENTRY         0x84 // User entries from external EEPROM.
#define CMD_GET_ROOM_USER_INFO      0x85 // Both valid and blocked users from buffers/internal EEPROM; userStatus, card/user ID. Added on Jun. 11, 2019.
#define CMD_GET_USER_INFO           0x86 // Given user's information; fob info and wifi account info.
#define CMD_SAVE_FOB_USER           0x87 // Add or update given user information.
#define CMD_DELETE_USER             0x88
#define CMD_ENABLE_USER             0x89
#define CMD_DISABLE_USER            0x8A
#define CMD_READ_TAG                0x8B // It gets the RFID reader to read RFID tag and to return the result; Tag ID (serial number) is a must but security code is an option.
#define CMD_PRINT_INT_EEPROM        0x8C // Print interal eeprom data to debug serial port.
#define CMD_PRINT_EXT_EEPROM        0x8D // Print exteral eeprom data to debug serial port.
#define CMD_SCAN_AND_SAVE_FOB       0x8E // Scan user fob and register as new user.
#define CMD_SCAN_USER_FOB           0x8F // Scan user fob
#define CMD_FORMAT_FOB_SEC_CODE     0x90 // Format either a given security code block or all blocks.
#define CMD_READ_WIFI_MAC           0x91
#define CMD_SAVE_WIFI_USER          0x92
#define CMD_GET_VALID_WIFI_USER     0x93
#define CMD_GENERATE_LOGIN_KEY      0x94 // only for WiFi Server.

#define CMD_CANCEL_TASK_IN_PROGRESS 0xB0 // To get the current task (command) canceled.

//==> must be synched with values in both Command.hpp and 'DFT_SMART_DOOR_TEST_HTML' in smHomeMain.ino file.
#define CMD_SMART_CTRL_W_DOG_RESET  0xC0
#define CMD_SMART_CTRL_SOFT_RESET   0xC1
#define CMD_REST_RFID_READER        0xC2
#define CMD_SHOW_CTRL_DEV_STATUS    0xC3
#define CMD_SHOW_WIFI_DEV_STATUS    0xC4

#define CMD_TEST_CMD_1              0xF1
#define CMD_TEST_CMD_2              0xF2
#define CMD_TEST_CMD_3              0xF3
#define CMD_TEST_CMD_4              0xF4
//<==

//#define CMD_RESPOND                 0xFE
#define CMD_UNKNOWN                 0xFF

////// Args
#define CMD_ARG_LIGHT_OFF           0
#define CMD_ARG_LIGHT_ON            1

// Note: Keep the value in increment by 1. If you change either the order or any increment gap,
//       you must change all affected code.
#define CMD_ARG_RED_LIGHT_ON        1
#define CMD_ARG_GREEN_LIGHT_ON      2
#define CMD_ARG_BLUE_LIGHT_ON       3
#define CMD_ARG_ALL_LIGHT_ON        4

#define CMD_ARG_GREEN_LIGHT_GROW    5
#define CMD_ARG_BLUE_LIGHT_GROW     6
#define CMD_ARG_ALL_LIGHT_GROW      7
#define CMD_ARG_ALT_LIGHT_GROW      8

#define CMD_ARG_RED_LIGHT_OFF       9
#define CMD_ARG_GREEN_LIGHT_OFF     10
#define CMD_ARG_BLUE_LIGHT_OFF      11
#define CMD_ARG_ALL_LIGHT_OFF       12

#define CMD_ARG_GREEN_LIGHT_STOP    13
#define CMD_ARG_BLUE_LIGHT_STOP     14
#define CMD_ARG_ALL_LIGHT_STOP      15
#define CMD_ARG_ALT_LIGHT_STOP      16


#define CMD_RSP_ARG_ACK             0x00
#define CMD_RSP_ARG_NAK             0xFF

#define BIG_ROOM_BASE_ID            0x30
#define SMALL_ROOM_BASE_ID          0x40
#define ROOM_NEAR_LAUNDRY_BASE_ID   0x50
#define LAB_ROOM_BASE_ID            0x60
#define FAMILLY_ROOM_BASE_ID        0x70

#define BIG_ROOM_INDEX              1
#define SMALL_ROOM_INDEX            2
#define ROOM_NEAR_LAUNDRY_INDEX     3
#define LAB_ROOM_INDEX              4
#define FAMILLY_ROOM_INDEX          5

// An argument for the command 'CMD_ALERT_STATUS_CHANGE'; it is used to notify there is a change in WiFi user record in the Control Server.
#define ARG_ALERT_WIFI_REC_CHANGE   1
#define ARG_ALERT_WIFI_REC_ADD      2
#define ARG_ALERT_WIFI_REC_DELETE   3


// shared with Command.hpp and usettings.js.
#define MAX_NOF_USERS_PER_ROOM      3 //15 //(room id 1 ~ 15; 0 is reserved.)  // Mar. 21, 2020: set it to 3, since the update countdown is supported for only 3 users per room.
//#define MAX_USER_SETTING_INFO       20
#define MAX_NOF_USERS_PER_ID        1 //Apr. 18, 2020: decreased to 1. //Mar. 7, 2020: decreased from 3 to 2.  // max number of users with the same ID including blocked users. This number limits the number user to disply on usettings.html when 'Read User' button is clicked; this number must be equal to the value in variable maxUserRows in uSettings.html.

#define USR_ST__MASK_USERS_ROOM     0x70
#define USR_ST__MASK_ALL_USERS_ROOM 0xF0
#define USR_ST__MASK_BLOCKED_USERS_ROOM  0xF0

//==> Shared with both Control Server and nodeJs.
#define USR_ST__MASK_BLOCKED_USER   0x80
#define UNKNOWN_USER__USR_ST        0x86 //(MASK_BLOCKED_USER__USR_ST | CMD_ADD_DEL_GEN_USER_FRIST__USR_ST)
//<==
#define USR_ST__MASK_USERS_ID       0x0F
#define USR_ST__MASK_FULL_USERS_ID  0x7F

//#define INVALID_USER_WIFI_STATUS    0xFF // May 27, 2020: commented out to apply new userWifiStatus value format.

//uSettingInfo.dataHead[]
#define USET_INFO_CMD__HDDT_IDX     0
#define USET_INFO_ARG__HDDT_IDX     1
#define USET_INFO_LEN__HDDT_IDX     2

// CMD_SAVE_FOB_USER args index. Note: this value is different from ones in the Control Server.
#define COMMAND__UPDATE_USR_IX          0
#define REC_NUM__UPDATE_USR_IX          1
#define USER_IDX__UPDATE_USR_IX         2
#define VALID_USER__UPDATE_USR_IX       3
#define CARD_ID_0__UPDATE_USR_IX        4
#define CARD_ID_1__UPDATE_USR_IX        5
#define CARD_ID_2__UPDATE_USR_IX        6
#define CARD_ID_3__UPDATE_USR_IX        7
#define REG_YEAR_IDX__UPDATE_USR_IX     8
#define REG_MONTH__UPDATE_USR_IX        9
#define REG_DAY__UPDATE_USR_IX          10
#define SEC_CODE__UPDATE_USR_IX         11
#define SEC_UPDATE__UPDATE_USR_IX       12
#define SEC_U_PERIOD__UPDATE_USR_IX     13
#define DELETE__UPDATE_USR_IX           14

//==> shared with access.hpp (Laon Control Server) and usettings.js.
#define SIZE_CARD_ID                    4  
#define SIZE_INITIAL                    6
#define MIN_SIZE_INITIAL                2
#define SIZE_REG_DATE                   3
#define SIZE_PASSWORD                   8
// => shared with smHomeWIFiWebServer_h.js (nodeJs).
#define SIZE_KEY_CODE                   4
#define SIZE_PASS_CODE_GROUP            4
#define SIZE_PASS_REPEAT_CODE           16
#define SIZE_EACH_PASS_CODE_GROUP       256
// <=
//#define INVALID_SECURITY_CODE_INDEX     9  // it is represented as 'n/a' in the Security Index of usettings.html.
#define SECURITY_CODE_INDEX_FOR_ALL     5  // used to format all SC space. it is the same value as SEC_CODE_IDX_OPT_ALL in usettings.js.
//<==

#ifdef SERVER_ID_SIDE_DOOR
# define NOF_MAX_VALID_USER_IDX         15 //Apr. 3, 2023: replaced 10 with 15. You must define TARGET_HW as DEV_SIDE in
                                           // access.hpp (Laon Control Server) if you want both servers to be synced.
                                           // However, the synch with the nubmer is optional.
                                           //10 // shared with access.hpp (Laon Control Server), but not necessary to be the same value.
#else
# define NOF_MAX_VALID_USER_IDX         10 // shared with access.hpp (Laon Control Server), but not necessary to be the same value.
#endif

// CMD_SAVE_WIFI_USER args indics. Note: this value is different from ones in the wifi server and the nodeJs.
// 1; 11:1; D4FEC5DB; BigRm1; bigroom1; 200415; ff; 000000000000; 0
//`>26D9 92 01 11 01 D4FEC5DB 426967526D31 626967726F6F6D31 200415 FF 000000000000 00
#define COMMAND__UPD_WF_USR_IX          0
#define REC_NUM__UPD_WF_USR_IX          1
#define USER_IDX__UPD_WF_USR_IX         2
#define VALID_USER__UPD_WF_USR_IX       3
#define CARD_ID_0__UPD_WF_USR_IX        4
#define CARD_ID_1__UPD_WF_USR_IX        5
#define CARD_ID_2__UPD_WF_USR_IX        6
#define CARD_ID_3__UPD_WF_USR_IX        7
#define INITIAL_0__UPD_WF_USR_IX        8
#define INITIAL_1__UPD_WF_USR_IX        9
#define INITIAL_2__UPD_WF_USR_IX        10
#define INITIAL_3__UPD_WF_USR_IX        11
#define INITIAL_4__UPD_WF_USR_IX        12
#define INITIAL_5__UPD_WF_USR_IX        13
#define PASSWD_0__UPD_WF_USR_IX         14
#define PASSWD_1__UPD_WF_USR_IX         15
#define PASSWD_2__UPD_WF_USR_IX         16
#define PASSWD_3__UPD_WF_USR_IX         17
#define PASSWD_4__UPD_WF_USR_IX         18
#define PASSWD_5__UPD_WF_USR_IX         19
#define PASSWD_6__UPD_WF_USR_IX         20
#define PASSWD_7__UPD_WF_USR_IX         21
#define REG_YEAR_IDX__UPD_WF_USR_IX     22
#define REG_MONTH__UPD_WF_USR_IX        23
#define REG_DAY__UPD_WF_USR_IX          24
#define HOST_1__UPD_WF_USR_IX           25
#define MAC_1__0__UPD_WF_USR_IX         26
#define MAC_1__1__UPD_WF_USR_IX         27
#define MAC_1__2__UPD_WF_USR_IX         28
#define MAC_1__3__UPD_WF_USR_IX         29
#define MAC_1__4__UPD_WF_USR_IX         30
#define MAC_1__5__UPD_WF_USR_IX         31
//#define HOST_2__UPD_WF_USR_IX           25
//#define MAC_2__0__UPD_WF_USR_IX         26
//#define MAC_2__1__UPD_WF_USR_IX         27
//#define MAC_2__2__UPD_WF_USR_IX         28
//#define MAC_2__3__UPD_WF_USR_IX         29
//#define MAC_2__4__UPD_WF_USR_IX         30
//#define MAC_2__5__UPD_WF_USR_IX         31
//#define DELETE__UPD_WF_USR_IX           32
//#define RECORD_GP_SIZE__UPD_WF_USR      33
#define DELETE__UPD_WF_USR_IX           29

#define RECORD_GP_SIZE__UPD_WF_USR      30

#define WIFI_CLIENT_SCAN_ADD_REQ_IN_PROGRESS  0
#define WIFI_CLIENT_REQ_IN_PROGRESS  0x80

// do not change the order and make sure the number is in ascending order.
#define LOGIN_KEY_GEN_OFF               0
//#define LOGIN_KEY_GEN_DIS_REQ           1
#define LOGIN_KEY_GEN_EN_REQ            2
#define LOGIN_KEY_GEN_ENABLED           3
#define LOGIN_KEY_GEN_IN_PROGRESS       4

//==> Shared with both Control Server and nodeJs.
//#define VALID_USER_WIFI_STATUS__iEE      0x00
//#define BLOCKED_USER_WIFI_STATUS__iEE    0x80
//#define INVALID_USER_WIFI_STATUS__iEE    0xFF
//<==

#define COOKIE_RESET_TIME_IN_SEC       180 // cookie reset time in seconds.  // shared with nodeJs.

//<== UART =====================================================================
#define RESULT_OK                   0
#define RESULT_NG                   1
////// User Status
//#define TOT_NOF_IP__USER_STATUS             8

// For ROOM 1 ~ 3 and one reserved for potential user.
// i.g. 192.168.0.20, 22
#define MAX_NOF_ROOM__USER_STATUS           4
#define MAX_NOF_IP_PER_ROOM__USER_STATUS    2

// #if TOT_NOF_IP__USER_STATUS != (MAX_NOF_ROOM__USER_STATUS * MAX_NOF_IP_PER_ROOM__USER_STATUS)
// # error "TOT_NOF_IP__USER_STATUS must be equal to (MAX_NOF_ROOM__USER_STATUS * MAX_NOF_IP_PER_ROOM__USER_STATUS) and also be less than or equal to 8."
// #endif

#define PING_ST_IDLE                0
#define PING_ST_REQ_COMPLETE        1
#define PING_ST_CUR_REQ_DONE        2
#define PING_ST_IN_PROGRESS         3
#define PING_ST_IN_FAIL_TRY         4
#define PING_ST_END_FAIL_TRY        5

#define PING_IP_HOST_DEFAULT        0xFF
#define PING_IP_HOST_FAILURE        0x00
#define PING_MAX_NOF_IP_HOST        0x08

#define PING_DEFAULT_INDEX          0xFF

#define RSP_LEN_CMD_GET_VALID_WIFI_USER 18
#define RSP_LEN_CMD_GET_VALID_WIFI_USER_DONE 4

#define RSP_LEN_ARG_ALERT_WIFI_REC_CHANGE 19
#define RSP_LEN_ARG_ALERT_WIFI_REC_ADD    18
#define RSP_LEN_ARG_ALERT_WIFI_REC_DELETE 5
//<== Error Code =====================================================================
//----------- Interface Error ----------// 
// These errors are mainly for wifi web servers. It must be greater than or equal to 0x80 
// in order not to get confused with user ID values like in Save Fob User response.
// Therefore these error codes are shared among Smart Home Door Control sub-system, 
// wifi web server, and wifi client..
# define ERROR_IF_BASE              0x80
# define ERROR_IF_INVALID_CMD_ARG   0x81
# define ERROR_IF_UNKNOWN_RSP_ARG   0x82

// To find the point which is about 1 sec later from now on, you can use following trick:
// x = (millis() >> 10) + 1 ==>  if (millis() >> 10) == x, then it is 1,024 msec.
#define NOF_SHIFT_TO_DELAY_ABOUT_1_SEC    10
#define NOF_SHIFT_TO_DELAY_ABOUT_250_mSEC 8
#define NOF_SHIFT_TO_DELAY_ABOUT_125_mSEC 7

#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
# define DOOR_UNLOCK_TIMER_EXPIRY_IN_SEC  3   // to get the door locked after specified seconds.
#else
# define DOOR_UNLOCK_TIMER_EXPIRY_IN_SEC  10   // to get the door locked after specified seconds.
#endif

// shared with both Control server and nodeJs.
#define FIRST_SUPER_USER__USR_ST    0x01
#define FIRST_POWER_USER__USR_ST    0x03
#define FIRST_SPECIAL_USER__USR_ST  0x0E

// if  missed the lock time, the match time will come back in 16 seconds by this value 0x0F.
#define LOCK_TIME_MISS_RECOVERY        0x0F //0x3F
// if missed the cookie countdown time, the match time will come back in 32 seconds by this value 0x1F.
#define COOKIE_COUNT_DWN_MISS_RECOVERY 0x1F //0x3F 
// if missed the countdown time, the match time will come back in 3 seconds by this value 0x03.
#define RESEND_COUNT_DWN_MISS_RECOVERY 0x03 //0x3F 
// if  missed the lock time, the match time will come back in 2 second by this value 0x07.
// it uses different time base.
#define GROW_TIME_MISS_RECOVERY        0x07 //0x3F

#define GROW_INIT_INTENSITY            1023 // duty cycle 100% ( always high; LED off )
#define GROW_INTENSITY_STEP            50 //100
//==========================================================================================
//==========================================================================================
typedef struct UserSettingInfo
{
    unsigned char dataHead[3]; // associated command, index, length of received setting data.
    char aSettingData[NOF_MAX_USR_REC_2_UPDATE + 1];  // it will temporarily be user setting related data which is replied by the door control.
} UserSettingInfo_t;

typedef struct WifiUserRecord
{
    unsigned char userWifiStatus;
    unsigned short cookie;
    char initial[SIZE_INITIAL];     // for both eeprom and the tag.
    char password[SIZE_PASSWORD];
} WifiUserRecord_t;

typedef struct ValidWifiUserInfo
{
    unsigned char nofValidUser; // The number of valid user entry in the internal EEPROM.
    WifiUserRecord_t* paUser[NOF_MAX_VALID_USER_IDX];
    WifiUserRecord_t newUser;  // to keep wifi user recored to be updated.
} ValidWifiUserInfo_t;

typedef struct CmdResendTag
{
    unsigned char cmd;
    unsigned char cmdData;
    unsigned char arg;
    unsigned char cntDwn;  // re-send the pending command until this counter becoems zero.
    unsigned char nextTime;    

} CmdResend_t;

typedef struct WifiWebServerControl
{
    unsigned char ctrlServerState; // Laon Control Server state.
    unsigned char pollCnt;
    CmdResend_t toResend;
    unsigned char bLockEn;   // enabled locking the door after given time elpased.
    unsigned char lockTime;  // used to lock the door when the elapsed time is equal to this value.
    unsigned char cookieResetCntDwn; // used to reset all cookie values to zero when it reaches to 0.
    unsigned char t2UdtCookieResetCntDwn; // used to keep a time to count down the cookieResetCntDwn.
#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    unsigned char growMode;  // if it is greater than 0, it ligth grow mode is on. Each value means as follows:
                             //  - 0x01: grow Green LED, 0x02 = grow Blue.
                             //  - 0x03: grow both Green and Blue LED,
                             //  - 0x11: grow alternatively between Green and Blue; MSB indicates that the current
                             //          LED is Green. When growLightIntense becomes greater than 0xFFFF,
                             //          current LED is turned off and this value changed to 0x12 to get Blue is growed.
                             //  - 0x12: grow alternatively between Green and Blue; MSB indicates that the current LED is Blue.
    short growLightIntense;  // When growMode is greater than 0x10, it is valid. When this intense value turns to
                             // a value greater than 0xFFFF, it sets to 0.
    unsigned char t2ChangeLightIntense; // used to keep a time to change the light intensity.
#endif
} WifiWebServerControl_t;

extern UserSettingInfo_t uSettingInfo;
extern ValidWifiUserInfo_t user;
extern WifiWebServerControl_t ctrlSys;

#ifdef USE_PING
    extern unsigned char userIp[MAX_NOF_ROOM__USER_STATUS][MAX_NOF_IP_PER_ROOM__USER_STATUS];
    // If the remote respondes, one of bits are set to 1. Otherwise 0. If two hosts that are stored in userIp[0][0] and userIp[1][1] responded to its ping,
    // then both bit 0 and bit 3 are set to 1 ( userIpStatus = 0000 1001 ).
    extern unsigned char userIpStatus;
#endif
//==========================================================================================
//==========================================================================================
extern unsigned char Is_keycode_match(uint8_t ix);
extern unsigned short HexStrToUint16(String hx);
extern unsigned short HexCharToUint16(char* pCh, unsigned char nb);
extern unsigned short GetLaonId(String ck);
extern unsigned short Is_authenticated();
extern unsigned short Is_admin_authenticated();
extern bool Is_superUser();
extern void BuildLockStatusXML();
extern void HandleAdmin(void);
extern void HandleLoginCodeGen(void);
extern void HandleLogin(void);
extern void HandleNotFound(void);
extern void ShowDoorCtrlPage(void);
extern void HandleTestMenu(void);
extern void HandleRoot(void);
extern void HandleXML();
extern void HandleLoginJs(void);
//extern void HandleSmHomeJs(void);
//extern void HandleSmHomeCSS(void);
extern void HandleUSettingsJs(void);
extern void HandleSystem(void);
extern void HandleUserLog(void);
extern void HandleUserSettings(void);
extern bool ReadAndSendFile(String file, String contType);

extern void InitSmartDoor(void);
extern void LockTheDoor(bool bUdt);
extern void DoorLockControl(char cmd, char id);
extern void SendLockRequest(bool bUdt);
//extern void UnlockDoor(char cmd, char id);
extern void LightOnOff(char cmd, unsigned char ix);
extern char SendCommand(char * cmd, char len);
extern char SendResponse(char cmd, char * rsp, char len);
extern void SendPollCommand(bool bNew);
extern void SendGetValidWifiUserCmd(bool bNew);
extern void InitResendBuf();
extern bool GetDoorlockState(void);
extern bool HandleDoor(void);
extern void ExecuteCmd(void);
extern bool HandleUartMsg(void);
extern void OnUartRxd(void);
extern void SwitchRxBufIndex(void);


extern void InitUserStatusBuf(void);
extern void HandleUserStatus(void);
extern void ExecPing(void);
extern void PrintNumOneByteDbg(char h, char v);
extern int AsciiToInt(const char *pCh, char len);
extern char IndexOf(char ch, char start, const char *pCh, char len);
# ifdef USE_PING
    extern bool SmPingRspRx(const PingerResponse& response);
    extern bool SmPingEnd(const PingerResponse& response);
# endif
#endif
