
#ifdef VS_PRJ
# include ".\laonSmHomeWiFiWebServer.h" //Aug. 6, 2018: Full path name is required since I have updated the Arduino board library (ESP8266 2.4.2) today.
#endif


extern ESP8266WebServer server;
//==========================================================================================

//// User Status
#ifdef USE_PING
Pinger pinger;

unsigned char userIp[MAX_NOF_ROOM__USER_STATUS][MAX_NOF_IP_PER_ROOM__USER_STATUS] = {
    { PING_IP_HOST_DEFAULT, PING_IP_HOST_DEFAULT },
    { PING_IP_HOST_DEFAULT, PING_IP_HOST_DEFAULT },
    { PING_IP_HOST_DEFAULT, PING_IP_HOST_DEFAULT },
    { PING_IP_HOST_DEFAULT, PING_IP_HOST_DEFAULT } };

// If the remote responds, one of bits are set to 1. Otherwise 0. If two hosts that are stored in userIp[0][0] and userIp[1][1] responded to its ping,
// then both bit 0 and bit 3 are set to 1 ( userIpStatus = 0000 1001 ).
unsigned char userIpStatus;
char pingStatus;
char pingCurrentIdx;
#endif

//==========================================================================================
void AdminMenu(void);
//void HandleAdmin(void);
//void HandleLogin(void);
void HandleNotFound(void);
void ShowDoorCtrlPage(void);
void HandleTestMenu(void);
void HandleRoot(void);
void setup(void);
void SetupOTA(void);
void loop(void);
void PrintStations(void);

//bool ReadAndSendFile(String file, String contType);
String GetSmHomeContentType(String filename);
bool HandleSmHomeFileRead(String path);
void HandleSystem(void);

//==========================================================================================
//==========================================================================================
#ifdef USE_PING
void InitUserStatusBuf(void)
{
    for (char i = 0; i < MAX_NOF_ROOM__USER_STATUS; i++) {
        for (char j = 0; j < MAX_NOF_IP_PER_ROOM__USER_STATUS; j++) {
            userIp[i][j] = 0xFF;
        }
    }
    userIpStatus = 0;
}
#endif

//==========================================================================================
char IndexOf(char ch, char start, const char *pCh, char len)
{
    const char * p = pCh;
    pCh += start;
    // while( *pCh != 0 ) {
    //     if( *pCh == ch ) return ((char)(pCh-p));
    //     pCh++;
    // }

    if (start >= len) return -1;

    for (char i = 0; i < len; i++) {
        if (*pCh == ch) return ((char)(pCh - p));
        pCh++;
    }
    return -1;
}

//==========================================================================================
int AsciiToInt(const char *pCh, char len)
{
    int r = 0;
    char i;
    char dbg[7]; //TTEST. get rid of it after test.             

    if (len <= 0) return -1;

    for (i = len; i > 0; i--) {
        r *= 10; // it was decimal when it was sent.
                 //dbg[len-i+1] = *pCh; //TTEST. get rid of it after test.             
        r += (*pCh - 0x30);
        pCh++;
    }
    //==>//TTEST. get rid of it after test.             
    // dbg[0] = '+';
    // dbg[len-i+1] = '\0';
    // dbg[len-i+2] = '\0';
    // Serial.println((const char *)dbg); //TTEST. get rid of it after test.                        
    //<==    
    return r;
}

//==========================================================================================
void PrintNumOneByteDbg(char h, char v)
{
    char dbg[6];
    dbg[0] = h;
    dbg[3] = (char)(v % 10);
    v -= dbg[3];
    dbg[2] = (char)(v % 100);
    v -= dbg[2];
    dbg[2] /= 10;

    dbg[1] = (char)(v / 100);

    dbg[1] += 0x30;
    dbg[2] += 0x30;
    dbg[3] += 0x30;
    dbg[4] = '\0';
    Serial.println((const char *)dbg); //TTEST. get rid of it after test.
}

