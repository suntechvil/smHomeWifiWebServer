/*
*  This sketch demonstrates how to set up a simple HTTP-like server.
*  The server will set a GPIO pin depending on the request
*    http://server_ip/led/0 will set the GPIO2 low,
*    http://server_ip/led/1 will set the GPIO2 high
*  server_ip is the IP address of the ESP8266 module, will be
*  printed to Serial when the module is connected.
*/

//Aug. 6, 2018: Arduino IDE seems to include all source c file and header files if these are in the same project folder
//              Therefore I'm commenting out this header include.
//#include "P:\Project\esp8266ex\SmartDoorWifiWebServer\SmartDoorWiFiWebServer.h"
#ifdef VS_PRJ
# include ".\laonSmHomeWiFiWebServer.h" //Aug. 6, 2018: Full path name is required since I have updated the Arduino board library (ESP8266 2.4.2) today.
#endif
//#define USE_YIELD_FUNC

extern bool bNeedLogin;

extern bool bDoorClosed; // Server's(ATmega's) door lock/unlock status. it keeps the status of door lock/unlock.
extern bool bLightOn;      // Server's(ATmega's) light status. it keeps the status of light.


//==> UART =======================================================================
#ifdef EN_UART_RX
extern char indexBuf;               // It points to next available rx buffer element space.  
extern char rxDoneInd;              //
extern char rxBufIdx;
//char rxBufTxIdx;             //
//static char rxBuf[NOF_RX_BUF_ARRAY][NOF_BYTE_ONE_RX_BUF + 1];
extern char rxCmdIdx;
//char rxCmdLen;  
extern char rxCmdReady;
extern char lastCmd;
extern char rxCmdBuf[NOF_BYTE_RX_CMD_BUF + 1];
extern unsigned long rxTimeOut;
//#define DBG_PRINT_RX_IND_MSG //Aug. 5, 2018: Defined it for debugging in order to get some message is sent to the server upon receiption of UART command/response.
#endif
//<== UART =====================================================================

#ifdef USE_PING
extern Pinger pinger;

//extern unsigned char userIp[MAX_NOF_ROOM__USER_STATUS][MAX_NOF_IP_PER_ROOM__USER_STATUS];

//// If the remote respondes, one of bits are set to 1. Otherwise 0. If two hosts that are stored in userIp[0][0] and userIp[1][1] responded to its ping,
//// then both bit 0 and bit 3 are set to 1 ( userIpStatus = 0000 1001 ).
//extern unsigned char userIpStatus;
extern char pingStatus;
extern char pingCurrentIdx;
#endif


//******* Revision History *******
//Note: this version should be the same as the one in the nodeJsSmHomeWifiWebServer project
//      if methods are snychronized."
/*
  ver. 0.0.5, 2018.8.6: Support Access Point.
  ver. 0.1.0, 2018.11.11: support sending file/files in pieces.
  ver. 0.2.0, 2019.6.12: support user settings page.
  ver. 0.2.1, 2019.6.23: downloaded user settings feature into the target for the first time.
  ver. 0.2.2, 2019.7.01: Getting room users' list works. Working on getting selected user's records.
  ver. 0.2.3, 2019.10.08: Support new eeprom map.
  ver. 0.2.4, 2020.2.17: New eeprom map works; added security code index into Save Fob User command.
  ver. 0.2.5, 2020.2.22: Scan a Fob command works with the Smart Door Control Subsystem v. SW- 3.3.
  ver. 0.2.6, 2020.3.4: usettings.html has been splitted into itself and usettings.js.
  ver. 0.2.7, 2020.3.8: a return value to show in-progress of Scan a Fob has been changed from 0 to 0x80.
  ver. 0.2.8, 2020.3.13: Format fob command is supported.
  ver. 0.2.9, 2020.3.21: Security update works (need fail situation). RFID reader reset command has been added to test web page.
              Both error code and commands for test have been changed. It works with Smart Home Server ver. SW-3.6.
  ver.3.3.2.9, 2020.3.22: version format has been changed as follows:
  ----------------------------------------------------------
  * Laon Home Control System consists of following sub-systems:
  ----------------------------------------------------------
    - RFID sub - system,
    - ESP8266 web server sub - system,
    - Light control sub - system,
    - Laon Home Server sub - system
 
  ----------------------------------------------------------
  * Version contorl for Laon Home Control System:
  ----------------------------------------------------------
    h.i.v.r
       - h : hardware version number : increamented whenever there is hardware changes.
       - i : system integration version number : increamented whenever system interface is changed. This number is independant from the change in 'h'.
            It increases, e.g, whenever any interface command value is changed/added. Each sub - systems can work together when these first 2 versions are equal.
       - v : sub - system version number : increamented when there is any major code change in its sub - system.
       - r : sub - system revision number : increamented when there is any minor change in its sub - system.
  ver.3.3.2.10, 2020.3.26: new command to cancel command in progress.
  ver.3.4.0.0, 2020.3.28: the value to request formatting all SC space has been changed. Security Index text box has been replaced with option dropdown menu.
  ver.3.5.0.0, 2020.3.28: 'Fix' in the 'Security Update' option has been replaced with 'Now' and 'Wifi' has been added. When you set it to 'Wifi', 
                          the SC is updated after the wifi user opens the door; at the time when the user opens the door with his/her Fob, 
                          the Fob SC is immediately increased by 1. This feature is automatically enabled when the option is set to either 'On' or 'Once' as well.
                          This feature is turned off when the option is set to 'Off'.
  ver.3.5.0.1, 2020.4.25: New commands have been added to support valid Wifi user records in order to use them in log-in.
                          Max 10 valid Wifi users' info can be stored in the Wifi Server.
                          Door can be opened with valid Wifi users' initial and password.
  ver.3.5.1.0, 2020.5.3: New login flow which uses TSID, login key code has been applied. Login code generation has been implemented.
  ver.3.6.0.0, 2020.5.27: userWifiStatus values follows the value format of userStatus.
  ver.4.0.0.0, 2020.9.15: 3 LEDs have been added to the board. Replaced the capacitor value for reset with 100uF; 
                         it used to be 100nF and recommended value is 10uF, but I'm using 100uF.
                         Toggle switch has been added to set download mode easy.
  ver. 4.0.0.1 2020.9.17: added various buttons for 3 LEDs' control in the doorctrl.html.
                         3 LEDs are now controlled by Wifi client.
                         Bug fixed in timer routines in the 'loop()'.
  ver. 4.0.0.2 2020.9.18: LED grow function has been improved. Button layouts in the Door Control web page has been improved.
                          Ready for Demo.
  ver. 4.1.0.0 2020.10.18: bugFix: user ID is now sent to Control Server in either Unlock or Get Unlock command.
  ver. 6.2.0.0 2023.4.3: The max valid user size has been increased to 15 from 10, espeically for the side door since there are more users than other door.
  ver. 4.1.0.1 2023.12.31 -\
  ver. 6.2.0.1 2023.12.31 --`--> 1) Split 'setup' function into 3 sub function to reduce system crashes. 2) Disabled 'ping'.
*/
// the first two number is main version which must be the same number with Laon Control Server version.
#if SERVER_ID == SERVER_ID_SIDE_DOOR
    const char * buildVer = "6.2.0.1";
#else
    const char * buildVer = "4.1.0.1";
#endif

const char * buildDate = __DATE__ ",  " __TIME__ "\r\n";



//==> Station & Access Point =====================================================
//==> Aug. 6, 2018: Added access point.

# define SM_HOME_DEMO_SSID     "LaonMakers"
# define SM_HOME_DEMO_PW       ""
//# define SM_HOME_DEMO_PW       "laonmakers"

