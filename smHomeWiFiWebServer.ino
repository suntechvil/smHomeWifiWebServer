
#include "wiring_private.h"
//Aug. 6, 2018: Arduino IDE seems to include all source c file and header files if these are in the same project folder
//              Therefore I'm commenting out this header include.
//#include "P:\Project\esp8266ex\SmartDoorWifiWebServer\SmartDoorWiFiWebServer.h"
#ifdef VS_PRJ
# include ".\laonSmHomeWiFiWebServer.h" //Aug. 6, 2018: Full path name is required since I have updated the Arduino board library (ESP8266 2.4.2) today.
#endif

#define USET_GET_ROOM_USERS       1
#define USET_GET_ONE_USER_INFO    10
#define USET_SCAN_FOB             20
#define USET_GET_NEW_USER_MAC     30
#define USET_SCAN_RFID_N_ADD      15
#define USET_SAVE_FOB_USER        25
#define USET_FORMAT_FOB_SEC_CODE  27
#define USET_SAVE_WIFI_USER       35
#define USET_GEN_LOGIN_KEY        37

//==> Shared with usettings_svr.js (nodeJs)
#define WIFI_CMD_LOGIN_TSID_REQ   1
#define WIFI_CMD_LOGIN_CODEGEN_PARAM_REQ 2
//<==
//==========================================================================================
extern const char * buildVer;
extern const char * buildDate;
// Create an instance of the server
// specify the port to listen on as an argument
//WiFiServer server(80);
extern ESP8266WebServer server;
extern const char * pIndexHtml;
extern const char * DFT_SMART_DOOR_INDEX_HTML;
extern const char * DFT_SMART_DOOR_TEST_HTML;
//extern const char dftAdminMac[DFT_NOF_ADMIN_MAC][NOF_MAC_BYTE];
extern const char ** ppAdminMac;
extern const char * pAdminMac;

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

extern bool bDoorClosed; // Server's(ATmega's) door lock/unlock status. it keeps the status of door lock/unlock.
extern bool bLightOn;      // Server's(ATmega's) light status. it keeps the status of light.
extern byte command[];

#ifdef USE_PING
    extern Pinger pinger;
    //extern unsigned char userIp[MAX_NOF_ROOM__USER_STATUS][MAX_NOF_IP_PER_ROOM__USER_STATUS];
    //
    //// If the remote respondes, one of bits are set to 1. Otherwise 0. If two hosts that are stored in userIp[0][0] and userIp[1][1] responded to its ping,
    //// then both bit 0 and bit 3 are set to 1 ( userIpStatus = 0000 1001 ).
    //extern unsigned char userIpStatus;
    extern char pingStatus;
    extern char pingCurrentIdx;
#endif

//==========================================================================================
bool bNeedLogin = true;
UserSettingInfo_t uSettingInfo;
ValidWifiUserInfo_t user;
String webSite, javaScript, XML;

//==========================================================================================
//==========================================================================================
void AdminMenu(void);
//void HandleAdmin(void);
//void HandleLogin(void);
//void HandleNotFound(void);
//void ShowDoorCtrlPage(void);
//void HandleTestMenu(void);
//void HandleRoot(void);
void setup(void);
void SetupOTA(void);
void loop(void);
void PrintStations(void);
//void HandleTestMenu(void);
bool Is_NewLaonId(unsigned short id);

bool ReadAndSendFile(  String file, String contType );
String GetSmHomeContentType( String filename );
bool HandleSmHomeFileRead( String path );
//void HandleSmHomeJs(void);
//void HandleSmHomeCSS(void);
//void HandleSystem(void);
//void HandleUserLog(void);
//void HandleUserSettings(void);

unsigned char ParseBCD2Num(unsigned char n);
const char * NumToString(unsigned char n, char * dt);
const char * NumToMin2DigitString(unsigned char n, char * dt, bool b2D);
const char * NumTo2DigitHexString(unsigned char n, char * dt);
const char* NumTo4DigitHexString(unsigned short v, char* dt);
//unsigned short GetLaonId(String ck);
//==========================================================================================
//==========================================================================================


// void returnOK()
// {
//   server.sendHeader("Connection", "close");
//   server.sendHeader("Access-Control-Allow-Origin", "*");
//   server.send(200, "text/plain", "OK\r\n");
// }

////////////////////////////////////////////////////////////////////////


//==========================================================================================
//system management page, also called for disconnect
void HandleSystem(void)
{
    const String content =
        "<html><head></head>"
        "<body style='font-size:40px'>System Management.<br><hr>"
        "<blockquote>"
            "<p><a href='/usettings'>1. User Settings </a></p>"
            "<p><a href='/keycodegen'>2. Log-in Key Generation </a></p>"
            "<p><a href='/ulog'>3. Log </a></p>"
            "<p><a href='/ustatus'>4.User Status </a></p>"
            "<p><a href='/'>5. Door Control </a></p>"
        "</blockquote><hr><br><br>"
        "</body></html> ";

    server.sendContent("HTTP/1.1 200 OK\r\n"); //send new p\r\nage
    server.sendContent("Content-Type: text/html\r\n");
    server.sendContent("\r\n");
    server.sendContent(content);
}

///////////////////////////////////////////////////////////////////////
//admin page, also called for disconnect
void AdminMenu(void)
{
    unsigned short id;
    String cookie = server.header("Cookie");
    Serial1.println("Enter to Admin Menu");
        
    id = GetLaonId(cookie);

    if ( id >= VALID_SUPER_USER_II ) {
        if (server.hasArg("AdmSysMg")) {
            if ((ReadAndSendFile("/ustatus.html", "text/html")) == false) {
                HandleNotFound();
            }
        } else {
            HandleSystem();
        }
    } else if (id >= VALID_SUPER_USER_I) {
        HandleSystem();

    }
}


///////////////////////////////////////////////////////////////////////
//admin page, also called for disconnect
void HandleAdmin(void)
{

    if ((Is_admin_authenticated()) == INVALID_USER) {
        //String msg;

        //if ( (Is_superUser()) == false) {// May 24, 2020: it might not need so temporarily commented out.
            Serial1.println("Invalid Admin");
            if( (Is_authenticated()) > INVALID_USER) server.sendHeader("Location", "/");
            else server.sendHeader("Location", "/login");
            server.sendHeader("Cache-Control", "no-cache");
            //server.sendHeader("Set-Cookie", "LAONID=2");
            
        //} else {
        //    Serial1.println("Valid Admin");
        //    server.sendHeader("Location", "/admin");
        //    server.sendHeader("Cache-Control", "no-cache");
        //    ////server.sendHeader("Set-Cookie", "LAONID=" COOKIE_VALID_ADMIN_ID);
        //    //server.sendHeader("Set-Cookie", "LAONID=100");
        //    
        //}
        server.send(307);
    } else {
        AdminMenu();        
    }
    
}


//==> TTEST: Must be deleted in release. for debugging purpose only.
///////////////////////////////////////////////////////////////////////
//admin page, also called for disconnect
void HandleLoginCodeGen(void)
{
    unsigned char i;
    uint8_t n[] = { 'a','d','m','i','n', 0, 0, 0 };    
    //for (var i = 0; i < hpp.SIZE_INITIAL; i++) {
    for (i = 0; i < SIZE_INITIAL; i++) {
        user.newUser.initial[i] = n[i];
    }
    

    for (i = 0; i < SIZE_PASSWORD; i++) {
        user.newUser.password[i] = n[i];
    }

    
    if ((ReadAndSendFile("/logincodegen.html", "text/html")) == false) {
        HandleNotFound();

    } else user.newUser.cookie = LOGIN_KEY_GEN_IN_PROGRESS;
}
//<==