//==========================================================================================
void ExecPing(void)
{
#ifdef USE_PING
    char i, j, ix;
    bool r;

    for (i = 0; i < MAX_NOF_ROOM__USER_STATUS; i++) {
        for (j = 0; j < MAX_NOF_IP_PER_ROOM__USER_STATUS; j++) {
            if (userIp[i][j] != PING_IP_HOST_DEFAULT) {
                ix = (i * MAX_NOF_IP_PER_ROOM__USER_STATUS) + j;
                if (((ix != pingCurrentIdx) && (ix > pingCurrentIdx)) ||
                    (PING_DEFAULT_INDEX == pingCurrentIdx)) { // new ping required.

                    r = pinger.Ping(IPAddress(192, 168, 0, (uint8_t)userIp[i][j]));
                    if (r == true) {
                        //Serial.println("Ping Fail: " + String( (unsigned char) userIp[i][j]) );
                        //Serial.println("Ping Fail: ");
                        PrintNumOneByteDbg('P', (char)userIp[i][j]);
                        pingCurrentIdx = ix;              // it gets current host not to be pinged again.
                        pingStatus = PING_ST_IN_PROGRESS; // mark it in progress.

                    } else {
                        if (pingStatus < PING_ST_END_FAIL_TRY) { // still need to ping current host again because it hasn't been even got started.
                            pingStatus++;
                            if (pingStatus < PING_ST_IN_FAIL_TRY) {
                                pingStatus = PING_ST_IN_FAIL_TRY;
                            }
                            //Serial.println("Ping Rsp: " + String( (unsigned char) userIp[i][j] ) );
                            //Serial.println("Ping Rsp: ");
                            PrintNumOneByteDbg('F', (char)userIp[i][j]);
                        } else { // Skip pinging current host.
                            userIp[i][j] = PING_IP_HOST_FAILURE;
                            pingStatus = PING_ST_CUR_REQ_DONE; // It gets next available host pinged.
                            pingCurrentIdx = ix;
                        }
                    }
                    return;
                }
            }
        }
    }

    if ((i*j) == (MAX_NOF_ROOM__USER_STATUS * MAX_NOF_IP_PER_ROOM__USER_STATUS)) {
        pingCurrentIdx = PING_DEFAULT_INDEX;
        pingStatus = PING_ST_IDLE;
        PrintNumOneByteDbg('X', 100);
    }
#else
    return;
#endif
}
//==========================================================================================
//handle UserStatus page, also called for disconnect

void HandleUserStatus(void)
{
    unsigned short u = Is_admin_authenticated();

    if (u == INVALID_USER) {

        if ((Is_superUser()) == false) {
            Serial.println("Invalid Admin");
            if ((Is_authenticated()) > INVALID_USER) server.sendHeader("Location", "/");
            else server.sendHeader("Location", "/login");
            server.sendHeader("Cache-Control", "no-cache");
            //server.sendHeader("Set-Cookie", "LAONID=2");

        } else {
            Serial.println("Valid Admin");
            server.sendHeader("Location", "/admin");
            server.sendHeader("Cache-Control", "no-cache");
            ////server.sendHeader("Set-Cookie", "LAONID=" COOKIE_VALID_ADMIN_ID);
            //server.sendHeader("Set-Cookie", "LAONID=100");

        }
        server.send(307);

    } else if (u >= VALID_SUPER_USER_I) {
        if (server.hasArg("UPDATE")) {

            //for( char i = 0; i < (char) server.args(); i++ ) {
            if (server.hasArg("pReq")) {
#ifdef USE_PING
                //if( (server.argName((int)i)) == "pReq" ) {
                if (pingStatus == PING_ST_IDLE) {

                    char i = 0;
                    char s, m, e, j, r;
                    char len; // = (char) server.arg((int)i).length();

                              //2018.11.20: bug fix: direct conversion like 'const char * pCh = server.arg("pReq").c_str();'
                              //                     caused the issue which prevent first group of host address like 20 (and 22)
                              //                     from being included to the memory which is pointed by pCh.
                    String param = server.arg("pReq");
                    const char * pCh = param.c_str(); //const char * pCh = server.arg((int)i).c_str();

                    len = 0;

                    while (true) {

                        if (*(pCh + len) == ' ') {
                            if (len > 0) len--;
                            break;
                        }

                        if (len > 59) break; // It should not hit it.

                        len++;


                    }

                    InitUserStatusBuf();
                    s = 0;
                    i = 0;

                    Serial.println(pCh); //TTEST. get rid of it after test.

                    while (i <  MAX_NOF_ROOM__USER_STATUS) {
                        j = 0;
                        r = IndexOf('-', (s + 1), pCh, len); // Combinations of 'x', '-', and '_' were worse than this '-', ',', '.' in my experiment.
                        if (r == -1) break;
                        if (r >= len) break;

                        s = r;
                        m = IndexOf(',', (s + 1), pCh, len);
                        e = IndexOf('.', (s + 1), pCh, len);
                        if (m != -1) {
                            if (m < e) {
                                userIp[i][j] = (unsigned char)AsciiToInt((pCh + s + 1), (m - s - 1));
                                if (userIp[i][j] > 250) userIp[i][j] = PING_IP_HOST_DEFAULT;
                                s = m;
                                j++;
                            }
                        }

                        if (e == -1) {
                            e = len; //(char) param.length();
                        } else if (e >= len) {
                            e = len;
                        }

                        userIp[i][j] = (unsigned char)AsciiToInt((pCh + s + 1), (e - s - 1));
                        if (userIp[i][j] > 250) userIp[i][j] = PING_IP_HOST_DEFAULT;

                        s = e;
                        i++;
                    }

                    server.send(204);

                    // InitUserStatusBuf(); //TTEST. get rid of it after test.
                    // userIp[0][0] = 20; //TTEST. get rid of it after test.
                    // Serial.println("Ping Started (20)"); //TTEST. get rid of it after test.

                    //==> TTEST. get rid of it after test.                    
                    PrintNumOneByteDbg('l', len);
                    for (i = 0; i < MAX_NOF_ROOM__USER_STATUS; i++) {
                        for (j = 0; j < MAX_NOF_IP_PER_ROOM__USER_STATUS; j++) {
                            if (userIp[i][j] != 0xFF) {
                                PrintNumOneByteDbg('v', (char)userIp[i][j]);
                            }
                        }
                    }
                    //<==

                    // for( i = 0, s = 0; i < MAX_NOF_ROOM__USER_STATUS; i++ ) {
                    //     for( j = 0; j < MAX_NOF_IP_PER_ROOM__USER_STATUS; j++, s++) {
                    //         if( userIp[i][j] != 0xFF ) {
                    //             if( pinger.Ping( IPAddress( 192, 168, 0, (uint8_t) userIp[i][j] ) ) == false ) {
                    //                 //Serial.println("Ping Fail: " + String( (unsigned char) userIp[i][j]) );
                    //                 Serial.println("Ping Fail: ");
                    //             } else {
                    //                 //Serial.println("Ping Rsp: " + String( (unsigned char) userIp[i][j] ) );
                    //                 Serial.println("Ping Rsp: ");
                    //                 userIpStatus |= (1 << s);
                    //             }
                    //         }
                    //     }
                    // }

                    pingCurrentIdx = PING_DEFAULT_INDEX;
                    ExecPing();

                } else {
                    const String contents =
                        "<html><head>"
                        "<title>Too Many Requests</title>"
                        "</head>"
                        "<body>"
                        "<h1>Server Busy</h1>"
                        "<p>Earlier ping reqeust is in progress. Try it again later.</p>"
                        "</body></html>";

                    server.sendContent("HTTP/1.1 429 Too Many Requests\r\n"); //send new p\r\nage
                    server.sendContent("Content-Type: text/html\r\n");
                    server.sendContent("\r\n");
                    server.sendContent(contents);
                }
                return;
                //}
#else
            const String contents =
                "<html><head>"
                "<title>Ping Disabled</title>"
                "</head>"
                "<body>"
                "<h1>Ping Feature</h1>"
                "<p>Ping was disabled! Define 'USE_PING' to enable it.</p>"
                "</body></html>";

            server.sendContent("HTTP/1.1 429 Too Many Requests\r\n"); //send new p\r\nage
            server.sendContent("Content-Type: text/html\r\n");
            server.sendContent("\r\n");
            server.sendContent(contents);
            return;
#endif
            }
            server.send(204); // 204: No Content. The server successfully processed the request and is not returning any content.

        } else {
            if ((ReadAndSendFile("/ustatus.html", "text/html")) == false) {
                HandleNotFound();
            }
        }
    } else {
        Serial.println("No access right !");
        server.send(200);
    }
}