# define SM_HOME_SSID          "[YOUR SSID]"
# define SM_HOME_PW            "[YOUR ROUTER PASSWORD"

#ifdef EN_SOFT_AP
    const char * APssid = "SmartHome";

# ifdef SOFT_AP_USE_PASSWORD
    // password must be equalt to or greater than 8, otherwise softAP method will return false.
    const char * APpassword = "homesmart";
# else
    const char * APpassword = 0;
# endif
    const char * WiFiHostname = "www.smc.com";   // Host name of the device
                                                //IPAddress apGW(192, 168, 33, 1);          // The IP address of the access point gateway.
                                                //IPAddress apIP(192, 168, 33, 33);         // The IP address of the access point
    IPAddress apIP(192, 168, 33, 1);          // The IP address of the access point
#endif
//<==
//<== Station & Access Point =====================================================
#if MY_WIFI_AP == DFT_HOME__WIFI_IX
    const char* ssid = SM_HOME_SSID;
    const char* password = SM_HOME_PW;
#elif MY_WIFI_AP == TPL__WIFI_IX
    const char* ssid = "Toronto Public Library";
    const char* password = "";
#elif MY_WIFI_AP == SECONDCUP__WIFI_IX
    const char* ssid = "Second Cup - WiFi";
    const char* password = "secondcup2018";
#elif MY_WIFI_AP == TIMHORTONS__WIFI_IX
    const char* ssid = "Tim Hortons WiFi";
    const char* password = "";
    //#elif MY_WIFI_AP == G_GALAXY__WIFI_IX
#elif MY_WIFI_AP == LAON__WIFI_IX
    //This is the Netgear WNR100 for a demonstration.
    const char* ssid = SM_HOME_DEMO_SSID;
    const char* password = SM_HOME_DEMO_PW;    
#elif MY_WIFI_AP == SEL_GPIO__WIFI_IX

// Do not change the order. If you need to change, then change code associated to this table accordingly.
const char aSsid[NOF_WIFI_SSID][NOF_WIFI_SSID_PW_IN_EACH_GP][NOF_CHAR_WIFI_SSID] = {
    { SM_HOME_DEMO_SSID, SM_HOME_DEMO_PW },
    { SM_HOME_SSID, SM_HOME_PW },
    { "Toronto Public Library", "" },
    { "Second Cup - WiFi", "secondcup2018" },        
    { "Tim Hortons WiFi", "" }
    //{ "gGalaxy", "pi@coffeeshop" } //Sep. 12, 2020: no longer available.    
};

#else
# if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
const char* ssid;
const char* password;

const char aSsid[NOF_WIFI_SSID][NOF_WIFI_SSID_PW_IN_EACH_GP][NOF_CHAR_WIFI_SSID] = {
    //{ "gGalaxy", "pi@coffeeshop" }, //Sep. 12, 2020: no longer available.
    { SM_HOME_DEMO_SSID, SM_HOME_DEMO_PW },        
    { "Second Cup - WiFi", "secondcup2018" },
    { "Tim Hortons WiFi", "" },
    { "Toronto Public Library", "" },
    { SM_HOME_SSID, SM_HOME_PW }
};
# else
const char ssid[] = SM_HOME_SSID;
const char password[] = SM_HOME_PW;
# endif
#endif 

// Create an instance of the server
// specify the port to listen on as an argument
//WiFiServer server(80);
ESP8266WebServer server(80);


#ifdef USE_MULTI_WIFI_CONNECT
ESP8266WiFiMulti wifiMulti;
#endif

#ifdef USE_SMARTHOME_DNS_SERVER
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif
const char * pIndexHtml;
//2018.10.26: Added PROGMEM to get the string to be stored in a flash.
//2018.8.6: It is certain that this station cannot be connected to wifi router if this this array is bigger than
//          a limit which I don't know yet. Therefore I have decided to keep it as small as possible.
//          As the result, I don't use following script any more.
//2018.6.24: Don't use '<br>' in <style>. Test following html in https://www.w3schools.com before downloading.
//2018.4.1: It seems that it's array capacity is almost the limit. 
// Therefore additon of statments caused it cannot camp to a valid AP.
//const char DFT_SMART_DOOR_INDEX_HTML[] PROGMEM =
const char * DFT_SMART_DOOR_INDEX_HTML =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<style>"
"header, footer { padding:1em; color:white; background-color:black; clear:left; text-align:center; }"
"div { margin: 20px 50px 20px 50px; }"
"a { font-size:30px; }"
"button, input.main { outline:none; font-size:40px; width:100%; height:100px; border-radius:50px; }" //outline:none behaves like blur().
"</style>"
"<script>\n"
"var myStatusTimer;\n"
"var maxStatusQueryCnt=0;\n"
"var $ = function(id) { return document.getElementById(id); }\n"
"var xmlHttp= new CreateXmlHttpObject();\n"
"function CreateXmlHttpObject(){\n"
"var xHttp;\n"
"if(window.XMLHttpRequest){\n"
"xHttp=new XMLHttpRequest();\n"
"}else{\n"
"xHttp=new ActiveXObject('Microsoft.XMLHTTP');\n"
"}\n"
"return xHttp;\n"
"}\n"
"var handleServerResponse=function(){\n"
"if(xmlHttp.readyState==4 && xmlHttp.status==200){\n"
"if(maxStatusQueryCnt < 10){\n"
"xmlResponse=xmlHttp.responseXML;\n"
"xmldoc = xmlResponse.getElementsByTagName('response');\n"
"message = xmldoc[0].firstChild.nodeValue;\n"
"} else { message = '   please click on Lock Status button!';}\n"
"$('doorst').innerHTML=message;\n"
"}\n"
"}\n"
"var checkStatus=function(){\n"
"if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n"
"if(maxStatusQueryCnt < 10){\n"
"xmlHttp.open('PUT','/xml',true);\n"
"xmlHttp.onreadystatechange=handleServerResponse;\n"
"xmlHttp.send(null);\n"
"maxStatusQueryCnt++;\n"
"} else if(maxStatusQueryCnt == 10){\n"
"clearInterval(myStatusTimer);\n"
"maxStatusQueryCnt=100;\n"
"$('lstat').disabled=false;\n"
"}\n"
"}\n"  //"setTimeout('process()',2000);\n"
"}\n"
"var startStatusCheck = function(){\n"
"myStatusTimer=setInterval( function() {checkStatus();}, 3000);\n"
"}\n"
"var restartCheckStatus=function(){\n"
"$('lstat').disabled=true;\n"
"maxStatusQueryCnt=0;\n"
"startStatusCheck();\n"
"$('doorst').innerHTML='   please wait !';\n"
"}\n"
"window.onload = startStatusCheck;\n"
"</script>\n"
"</head>"
"<body>"
"<header>"
"<H1>Laon WiFi Server</H1>"
"</header><br><br>"
"<div>"
"<H1>Door Control</H1>"
"<form action='/?cmd=1&DrCtrl=3' method='post'>"
"<input type='submit' value='Unlock' class='main'>" //CMD_UNLOCK_DOOR
"</form><br>"
"<form action='/?cmd=1&DrCtrl=4' method='post'>"
"<input type='submit' value='Get Unlocked' class='main'>" //CMD_UNLOCK_DOOR
"</form><br>"
"<button id='lstat' disabled='disabled' onclick='restartCheckStatus()'>Lock Status</button>"
"</div><br>"
"&nbsp;&nbsp;Door Status: <i><a id='doorst'></a></i><br><br>"
"<div>"
"<h1><mark>Light Control</mark></h1>"
"<form action='/?cmd=1' method='post'>"
"<input type='radio' name='LightCtrl' value='1' checked> Light On &nbsp;&nbsp;&nbsp;"
"<input type='radio' name='LightCtrl' value='0'> Light Off <br><br>"
"<input type='submit' value='Light' width='200px'>"
"</form>"
"Click to disconnect <a href = '/login?DISCONNECT=YES'>disconnect</a><br>"
"<a href='/?cmd=1&AdmSysMg=1'> System Management </a>"
"</div><br><br>"
"<footer>"
"<i>Revision: 3.5.0, Build : Mar. 14, 2020, Copyright &copy; 2020 by Gi Tae Cho.All Right Reserved.</i>"
"</footer>"
"</body>"
"</html>";