///////////////////////////////////////////////////////////////////////
//login page, also called for disconnect
void HandleLogin(void)
{
    String msg;
    String dir = "";
    unsigned char i, ix, uid;
    unsigned short sid; // , ck;
    char dt[5];
    const char* pId;

    //String content_2;

    //digitalWrite(STATUS_LED_GREEN, MY_LED_ON);    // turn the LED ON by making the voltage LOW
    
    sid = 0;
    if (server.hasHeader("Cookie")) {
      
        Serial1.print("Found cookie : ");
        msg = server.header("Cookie");
        Serial1.println();
        sid = GetLaonId(msg);
    }
    
    msg = "";

    if (server.hasArg("DISCONNECT")) {
        if (( sid > 0) && (sid < VALID_ADMIN_USER) ) { // general user
            
            for (ix = 0; ix < user.nofValidUser; ix++) {
                if (sid == user.paUser[ix]->cookie) { // found the user with the given cookie LAONID value.
                    user.paUser[ix]->cookie = 0;
                    break;
                }
            }
        }

        Serial1.println("Disconnection");
        server.sendHeader("Location","/login");
        server.sendHeader("Cache-Control","no-cache");
        server.sendHeader("Set-Cookie","LAONID=0");
        server.send(307);
        digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
        return;
    }
    
    if(server.hasArg("TSID")) { // valid login request from a wifi client.
        String str;
        sid = HexStrToUint16(server.arg("TSID"));

        if (server.hasArg("UID")) { // the wifi client has its own login code groups in its local storage.

            /*str = server.arg("UID");
            str.trim();
            Serial1.println(str);
            pCh = (char*)str.c_str();
            uid = *pCh++ - 0x30;
            if (*pCh != 0) {
                uid = (uid << 4) + (*pCh - 0x30);
            }*/

            uid = (uint8_t) HexStrToUint16(server.arg("UID"));

            if (user.nofValidUser > 0) {
                for (ix = 0; ix < user.nofValidUser; ix++) {

                    if (uid == user.paUser[ix]->userWifiStatus) {


                        if (Is_keycode_match(ix)) {
                            dir = "/";
                            sid &= MASK_VALID_USER;
                            
                            while (sid <= MASK_VALID_USER) {
                                if (Is_NewLaonId(sid)) {
                                    user.paUser[ix]->cookie = sid;
                                    break;
                                }
                                sid++;
                            }

                            if (sid > MASK_VALID_USER) { // it shouldn't happen often.
                                user.paUser[ix]->cookie = sid = MASK_VALID_USER;
                            }

                            //msg = "1";
                            pId = NumTo4DigitHexString(sid, dt);
                            msg = pId;
                        }
                        
                        if (user.newUser.cookie == LOGIN_KEY_GEN_ENABLED) { // login-key generation is enabled.
                            //server.writeHead(307, { 'Location': '/', 'Cache-Control': 'no-cache'});
                            for (i = 0; i < SIZE_INITIAL; i++) {
                                if (user.newUser.initial[i] != user.paUser[ix]->initial[i]) break;
                            }

                            if (i == SIZE_INITIAL) {
                                if ((ReadAndSendFile("/logincodegen.html", "text/html")) == false) {
                                    HandleNotFound();

                                } else user.newUser.cookie = LOGIN_KEY_GEN_IN_PROGRESS;

                                return;
                            }
                            //break;
                        }

                        if (msg != "") break;
                    }
                }
            }

        }  else if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {

            if (server.arg("USERNAME") == "admin") {

                if (server.arg("PASSWORD") == "admin") {
                    dir = "/";
                    //msg = "1";

                    sid |= VALID_ADMIN_USER;
                    sid &= MASK_VALID_USER;
                    //user.paUser[ix]->cookie = sid; //May 24, 2020: the buffer cookie is not available for 'admin' yet.
                    
                } else if ((server.arg("PASSWORD") == "admin1") || (server.arg("PASSWORD") == "admin2")) {
                    dir = "/";
                    //msg = "10";
                    sid |= VALID_ADMIN_USER;
                    sid &= MASK_VALID_USER;                    
                    
                } else if (server.arg("PASSWORD") == "admin100") {
                    dir = "/";
                    //msg = "100";
                    sid |= VALID_SUPER_USER_I;
                    sid &= MASK_VALID_SUPER_USER_I;                    

                } else if (server.arg("PASSWORD") == "admin101") {
                    dir = "/ustatus";
                    //msg = "101";
                    sid |= VALID_SUPER_USER_II;                    
                }

                if (dir != "") {
                    pId = NumTo4DigitHexString(sid, dt);
                    msg = pId;
                }

            } else if (user.nofValidUser > 0) {
                
                char* pCh;

                for (ix = 0; ix < user.nofValidUser; ix++) {
                    str = server.arg("USERNAME");
                    str.trim();
                    Serial.println(str);
                    pCh = (char*)str.c_str();

                    for (i = 0; i < SIZE_INITIAL; i++) {
                        if (*pCh == 0) {
                            if (i >= MIN_SIZE_INITIAL) {
                                i = SIZE_INITIAL;
                            }
                            break;
                        }
                        if (*pCh++ != user.paUser[ix]->initial[i]) break;
                    }

                    if (i != SIZE_INITIAL) continue;

                    str = server.arg("PASSWORD");
                    str.trim();
                    Serial.println(str);
                    pCh = (char*)str.c_str();

                    for (i = 0; i < SIZE_PASSWORD; i++) {
                        if (*pCh++ != user.paUser[ix]->password[i]) break;
                    }

                    if (i != SIZE_PASSWORD) continue;

                    if (Is_keycode_match(ix)) {
                        dir = "/";
                        sid &= MASK_VALID_USER;

                        while (sid <= MASK_VALID_USER) {
                            if (Is_NewLaonId(sid)) {
                                user.paUser[ix]->cookie = sid;
                                break;
                            }
                            sid++;
                        }

                        if (sid > MASK_VALID_USER) { // it shouldn't happen often.
                            user.paUser[ix]->cookie = sid = MASK_VALID_USER;
                        }

                        //msg = "1";
                        pId = NumTo4DigitHexString(sid, dt);
                        msg = pId;
                        //break;
                    }

                    if (user.newUser.cookie == LOGIN_KEY_GEN_ENABLED) { // login-key generation is requested.
                            //server.writeHead(307, { 'Location': '/', 'Cache-Control': 'no-cache'});
                        for (i = 0; i < SIZE_INITIAL; i++) {
                            if (user.newUser.initial[i] != user.paUser[ix]->initial[i]) break;
                        }

                        if (i == SIZE_INITIAL) {
                            if ((ReadAndSendFile("/logincodegen.html", "text/html")) == false) {
                                HandleNotFound();

                            } else user.newUser.cookie = LOGIN_KEY_GEN_IN_PROGRESS;

                            return;
                        }
                        //break;
                    }

                    if (msg != "") break;
                }
            }
        }

        if( msg != "" ) {
            unsigned long t;
            server.sendHeader("Location", dir);
            server.sendHeader("Cache-Control","no-cache");
            server.sendHeader("Set-Cookie","LAONID=" + msg);
            //server.sendHeader("Set-Cookie","HttpOnly"); //It has been set, but doesn't do what it promsed.
            server.send(307);
            Serial1.println("Log in Successful");
            
            t = millis();
            
            ctrlSys.t2UdtCookieResetCntDwn = (unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC);
            ctrlSys.t2UdtCookieResetCntDwn = (ctrlSys.t2UdtCookieResetCntDwn + 1) & COOKIE_COUNT_DWN_MISS_RECOVERY; //t &= 0x0000FC00;
            ctrlSys.cookieResetCntDwn = COOKIE_RESET_TIME_IN_SEC;

            //digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
            return;
        }
        
        
        //msg = "Wrong username/password! try again.";
        Serial1.println("Log in Failed");
        //content_2 = "";
        server.send(204);
        //server.end(); // compiler error; it is not undefined.
        return;
    } 
    //else {
    //    const char* pCs;
    //    char d[2];
    //    ck = (unsigned short)millis();
    //    content_2 = "<input type='text' id='tmpid' name='TSID' value='";
    //    i = (unsigned char)(ck >> 8);
    //    pCs = NumTo2DigitHexString(i, d);
    //    content_2 += pCs;
    //    i = (unsigned char)(ck >> 8);
    //    pCs = NumTo2DigitHexString(i, d);
    //    content_2 += pCs;
    //    content_2 += "' hidden><input type='text' id='keycode' name='KEYCODE' value='' hidden>";
    //}

    //// Don't use '<br>' in <style>. Test following html in https://www.w3schools.com before downloading.
    //const String content_1 =
    //    "<html><head><meta charset='UTF-8'><style>"
    //    " body { font-size:40px; background-color: lightblue;}" //" input[type=text] { font-size:50px; width:10em;}"                
    //    " input { font-size:50px; width:10em; padding-left:7px;}"
    //    " label#uname { width:4em; display:inline-block;}"
    //    "</style><script type='text/javascript'>"
    //    " var $ = function(id) { return document.getElementById(id);}"
    //    " function LoginKey(e) {"
    //    " var dt; var str; var i;"
    //    " if (typeof (Storage) !== 'undefined') {"
    //    "  str = localStorage.getItem('codeGrp0');"
    //    "  if (str > 0) {"
    //    "   dt = str.split(',');"
    //    "   if (dt.length > 0) {"
    //    "    i = parseInt($('tmpid').value) & 0x00FF; $('keycode').value = dt[i];"
    //    "   }}$('login').submit();}"
    //    "</script></head>"
    //    "<body>Welcome to Smart Door Control Server !<br>&nbsp;&nbsp; Please login.<br><br><hr><br>"
    //    "<blockquote><form id='login' action = '/login' method = 'POST'> "
    //    "<label for='uname'>User:</label><input type='text' name='USERNAME' id='uname' value='admin'><br><br>"
    //    "<label for='pwd'>Password:</label><input type='password' name='PASSWORD' id='pwd' value='admin'><br><br><br>";
    //
    //const String content_3 =
    //    //"<input type='submit' name='SUBMIT' value='Submit'><br><br>"
    //    "<input type='button' name='SUBMIT' onclick='LoginKey(event)' value='Submit'><br><br>"        
    //    "</form><i>Tip: to log in, please use : admin / admin. </i></blockquote><hr><br><br>";

    //String content_4 = msg + "<br>You also can go <a href='/inline'>here</a></body></html> ";
    //    //"User: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type='text' name='USERNAME' placeholder='user name'><br><br>"
    //    //"Password: <input type='password' name='PASSWORD' placeholder='password'><br><br><br>"
    //// 206: Partial Content (RFC 7233). The server is delivering only part of the resource (byte serving) due to 
    ////      a range header sent by the client. The range header is used by HTTP clients to enable resuming of 
    ////      interrupted downloads, or split a download into multiple simultaneous streams.
    //// server.send(206, "text/html", content_1);
    //// server.send(200, "text/html", content_2);
    //
    //// server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    //// server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    //server.sendContent("HTTP/1.1 200 OK\r\n"); //send new p\r\nage
    //server.sendContent("Content-Type: text/html\r\n");
    //server.sendContent("\r\n");
    //server.sendContent(content_1);
    //if(content_2 != "") server.sendContent(content_2);
    //server.sendContent(content_3);
    //server.sendContent(content_4);
    ////server.client().stop(); // Stop is needed because we sent no content length

    if ((ReadAndSendFile("/login.html", "text/html")) == false) {
        HandleNotFound();
    }
    Serial1.println(".");
}


///////////////////////////////////////////////////////////////////////
//Check if header is present and correct
unsigned char Is_keycode_match(uint8_t ix)
{
    uint8_t i;
    uint8_t rIx[2], kIx[2], rpt[2];

    uint8_t key[SIZE_KEY_CODE], pcode[2];    
    unsigned short pc, lgcode, tsid;
    //String str;

    
    lgcode = HexStrToUint16(server.arg("KEYCODE"));
    tsid = HexStrToUint16(server.arg("TSID"));

    rIx[0] = (tsid >> 8) & 0x0F;
    rIx[1] = tsid & 0x0F;
    kIx[0] = rIx[0] & 0x03;
    kIx[1] = rIx[1] & 0x03;

    key[0] = 0;
    for (i = 0; i < SIZE_INITIAL; i++) {
        key[0] += user.paUser[ix]->initial[i];
    }

    key[0] += (0xFF - user.paUser[ix]->initial[0]);

    key[1] = 0;
    for (i = 0; i < SIZE_PASSWORD; i++) {
        key[1] += user.paUser[ix]->password[i];
    }
    key[1] *= user.paUser[ix]->password[0];

    key[2] = key[0] + key[1];
    key[3] = key[1] * key[2];

    i = rIx[0] % SIZE_PASSWORD;
    // for MSB side pass code.
    if (rIx[0] < SIZE_PASSWORD) {
        rpt[0] = key[kIx[0]] + user.paUser[ix]->password[i];
    } else {
        rpt[0] = key[kIx[0]] + (0xFF - user.paUser[ix]->password[i]);
    }

    i = rIx[1] % SIZE_PASSWORD;
    // for LSB side pass code.
    if (rIx[1] < 8) {
        rpt[1] = key[kIx[1]] + user.paUser[ix]->password[i];
    } else {
        rpt[1] = key[kIx[1]] + (0xFF - user.paUser[ix]->password[i]);
    }

    // MSB side pass code.
    pcode[0] = rpt[0] * ((tsid >> 8) & 0xFF);

    // LSB side pass code.
    pcode[1] = rpt[1] * (tsid & 0xFF);

    pc = (pcode[0] << 8) + pcode[1];


    if (lgcode == pc) return VALID_USER; //hpp.VALID_SUPER_USER_I;

    return INVALID_USER;
}
///////////////////////////////////////////////////////////////////////
//Check if header is present and correct
unsigned short Is_authenticated()
{
    unsigned short id;
#ifdef GTC_DBG  
    Serial1.println("Enter Is_authenticated");
#endif
    
    id = INVALID_USER;

    if (server.hasHeader("Cookie")) {

        String cookie = server.header("Cookie");
        Serial1.println(cookie);
        
        id = GetLaonId(cookie);
        
#ifdef GTC_DBG
        Serial1.print("Found cookie: ");
         if ( id >= VALID_SUPER_USER_II) Serial1.println("Authenticated(4)");
         //if (cookie.indexOf("LAONID=" COOKIE_VALID_ADMIN_ID) != -1) {
         else if ( id >= VALID_SUPER_USER_I) Serial1.println("Authenticated(3)");
         else if (id >= VALID_ADMIN_USER) Serial1.println("Authenticated(2)");
         else if (id > INVALID_USER) Serial1.println("Authenticated(1)");
#endif

    }
    
    if(id == INVALID_USER ) Serial1.println("Authentification Failed");

    return id;
}