#ifdef USE_PING
//==========================================================================================
bool SmPingRspRx(const PingerResponse& response)
{
    if (response.ReceivedResponse)
    {
        Serial.printf(
            "Reply from %s: bytes=%d time=%lums TTL=%d\r\n",
            response.DestIPAddress.toString().c_str(),
            response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
            response.ResponseTime,
            response.TimeToLive);
    } else
    {
        Serial.printf("Request timed out.\r\n");
    }

    // Return true to continue the ping sequence.
    // If current event returns false, the ping sequence is interrupted.
    return true;
}

//==========================================================================================
bool SmPingEnd(const PingerResponse& response)
{
    bool r = false;
    // Evaluate lost packet percentage
    float loss = 100;
    if (response.TotalReceivedResponses > 0)
    {
        loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
    }

    // Print packet trip data
    Serial.printf("Ping statistics for %s:\n", response.DestIPAddress.toString().c_str());
    Serial.printf(
        "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\r\n",
        response.TotalSentRequests,
        response.TotalReceivedResponses,
        response.TotalSentRequests - response.TotalReceivedResponses,
        loss);

    // Print time information
    if (response.TotalReceivedResponses > 0)
    {
        Serial.printf("Approximate round trip times in milli-seconds:\r\n");
        Serial.printf(
            "    Minimum = %lums, Maximum = %lums, Average = %.2fms\r\n",
            response.MinResponseTime,
            response.MaxResponseTime,
            response.AvgResponseTime);

        r = true;

        if (response.TotalReceivedResponses > 1) { // the remote host is available.
            if (pingStatus != PING_ST_IN_PROGRESS) {
                Serial.printf("Warning: not in proper status.\r\n");
            }

            if (pingCurrentIdx <= PING_MAX_NOF_IP_HOST) {
                userIpStatus |= (1 << pingCurrentIdx);
            }
        }
    }


    // Print host data
    Serial.printf("Destination host data:\r\n");
    Serial.printf("    IP address: %s\r\n", response.DestIPAddress.toString().c_str());
    if (response.DestMacAddress != nullptr)
    {
        Serial.printf("    MAC address: " MACSTR "\r\n", MAC2STR(response.DestMacAddress->addr));
    }
    if (response.DestHostname != "")
    {
        Serial.printf("    DNS name: %s\r\n", response.DestHostname.c_str());
    }

    pingStatus = PING_ST_CUR_REQ_DONE;
    return r;
}
#endif
//==========================================================================================