//2018.10.26: Added PROGMEM to get the string to be stored in a flash.
//const char DFT_SMART_DOOR_TEST_HTML[] PROGMEM =
const char * DFT_SMART_DOOR_TEST_HTML =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<style>"
"header, footer { padding:1em; color:white; background-color:black; clear:left; text-align:center; }"
"div { margin: 20px 50px 20px 50px; }"
"a { font-size:30px; }"
"button { font-size:40px; width:100%; height:100px; border-radius:50px; }"
"</style>"
"</head>"
"<body>"
"<header>"
"<H1>System Test</H1>"
"</header><br><br>"
"<div>"
"<H1>Test Commands</H1>"
"<a href='/?cmd=1&test=241'><button>Test I</button></a><br><br>"//CMD_UNLOCK_DOOR
"<a href='/?cmd=1&test=242'><button>Test II</button></a><br><br>"//CMD_UNLOCK_DOOR_SERVER
"<a href='/?cmd=1&test=243'><button>Test III</button></a><br><br>"
"<a href='/?cmd=1&test=243'><button>Test IV</button></a><br><br>"
"<button onclick='process()'>Lock Status</button><br>"
"<a href='/?cmd=1&test=196'><button>WiFi Device Status</button></a><br><br>"
"<a href='/?cmd=1&test=195'><button>Ctrl Device Status</button></a><br><br>"
"<a href='/?cmd=1&test=194'><button>Reset: RFID Reader</button></a><br><br><br><br><br>"
"<a href='/?cmd=1&test=193'><button>Reset (soft): Ctrl Server</button></a><br><br>"
"<a href='/?cmd=1&test=192'><button>Reset (full): Ctrl Server</button></a><br>"
"</div><br>"
"&nbsp;&nbsp;Test Status: <a id='testst'></a><br><br>"
"<div>"
"Click to disconnect <a href = '/login?DISCONNECT=YES'>disconnect</a><br>"
"<a href='/?cmd=1&AdmSysMg=1'> System Management </a>"
"</div><br><br>"
"<footer>"
"<i>Revision: 3.5.0, Build : Apr. 20, 2020, Copyright &copy; 2020 by Gi Tae Cho.All Right Reserved.</i>"
"</footer>"
"</body>"
"</html>";

const char dftAdminMac[DFT_NOF_ADMIN_MAC][NOF_MAC_BYTE] = { DFT_ADMIN_GAL_NOTE_MAC, DFT_ADMIN_GAL_2_MAC };
const char ** ppAdminMac; // 'ppAdminMac[i][j]' after 'ppAdminMac = dftAdminMac;' got it crashed.

const char * pAdminMac;

WifiWebServerControl_t ctrlSys;
//==========================================================================================
void setup(void);
void setup_II(void);
void setup_III(void);

void setup_pin_UART(void);
signed char setup_SoftAPMode(char phase);
signed char setup_StaMode(char phase);
void setup_mySoftAP(void);
void setup_variables(void);
void rebootEsp(const char * pCh);
#ifdef USE_OTA
void SetupOTA(void);
#endif
void PrintStations(void);
void loop(void);
void StartServer(void);
signed char StartSpiffsFileSystem(void);

//void HandleUserLog(void);
//void HandleUserSettings(void);
//==========================================================================================
//==========================================================================================

void setup(void)
{
    char ct, ix;
    int i, j;
    char dt[2];
    signed char r;

    setup_pin_UART();

#ifdef EN_SOFT_AP
    setup_SoftAPMode(HOST_NAME__SETUP);
#endif
    for (i = 0; i < 2; i++) {
#ifdef EN_SOFT_AP
        r = setup_SoftAPMode(WIFI_MODE__SETUP);
#else
        r = setup_StaMode(WIFI_MODE__SETUP);
#endif    
        if( r == RESULT_OK ) break;
        else {
            //delay(600); //23.12.25: found long delay kicks watchdog most time.
            for( j = 0; j < 6; j++ ) {
                delay(100);
            }
            wdt_reset();   // Nov. 7, 2023 added.
        }
    }

    if( r < RESULT_OK ) {
        rebootEsp("+Setting WiFi mode Failed!\r\n   Rebooting...");
    }
}



//==========================================================================================
void setup_II(void) {
    signed char r;

#ifdef USE_YIELD_FUNC
    yield(); // Nov. 3, 2023: added to avoid a watchdog timeout.
#endif

    setup_variables();

    if ( (StartSpiffsFileSystem()) < 0) {
        rebootEsp((const char *) 0);
    }
    wdt_reset();

    StartServer();

    wdt_reset();

#ifdef EN_SOFT_AP // Nov. 7, 2023 added to enclose following #if/#else/#endif.
    r = setup_SoftAPMode(WIFI_CONFIG__SETUP);
#else
    r = setup_StaMode(WIFI_CONFIG__SETUP);
#endif
    if( r < RESULT_OK ) {
        rebootEsp((const char *) 0);
    }

//     //delay(600);  //23.12.25: found long delay kicks watchdog most time. //Jun. 20, 2020: the delay time, 600 msec, seems to be optimal value in my experiment.
//     for( j = 0; j < 6; j++ ) {
//         delay(100);
// #ifdef USE_YIELD_FUNC
//         yield();
// #endif
//     }
    wdt_reset();
#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    digitalWrite(STATUS_LED_RED, MY_LED_OFF);
#endif
}