///////////////////////////////////////////////////////////////////////
//Check if header is present and correct
unsigned short Is_admin_authenticated()
{
    unsigned short id;
#ifdef GTC_DBG  
    Serial1.println("Enter Is_admin_authenticated");
#endif

    id = INVALID_USER;

    if (server.hasHeader("Cookie")) {
        String cookie = server.header("Cookie");

        id = GetLaonId(cookie);

#ifdef GTC_DBG
        Serial1.print("Found cookie: ");
        Serial1.println(cookie);
        if (id >= VALID_SUPER_USER_II) Serial1.println("Admin II Authenticated");
        //if (cookie.indexOf("LAONID=" COOKIE_VALID_ADMIN_ID) != -1) {
        else if (id >= VALID_SUPER_USER_I) Serial1.println("Admin I Authenticated");
#endif  
    }

    if (id < VALID_SUPER_USER_I) {
        id = INVALID_USER;
        Serial1.println("Admin Auth. Failed");
    }

    return id;
}

//==========================================================================================
//Check if header is present and correct
bool Is_superUser()
{
    char i, j;
    char mac[5];
    ip4_addr_t toCheck;
    struct eth_addr *ret_eth_addr = NULL;
    struct ip4_addr  const *ret_ip_addr = NULL;
    IPAddress IPtoCheck;
    int arp_find;

    //ppAdminMac = (const char **)dftAdminMac;
    
    IPtoCheck = server.client().remoteIP();

    Serial1.print("Connected from ");
    Serial1.println(IPtoCheck.toString());
    //Serial1.println(server.hostHeader());

    IP4_ADDR(&toCheck, IPtoCheck[0], IPtoCheck[1], IPtoCheck[2], IPtoCheck[3]);
    
    //etharp_request(netif_default, &toCheck);
    delay(20);
    arp_find = etharp_find_addr(netif_default, &toCheck, &ret_eth_addr, &ret_ip_addr);
    delay(20);
    //if (arp_find != -1 && ret_eth_addr != NULL && (Ping.ping(IPtoCheck, 1) || Ping.ping(IPtoCheck, 1))) {
    if (arp_find != -1 && ret_eth_addr != NULL) { // This if statment prevent it from crashing.
        Serial1.print("MAC address - ");
        
        for (i = 0; i < (NOF_MAC_BYTE-1); i++) {

            sprintf(mac, "%02x:", ret_eth_addr->addr[i]);
           
            Serial1.print(mac);
        }

        sprintf(mac, "%02x", ret_eth_addr->addr[i]);
        Serial1.println(mac);

        for (i = 0; i < DFT_NOF_ADMIN_MAC; i++) {
            //Nov. 17, 2019: Replaced to resolve link error in makeEspArduino.mk
			//pAdminMac = (const char *)dftAdminMac[i];
			pAdminMac = (const char *)ppAdminMac[i];
			
            for (j = 0; j < NOF_MAC_BYTE; j++) {
                //if (ppAdminMac[i][j] != ret_eth_addr->addr[j]) break; // ppAdminMac[i][j] got it crashed; const char ** ppAdminMac.
                if (*pAdminMac++ != ret_eth_addr->addr[j]) break;
                               
            }
            if (j >= NOF_MAC_BYTE) return true;
        }
    } else {
        Serial1.print("Failed to get MAC address");
    }
    return false;
}
///////////////////////////////////////////////////////////////////////
//send door lock/unlock control command.
/*void sendDoorCtrl(char on) {
    if( on == '1' ) {
        server.sendHeader("Location","/?DOORCTRL=1");
    } else {
        server.sendHeader("Location","/?DOORCTRL=0");
    }
    server.sendHeader("Cache-Control","no-cache");
    server.send(301);
    Serial1.println(">");
}*/

bool Is_NewLaonId(unsigned short id)
{
    unsigned char ix;
    for (ix = 0; ix < user.nofValidUser; ix++) {
        if (user.paUser[ix]->cookie == id) return false;
    }
    return true;
}
//==========================================================================================
bool ReadAndSendFile( String file, String contType )
{
    bool r = false;

#ifndef USE_DEFAULT_INDEX_PAGE
    if( SPIFFS.exists( file ) ) { // "/index.html"
    
        File f = SPIFFS.open( file, "r" );
        if (f) {
            size_t sz = server.streamFile(f, contType);
            // static String smHomeHtml = f.readString();
            // pF = smHomeHtml.c_str();
// # ifdef DBG_PRINT_INDEX_HTML
//             Serial1.println(smHomeHtml);
// # endif            
            //f.close(); //2018.11.08: Moved down.
            Serial1.println("File " + file + " has been sent");
            r = true;
        } else {
            //pF = DFT_SMART_DOOR_INDEX_HTML;
            server.send(200, "text/html", DFT_SMART_DOOR_INDEX_HTML );
            Serial1.println("+" + file + " file open failed");
        }
        f.close();
    } else {
        //pF = DFT_SMART_DOOR_INDEX_HTML;
        server.send(200, "text/html", DFT_SMART_DOOR_INDEX_HTML );
        Serial1.println("+" + file + " doesn't exist");        
    }
#endif

    return r;
}

//==========================================================================================
String GetSmHomeContentType( String filename )
{ // convert the file extension to the MIME type
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}


//==========================================================================================
bool HandleSmHomeFileRead( String path )
{ // send the right file to the client (if it exists)
    Serial1.println("HandleSmHomeFileRead: " + path);
    if (path.endsWith("/")) path += "login.html"; //"index.html"; // If a folder is requested, send the index file
    String contentType = GetSmHomeContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz)) {                    // If there's a compressed version available
            path += ".gz";                                         // Use the compressed verion
        }

        File file = SPIFFS.open(path, "r");                    // Open the file
        size_t sent = server.streamFile(file, contentType);    // Send it to the client
        file.close();                                          // Close the file again
        Serial1.println(String("\tSent file: ") + path);
        return true;
    }
    Serial1.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

//#define SEND_FILE_BY_SEND_CONTENTS
void ShowDoorCtrlPage(void) {
    bool r;
#ifdef SEND_FILE_BY_SEND_CONTENTS
    const char * pF;    
#endif
    
    // //server.send(200, "text/html", DFT_SMART_DOOR_INDEX_HTML);   
#ifdef USE_DEFAULT_INDEX_PAGE
    server.send(200, "text/html", DFT_SMART_DOOR_INDEX_HTML );
#else

# ifdef SEND_FILE_BY_SEND_CONTENTS

    File f = SPIFFS.open( "/doorctrl.html", "r" );
    String rdS = f.readString();
    pF = rdS.c_str();    

    server.sendContent("HTTP/1.1 200 OK\r\n"); //send new p\r\nage
    server.sendContent("Content-Type: text/html\r\n");
    server.sendContent("\r\n");
    server.sendContent(pF);
    //delay(1);
    f.close();

    // f = SPIFFS.open( "/smhome.css", "r" );
    // // String rdS = f.readString();
    // // pF = rdS.c_str();
    // pF = f.readString().c_str();

    
    // server.sendContent("Content-Type: text/css\r\n");
    // server.sendContent("\r\n");
    // server.sendContent(pF);
    // f.close();
    // delay(1);

    // f = SPIFFS.open( "/smhome.js", "r" );
    // // String rdS = f.readString();
    // // pF = rdS.c_str();
    // pF = f.readString().c_str();
    // server.sendContent("Content-Type: application/javascript\r\n");
    // server.sendContent("\r\n");
    // server.sendContent(pF);
    // delay(1);
    // f.close();

    
# else
    r = ReadAndSendFile( "/doorctrl.html", "text/html" ); 
    
    if( r == false ) {
        HandleNotFound();
    }
# endif
    
#endif
}

//==========================================================================================
void HandleUserSettings(void) {
    bool rlt = true;
    unsigned char idx;
    unsigned short v;
    char cmd[MAX_CMD_BUFFER_ARG_SIZE];
    
    if (server.hasArg("GETVAL")) {

#ifdef VS_PRJ
# pragma region handleUserSettings_GETVAL
#endif

        idx = (unsigned char) server.arg("GETVAL").toInt();
        
        
        switch (idx) { //Send a command to the door control/user's wifi device in order to get users' data.
        case USET_GET_ROOM_USERS:   // 1: Get both valid and blocked user IDs of the given room.
            
            cmd[0] = CMD_GET_ROOM_USER_INFO;
            cmd[1] = (char) server.arg("rmReq").toInt(); // Room index; 1: big room, 2: small room, 3: the room near laundary.            
            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[0]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[1]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.
            
            SendCommand(cmd, 2);

            //server.sendHeader("Location", "/usettings");
            //server.sendHeader("Cache-Control", "no-cache");
            //server.send(307);

            // 204: No Content. The server successfully processed the request and is not returning any content.
            // 205: Reset Content: The server successfully processed the request, but is not returning any content. 
            //      Unlike a 204 response, this response requires that the requester reset the document view.
            server.send(204);
            break;
        case USET_GET_ONE_USER_INFO:  // 10: Get all information of the given user ID.
            cmd[0] = CMD_GET_USER_INFO;
            cmd[1] = (char)server.arg("ruReq").toInt(); // Room index; 1: big room, 2: small room, 3: the room near laundary.            
            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[0]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[1]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.

            SendCommand(cmd, 2);

            server.send(204);
            break;

        case USET_SCAN_FOB:  // 20: Get the door control to scan a fob and to return its scanned RFID code.
            cmd[0] = CMD_SCAN_USER_FOB;
            cmd[1] = (char)server.arg("sfReq").toInt(); // Room index; 1: big room, 2: small room, 3: the room near laundary.            
            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[0]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[1]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.

            SendCommand(cmd, 2);

            server.send(204);
            break;

        case USET_GET_NEW_USER_MAC:  // 30: Get new user's wifi MAC. It will work only when the new user try accessing this server's default homepage.
            break;
        default:
            rlt = false;
            break;
        }

#ifdef VS_PRJ
# pragma endregion
#endif

    } else if (server.hasArg("SETVAL")) {

#ifdef VS_PRJ
# pragma region handleUserSettings_SETVAL
#endif

        String str;
        char * pCh;
        unsigned char i, j, k, l, n, r;

        idx = (unsigned char) server.arg("SETVAL").toInt();
        
        switch (idx) { //Send a command and data to the door control in order to get it to save give data to its internal/external eeprom.
        case USET_SCAN_RFID_N_ADD: // 15: Get all given information to be saved for the given user.
            cmd[0] = CMD_SCAN_AND_SAVE_FOB;
            cmd[1] = (char)server.arg("fsaReq").toInt(); // Room index; 1: big room, 2: small room, 3: the room near laundary.            
            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[0]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[1]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.

            SendCommand(cmd, 2);

            server.send(204);
            break;

        case USET_SAVE_FOB_USER: // 25: Get fob information saved for the given user.
            cmd[COMMAND__UPDATE_USR_IX] = CMD_SAVE_FOB_USER;
            
            //Oct. 9, 2019: 
            // fsfuReq:  '1;31:1;3100f1c1;170707;A;0;F;0'  for Laundry user #1 (0x31), card id: 3100f1c1, 
            //  date: 2017-0707, security code: 10, security update: off, update period: 15, delete: false.
            str = server.arg("fsfuReq");
            str.trim();
            
            Serial.println(str);

            pCh = (char * ) str.c_str();
            r = 0;
            // the number of record rows in the fob user table.
            if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
            else n = 0x30;
            cmd[REC_NUM__UPDATE_USR_IX] = (*pCh++) - n;
            
            if (*pCh != ';') {
                cmd[REC_NUM__UPDATE_USR_IX] <<= 4;

                if (*pCh > 0x39) n = 0x37;
                else n = 0x30;
                cmd[REC_NUM__UPDATE_USR_IX] += ((*pCh++) - n);

                if (*pCh != ';') r = 5;
            }
            pCh++;
            // user id.
            if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
            else n = 0x30;
            cmd[USER_IDX__UPDATE_USR_IX] = ((*pCh++) - n);
            
            if (*pCh != ':') {
                cmd[USER_IDX__UPDATE_USR_IX] <<= 4;

                if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
                else n = 0x30;
                cmd[USER_IDX__UPDATE_USR_IX] += ((*pCh++) - n);
            }

            for (i = 0, j = VALID_USER__UPDATE_USR_IX; i < cmd[REC_NUM__UPDATE_USR_IX]; i++) {
                
                // check the mark of the start of a record.
                if (*pCh++ != ':') {
                    r = 6;
                    break;
                }

                // valid user flag
                cmd[j++] = (*pCh++) - 0x30;

                if (*pCh++ != ';') {
                    r = 7;
                    break;
                }

                // fob no.
                for (k = 0; k < SIZE_CARD_ID; k++) {

                    if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    cmd[j] = ((*pCh++) - n) << 4;

                    if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    cmd[j++] += ((*pCh++) - n);
                }

                if (*pCh++ != ';') {
                    r = 8;
                    break;
                }

                // registration date. BCD value.
                for (k = 0; k < 3; k++) {

                    if (*pCh > 0x39) n = 0x37;  // it shouldn't happen. it must be one of '0' to '9'.
                    else n = 0x30;
                    cmd[j] = ((*pCh++) - n) << 4;

                    if (*pCh > 0x39) n = 0x37;  // it shouldn't happen. it must be one of '0' to '9'.
                    else n = 0x30;
                    cmd[j++] += ((*pCh++) - n);
                }

                if (*pCh++ != ';') {
                    r = 9;
                    break;
                }

                // security code index 1 bytes (1 digit)
                cmd[j++] = (*pCh++) - 0x30;
                
                if (*pCh++ != ';') {
                    r = 10;
                    break;
                }

                // security code 2 bytes (4 nibbles)
                for (k = 0, v = 0; k < 4; k++) {
                    v <<= 4;
                    if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    v += ((*pCh++) - n);

                    if ( *pCh == ';') {
                        pCh++;
                        break;
                    }
                }

                cmd[j++] = (char) v;         // LSB byte of the security code.
                cmd[j++] = (char)(v >> 8);   // MSB byte of the security code.

                // security update/ update cycle
                for (k = 0; k < 2; k++) {
                    if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    cmd[j] = ((*pCh++) - n);

                    if (*pCh != ';') {
                        cmd[j] <<= 4;

                        if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
                        else n = 0x30;
                        cmd[j] += ((*pCh++) - n);

                        if (*pCh != ';') {
                            r = 11;
                            break;
                        }
                    }
                    
                    pCh++;
                    j++;
                }

                if (r > 0)  break;
                // delete flag
                cmd[j++] = (*pCh++) - 0x30;
            }
            
            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[COMMAND__UPDATE_USR_IX]; // assigned command to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[USER_IDX__UPDATE_USR_IX]; // assigned user ID to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.

            if (j > MAX_CMD_BUFFER_ARG_SIZE) {
                j = MAX_CMD_BUFFER_ARG_SIZE - 1;
                cmd[j++] = 0xFF;
                r = 12;
            }

            if (r == 0) {
                SendCommand(cmd, j);
            } else {
                //Serial.write(cmd, j);

                uSettingInfo.aSettingData[0] = 0; // set record length to zero in order to signal an error.
                uSettingInfo.aSettingData[1] = ERROR_IF_INVALID_CMD_ARG;
                uSettingInfo.aSettingData[2] = r;
                /*for (i = 0; i < cmd[REC_NUM__UPDATE_USR_IX]; i++) {
                    uSettingInfo.aSettingData[i + 2] = RESULT_NG;
                }*/                
                
                uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = CMD_HEADER_FOOTER_SIZE + 3; // +3 for cmd, and 2 args.
                //SendCommand(cmd, j);
            }

            server.send(204);
            break;

        case USET_FORMAT_FOB_SEC_CODE: // 27: Format either a given security code block or all blocks.
            cmd[0] = CMD_FORMAT_FOB_SEC_CODE;
            str = server.arg("ffsReq");
            str.trim();
            if (str == "all") {
                cmd[1] = SECURITY_CODE_INDEX_FOR_ALL; // INVALID_SECURITY_CODE_INDEX;
            } else {
                //cmd[1] = (char)server.arg("fFsReq").toInt(); // Security code block index.
                cmd[1] = (char)str.toInt(); // Security code block index.
            }

            str = server.arg("ffcId"); // card id for Format Fob.
            str.trim();

            Serial.println(str);

            pCh = (char*)str.c_str();

            //for (i = 2; i < 10; i++) { // Fob ID number; 8 characters.
            //    cmd[i] = *pCh++;
            //}

            // fob no.
            for (i = 2, k = 0; k < SIZE_CARD_ID; k++) {

                if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                else n = 0x30;
                cmd[i] = ((*pCh++) - n) << 4;

                if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                else n = 0x30;
                cmd[i++] += ((*pCh++) - n);
            }

            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[0]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[1]; // assigned it to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.

            SendCommand(cmd, i);

            server.send(204);
            break;

        case USET_SAVE_WIFI_USER: // 35: Get wifi device information to be saved for the given user.
        case USET_GEN_LOGIN_KEY:  // 37: Generate Log-in Key for new or a valid wifi user.

            //Mar. 1, 2020: 
            // fwifiReq:  '1;31:1;Elza;pw123456;170707;255;000000000000;255;FFFFFFFFFFFF;0'  for Laundry user #1 (0x31), valid user: true, 
            // user name: Elza, pw: pw123456, date: 2017-0707, host1: 255, mac1: n/a, host2: 255, mac2: n/a, delete: false.
            // May 30, 2020:
            // fwifiReq:  1;11:1;D4FEC5DB;big1;11111111;200415;ff;000000000000;0
            // cmd[]: `>26D9 92 01 11 11 D4FEC5DB 626967310000 3131313131313131 200415 FF 00000000000000 18
            // rsp from Control server: `< 09F6. ... `>07F8.05005D `<07F805005F

            if (idx == USET_SAVE_WIFI_USER) {
                str = server.arg("fwifiReq");
                cmd[COMMAND__UPD_WF_USR_IX] = CMD_SAVE_WIFI_USER; // 0x92
            } else {
                str = server.arg("fkeyGenReq");
                cmd[COMMAND__UPD_WF_USR_IX] = CMD_GENERATE_LOGIN_KEY; // 0x94
            }

            str.trim();

            Serial.println(str);

            pCh = (char*)str.c_str();
            r = 0;
            // the number of record rows in the wif user table.
            if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
            else n = 0x30;
            cmd[REC_NUM__UPD_WF_USR_IX] = (*pCh++) - n;

            if (*pCh != ';') {
                cmd[REC_NUM__UPD_WF_USR_IX] <<= 4;

                if (*pCh > 0x39) n = 0x37;
                else n = 0x30;
                cmd[REC_NUM__UPD_WF_USR_IX] += ((*pCh++) - n);

                if (*pCh != ';') r = 1;
            }
            pCh++;
            // user id (Fob user ID; but not the userStatus because it doesn't inform the user's status like blocked or valid).
            if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
            else n = 0x30;
            
            cmd[USER_IDX__UPD_WF_USR_IX] = ((*pCh++) - n);

            if (*pCh != ':') {
                cmd[USER_IDX__UPD_WF_USR_IX] <<= 4;

                if (*pCh > 0x39) n = 0x37; // It is an upper case if it is greater than 9.
                else n = 0x30;
                cmd[USER_IDX__UPD_WF_USR_IX] += ((*pCh++) - n);
            }

            user.newUser.userWifiStatus = cmd[USER_IDX__UPD_WF_USR_IX];

            //TODO:: Mar. 2, 2020: if it is more than one row, then cmd[] buffer overflow takes place. resolve it.
            for (i = 0, j = VALID_USER__UPD_WF_USR_IX; i < cmd[REC_NUM__UPD_WF_USR_IX]; i++) {
            //i = 0; j = VALID_USER__UPDATE_USR_IX;

                // access if this command packing causes the overflow of the cmd[] buffer.
                // the last +3 includes the command, the number of row, and user ID, 
                // all of which are packed to cmd[] only once.
                if ((((i + 1) * WIFI_USR_BYTE_4_UPDATE) + 3) > MAX_CMD_BUFFER_ARG_SIZE) {
                    r = 2;
                    break;
                }

                // check the mark of the start of a record.
                if (*pCh++ != ':') {
                    r = 3;
                    break;
                }

                // valid user flag
                if ((*pCh++) == '0') {
                    ////cmd[j] = (*pCh++) - 0x30;
                    //cmd[j] = BLOCKED_USER_WIFI_STATUS__iEE; // May 27, 2020: commented out to apply new userWifiStatus value format outside this 'if' statement.
                    ////if (cmd[j++] == 0) 
                    user.newUser.userWifiStatus |= USR_ST__MASK_BLOCKED_USER;
                }
                cmd[j++] = user.newUser.userWifiStatus; // VALID_USER_WIFI_STATUS__iEE; // May 27, 2020: applied new userWifiStatus value format.

                if (*pCh++ != ';') {
                    r = 4;
                    break;
                }

                // fob no.
                for (k = 0; k < SIZE_CARD_ID; k++, j++) {

                    if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    cmd[j] = ((*pCh++) - n) << 4;

                    if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    else n = 0x30;
                    cmd[j] += ((*pCh++) - n);
                }

                if (*pCh++ != ';') {
                    r = 41;
                    break;
                }

                // user name
                for (k = 0; k < SIZE_INITIAL; k++, j++) {
                    if (*pCh == ';') break;
                    else {
                        cmd[j] = (*pCh++);
                        user.newUser.initial[k] = cmd[j];
                    }

                }
                
                for (; k < SIZE_INITIAL; k++, j++) {
                    cmd[j] = 0;
                    user.newUser.initial[k] = 0;
                }

                if (*pCh++ != ';') {
                    r = 5;
                    break;
                }

                // password
                for (k = 0; k < SIZE_PASSWORD; k++, j++) {
                    //if (*pCh == ';') break; //commented out because password can contain this character and also the password must be 8 bytes.
                    //else 
                    cmd[j] = (*pCh++);
                    user.newUser.password[k] = cmd[j];
                }
                
                //for (; k < SIZE_PASSWORD; k++) cmd[j++] = 0;

                if (*pCh++ != ';') {
                    r = 6;
                    break;
                }

                // registration date. BCD value.
                for (k = 0; k < 3; k++, j++) {

                    if (*pCh > 0x39) n = 0x37;  // it shouldn't happen. it must be one of '0' to '9'.
                    else n = 0x30;
                    cmd[j] = ((*pCh++) - n) << 4;

                    if (*pCh > 0x39) n = 0x37;  // it shouldn't happen. it must be one of '0' to '9'.
                    else n = 0x30;
                    cmd[j] += ((*pCh++) - n);
                }

                if (*pCh++ != ';') {
                    r = 7;
                    break;
                }

//#ifdef SEND_TWO_PAIR_OF_HOST_N_MAC 
                //Note:  commented following block of code out because it causes overflow of some arrays; cmd[] and command[].
                //  - cmd[] needs (5 header/footer + 1 command + .


                // Reuced it into one pair of host and mac in order to save buffers in the Control Server.
                //// Two pair of host and mac.
                //for (k = 0; k < 2; k++) {
                    // host I/II

                    ////cmd[j++] = (*pCh++);
                    //if (*pCh > 0x46) n = 0x57;       // It is an lower case if it is greater than 'F'.
                    //else if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    //else n = 0x30;
                    //cmd[j] = ((*pCh++) - n) << 4;

                    //if (*pCh > 0x46) n = 0x57;       // It is an lower case if it is greater than 'F'.
                    //else if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                    //else n = 0x30;
                    //cmd[j] += ((*pCh++) - n);                    
                    
                    cmd[j++] = (char) HexCharToUint16(pCh, 1);
                    pCh += 2;

                    if (*pCh++ != ';') {
                        r = 8;
                        break;
                    }

                    // mac I/II  //mac exmaple. C4E98445F7A7
                    for (l = 0; l < 6; l++, j++) {

                        //if (*pCh > 0x46) n = 0x57;       // It is an lower case if it is greater than 'F'.
                        //else if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                        //else n = 0x30;
                        //cmd[j] = ((*pCh++) - n) << 4;

                        //if (*pCh > 0x46) n = 0x57;       // It is an lower case if it is greater than 'F'.
                        //else if (*pCh > 0x39) n = 0x37;  // It is an upper case if it is greater than 9.
                        //else n = 0x30;
                        //cmd[j] += ((*pCh++) - n);

                        cmd[j] = (char)HexCharToUint16(pCh, 1);
                        pCh += 2;
                    }

                    if (*pCh++ != ';') {
                        r = 9;
                        break;
                    }
                //}
//#else
//                pCh += 28; // each byte consists of 2 char;  2 char * (1 byte host + 6 byte mac) * 2 pair =  28;
//#endif
                if (r > 0) break;

                // delete flag
                cmd[j] = (*pCh++) - 0x30;

                if (cmd[j++] > 0) user.newUser.userWifiStatus |= USR_ST__MASK_BLOCKED_USER;
            }

            uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] = cmd[COMMAND__UPD_WF_USR_IX]; // assigned command to verify that arriving response is what is waiting for.
            uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] = cmd[USER_IDX__UPD_WF_USR_IX]; // assigned user ID to verify that arriving response is what is waiting for.            

            if (idx == USET_SAVE_WIFI_USER) {

                if (j > MAX_CMD_BUFFER_ARG_SIZE) { //TODO:: buffer overflow. Needs to handle it before the overflow.
                    j = MAX_CMD_BUFFER_ARG_SIZE - 1;
                    cmd[j++] = 0xFF;
                    r = 10;
                }

                user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate login-key generation is off as a default here.

                if (r == 0) {
                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = 0;      // length.
                    SendCommand(cmd, j);
                    
                    if ((user.newUser.userWifiStatus & USR_ST__MASK_BLOCKED_USER) == 0)
                        user.newUser.cookie = LOGIN_KEY_GEN_EN_REQ; // To indicate login-key generation enable is requested.
                }

            } else if (r == 0) { // idx == USET_GEN_LOGIN_KEY

                uSettingInfo.aSettingData[0] = cmd[REC_NUM__UPD_WF_USR_IX];

                if ((user.newUser.userWifiStatus & USR_ST__MASK_BLOCKED_USER) == 0) {
                    
                    j = user.newUser.userWifiStatus; // &USR_ST__MASK_FULL_USERS_ID;

                    for (i = 0; i < user.nofValidUser; i++) {
                        if (user.paUser[i]->userWifiStatus == j) break;
                    }

                    if (i < user.nofValidUser) {

                        for (j = 0; j < SIZE_INITIAL; j++) {
                            if (user.newUser.initial[j] == 0) {
                                if (user.paUser[i]->initial[j] == 0)
                                    if (j >= MIN_SIZE_INITIAL) j = SIZE_INITIAL;
                                break;
                            }
                            if (user.paUser[i]->initial[j] != user.newUser.initial[j]) break;
                        }

                        if (j == SIZE_INITIAL) {
                            for (j = 0; j < SIZE_PASSWORD; j++) {
                                if (user.paUser[i]->password[j] != user.newUser.password[j]) break;
                            }

                            if (j == SIZE_PASSWORD) {

                                // additional data "-0" or "-1" for Key Gen en/disable request only for 
                                // 'Enable Key Generation' button click; '-' in the data is a delimiter.
                                // '0' means disable request while '1' for enable request.
                                if (*pCh++ != '-') r = 11;
                                else {
                                    
                                    // Be aware that user.newUser.userWifiStatus has been overwrited with either new or old value
                                    // when the PC is reached to this point; it means that the value of user.newUser.cookie is no longer valid.
                                    if (*pCh == '1') { // 'login key gen enable' requested.
                                        user.newUser.cookie = LOGIN_KEY_GEN_EN_REQ; // To indicate login-key generation is requested.
                                    } else {           // 'login key gen disable' requested.
                                        
                                        // Set it to LOGIN_KEY_GEN_OFF immediately to give the 'disable request' a priority to 'enable request'
                                        //user.newUser.cookie = LOGIN_KEY_GEN_DIS_REQ; // To indicate login-key generation disable is requested.
                                        user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate login-key generation is set to off.
                                    }

                                    //if (r == 0) {
                                        uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = CMD_HEADER_FOOTER_SIZE + 4; // +4 for cmd, and 3 args.

                                        uSettingInfo.aSettingData[1] = cmd[USER_IDX__UPD_WF_USR_IX];
                                        uSettingInfo.aSettingData[2] = RESULT_OK;
                                        /*for (i = 0; i < cmd[REC_NUM__UPDATE_USR_IX]; i++) {
                                            uSettingInfo.aSettingData[2 + i] = RESULT_OK;
                                        }*/

                                    //}
                                }

                            } else r = 13;
                        } else r = 14;
                    } else r = 15;
                } else r = 16;
            }

            if (r > 0) {
                const char* pCs;
                char aN[4];

                uSettingInfo.aSettingData[0] = 0; // set record length to zero in order to signal an error.
                uSettingInfo.aSettingData[1] = ERROR_IF_INVALID_CMD_ARG;
                uSettingInfo.aSettingData[2] = r;
                /*for (i = 0; i < cmd[REC_NUM__UPDATE_USR_IX]; i++) {
                    uSettingInfo.aSettingData[i + 2] = RESULT_NG;
                }*/

                uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = CMD_HEADER_FOOTER_SIZE + 4; // +4 for cmd, and 3 args.

                //SendCommand(cmd, j);

                pCs = NumToString(r, aN);
                str = "Err: ";
                str += pCs;
                Serial.println(str);
            }

            server.send(204);

            break;

        default:
            rlt = false;
            break;
        }