//==========================================================================================
void setup_III(void) {
    int i, j;

#ifdef USE_OTA
    SetupOTA(); //2018.6.24: Added to get the OTA function is initiated.
#elif defined EN_SOFT_AP
    {
        IPAddress myIP = WiFi.softAPIP();
        Serial1.print("+AP IP address: ");
        Serial1.println(myIP);

        Serial.print("+AP IP address: ");
        Serial.println(myIP);
    }
#else
    //Serial1.println("Ready");
    Serial1.print("IP address: ");
    Serial1.println(WiFi.localIP());

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif

#ifdef USE_SMARTHOME_DNS_SERVER
                /* setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
#endif

#ifdef EN_LATE_SERVER_START
    StartServer();
#endif //#ifdef EN_LATE_SERVER_START

#ifndef EN_SOFT_AP
    // Print the IP address to the UART0 again to make sure it has been printed on the PC terminal.
    Serial.println(WiFi.localIP());
#endif

#ifdef EN_LATE_INIT_FILE_SYSTEM
    if ( (StartSpiffsFileSystem()) < 0) return;
    wdt_reset();
#endif

#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    Serial1.println("+Prototype Box WiFi Server.");
#elif SERVER_ID == SERVER_ID_SIDE_LAB
    Serial1.println("+WiFi Server for Lab Door.");
#elif SERVER_ID == SERVER_ID_SIDE_DOOR
    Serial1.println("+WiFi Server for Side Door.");
#else
    Serial1.println("+WiFi Server for Main Door.");
#endif

    // Init to get client MAC address later.
    etharp_init();

#if 0
    ctrlSys.ctrlServerState = INIT__ST; // init
    ctrlSys.pollCnt = 0;           // init
    ctrlSys.bLockEn = false;       // init
    ctrlSys.lockTime = 0;          // init
    ctrlSys.cookieResetCntDwn = 0; // init
    user.nofValidUser = 0;         // init
    for (i = 0; i < NOF_MAX_VALID_USER_IDX; i++) user.paUser[i] = (WifiUserRecord_t*)0;  // init
    // May 27, 2020: replaced to apply new userWifiStatus value format.
    user.newUser.userWifiStatus = UNKNOWN_USER__USR_ST; // INVALID_USER_WIFI_STATUS; // init
    user.newUser.cookie = LOGIN_KEY_GEN_OFF; // init. This newUser.cookie is used to indicate whether key generation is requested or not.
#endif
    ////loadCredentials(); // Load WLAN credentials from network
    //delay(10);
    //Serial.print("\r\n+Version: ");
    //Serial.println(buildVer);
    //Serial.print("\r\n+Build date: ");
    //Serial.println(buildDate);
    //Serial.println(" ");

#ifdef USE_PING
    // Init buffers for UserStatus.
    InitUserStatusBuf();
    userIpStatus = 0;
    pingStatus = PING_ST_IDLE;
    pingCurrentIdx = PING_DEFAULT_INDEX;

    //pinger.OnReceive([](const PingerResponse& response)
    pinger.OnReceive(SmPingRspRx);
    pinger.OnEnd(SmPingEnd);
#endif    
}


//==========================================================================================
signed char setup_SoftAPMode(char phase)
{
    char ct, ix;
    int i, j;
    char dt[2];
    bool b;
    signed char r;
    
    switch( phase) {
    case HOST_NAME__SETUP:
        if( WiFi.hostname(WiFiHostname) == true ) r = RESULT_OK;
        else r = -1;
        break;

    case WIFI_MODE__SETUP:
        if( WiFi.mode(WIFI_AP_STA) == true ) r = RESULT_OK;
        else r = -1;
        break;
        
    case WIFI_CONFIG__SETUP:
        //// AP
        Serial1.println("\r\nStarting Access Point ...");
        r = RESULT_OK;
        // Start the access point
        for (i = 0; i < 2; i++) {
            //WiFi.softAPConfig(apIP, apGW, IPAddress(255, 255, 255, 0));
            b = WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
            if( b == true ) break;
            else    delay(10);
        }

        if( b == true ) {
            for (i = 0; i < 2; i++) {
                b = WiFi.softAP(APssid, APpassword);
                //ct = (char)WiFi.softAP(APssid);
                if( b == true ) break;
                else {
                    Serial1.println("'");
                    //delay(600); //23.12.25: found long delay kicks watchdog most time.
                    for( j = 0; j < 6; j++ ) {
                        delay(100);
                    }
                    wdt_reset();   // Nov. 7, 2023 added.
                    /*delay(5000);
                    ESP.restart();*/
                }
            }

            if( b == true ) {
                Serial1.print("+Access Point \"");
                Serial1.print(APssid);
                Serial1.println("\" started\r\n");
                
                IPAddress myIP = WiFi.softAPIP();
                Serial1.print("+AP IP address: ");
                Serial1.println(myIP);                
                // 23.12.30: got rid of delay.
                // //delay(600); //23.12.25: found long delay kicks watchdog most time.
                // for( j = 0; j < 6; j++ ) {
                //    delay(100);
                //}
                //wdt_reset();   // Nov. 7, 2023 added.

                Serial.print("+AP IP address: ");
                Serial.println(myIP);

# ifdef SOFT_AP_USE_PASSWORD
                Serial1.print("+Password: ");
                Serial1.println(APpassword);
# else
                Serial1.println("+No Password.");
# endif
                
            } else r = -2;
        } else r = -1;

        if (r < RESULT_OK) {
            Serial1.print("\r\n -Return Code: ");
            Serial1.println((int) r);
            Serial1.print("\r\n -Version: ");
            Serial1.println(buildVer);
            //delay(5); //2018.10.26: Added to try geting build date message not to be overwritten by following messages.
            Serial1.print("\r\n -Build date: ");
            Serial1.println(buildDate);

            Serial1.println("\r\n+AP Failed!\r\n   Rebooting...");
            // wdt_reset();   // Nov. 7, 2023 added.
            // //delay(5000); //23.12.25: found long delay kicks watchdog most time.
            // for( j = 0; j < 50; j++ ) {
            //     delay(100);
            // }
            // wdt_reset();   // Nov. 7, 2023 added.
            // ESP.restart();
        }
        break;

    default:
        r = -100;
        break;
    }

    return r;
}