#ifdef VS_PRJ
# pragma endregion
#endif

    } else {
        v = GetLaonId(server.header("Cookie"));

        if (v >= VALID_SUPER_USER_I) {
            rlt = ReadAndSendFile("/usettings.html", "text/html");

        } else rlt = false;
    }

    if (rlt == false) {
        HandleNotFound();
    }
}

//==========================================================================================
void HandleUserLog(void) {
    bool r;

    if (GetLaonId(server.header("Cookie")) >= VALID_SUPER_USER_I) {
        r = ReadAndSendFile("/ulog.html", "text/html");

    } else r = false;

    if (r == false) {
        HandleNotFound();
    }
}


//==========================================================================================
void HandleLoginJs(void)
{
    const char* pF;
    File f = SPIFFS.open("/login.js", "r");
    String rdS = f.readString(); // Mar. 4, 2020: if the file size is big, then use server.streamFile(...) instead.
    pF = rdS.c_str();
    server.send(200, "text/javascript", pF);

    f.close();
    Serial1.println("login.js sent!");
}

//==========================================================================================
//void HandleSmHomeJs(void)
//{
//    const char * pF;
//    File f = SPIFFS.open( "/smhome.js", "r" );
//    String rdS = f.readString(); // Mar. 4, 2020: if the file size is big, then use server.streamFile(...) instead.
//    pF = rdS.c_str();
//    //pF = f.readString().c_str();
//    // server.sendContent("Content-Type: text/javascript\r\n");
//    // server.sendContent("\r\n");
//    // server.sendContent(pF);
//    
//    server.send(200, "text/javascript", pF);
//
//    f.close();
//}

//void HandleSmHomeCSS(void)
//{
//    const char * pF;
//    File f = SPIFFS.open( "/smhome.css", "r" );
//    String rdS = f.readString(); // Mar. 4, 2020: if the file size is big, then use server.streamFile(...) instead.
//    pF = rdS.c_str();
//    //pF = f.readString().c_str();
//    // server.sendContent("Content-Type: text/css\r\n");
//    // server.sendContent("\r\n");
//    // server.sendContent(pF);
//    
//    server.send(200, "text/css", pF);
//
//    f.close();
//}


void HandleUSettingsJs (void)
{
    // Mar. 4, 2020: found that following code was not able to get the file received by the web client.
    // It seems to be too big file to send. Therefore I used the same code where ReadAndSendFile(...) method uses.
    //const char* pF;
    //File f = SPIFFS.open("/usettings.js", "r");    
    //String rdS = f.readString();
    //pF = rdS.c_str();
    //server.send(200, "text/javascript", pF);
    //f.close();

    if (SPIFFS.exists("/usettings.js")) { // "/index.html"
        File f = SPIFFS.open("/usettings.js", "r");
        if (f) {
            size_t sz = server.streamFile(f, "text/javascript");
            f.close();
            //Serial1.println("File usettings.js has been sent");
        }
    }
}
//==========================================================================================
void HandleTestMenu(void)
{
    char cmd;
    unsigned char dt[25];

    if (GetLaonId(server.header("Cookie")) < VALID_ADMIN_USER) {
        HandleNotFound();
    }  else if (server.hasArg("cmd")) {
        if (server.hasArg("test")) {
            cmd = (char) server.arg("test").toInt();
            if (cmd == CMD_SHOW_WIFI_DEV_STATUS) {
                //String str;
                unsigned char i, j, ix;
                Serial1.println("* WifiDev: nofValidUser = ");

                dt[0] = (user.nofValidUser + 0x30);
                dt[1] = '`';
                dt[2] = 0;

                Serial1.println((const String&)(String((const char*)dt)));

                Serial1.println("usrSt, initial, pw");
                ix = 0;

                for (i = 0; i < user.nofValidUser; i++) {
                    dt[ix++] = ((user.paUser[i]->userWifiStatus >> 4) + 0x30);
                    dt[ix++] = ((user.paUser[i]->userWifiStatus & 0x0F) + 0x30);
                    dt[ix++] = ' ';

                    for (j = 0; j < SIZE_INITIAL; j++, ix++) {
                        dt[ix] = user.paUser[i]->initial[j];
                        if (dt[ix] == 0) dt[ix] = ' ';                        
                    }
                    dt[ix++] = ' ';

                    for (j = 0; j < SIZE_PASSWORD; j++) {
                        dt[ix++] = user.paUser[i]->password[j];
                    }
                    dt[ix++] = '`';
                    dt[ix++] = 0;
                    Serial1.println((const String &) (String((const char *)dt)));
                    //delay(100);
                    ix = 0;
                }

            } else {
                SendCommand(&(cmd), 1);
            }

            server.sendHeader("Location", "/test");
            server.sendHeader("Cache-Control", "no-cache");
            server.send(307);
        }
        //delay(10);
    } else {
        server.send(200, "text/html", DFT_SMART_DOOR_TEST_HTML);
    }
}


//==========================================================================================
//root page can be accessed only if authentification is ok
void HandleRoot(void)
{
#ifdef GTC_DBG   
    Serial1.println("Enter HandleRoot");
#endif
    String header;

    if( bNeedLogin == true ) {
        if ( Is_authenticated() == INVALID_USER ) {
          
            server.sendHeader("Location","/login");
            server.sendHeader("Cache-Control","no-cache");
            //301: Moved Permanently. This and all future requests should be directed to the given URI. 
            //307: Temporary Redirect (since HTTP/1.1). It works when the requested 'method' is either GET or HEAD.
            server.send(307);
            return;
        }
//#ifndef USE_DOOR_FOLDER_AS_DOOR_CTRL_PAGE
        if ( server.hasArg("cmd") ) {
# ifdef GTC_DBG 
          Serial1.println("Has command");
# endif
            if (server.hasArg("test")) {
                HandleTestMenu();
                return;
            } else {
                if (HandleDoor() == true) return; // if there is a valid command, don't draw the page again.          
            }
        }
//#endif

        ShowDoorCtrlPage();

    } else {
        //HandleDoor();
        ShowDoorCtrlPage();
    }    
}

//==========================================================================================
//no need authentification
void HandleNotFound(void) 
{
#ifdef REDIRECT_TO_ROOT_WHEN_NOT_FOUND_HANDLE
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
#else
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    
    for (uint8_t i=0; i<server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
    /*message += "<div>"
        "<h1><mark>Click to access the smart home</mark></h1>"
        "use id: admin/password: admin <a href = \"/login\">Login</a><br>"
        "</div><br><br>";
    server.send(302, "text/plain", message);*/
    
    //404: Not Found. The requested resource could not be found but 
    //     may be available in the future.
    //     Subsequent requests by the client are permissible.
    server.send(404, "text/plain", message);
    
#endif
}

//==========================================================================================
void BuildLockStatusXML()
{
    XML = "<?xml version='1.0'?>";
    XML += "<response>";
    XML += (((GetDoorlockState()) == false) ? "Unlocked" : "Locked");
    XML += "</response>";
}


//==========================================================================================
void HandleXML()
{
    unsigned char idx, cmd, cnt;
    const char* pCs;
    char aN[ SIZE_PASSWORD + 1 ], i, j, k;//, aN2[4]; //2020.4.12: increased the size of aN[] from 4 to 9 to fix 
                                          // too long initial and password especially there is no 0 to terminate the string.    
    
    if ( server.hasArg("USST") ) {
        idx = (unsigned char) server.arg("USST").toInt();
        if ( idx == 1 ) {
            XML = "<?xml version='1.0'?>";
            XML += "<response>";
            XML += "<u><rid>1</rid><ul>20</ul><ol>?</ol><cd>2018-11-14 10:30</cd><web>n/a</web><lg>n/a</lg></u>";
            XML += "<u><rid>2</rid><ul>40</ul><ol>?</ol><cd>2018-11-14 10:30</cd><web>n/a</web><lg>n/a</lg></u>";
            XML += "<u><rid>3</rid><ul>60</ul><ol>?</ol><cd>2018-11-14 10:30</cd><web>n/a</web><lg>n/a</lg></u>";
            //XML += "<u><rid>4</rid><ul>200</ul><ol>?</ol><cd>2018-11-14 10:30</cd><web>n/a</web><lg>n/a</lg></u>";
            XML += "</response>";
            
            //server.send(200, "text/xml", XML);
            // for( uint8_t i = 0, s = 0; i < MAX_NOF_ROOM__USER_STATUS; i++ ) {
            //     for( uint8_t j = 0; j < MAX_NOF_IP_PER_ROOM__USER_STATUS; j++, s++) {
            //         if( userIp[i][j] != 0xFF ) {
            //             if( pinger.Ping( IPAddress( 192, 168, 0, (uint8_t) userIp[i][j] ) ) == false ) {
            //                 Serial1.println("Ping Fail: " + String( (unsigned char) userIp[i][j]) );
            //             } else {
            //                 Serial1.println("Ping Rsp: " + String( (unsigned char) userIp[i][j] ) );
            //                 userIpStatus |= (1 << s);
            //             }
            //         }
            //     }
            // }
            //return;
        } else if ( idx == 2 ) {
            XML = "<?xml version='1.0'?>";
            XML += "<response>";
#ifdef USE_PING
            for( char i = 0, j = 0; i < MAX_NOF_ROOM__USER_STATUS; i++, j += 2 ) {
                if( userIp[i][0] != PING_IP_HOST_DEFAULT ) {
                    //String n = String( (unsigned char) userIp[i][0] );                    
                    XML += "<u><rid>";
                    XML.concat((unsigned char) userIp[i][0]);
                    XML += "</rid><ol>";
                    if( (userIpStatus & (1<<j)) > 0 ) {
                        XML += "Y";
                    } else if ( userIp[i][0] == PING_IP_HOST_FAILURE ) {
                        XML += "f";
                    } else {
                        XML += "N";
                    }
                    if( userIp[i][1] != PING_IP_HOST_DEFAULT ) {
                        if( (userIpStatus & (1<<(j+1))) > 0 ) {
                        XML += ", Y";
                        } else if ( userIp[i][1] == PING_IP_HOST_FAILURE ) {
                            XML += ", f";                    
                        } else {
                            XML += ", N";
                        }                        
                    }
                    XML +="</ol></u>";
                }
            }
#else
            XML += "<u><rid>192.168.0.0</rid><ol>N</ol></u>";
#endif
            XML += "</response>";
        }
        
    } else if ( server.hasArg("USET") ) {
        XML = "<?xml version='1.0'?>";
        //XML += "<response><rlt>";
            
        cmd = (unsigned char) server.arg("USET").toInt();
        idx = (unsigned char)server.arg("IDX").toInt();
        cnt = (unsigned char)server.arg("CDN").toInt(); // the number of response request to be sent later by the wifi client.

        XML += "<response><cmd>";
        pCs = NumToString(cmd, aN);
        /*for (i = 0; aN[i] != 0; i++) {
            XML += aN[i];
        }*/
        XML += pCs;
        XML += "</cmd><idx>";
        //XML += NumToString(idx, aN2);
        pCs = NumToString(idx, aN);
        /*for (i = 0; aN[i] != 0; i++) {
            XML += aN[i];
        }*/
        XML += pCs;
        XML += "</idx><rlt>";

        if (uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] > 0) { // there is data.

            uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] -= (CMD_HEADER_FOOTER_SIZE + 2); // setting data length. '2' is for command and the first arg which is a room index.


            switch (cmd) { //Reply each command response from either the door control or user's wifi device to admin's wifi device.
            case USET_GET_ROOM_USERS:   // 1: Reply both valid and blocked user IDs to its admin.
#ifdef VS_PRJ
# pragma region handleXML_USET_SAVE_WIFI_ACCOUNT
#endif

                if ((uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_GET_ROOM_USER_INFO) && (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == idx)) {
                    //char i, j, d;
                    //XML += "ok</rlt><len>" + uSettingInfo.dataHead[2] + "</len><ol>";
                    XML += "ok</rlt><len>";
                    //XML += NumToString(uSettingInfo.dataHead[2], aN1);
                    pCs = NumToString(uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX], aN);
                    XML += pCs;
                    XML += "</len><ol>";

                    // ==> Sep. 29, 2019: added.
                    // the numbers in uSettingInfo.aSettingData[] must be in ascending order.
                    // therefore it sorts the number in ascending order.
                    for (i = 1; i < uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX]; i++) {

                        aN[0] = uSettingInfo.aSettingData[i-1]; // keep smaller number.
                        k = aN[0] & USR_ST__MASK_FULL_USERS_ID;                        

                        for (j = i; j < uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX]; j++) {
                            if (k > (uSettingInfo.aSettingData[j] & USR_ST__MASK_FULL_USERS_ID)) {
                                aN[0] = uSettingInfo.aSettingData[j]; // keep smaller number.
                                uSettingInfo.aSettingData[j] = uSettingInfo.aSettingData[i - 1]; // assign bigger one.
                                k = aN[0] & USR_ST__MASK_FULL_USERS_ID;
                            }
                        }

                        uSettingInfo.aSettingData[i - 1] = aN[0];
                    }
                    //<==

                    for (i = 0, j = 1; i < uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX]; i++, j++) {

                        idx = (uSettingInfo.aSettingData[i] & USR_ST__MASK_USERS_ID);

                        if (idx == j) {

                            XML += "<li>";
                            //XML += NumToString(j, aN1);
                            pCs = NumToString(j, aN);
                            XML += pCs;
                            if ((uSettingInfo.aSettingData[i] & USR_ST__MASK_BLOCKED_USER) > 0) {
                                XML += " (block)";
                            }

                            XML += "</li>";

                        } else if (idx > j) {
                            for (; j < idx; j++) {
                                XML += "<li>";
                                //XML += NumToString(j, aN1);
                                pCs = NumToString(j, aN);
                                XML += pCs;
                                XML += " (new)</li>";
                            }

                            XML += "<li>";
                            //XML += NumToString(j, aN2);
                            pCs = NumToString(j, aN);
                            XML += pCs;
                            XML += "</li>";

                        } else break;
                    }

                    if (i >= 0) { // the first user id is not zero.
                        for (; j <= MAX_NOF_USERS_PER_ROOM; j++) {
                            XML += "<li>";
                            //XML += NumToString(j, aN1);
                            pCs = NumToString(j, aN);
                            XML += pCs;
                            XML += " (new)</li>";
                        }
                    }

                    XML += "</ol>";

                } else {
                    XML += "ng</rlt><err>";
                    if (uSettingInfo.aSettingData[0] == 0) {
                        pCs = NumToString(uSettingInfo.aSettingData[1], aN);
                        XML += pCs;
                    } else {
                        XML += "0";
                    }
                    XML += "</err>";
                }

                break;
#ifdef VS_PRJ
# pragma endregion
#endif

            case USET_SCAN_RFID_N_ADD: // 15: Reply the scanned and saved user fob.
            // TODO:: Feb. 20, 2020: the response to the CMD_SCAN_USER_FOB might be only User Fob Account data necessary; no User Wifi Account.
            //        Otherwise default User Wifi Account data can be a part of the response from the Door Control.
            case USET_SCAN_FOB:  // 20: Reply given user fob information to its admin.

            case USET_GET_ONE_USER_INFO:  // 10: Reply give user's information to its admin.
            