//==========================================================================================
signed char setup_StaMode(char phase)
{
    char ct, ix;
    int i, j;
    char dt[2];
    bool b;
    signed char r;
    wl_status_t wst;
    
    switch( phase ) {
    case WIFI_MODE__SETUP:
        if( WiFi.mode(WIFI_STA) == true ) r = RESULT_OK;
        else r = -1;
        break;

    case WIFI_CONFIG__SETUP:        
        r = RESULT_OK;
#if MY_WIFI_AP == ALL_SSID__WIFI_IX
# ifdef USE_MULTI_WIFI_CONNECT
        ct = 0;
        for( i = 0; i < 2; i++ ) {
            if( (wifiMulti.addAP(SM_HOME_DEMO_SSID, SM_HOME_DEMO_PW)) == true ) break;
        }
        
        if( i < 2 ) {
            ct++;
            //wifiMulti.addAP("Second Cup - WiFi", "secondcup2018");    // Nov. 6, 2023: no longer accessible.
            for( i = 0; i < 2; i++ ) {
                if( (wifiMulti.addAP("Tim Hortons WiFi", "")) == true ) break;
            }

            if( i < 2 ) {
                ct++;
                for( i = 0; i < 2; i++ ) {
                    if( (wifiMulti.addAP("Starbucks WiFi", "")) == true ) break;
                }

                if( i < 2 ) {
                    ct++;
                    for( i = 0; i < 2; i++ ) {
                        if( (wifiMulti.addAP("Toronto Public Library", "")) == true) break;
                    }

                    if( i < 2 ) {
                        ct++;
                        for( i = 0; i < 2; i++ ) {
                            if( (wifiMulti.addAP(SM_HOME_SSID, SM_HOME_PW)) == true ) break;
                        }

                        if( i < 2 ) ct++;
                        else r = -5;
                    } else r = -4;
                } else r = -3;
            } else r = -2;
        } else r = -1;
            
        //wifiMulti.addAP("AP-TO", "ch@pst1ck");    // Nov. 6, 2023: no longer accessible.
        //wifiMulti.addAP("gGalaxy", "pi@coffeeshop"); //Sep. 12, 2020: no longer available.
        Serial1.println("Connecting Wifi...");
        for (i = 0; i < ct; i++) {
            if (wifiMulti.run() == WL_CONNECTED) {
                Serial1.println("");
                Serial1.println("WiFi connected");
                Serial1.println("IP address: ");
                Serial1.println(WiFi.localIP());
                r = RESULT_OK;
                break;
            } else {
                Serial1.println("Connection Failed...");
                //delay(1000); //23.12.25: found long delay kicks watchdog most time.
                for( j = 0; j < 10; j++ ) {
                    delay(100);
                }
                wdt_reset();
                r = -10;
            }
        }

        Serial1.println("");

# else  //# ifdef USE_MULTI_WIFI_CONNECT
    //// Station
#  if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
        for (i = 0; i < NOF_WIFI_SSID; i++) {
            ct = 0;
            r = RESULT_OK;
            ssid = aSsid[i][0];
            password = aSsid[i][1];
            Serial1.print("+Connecting to ");
            Serial1.println(ssid);

            wst = WiFi.begin(ssid, password);

            if( wst == WL_NO_SSID_AVAIL ) {
                delay(100);
                wst = WiFi.begin(ssid, password);
            
                if( wst == WL_NO_SSID_AVAIL ) {
                    r = -1;
                    Serial1.println("*");
                    continue;
                }
            }

            while ( wst != WL_CONNECTED ) {
                //delay(500); //23.12.25: found long delay kicks watchdog most time.
                for( j = 0; j < 5; j++ ) {
                    delay(100);
#   ifdef USE_YIELD_FUNC
                    // Nov. 6, 2023: added to replace long delay with following 'for' loop. 
                    yield();
#   endif
                }
                Serial1.print(".");
                wdt_reset();

                if (ct > NOF_WAIT_FOR_WIFI_CONNECT) {
                    r = -2;
                    Serial1.println("*");
                    break;
                } else {
                    wst = WiFi.status();
                    if( wst == WL_CONNECTED ) {
                        r = RESULT_OK;
                        //break;
                    } else if( wst == WL_NO_SSID_AVAIL || wst == WL_CONNECT_FAILED ) {
                        r = -3;
                        ct = NOF_WAIT_FOR_WIFI_CONNECT + 1;
                        break;
                    } else ct++;
                }
            }   // while ( wst != WL_CONNECTED )
            //if (ct <= NOF_WAIT_FOR_WIFI_CONNECT) break;
            if( r == RESULT_OK) break;

        }   // for (i = 0; i < NOF_WIFI_SSID; i++)

#  else //#  if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
        ct = 0;
        Serial1.print("+Connecting to ");
        Serial1.println(ssid);
        WiFi.begin(ssid, password);
        wst = WiFi.begin(ssid, password);

        if( wst == WL_NO_SSID_AVAIL ) {
            delay(100);
            wst = WiFi.begin(ssid, password);
        
            if( wst == WL_NO_SSID_AVAIL ) {
                r = -1;
                Serial1.println("*");
            }
        }

        if( r == RESULT_OK ) {
            while (wst != WL_CONNECTED) {
                //delay(500); //23.12.25: found long delay kicks watchdog most time.
                for( j = 0; j < 5; j++ ) {
                    delay(100);
#   ifdef USE_YIELD_FUNC
                    // Nov. 6, 2023: added to replace long delay with following 'for' loop.
                    yield();
#   endif
                }

                Serial1.print(".");
                if (ct > NOF_WAIT_FOR_WIFI_CONNECT) {
                    r = -2;
                    Serial1.println("*");
                    break;
                } else {
                    wst = WiFi.status();
                    if( wst == WL_CONNECTED ) {
                        r = RESULT_OK;
                        //break;
                    } else if( wst == WL_NO_SSID_AVAIL || wst == WL_CONNECT_FAILED ) {
                        r = -3;
                        //ct = NOF_WAIT_FOR_WIFI_CONNECT + 1;
                        break;
                    } else ct++;
                }
            }   // while (wst != WL_CONNECTED)
        }
#  endif    //#  else //#  if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX

        Serial1.println("");
        //if (i < NOF_WIFI_SSID) {
        if( r == RESULT_OK ) {
            Serial1.println("+WiFi connected");
        } else {
            Serial1.println("+Failed WiFi connection");
        }
# endif // #else of '# ifdef USE_MULTI_WIFI_CONNECT'
#else   // #if MY_WIFI_AP == ALL_SSID__WIFI_IX
        Serial1.print("\r\nConnecting to ");
        Serial.print("\r\nConnecting to ");

# if MY_WIFI_AP == SEL_GPIO__WIFI_IX 
        // * Access Point Selection Guide:
        //   ---------------------------------------------------------------------------------------
        //   |          GPIO16            |                      Acces Point                       |
        //   ---------------------------------------------------------------------------------------
        //   |  Open (Internal Pull-down) |  LaonMakers, [YOUR HOME ROUTER SSID]                   |
        //   |  Vcc                       | 'Toronto Public Libruary', 'Second Cup', TimHortons    |
        //   ---------------------------------------------------------------------------------------

#  if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
        if ((digitalRead(AP_SELECT_PIN)) == 0) ix = 0; // for both LaonMakers and KJ
        else ix = 2; // for Libruary, SecondCup, TimHortons.
#  else
        ix = 1; // for KJ //for Darhn
#  endif
        dt[0] = ix + 0x30;
        dt[1] = 0;
        Serial1.println((const char *) dt);

        //if (ct < NOF_WIFI_SSID) ix = (int)ct;
        //else ix = 0; // set it to default 'LaonMakers'

        
        Serial1.println((const char *) aSsid[ix][0]);
        Serial1.println((const char *) aSsid[ix][1]);

        Serial.println((const char*)aSsid[ix][0]);
        Serial.println((const char*)aSsid[ix][1]);

        //delay(50);
        wst = WiFi.begin((const char*)aSsid[ix][0], (const char*)aSsid[ix][1]);
        if( wst == WL_NO_SSID_AVAIL ) {
            delay(100);
            wst = WiFi.begin((const char*)aSsid[ix][0], (const char*)aSsid[ix][1]);
        
            if( wst == WL_NO_SSID_AVAIL ) {
                r = -1;
                Serial1.println("*");
            }
        }

        

# else //#  if MY_WIFI_AP == SEL_GPIO__WIFI_IX
        Serial1.println(ssid);
        Serial1.println(password);
        
        //delay(50);
        
        wst = WiFi.begin(ssid, password);
        if( wst == WL_NO_SSID_AVAIL ) {
            delay(100);
            wst = WiFi.begin(ssid, password);
        
            if( wst == WL_NO_SSID_AVAIL ) {
                r = -1;
                Serial1.println("*");
            }
        }
# endif
        //delay(100);
        if( r == RESULT_OK ) {
            ct = 0;
            i = ix;
            //while ( (WiFi.status()) != WL_CONNECTED) {
            while ( wst != WL_CONNECTED ) {

                if (ct > NOF_WAIT_FOR_WIFI_CONNECT ) {
                    Serial1.println("*");
            
# if MY_WIFI_AP == SEL_GPIO__WIFI_IX
                    ////while ( (WiFi.waitForConnectResult()) != WL_CONNECTED) {
                    //while ((WiFi.status()) != WL_CONNECTED) {
                    while ( wst != WL_CONNECTED ) {
                        wdt_reset();

                        if (ct > NOF_WAIT_FOR_WIFI_CONNECT) {

                            ix++;
                            if (i < 2) if (ix > 1) ix = NOF_WIFI_SSID;

                            if (ix >= NOF_WIFI_SSID) {
                                r = -11;
                                Serial1.println("+Connection Failed!\r\n   Rebooting...");
                                Serial.println("+Connection Failed!\r\n   Rebooting...");
                                // delay(2000);
                                // ESP.restart();
                                break;
                            } else {

                                Serial1.print("\r\nConnecting to ");
                                Serial.print("\r\nConnecting to ");

                                Serial1.println((const char*)aSsid[ix][0]);
                                Serial1.println((const char*)aSsid[ix][1]);

                                Serial.println((const char*)aSsid[ix][0]);
                                Serial.println((const char*)aSsid[ix][1]);

                                //delay(50);
                                wst = WiFi.begin((const char*)aSsid[ix][0], (const char*)aSsid[ix][1]);
                                if( wst == WL_NO_SSID_AVAIL ) {
                                    delay(100);
                                    wst = WiFi.begin((const char*)aSsid[ix][0], (const char*)aSsid[ix][1]);
                                
                                    if( wst != WL_NO_SSID_AVAIL ) {
                                        ct = 0;
                                    }
                                } else ct = 0;
                            }
                            //break;

                        } else {
                            
                            Serial1.print("x");

                            wst = WiFi.status();
                            if( wst == WL_CONNECTED ) { // it will get exited both inner 'while' and outer one.
                                r = RESULT_OK; 
                                //break;
                            } else if( wst == WL_NO_SSID_AVAIL || wst == WL_CONNECT_FAILED ) {
                                //r = -3;
                                ct = NOF_WAIT_FOR_WIFI_CONNECT + 1; // to get it moved to the next ssid
                                //break;
                            } else {
                                // Nov. 6, 2023: added to replace long delay with following 'for' loop.
                                for( j = 0; j < 5; j++ ) { 
                                    delay(100);
#  ifdef USE_YIELD_FUNC
                                    yield();
#  endif
                            }

                            ct++;
                            }
                        }
                    }   // inner 'while ( wst != WL_CONNECTED )'

                    if( r != RESULT_OK) break;

# else //#  if MY_WIFI_AP == SEL_GPIO__WIFI_IX
                    r = -3;
                    break;
# endif

                } else {
                    wst = WiFi.status();
                    if( wst == WL_CONNECTED ) { // it will get exited the outer 'while' loop.
                        r = RESULT_OK;
                        //break;
                    } else if( wst == WL_NO_SSID_AVAIL || wst == WL_CONNECT_FAILED ) {
                        //r = -3;
                        ct = NOF_WAIT_FOR_WIFI_CONNECT + 1; // to get it entered the 2nd 'while' loop.
                        //break;
                    } else {
                        wdt_reset();
                        //delay(500); //23.12.25: found long delay kicks watchdog most time.
                        for( j = 0; j < 5; j++ ) {
                            delay(100);
# ifdef USE_YIELD_FUNC
                            yield();
# endif
                        }
                        ct++;
                    }
                }
                Serial1.print(".");
            }   //  outer 'while ( wst != WL_CONNECTED )'
        }       // if( r == RESULT_OK )
#endif  // #else //#if MY_WIFI_AP == ALL_SSID__WIFI_IX
        break;

    default:
        r = -100;
        break;

//     case 3:
// #if MY_WIFI_AP != SEL_GPIO__WIFI_IX
//         ct = 0;//
//         //while ( (WiFi.waitForConnectResult()) != WL_CONNECTED) {
//         while ((WiFi.status()) != WL_CONNECTED) {
            
//             if (ct > 6) {
//                 Serial1.println("+Connection Failed!\r\n   Rebooting...");
//                 Serial.println("+Connection Failed!\r\n   Rebooting...");
//                 //delay(2000); //23.12.25: found long delay kicks watchdog most time.
//                 for( j = 0; j < 20; j++ ) {
//                     delay(100);
//                 }
//                 ESP.restart();
//             }

//             Serial1.print("x");
//             //delay(500); //23.12.25: found long delay kicks watchdog most time.
//             for( j = 0; j < 5; j++ ) {
//                 delay(100);
// #  ifdef USE_YIELD_FUNC
//                 yield();
// #  endif
//             }
//             ct++;
//         }
// #endif //# if MY_WIFI_AP != SEL_GPIO__WIFI_IX
    }   // switch (phase)

    return r;
}