#ifdef VS_PRJ
# pragma region handleXML_USET_GET_ONE_USER_INFO
#endif                
                //// The user entry data corresponding to following struct is saved to the internal eeprom.
                //// All information within this struct is for a single user.
                //// This data does immediately follows the valid user address block (VUAB) within the internal eeprom.
                //// It consists of 32 bytes and has most detailed user information. It must not greater than 32 bytes.
                //typedef struct UserRecordIntEeprom
                //{
                //    char userStatus;                // refers to the 'userStatus' in ValidUserInfo_t structure.
                //    char cardId[SIZE_CARD_ID];      // RFID Tag UID/Serial Number.
                //    char regDateFob[SIZE_REG_DATE]; // BCD: YYMMDD. registered date of a fob user.
                //    char ixToSecurityCode;          // index to the security code location in the tag.
                //    unsigned short securityCode;    // refers to the 'securityCode' in ValidUserInfo_t structure.
                //    char updateMode;              // Update status: updateMode request/attempt/failed/done.
                //    char nUpdateMode;             // one's complement of the 'updateMode' which is the 2nd safty measure.
                //    // It is updated only when 'updateMode request' or 'done' is done.
                //    char updateCycle;               // updateMode frequency: 0 = once upon request, x = every x access logs.
                //    char userWifiStatus;            // user status; valid/blocked/invalid (0x00/0x80/0xFF respectively)
                //    char initial[SIZE_INITIAL];     // for both eeprom and the tag.
                //    char password[SIZE_PASSWORD];
                //    char regDateWifif[SIZE_REG_DATE];// BCD: YYMMDD. wifi user registeration date.
                //} UserRecordIntEeprom_t;
                
                
                if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_GET_USER_INFO) {
                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == idx) {
                        i = 1; // valid user info
                    } else i = 0;
                } else if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SCAN_USER_FOB) { // scaned user info
                    if ((uSettingInfo.aSettingData[0] & USR_ST__MASK_FULL_USERS_ID) == idx) { // new user
                        i = 2;
                    } else if (uSettingInfo.aSettingData[0] == WIFI_CLIENT_REQ_IN_PROGRESS) i = 0; // error.
                    else i = 3; // existing user
                } else if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SCAN_AND_SAVE_FOB) { // scaned user info
                    if ((uSettingInfo.aSettingData[0] & USR_ST__MASK_FULL_USERS_ID) == idx) { // new user
                        i = 4;
                    } else i = 5; // existing user
                } else i = 0;                

                //if (((uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_GET_USER_INFO) || (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SCAN_USER_FOB))
                //    && (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == idx)) {
                
                if( i > 0 ) {
                    //char i, j, k;
                    k = (uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] / SIZE_ONE_USER_RECORED);
                    //XML += "ok</rlt><len>" + uSettingInfo.dataHead[2] + "</len><ol>";
                    XML += "ok</rlt><len>";
                    //XML += NumToString(uSettingInfo.dataHead[2], aN1);
                    pCs = NumToString(k, aN);
                    XML += pCs;
                    XML += "</len>";

                    if (i > 1) { // response to either CMD_SCAN_USER_FOB command or CMD_SCAN_AND_SAVE_FOB.
                        XML += "<usr>";
                        pCs = NumToString(uSettingInfo.aSettingData[0], aN); // scanned user ID.
                        XML += pCs;
                        XML += "</usr>";
                    }

                    cnt = UNKNOWN_USER__USR_ST;

                    for (i = 0; k > 0; k--) {

                        // valid/blocked user
                        if (((uSettingInfo.aSettingData[i++] & USR_ST__MASK_BLOCKED_USER) > 0) || (uSettingInfo.aSettingData[0] == 0)) {
                            XML += "<rec><vu>n</vu><cid>";
                        } else {                            
                            XML += "<rec><vu>y</vu><cid>";
                        }

                        //i++; // to skip the reserved byte.

                        // card id
                        for (j = 0; j < SIZE_CARD_ID; j++) {
                            //pCs = NumToString(uSettingInfo.aSettingData[i++], aN);
                            pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i++], aN);
                            XML += pCs;
                        }

                        XML += "</cid><rdtf>";                        

                        // registration date of a fob user
                        for (j = 0; j < SIZE_REG_DATE; j++) {
                            pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i++], aN); // regDateFob[SIZE_REG_DATE]
                            XML += pCs;
                        }

                        XML += "</rdtf><ixsc>";                       

                        pCs = NumToString(uSettingInfo.aSettingData[i++], aN); // ixToSecurityCode; 
                        XML += pCs;

                        XML += "</ixsc><scd>";
                        // little endian
                        pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i+1], aN); // security code
                        XML += pCs;
                        pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i], aN); // security code
                        XML += pCs;
                        i += 2; // for 2 bytes security code.

                        XML += "</scd><ud>";
                        pCs = NumToString(uSettingInfo.aSettingData[i++], aN); // UpdateStatus
                        XML += pCs;

                        i++; // to skip the nUpdateMode.

                        XML += "</ud><ucyc>";
                        pCs = NumToString(uSettingInfo.aSettingData[i++], aN); // updateCycle
                        XML += pCs;

                        //i++; // to skip the reserved byte.
                        XML += "</ucyc><uwst>";

                        cnt = uSettingInfo.aSettingData[i++];
                        XML += NumTo2DigitHexString(cnt, aN); // userWifiStatus // May 31, 2020: replaced NumToString() to apply new userWifiStatus value format.
                        //XML += pCs;
                        XML += "</uwst><name>";

                        // initial
                        for (j = 0; j < SIZE_INITIAL; j++, i++) {
                            //pCs = (const char *)(&(uSettingInfo.aSettingData[i]));
                            //if (*pCs == 0) break;
                            //XML += pCs;
                            aN[j] = uSettingInfo.aSettingData[i];
                            if (aN[j] == 0) break;
                        }
                        aN[j] = 0; // to terminate the string if there is no termination value 0.
                        XML += (const char*) aN;

                        if (j < SIZE_INITIAL) i += (SIZE_INITIAL - j);  // to gets the 'i' point to the first character of the password.
                        XML += "</name><pwd>";

                        // password
                        for (j = 0; j < SIZE_PASSWORD; j++, i++) {
                            //pCs = (const char *)(&(uSettingInfo.aSettingData[i]));
                            //if (*pCs == 0) break;
                            //XML += pCs;
                            aN[j] = uSettingInfo.aSettingData[i];                            
                            if (aN[j] == 0) break;
                        }
                        aN[j] = 0; // to terminate the string if there is no termination value 0.
                        XML += (const char*)aN;

                        if (j < SIZE_PASSWORD) i += (SIZE_PASSWORD - j);  // to gets the 'i' point to the first byte of the registration date.
                        XML += "</pwd><rdtw>";

                        // registration date for a wifi user
                        for (j = 0; j < SIZE_REG_DATE; j++) {
                            pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i++], aN);
                            XML += pCs;
                        }

                        //i++; // to skip the reserved byte.
                        XML += "</rdtw></rec>";
                    }

                    j = 0;
                    if (user.newUser.cookie >= LOGIN_KEY_GEN_ENABLED) {
                        if (cnt == user.newUser.userWifiStatus) {
                            if ((cnt & USR_ST__MASK_BLOCKED_USER) == 0) {
                                j = 1; // to indicate that current user's login key gen was requested and is still valid.
                            }
                        }
                    }

                    if( j == 0 ) XML += "<kge>0</kge>";
                    else XML += "<kge>1</kge>";

                } else {
                    XML += "ng</rlt><err>";
                    if (uSettingInfo.aSettingData[0] == 0) {
                        pCs = NumToString(uSettingInfo.aSettingData[1], aN);
                        XML += pCs;
                    } else {
                        XML += "0";
                    }
                    XML += "</err>";
                }
                break;