//==========================================================================================
void setup_pin_UART(void)
{
    char ct, ix;
    int i, j;
    char dt[2];

    // prepare GPIO2
    //pinMode(2, OUTPUT);
    //digitalWrite(2, 0);
    // initialize digital pin STATUS_LED_GREEN as an output.
#if !defined LOW || !defined HIGH || !defined LOCK_SMART_DOOR
# error "One or more of LOW, HIGH, or LOCK_SMART_DOOR is defined."
#endif
    digitalWrite(SMART_DOOR_CTRL_PIN, LOCK_SMART_DOOR); // Default door lock status is off.
    pinMode(SMART_DOOR_CTRL_PIN, OUTPUT);
    pinMode(AP_SELECT_PIN, INPUT_PULLDOWN_16);
    InitSmartDoor();
    
#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    digitalWrite(STATUS_LED_RED, MY_LED_ON);
    //digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);
    digitalWrite(STATUS_LED_BLUE, MY_LED_OFF);
    pinMode(STATUS_LED_RED, OUTPUT);
    //pinMode(STATUS_LED_GREEN, OUTPUT);
    pinMode(STATUS_LED_BLUE, OUTPUT);
    
#endif
    digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);
    pinMode(STATUS_LED_GREEN, OUTPUT);
    
    Serial.begin(57600); //Jul.29, 2018: baudrate 115200 has been replaced with 57600 since it is connected to the server Atmeg128 and the server cannot support 115200.
    Serial1.begin(115200); // Sep. 13, 2020: added to print debug msg through GPIO02
}


//==========================================================================================
void rebootEsp(const char * pCh) {
    if( pCh != 0 ) {
        Serial1.println(pCh);
        Serial.println(pCh);
    }

    for( char i = 0; i < 20; i++ ) {
        delay(100);
    }
    ESP.restart();
}



//==========================================================================================
void setup_variables(void)
{
    int i;

    //delay(10);
    Serial1.print("\r\n+Version: ");
    Serial1.println(buildVer);
    Serial1.print("\r\n+Build date: ");
    Serial1.println(buildDate);
    Serial1.println(" ");

    Serial.print("\r\n+Version: ");
    Serial.println(buildVer);
    Serial.print("\r\n+Build date: ");
    Serial.println(buildDate);
    Serial.println(" ");    

    Serial1.println(ESP.getResetReason());

    ctrlSys.ctrlServerState = INIT__ST; // init
    ctrlSys.pollCnt = 0;           // init
    ctrlSys.bLockEn = false;       // init
    ctrlSys.lockTime = 0;          // init
    ctrlSys.cookieResetCntDwn = 0; // init

#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX    
    ctrlSys.growMode = 0;               // init
    ctrlSys.growLightIntense = GROW_INIT_INTENSITY;    // init
    ctrlSys.t2ChangeLightIntense = 0;   // init
    ctrlSys.t2UdtCookieResetCntDwn = 0; // init
# endif

    user.nofValidUser = 0;         // init    
    for (i = 0; i < NOF_MAX_VALID_USER_IDX; i++) user.paUser[i] = (WifiUserRecord_t*)0;  // init
    // May 27, 2020: replaced to apply new userWifiStatus value format.
    user.newUser.userWifiStatus = UNKNOWN_USER__USR_ST; // INVALID_USER_WIFI_STATUS; // init
    user.newUser.cookie = LOGIN_KEY_GEN_OFF; // init. This newUser.cookie is used to indicate whether key generation is requested or not.
}


//==========================================================================================
void StartServer(void) {
    server.on("/", HandleRoot);
#ifdef USE_DOOR_FOLDER_AS_DOOR_CTRL_PAGE
    server.on("/door", HandleDoor);
#endif
    server.on("/login", HandleLogin);
    //server.on("/login.js", HTTP_GET, HandleLoginJs);
    server.on("/xml", HandleXML); // Send the status of the door lock to its client.
    server.on("/ustatus", HandleUserStatus);
    server.on("/admin", HandleAdmin);
    server.on("/keycodegen", HandleLoginCodeGen);


#ifdef USE_DOOR_CONTROL_PAGE_DIRECT_ACCESS
    server.on("/doorCtrl", ShowDoorCtrlPage);
#endif
    //server.on("/inline", []() {
    //    server.send(200, "text/plain", "this works without need of authentification");
    //    digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
    //});

    //server.on("/smhome.js", HTTP_GET, HandleSmHomeJs);
    //server.on("/smhome.css", HTTP_GET, HandleSmHomeCSS);

    server.on("/usettings", HandleUserSettings);
    server.on("/usettings.js", HTTP_GET, HandleUSettingsJs);

    server.on("/ulog", HandleUserLog);

    server.on("/test", HandleTestMenu);

    server.onNotFound(HandleNotFound);
    // server.onNotFound([]() {               // If the client requests any URI
    // if (!HandleSmHomeFileRead(server.uri()))     // send it if it exists
    //     HandleNotFound(); // otherwise, respond with a 404 (Not Found) error
    // });

    // //examples
    // server.serveStatic("/", SPIFFS, "/index.html");
    // server.serveStatic("/index.html", SPIFFS, "/index.html");
    // server.serveStatic("/css/fonts.css", SPIFFS, "/css/fonts.css");
    // server.serveStatic("/images/favicon.png", SPIFFS, "/images/favicon.png");
    // server.serveStatic("/js/jquery-2.1.4.min.js", SPIFFS, "/js/jquery-2.1.4.min.js");

    //TTEST: Must be deleted in release. for debugging purpose only.
    server.on("/logincodegen", HandleLoginCodeGen);

    //here the list of headers to be recorded    
    const char* headerkeys[] = { "User-Agent","Cookie" };
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    //ask server to track these headers
    server.collectHeaders(headerkeys, headerkeyssize);

    // Start the server
    server.begin();
    Serial1.println("+Server started");
}


//==========================================================================================
signed char StartSpiffsFileSystem(void) {
    int i;
    Serial1.println("+Mounting File Sys...");
#ifdef USE_YIELD_FUNC
    // Nov. 6, 2023: added to replace long delay with following 'for' loop.
    for( i = 0; i < 10; i++ ) { 
        delay(100);
        yield();
    }
#else
    //delay(1000); //23.12.25: found long delay kicks watchdog most time.
    for( i = 0; i < 10; i++ ) {
        delay(100);
        wdt_reset();
    }
#endif
    digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)

    if (!SPIFFS.begin()) {
        Serial1.println("+Failed to mount file system");
        return -1;
    } else {
        Serial1.println("+Mounted !");
    }

#ifndef USE_DEFAULT_INDEX_PAGE
    if ((!SPIFFS.exists("/index.html")) ||
        (!SPIFFS.exists("/doorctrl.html")) ||
        //(!SPIFFS.exists("/smhome.css")) ||
        //(!SPIFFS.exists("/smhome.js")) ||
        (!SPIFFS.exists("/login.html")) ||
        //(!SPIFFS.exists("/login.js")) ||
        (!SPIFFS.exists("/ustatus.html")) ||
        (!SPIFFS.exists("/usettings.html")) ||
        (!SPIFFS.exists("/usettings.js"))) {
        Serial1.println("+file open failed");
    }
#endif
    return 0;
}


//==========================================================================================
// This fucntion has been copied from BasicOTA.ino project. Original function name is setup().
#ifdef USE_OTA
void SetupOTA(void)
{

// Jun. 20, 2020: moved following statements to the place where SetupOTA() is invoked in order to disable OTA feature by compiler option.
//#ifndef EN_SOFT_AP
//    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
//        Serial1.println("+Connection Failed!\r\n   Rebooting...");
//        delay(5000);
//        ESP.restart();
//    }
//#endif

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial1.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial1.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial1.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial1.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial1.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial1.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial1.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial1.println("End Failed");
    });
    ArduinoOTA.begin();

    //Serial1.println("Ready");
    Serial1.print("IP address: ");//
    Serial1.println(WiFi.localIP());
}
#endif

//==========================================================================================
#ifdef EN_SOFT_AP
void PrintStations(void) {
    static int prevNumber = 0;
    if (WiFi.softAPgetStationNum() != prevNumber) {
        prevNumber = WiFi.softAPgetStationNum();
        Serial1.print(prevNumber);
        Serial1.println(" station(s) connected");
    }
}
#endif