#ifdef VS_PRJ
# pragma endregion
#endif
            
            case USET_SAVE_FOB_USER:  // 25: Reply to the command 'Update Fob User'.
            case USET_SAVE_WIFI_USER: // 35: Reply the response to Save Wifi User command.
            case USET_GEN_LOGIN_KEY:  // 37: Generate Log-in Key for new or a valid wifi user.

                if (cmd == USET_SAVE_FOB_USER)       cmd = CMD_SAVE_FOB_USER;      // 0x97
                else if (cmd == USET_SAVE_WIFI_USER) cmd = CMD_SAVE_WIFI_USER;     // 0x92
                else                                 cmd = CMD_GENERATE_LOGIN_KEY; // 0x94


                //if ((uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SAVE_FOB_USER) & (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == idx)) { //Oct. 09, 2019: replaced with following statments because it is user id rather than idx.
                if ((uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == cmd) & (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == uSettingInfo.aSettingData[1])) { // command and the user id are match.
                    //char i, j;

                    XML += "ok</rlt><len>";
                    pCs = NumToString(uSettingInfo.aSettingData[0], aN); //
                    XML += pCs;
                    XML += "</len>";
                    
                    //for (i = 1; i < uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX]; i++) {
                    for (i = 2, j = 0; j < uSettingInfo.aSettingData[0]; i++, j++) {
                        if (uSettingInfo.aSettingData[i] == 0) {
                            XML += "<sRlt>ok</sRlt>";
                        } else {
                            XML += "<sRlt>ng</sRlt>";
                        }
                    }

                    if (cmd != CMD_SAVE_FOB_USER) {
                        if (user.newUser.cookie == LOGIN_KEY_GEN_EN_REQ) {
                            user.newUser.cookie = LOGIN_KEY_GEN_ENABLED;           // To indicate login-key generation is enabled.
                            XML += "<kge>1</kge>";
                        }
                        // commented out because I set it to LOGIN_KEY_GEN_OFF at the time when
                        // the request was received in HandleUserSettings() since the 'disable request'
                        // was given a priority to 'enable request'.
                        //else if (user.newUser.cookie == LOGIN_KEY_GEN_DIS_REQ) { // assumed 'cmd == USET_GEN_LOGIN_KEY'.
                        //    user.newUser.cookie = LOGIN_KEY_GEN_OFF;               // To indicate login-key generation is off.
                        //}
                        else {
                            XML += "<kge>0</kge>";
                        }
                    }

                } else {

                    XML += "ng</rlt><err>";
                    if (uSettingInfo.aSettingData[0] == 0) {
                        pCs = NumToString(uSettingInfo.aSettingData[1], aN);
                        XML += pCs;
                    } else {
                        XML += "0";
                    }
                    XML += "</err><err2>";
                    pCs = NumToString(uSettingInfo.aSettingData[2], aN);
                    XML += pCs;
                    XML += "</err2>";

                    if (cmd != CMD_SAVE_FOB_USER) {
                        if (user.newUser.cookie == LOGIN_KEY_GEN_EN_REQ) {
                            user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate login-key generation is set to off.

                        }
                        // commented out because I found that restoring this value is not right.
                        // because user.newUser value was overwritten with new when the request was received.
                        // Therefore it should be restored.
                        //else if (user.newUser.cookie == LOGIN_KEY_GEN_DIS_REQ) {
                        //    if (cmd == CMD_GENERATE_LOGIN_KEY) { // needs to restore 'enabled' because 'disable req' is only set when it is 'enabled'.
                        //        user.newUser.cookie = LOGIN_KEY_GEN_ENABLED; // To indicate login-key generation is off.
                        //    }
                        //}
                    }
                }
                break;

            case USET_FORMAT_FOB_SEC_CODE:  // 27: Reply to the command 'Format Fob'.
                
                if ((uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_FORMAT_FOB_SEC_CODE) & (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == uSettingInfo.aSettingData[1])) { // command and the security code block index are match.

                    if (uSettingInfo.aSettingData[0] == RESULT_OK) {
                        //char i, j;                   

                        XML += "ok</rlt><cid>";

                        // card id
                        for (i = 2, j = 0; j < SIZE_CARD_ID; j++) {
                            //pCs = NumToString(uSettingInfo.aSettingData[i++], aN);
                            pCs = NumTo2DigitHexString(uSettingInfo.aSettingData[i++], aN);
                            XML += pCs;
                        }
                        XML += "</cid>";

                    } else {
                        XML += "ng</rlt><err>";
                        
                        pCs = NumToString(uSettingInfo.aSettingData[0], aN);
                        XML += pCs;
                        
                        XML += "</err>";
                    }
                } else {
                    XML += "ng</rlt><err>";
                    
                    pCs = NumToString(uSettingInfo.aSettingData[1], aN);
                    XML += pCs;                    
                    XML += "</err>";
                }
                break;

            case USET_GET_NEW_USER_MAC:  // 30: Reply given user's wifi MAC to its admin.
                XML += "ng</rlt><err>0</err>";
                break;
            
            //case USET_SAVE_WIFI_USER: // moved to right below USET_SAVE_FOB_USER above.

            default:
                XML += "ng</rlt><err>222</err>"; // 0xDE == 222 to say it is DEfault.
                break;
            }



        ///////////////////////////////////////////////////////////////////////

        } else {  // 'if (uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] > 0)'

            if (cnt > 0) {

                XML += "wt</rlt>";
                switch (cmd) {
                case USET_SCAN_RFID_N_ADD:
                    // Send last command again in order to get fob scan result if it is ready by the Smart Control Sub-system.
                    if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SCAN_AND_SAVE_FOB) {
                        SendCommand((char*)uSettingInfo.dataHead, 2);
                    }
                    break;
                case USET_SCAN_FOB:
                    if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SCAN_USER_FOB) {
                        SendCommand((char*)uSettingInfo.dataHead, 2);
                    }
                    break;

                case USET_SAVE_FOB_USER: // 25: Get fob information saved for the given user.
                    if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_SAVE_FOB_USER) {
                        SendCommand((char*)uSettingInfo.dataHead, 2);
                    }
                    break;

                case USET_FORMAT_FOB_SEC_CODE:
                    if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == CMD_FORMAT_FOB_SEC_CODE) {
                        SendCommand((char*)uSettingInfo.dataHead, 2);
                    }
                    break;
                default:
                    break;
                }

            } else {
                XML += "ng</rlt>";
                aN[0] = CMD_CANCEL_TASK_IN_PROGRESS;
                aN[1] = uSettingInfo.dataHead[0];
                aN[2] = uSettingInfo.dataHead[1];

                //switch (cmd) {
                //case USET_SCAN_RFID_N_ADD:
                //case USET_SCAN_FOB:
                //case USET_SAVE_FOB_USER: // 25: Get fob information saved for the given user.
                //case USET_FORMAT_FOB_SEC_CODE:
                //    XML += "ng</rlt><err>555</err>";
                //    SendCommand(aN, 3);
                //    break;
                //default:
                //    break;
                //}

                XML += "<err>555</err>"; // 555 means that the repeat countdown value in the Wifi Client has been reached to zero.
                SendCommand(aN, 3);
            }
        }

        XML += "</response>";

    } else if (server.hasArg("ULGI")) {  // 'else if ( server.hasArg("USET") )'
        
        cmd = (unsigned char)server.arg("ULGI").toInt(); // user log in
        idx = (unsigned char)server.arg("NOFU").toInt();
        cnt = (unsigned char)server.arg("CDN").toInt(); // the number of response request to be sent later by the wifi client.

        XML = "<?xml version='1.0'?>";
        //XML += "<response><rlt>";
        XML += "<response><cmd>";
        pCs = NumToString(cmd, aN);
        XML += pCs;
        XML += "</cmd><rlt>";
        
        if (cmd == WIFI_CMD_LOGIN_TSID_REQ) {
            unsigned short tsid;

            XML += "ok</rlt><tsid>";

            tsid = (unsigned short) millis();
            if (tsid > 0xFF) {
                i = (unsigned char)(tsid >> 8);
                j = i & 0x03;
                pCs = NumTo2DigitHexString(i, aN);
                XML += pCs;
            } else j = 0;

            i = (unsigned char)(tsid);
            pCs = NumTo2DigitHexString(i, aN);
            XML += pCs;
            XML += "</tsid><gx1>";
            pCs = NumToString(j, aN);
            XML += pCs;
            XML += "</gx1><gx2>";
            j = (unsigned char)(tsid & 0x03);
            pCs = NumToString(j, aN);
            XML += pCs;
            XML += "</gx2></response>";

        } else if (cmd == WIFI_CMD_LOGIN_CODEGEN_PARAM_REQ) {

            if (user.newUser.cookie == LOGIN_KEY_GEN_IN_PROGRESS) {
                //let s;

                XML += "ok</rlt><id>";                
                pCs = NumTo2DigitHexString(user.newUser.userWifiStatus, aN);
                XML += pCs;
                XML += "</id><nm>";

                // initial
                for (i = 0; i < SIZE_INITIAL; i++) {                    
                    aN[i] = user.newUser.initial[i];
                    if (aN[i] == 0) break;
                }
                aN[i] = 0; // to terminate the string if there is no termination value 0.
                XML += (const char*)aN;
                XML += "</nm><pw>";

                // password
                for (i = 0; i < SIZE_PASSWORD; i++) {                    
                    aN[i] = user.newUser.password[i];
                    if (aN[i] == 0) break;
                }
                aN[i] = 0; // to terminate the string if there is no termination value 0.
                XML += (const char*)aN;
                XML += "</pw>";
                user.newUser.cookie = LOGIN_KEY_GEN_OFF;

            } else {

                XML += "ng</rlt><err>";                
                if (user.newUser.cookie > 0xFF) {
                    i = (unsigned char)(user.newUser.cookie >> 8);
                    j = i & 0x03;
                    pCs = NumTo2DigitHexString(i, aN);
                    XML += pCs;
                }

                i = (unsigned char)(user.newUser.cookie);
                pCs = NumTo2DigitHexString(i, aN);
                XML += pCs;
                XML += "</err>";
            }
            XML += "</response>";
        } else XML += "ng</rlt><err>250</err></response>";

    } else if (server.hasArg("GETL")) {  // 'else if ( server.hasArg("ULGI") )'
        BuildLockStatusXML();

    } else {  // 'else if ( server.hasArg("GETL") )'
        BuildLockStatusXML();
    }

    server.send(200, "text/xml", XML);
    //Serial1.println(XML);
}

//unsigned char ParseBCD2Num(unsigned char n)
//{
//    unsigned char d;    
//    
//    d = (n >> 4) * 10;
//    d += n & 0x0F;
//
//    return d;
//}

const char * NumToString(unsigned char n, char * dt)
{
    return (NumToMin2DigitString(n, dt, false));
}

const char * NumToMin2DigitString(unsigned char n, char * dt, bool b2D)
{
    unsigned char d, i;
    i = 0;
    d = 0;
    if (n >= 100) {
        d = n / 100; // the number of 100.
        n %= 100;    // value less than 100.
    }

    if (d > 0) dt[i++] = d + 0x30;    

    if (n >= 10) {
        d = n / 10; // the number of 10.
        n %= 10;    // value less than 10.
        dt[i++] = d + 0x30;
    } else if ( (d > 0) || (b2D == true) ) {
        dt[i++] = 0x30;
    }

    dt[i++] = n + 0x30;
    dt[i] = '\0';

    return (const char *)dt;
}

const char * NumTo2DigitHexString(unsigned char n, char * dt)
{
    unsigned char d, i;
    i = 0;
    d = n >> 4;

    if (d < 10) {
        dt[i++] = d + 0x30;
    } else {
        dt[i++] = d + 0x37;
    }

    d = n & 0x0F;

    if (d < 10) {
        dt[i++] = d + 0x30;
    } else {
        dt[i++] = d + 0x37;
    }

    dt[i] = '\0';

    return (const char *)dt;
}

const char* NumTo4DigitHexString(unsigned short v, char* dt)
{
    unsigned char d;
    signed char i;
    i = 4;
    dt[i--] = '\0';

    for (; i >= 0; i--) {
        d = (unsigned char) (v & 0x0F);
        v >>= 4;

        if (d < 10) {
            dt[i] = d + 0x30;
        } else {
            dt[i] = d + 0x37;
        }
    }
    return (const char*)dt;
}

unsigned short HexStrToUint16(String hx)
{
    unsigned short r;
    unsigned char i, v;
    char* pCh;
    
    //hx.trim();

    pCh = (char*)hx.c_str();

    //r = 0;
    if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
    else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
    else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

    r = (*pCh++) - v;

    if (*pCh != 0) {

        for (i = 1; i < 4; i++, pCh++) {

            if (*pCh == 0) break;
            r <<= 4;

            if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
            else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
            else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

            r += *pCh - v;
        }
    }

    return r;
}

/// Convert one or two Hex chracters into unsigned int value
/// params:
///   - pCh: a pointer to the begin of Hex characters.
///   - nb:  the number of resulting bytes to which Hex characters converted.
unsigned short HexCharToUint16(char* pCh, unsigned char nb)
{
    unsigned short r;
    unsigned char i, v;

    nb <<= 1; // double the size

    //r = 0;
    if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
    else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
    else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

    r = (*pCh++) - v;

    for (i = 1; i < nb; i++, pCh++) {

        r <<= 4;

        if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
        else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
        else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

        r += *pCh - v;
    }

    return r;
}

/// Get Laon ID from the cookie
/// params:
///   - ck: cookie string which contains LAONID
unsigned short GetLaonId(String ck)
{
    unsigned short r;
    unsigned char i, v;
    char* pCh;

    //hx.trim();

    if (ck.indexOf("LAONID=") == -1) return 0;

    pCh = (char*)ck.c_str();    
    
    if (*pCh != 'L') return 0;
    if (*(pCh+1) != 'A') return 0;
    if (*(pCh+2) != 'O') return 0;
    if (*(pCh+3) != 'N') return 0;

    pCh += 7;
    //r = 0;
    if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
    else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
    else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

    r = (*pCh++) - v;

    if (*pCh != 0) {

        for (i = 1; i < 4; i++, pCh++) {

            if (*pCh == 0) break;
            r <<= 4;

            if (*pCh < 0x3A)      v = 0x30; // It is one of number '0' to '9'; i.g. 0x31 ('1') - 0x30 = 1.
            else if (*pCh < 0x47) v = 0x37; // It is an lower case if it is less than 0x47 ('G'); i.g. 0x41 ('A') - 0x37 = 10.
            else                  v = 0x57; // It is an upper case ; i.g. 0x57 ('a') - 0x57 = 10.

            r += *pCh - v;
        }
    }

    return r;
}
//#include "HttpMsgExchange.c"