//==========================================================================================
void loop(void)
{
    unsigned char i, j;
    unsigned long t;
    //bool bTm = false;
#ifdef USE_SMARTHOME_DNS_SERVER
    static bool bDnsInit = false;
#endif
    static char ixSetupInit = 0;

    if( ixSetupInit < 2 ) {        
        if( ixSetupInit == 0 ) {
            setup_II();            
        } else if( ixSetupInit == 1 ) {
            setup_III();
        }
        wdt_reset();
        ixSetupInit++;
        return;
    }

    //for (i = 0; i < NOF_BYTE_RX_CMD_BUF; i++) {
    for (i = 0; i < 10; i++) { // reduced the repeat number to see if it reduces server crashing.
        if ((Serial.available()) > 0) {
#ifdef DBG_PRINT_RX_IND_MSG
            Serial1.println("+");
#endif
            // get incoming byte:        
            if ((HandleUartMsg()) == true) {
                wdt_reset();

                // Added to not to miss door lock time since missing will cause critical heat damage to the door strike solenoid.
                if (ctrlSys.bLockEn == true) {
                    t = millis();
                    //t &= 0x0000FC00;
                    //t &= 0x0003FC00; 
                    
                    // If 'j' gets greater than ctrlSys.lockTime before the comparison in the followed 'if' statement 
                    // due to accidentally too much delay before this statement, after 16 seconds, these two values
                    // become idnetical again by '& 0x0F' operation.
                    j = ((unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC)) & LOCK_TIME_MISS_RECOVERY; // 0x0F;
                    // ctrlSys.lockTime = (unsigned char) (t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC) is to get 8 bits of 't' starting from bit 12.
                    if ( ctrlSys.lockTime == j) {
                        LockTheDoor(true);
                        ctrlSys.bLockEn = false;
                        SendLockRequest(true);
                    }
                }
            }

        } else {
            t = millis();
            //bTm = true;

            if (rxCmdIdx > 0) {
                if (t >= rxTimeOut) { // It may be rx timeout.
                    if ((t - rxTimeOut) <= 2000) { //rxTimeOut is not a rolled back value. Therefore it is rx timeout.
                        rxCmdIdx = 0;
                        rxCmdBuf[rxCmdIdx] = 0;
                    }
                } else if ((rxTimeOut - t) >= 2000) { // t has rolled over already.
                    rxCmdIdx = 0;
                    rxCmdBuf[rxCmdIdx] = 0;
                }
            } else {

                j = (unsigned char) (t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC); // t &= 0x0000FC00;

                
                if (ctrlSys.cookieResetCntDwn > 0) {
                    // If 'j' gets greater than ctrlSys.t2UdtCookieResetCntDwn before the comparison in the followed 'if' statement 
                    // due to accidentally too much delay before this statement, after 32 seconds, these two values
                    // become idnetical again by '& 0x1F' operation.

                    j &= COOKIE_COUNT_DWN_MISS_RECOVERY; // 0x1F;

                    if (j == ctrlSys.t2UdtCookieResetCntDwn) {
                        ctrlSys.t2UdtCookieResetCntDwn = (j + 1) & COOKIE_COUNT_DWN_MISS_RECOVERY; // 0x1F;
                        ctrlSys.cookieResetCntDwn--;
                        if (ctrlSys.cookieResetCntDwn == 0) { // reset all cookie values; LAONID.                             
                            // make sure the variable 'i' value is not used after this 'for' loop 
                            // because outer 'for' loop is using the variable.
                            for (i = 0; i < user.nofValidUser; i++) user.paUser[i]->cookie = 0;
                        }
                    }
                }

                j &= RESEND_COUNT_DWN_MISS_RECOVERY; // 0x1F;

                if (ctrlSys.toResend.cntDwn > 0) {

                    if (j == ctrlSys.toResend.nextTime) {
                        switch (ctrlSys.toResend.cmd) {
                        case CMD_POLL: // 0x20 in loop().                            
                            // Example of getting 3 valid wifi users' info. I have got it when the Wifi server is reset 
                            // while the Control Server is in Idle.
                            //`>08F7.20 01 03 3E   <== command from the Ctrl Server when the this WiFi Server is in initialization.
                            //'>08F7.20 01 03 3E   <== command from the Ctrl Server when ...
                            //'>08F7.20 01 03 3E   <== command from the Ctrl Server when ...
                            
                            //`>06F9 20 42 '<08F7.20 01 03 40
                            //`>07F8 93 00 CF  '<17E8.93 00 03 11 626967310000 3131313131313131 D2  // a request and response of the first valid wifi user's info.
                            //`>07F8 93 01 CE  '<17E8.93 01 03 22 736D616C6C32 3232323232323232 D0  // a request and response of the 2nd valid wifi user's info.
                            //`>07F8 93 02 CD  '<17E8.93 02 03 33 4C61756E6433 3333333333333333 DA  // a request and response of the 3rd valid wifi user's info.
                            //`>07F8 93 03 CC  '<09F6.93 00 03 03 CB <== // a request of the 4th valid wifi user's info. received an indication to no more valid wifi users. 

                            SendPollCommand(false);
                            ctrlSys.toResend.cntDwn--;
                            break;

                        case CMD_GET_VALID_WIFI_USER: // 0x93 in loop().
                            SendGetValidWifiUserCmd(false);
                            ctrlSys.toResend.cntDwn--;
                            break;
                        }

                        if (ctrlSys.toResend.cntDwn == 0) {
                            InitResendBuf();
                        }
                    }
                } else if (ctrlSys.ctrlServerState == INIT__ST) {
                    if (ctrlSys.toResend.cmd == CMD_UNKNOWN) {
                        //SendPollCommand(true);
                        //==> Apr. 25, 2020: to defer sending the first POLL commnad in order to get Wifi Server Init info printed on the PuTTy
                        ctrlSys.toResend.cmd = CMD_POLL;
                        ctrlSys.toResend.cntDwn = 3;
                        // next resend time is set to one second later.
                        ctrlSys.toResend.nextTime = (j + 3) & RESEND_COUNT_DWN_MISS_RECOVERY; // 0x1F;  // t &= 0x0000FC00;
                        //<==
                    } else {
                        InitResendBuf();
                    }
                } else if (ctrlSys.toResend.cmd != CMD_UNKNOWN) {
                    InitResendBuf();
                }
            }
            break;
        }
    }

    wdt_reset();
    yield();

    server.handleClient();
    wdt_reset();

    if (ctrlSys.bLockEn == true) {
        //if( bTm == false ) 
            t = millis();
        //t &= 0x0000FC00;
        j = ((unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC)) & LOCK_TIME_MISS_RECOVERY; // 0x0F;

        if ( j == ctrlSys.lockTime ) {
            LockTheDoor(true);
            ctrlSys.bLockEn = false;
            SendLockRequest(true);
        }
    } else if ((digitalRead(SMART_DOOR_CTRL_PIN)) > 0) { // Jun. 4, 2020: Bug fix: added following 'else if' in order to 
    // resolve the kept unlock state which happened 'Light On' was sent while the door was being unlocked by 'Unlock' command.
        
        LockTheDoor(true); // Lock the smart door because it is accidently unlocked now by the Wifi Server.
    }

#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    if (ctrlSys.growMode > 0) {
        wdt_reset();
        yield();

        t = millis();
        //t &= 0x00003F00;
        j = ((unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_125_mSEC)) & GROW_TIME_MISS_RECOVERY;        
        
        if (j == ctrlSys.t2ChangeLightIntense) { // about 0.25 sec has elapsed.

            ctrlSys.t2ChangeLightIntense = (j + 1) & GROW_TIME_MISS_RECOVERY;

            if (ctrlSys.growMode == 0x03) {
                analogWrite(STATUS_LED_GREEN, ctrlSys.growLightIntense);
                i = STATUS_LED_BLUE;

            } else if ((ctrlSys.growMode & 0x03) == 0x01) {
                i = STATUS_LED_GREEN;
            } else {
                i = STATUS_LED_BLUE;
            }

            analogWrite( i, ctrlSys.growLightIntense);

            if ((ctrlSys.growMode & 0x80) > 0) { // it is in the increment mode of the intensity value.

                if (ctrlSys.growLightIntense == GROW_INIT_INTENSITY) { // the LED is completely off state.
                    ctrlSys.growMode &= 0x7F; // set to the decrement mode.

                    if (ctrlSys.growMode == 0x11) {
                        //digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);
                        ctrlSys.growMode = 0x12;

                    } else if (ctrlSys.growMode == 0x12) {
                        //digitalWrite(STATUS_LED_BLUE, MY_LED_OFF);
                        ctrlSys.growMode = 0x11;
                    }

                    ctrlSys.t2ChangeLightIntense = (j + 2) & GROW_TIME_MISS_RECOVERY; // increase off time.

                } else {
                    ctrlSys.growLightIntense += GROW_INTENSITY_STEP;

                    if (ctrlSys.growLightIntense > GROW_INIT_INTENSITY) {

                        ctrlSys.growLightIntense = GROW_INIT_INTENSITY;
                    }
                }

            } else { // it is in the decrement mode of the intensity value.

                if (ctrlSys.growLightIntense == 0) {  // the LED is in the brightest state.
                    ctrlSys.growMode |= 0x80; // set to the increment mode.

                    if (ctrlSys.growMode == 0x11) {
                        ctrlSys.growMode = 0x12;

                    } else if (ctrlSys.growMode == 0x12) {
                        ctrlSys.growMode = 0x11;
                    }

                } else {

                    ctrlSys.growLightIntense -= GROW_INTENSITY_STEP;

                    if (ctrlSys.growLightIntense < 0) ctrlSys.growLightIntense = 0;
                }
            }
        }
    }
#endif

    /*
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
    return;
    }

    // Wait until the client sends some data
    //Serial.println("waiting new client");
    Serial.print(".");
    while(!client.available()){
    delay(1);
    }
    */

#ifdef USE_SMARTHOME_DNS_SERVER
    if (bDnsInit == false) {
        bDnsInit = true;
        // setup MDNS responder
        if (!MDNS.begin(WiFiHostname)) {
            Serial1.println("Error setting up MDNS responder!");
        } else {
            Serial1.println("mDNS responder started");
            // Add service to MDNS-SD
            MDNS.addService("http", "tcp", 80);
        }
    }

    //DNS
    dnsServer.processNextRequest();
#endif

    //server.handleClient();

#ifdef USE_OTA
    ArduinoOTA.handle(); //2018.6.24: Added to get it handle OTA request.
#endif

#ifdef USE_PING
    if ((pingStatus == PING_ST_REQ_COMPLETE) || (pingStatus > PING_ST_IN_PROGRESS)) {
        PrintNumOneByteDbg('s', pingStatus);
        PrintNumOneByteDbg('i', pingCurrentIdx);
        ExecPing();
        wdt_reset();
    } else if (pingStatus == PING_ST_CUR_REQ_DONE) {
        pingStatus = PING_ST_REQ_COMPLETE;
    }
#endif

#ifdef EN_SOFT_AP
    PrintStations();
#endif
}

//==========================================================================================
void InitResendBuf()
{
    ctrlSys.toResend.cmd = CMD_UNKNOWN;     // init/reset
    ctrlSys.toResend.cmdData = 0; // init/reset
    ctrlSys.toResend.arg = 0;     // init/reset
    ctrlSys.toResend.cntDwn = 0;     // init/reset
    ctrlSys.toResend.nextTime = 0;   // init/reset
}
//==========================================================================================
//==========================================================================================
