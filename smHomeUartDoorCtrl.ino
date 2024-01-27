#ifdef VS_PRJ
# include ".\laonSmHomeWiFiWebServer.h" //Aug. 6, 2018: Full path name is required since I have updated the Arduino board library (ESP8266 2.4.2) today.
#endif
//==========================================================================================
extern ESP8266WebServer server;
extern const char * buildVer;
extern const char * buildDate;
extern bool bNeedLogin;

//==========================================================================================
//==> UART =======================================================================
#ifdef EN_UART_RX
char indexBuf;               // It points to next available rx buffer element space.  
char rxDoneInd;              //
char rxBufIdx;
//char rxBufTxIdx;             //
//static char rxBuf[NOF_RX_BUF_ARRAY][NOF_BYTE_ONE_RX_BUF + 1];
char rxCmdIdx;
//char rxCmdLen;  
char rxCmdReady;
char lastCmd;
char rxCmdBuf[NOF_BYTE_RX_CMD_BUF + 1]; // the '+ 1' is for a safe gurad byte.
unsigned long rxTimeOut;
//#define DBG_PRINT_RX_IND_MSG //Aug. 5, 2018: Defined it for debugging in order to get some message is sent to the server upon receiption of UART command/response.
#endif
//<== UART =====================================================================

bool bDoorClosed; // Control Server's(ATmega's) door lock/unlock status. it keeps the status of door lock/unlock.
bool bLightOn;      // Control Server's(ATmega's) light status. it keeps the status of light.

byte command[MAX_CMD_BUFFER_SIZE + 1]; // the '+ 1' is for a safe gurad byte.

//==========================================================================================


//==========================================================================================
//==========================================================================================
// Initialize global variables
void InitSmartDoor(void)
{
    bDoorClosed = true;     // init by InitSmartDoor().
    bLightOn = false;       // init by InitSmartDoor().
    indexBuf = 0;           // init by InitSmartDoor().
    rxDoneInd = RX_NOT_DONE_IND__SERVER; // init by InitSmartDoor().
    rxBufIdx = 0;           // init by InitSmartDoor().

    rxCmdIdx = 0;           // init by InitSmartDoor().
    //rxCmdLen = 0;
    rxCmdReady = 0;         // init by InitSmartDoor().
    lastCmd = CMD_UNKNOWN;  // init by InitSmartDoor().
    rxCmdBuf[0] = 0;        // init by InitSmartDoor().
    rxTimeOut = 0;          // init by InitSmartDoor().
}

//==========================================================================================
void LockTheDoor(bool bUdt)
{
    digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
    digitalWrite(SMART_DOOR_CTRL_PIN, LOCK_SMART_DOOR); // Lock the smart door.

    if( bUdt ) bDoorClosed = true;
}
//==========================================================================================
void DoorLockControl(char cmd, char id)
{
    char buf[2];
    //bDoorClosed = true;

    buf[0] = cmd;

    if (cmd <= CMD_LOCK_DOOR_SERVER) {

        if (cmd > 0) {
            
            if (cmd == CMD_LOCK_DOOR) {

                // Jun. 1, 2020: passed in true to set bDoorClosed to true; if the Control Server is available, 
                //               the server eventually locks the door as soon as it receives the lock command.
                LockTheDoor(true);
                
                // Jun. 1, 2020: commented out 'if' condition because Control 
                //if (bDoorClosed == true) { // Control Server's (ATmega's) door is lokced.
                    Serial1.println("+Locked by client");
                //}
                //else { 
                //    // Wifi Server cannot lock when unlocked by the Control server.
                //    // But it can be locked by the Control sever which will receive the command
                //    // which will be sent in this function.
                //    Serial1.println("+Might not be able to lock with client");
                //}
            } else if (bDoorClosed == false) { // Control Server's (ATmega's) door is unlokced.
                Serial1.println("+To be locked");
            } else {
                Serial1.println("+Locked already");
            }

            SendCommand(&(cmd), 1);
        }

    } else if (cmd <= CMD_UNLOCK_DOOR_SERVER) {
        
        buf[1] = id;

        SendCommand(buf, 2);
        if (cmd == CMD_UNLOCK_DOOR) {
            unsigned short t;

            if (bDoorClosed == true) { // Server's (ATmega's) door is lokced.           
                Serial1.println("+Unlocked by client");                
            }

            
            t = (unsigned short)millis();
            // set the time to lock the door; after given time is elapsed, the door will be locked.
            ctrlSys.lockTime = (unsigned char)((t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC) + DOOR_UNLOCK_TIMER_EXPIRY_IN_SEC);
            ctrlSys.lockTime &= LOCK_TIME_MISS_RECOVERY; // 0x0F;
            ctrlSys.bLockEn = true; // enable for it to get the door locked after given time is elapsed.

            bDoorClosed = false;
            digitalWrite(STATUS_LED_GREEN, MY_LED_ON);    // turn the LED ON by making the voltage LOW
            digitalWrite(SMART_DOOR_CTRL_PIN, UNLOCK_SMART_DOOR); // Unlock the smart door.
            //delay(3000);            
            //digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
            //digitalWrite(SMART_DOOR_CTRL_PIN, LOCK_SMART_DOOR); // Lock the smart door.
            //                                                    //bDoorClosed = true;
            //Serial1.println("+Locked by client");

        } else {

            if (bDoorClosed == true) { // Control Server's (ATmega's) door is lokced.
                Serial1.println("+To be unlocked");
            } else {
                Serial1.println("+Unlocked already");
            }
        }
    }
}

//==========================================================================================
//void UnlockDoor(char cmd, char id)
//{
//    char buf[2];
//
//    buf[0] = cmd;
//    buf[1] = id;
//    SendCommand(&(cmd), 2);
//
//    if (cmd == CMD_UNLOCK_DOOR) {
//        if (bDoorClosed == true) { // Server's (ATmega's) door is lokced.           
//            Serial1.println("+Door Unlocked");
//        } else {
//            Serial1.println("+Unlock by client");
//        }
//
//        //bDoorClosed = false;
//        digitalWrite(STATUS_LED_GREEN, MY_LED_ON);    // turn the LED ON by making the voltage LOW
//        digitalWrite(SMART_DOOR_CTRL_PIN, UNLOCK_SMART_DOOR); // Unlock the smart door.
//        delay(3000);
//        digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
//        digitalWrite(SMART_DOOR_CTRL_PIN, LOCK_SMART_DOOR); // Lock the smart door.
//                                                            //bDoorClosed = true;
//    }
//}



//==========================================================================================
void LightOnOff(char cmd, unsigned char ix)
{
#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    unsigned char i;
    unsigned char led[3] = { STATUS_LED_RED, STATUS_LED_GREEN, STATUS_LED_BLUE };
    unsigned long t;
#endif

    
#if SERVER_ID == SERVER_ID_SIDE_PROTO_BOX
    if (cmd == CMD_LIGHT_ON) {
        Serial1.println("+Light On");
        //digitalWrite(STATUS_LED_GREEN, MY_LED_ON);    // turn the LED ON by making the voltage LOW
        //digitalWrite(SMART_DOOR_CTRL_PIN, UNLOCK_SMART_DOOR); // Unlock the smart door.
        
        if (ix > 0) {
            if (ix < CMD_ARG_ALL_LIGHT_ON) {
                i = ix - CMD_ARG_RED_LIGHT_ON;
                digitalWrite(led[i], MY_LED_ON);

                if ( i > 0) { // one or both of Green and Blue is requested to turn on, so alternative grow must be cleared.
                    ctrlSys.growMode &= (0x0F - i); // if given LED used to be in grow mode, then clear the bit.
                }

            } else if (ix == CMD_ARG_ALL_LIGHT_ON) {
                SendCommand(&(cmd), 1);
                for (i = 0; i < 3; i++) digitalWrite(led[i], MY_LED_ON);
                ctrlSys.growMode = 0;

            } else if (ix < CMD_ARG_ALT_LIGHT_GROW) {
                // alternative grow must be cleared unconditionally if there was.
                ctrlSys.growMode = (ctrlSys.growMode & 0x0F) | ((ix - CMD_ARG_GREEN_LIGHT_GROW) + 0x01);
                ctrlSys.growLightIntense = GROW_INIT_INTENSITY;
                t = millis();
                //t &= 0x00003F00;
                i = ((unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_125_mSEC)) & GROW_TIME_MISS_RECOVERY;
                ctrlSys.t2ChangeLightIntense = (i + 1) & GROW_TIME_MISS_RECOVERY;

            } else if (ix == CMD_ARG_ALT_LIGHT_GROW) {
                ctrlSys.growMode = 0x11; // Alternative grow starting with Green LED.
                ctrlSys.growLightIntense = GROW_INIT_INTENSITY;

                t = millis();
                //t &= 0x00003F00;
                i = ((unsigned char)(t >> NOF_SHIFT_TO_DELAY_ABOUT_125_mSEC)) & GROW_TIME_MISS_RECOVERY;
                ctrlSys.t2ChangeLightIntense = (i + 1) & GROW_TIME_MISS_RECOVERY;
            }
        }

    } else if (cmd == CMD_LIGHT_OFF) {
        Serial1.println("+Light Off");
        //digitalWrite(STATUS_LED_GREEN, MY_LED_OFF);   // turn the LED OFF (HIGH is the voltage level)
        //digitalWrite(SMART_DOOR_CTRL_PIN, LOCK_SMART_DOOR); // Lock the smart door.

        if (ix >= CMD_ARG_RED_LIGHT_OFF) {

            if (ix < CMD_ARG_ALL_LIGHT_OFF) {
                i = ix - CMD_ARG_RED_LIGHT_OFF;
                digitalWrite(led[i], MY_LED_OFF);

                if (i > 0) { // one or both of Green and Blue is requested to turn off, so alternative grow must be cleared.
                    ctrlSys.growMode &= (0x0F - i); // if given LED used to be in grow mode, then clear the bit.
                }

            } else if ( (ix == CMD_ARG_ALL_LIGHT_OFF) || (ix > CMD_ARG_ALT_LIGHT_STOP)) {
                SendCommand(&(cmd), 1);
                for (i = 0; i < 3; i++) digitalWrite(led[i], MY_LED_OFF);

                ctrlSys.growMode = 0;

            } else if ( ix < CMD_ARG_ALL_LIGHT_STOP ) {
                //digitalWrite(led[ix - CMD_ARG_GREEN_LIGHT_STOP + 1], MY_LED_OFF);
                
                // alternative grow must be cleared unconditionally if there was.
                ctrlSys.growMode &= (0x0F - ((ix - CMD_ARG_GREEN_LIGHT_STOP) + 0x01));

            } else { //if (ix <= CMD_ARG_ALT_LIGHT_STOP) {
                ctrlSys.growMode = 0;
            }
        }
    }
#else

    if (cmd == CMD_LIGHT_ON) {
        if (ix == CMD_ARG_ALL_LIGHT_ON) {
            Serial1.println("+Light On");
            SendCommand(&(cmd), 1);
        }
    } else if (ix == CMD_ARG_ALL_LIGHT_OFF) {
        Serial1.println("+Light Off");
        SendCommand(&(cmd), 1);
    }
#endif
}

//==========================================================================================
// len is the total number of bytes of command and its arguments.
char SendCommand(char * cmd, char len)
{
    int i;
    byte r;
    //String str;

    command[CMD_HEADER_1_IDX] = CMD_HEADER_1_CH;
    command[CMD_HEADER_2_IDX] = CMD_HEADER_2_CH;
    command[CMD_MSG_LEN_IDX] = len + CMD_HEADER_FOOTER_SIZE;
    command[CMD_MSG_LEN_INV_IDX] = ~command[CMD_MSG_LEN_IDX];
    //command[CMD_COMMAND_IDX] = (byte) ( * cmd );

    len += CMD_COMMAND_IDX;

    for (i = CMD_COMMAND_IDX; i < len; i++, cmd++) {
        command[i] = (byte)(*cmd);
    }

    r = 0;
    for (i = len - 1; i >= 0; i--) {
        r += command[i];
    }

    command[len] = (byte)(~r); // CRC one byte.
    command[len + 1] = 0;

    //Serial.write(command, command[CMD_MSG_LEN_IDX] + 1); //TODO:: Aug. 1, 2018. The length is one byte bigger than it is supposed to be. It is for temporariy use to get put the end of data stream since Server is not able to handle properly without it yet. Need to be corrected later once the server is ready.
    i = (size_t)Serial.write(command, command[CMD_MSG_LEN_IDX]);

    if (i != command[CMD_MSG_LEN_IDX]) { // Tx Failure
        delay(5);

        i = (size_t)Serial.write(command, command[CMD_MSG_LEN_IDX]);

        if (i != command[CMD_MSG_LEN_IDX]) { // Tx Failure
            return RESULT_NG;
        }
    }
    lastCmd = command[CMD_COMMAND_IDX]; //Jun. 29, 2019: added to resolve unable to handling received response of CMD_GET_ROOM_USER_INFO command.
    return RESULT_OK;
}

//==========================================================================================
// len is the number of bytes of arguments; it doesn't include command.
char SendResponse(char cmd, char * rsp, char len)
{
    int i;
    byte r;

    command[CMD_HEADER_1_IDX] = CMD_RSP_HEADER_1_CH;
    command[CMD_HEADER_2_IDX] = CMD_RSP_HEADER_2_CH;
    command[CMD_MSG_LEN_IDX] = len + CMD_HEADER_FOOTER_SIZE + 1;
    command[CMD_MSG_LEN_INV_IDX] = ~command[CMD_MSG_LEN_IDX];
    command[CMD_COMMAND_IDX] = cmd;

    len += CMD_ARG_1_IDX;


    for (i = CMD_ARG_1_IDX; i < len; i++, rsp++) {
        command[i] = (byte)(*rsp);
    }


    r = 0;
    for (i = len - 1; i >= 0; i--) {
        r += command[i];
    }

    command[len] = (byte)(~r); // CRC one byte.
    command[len + 1] = 0;
#ifdef DBG_PRINT_RX_IND_MSG
    delay(100);
#endif
    //Serial.write(command, command[CMD_MSG_LEN_IDX] + 1); //TODO:: Aug. 1, 2018. The length is one byte bigger than it is supposed to be. It is for temporariy use to get put the end of data stream since Server is not able to handle properly without it yet. Need to be corrected later once the server is ready.
    i = (size_t)Serial.write(command, command[CMD_MSG_LEN_IDX]);
    if (i != command[CMD_MSG_LEN_IDX]) { // Tx Failure
        delay(5);

        i = (size_t) Serial.write(command, command[CMD_MSG_LEN_IDX]);

        if (i != command[CMD_MSG_LEN_IDX]) { // Tx Failure
            return RESULT_NG;
        }
    }
    return RESULT_OK;
}

void SendPollCommand(bool bNew)
{
    unsigned short t;
    char cmd = CMD_POLL;
    SendCommand(&cmd, 1);

    t = (unsigned short)millis();
    ctrlSys.toResend.cmd = cmd;
    if (bNew) ctrlSys.toResend.cntDwn = 2;
    // next resend time is set to one second later.
    ctrlSys.toResend.nextTime = ((unsigned char)((t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC) + 1)) & RESEND_COUNT_DWN_MISS_RECOVERY; // 0x1F;  // t &= 0x0000FC00;
}

void SendLockRequest(bool bUdt)
{
    char cmd;
    cmd = CMD_LOCK_DOOR;
    SendCommand(&(cmd), 1);

    if (ctrlSys.bLockEn == false) {
        if (bUdt) bDoorClosed = true; // needs to set it true sometimes in case the Control Server is not available.
    }
}

void SendGetValidWifiUserCmd(bool bNew)
{
    unsigned short t;
    char cmd[2];
    cmd[0] = CMD_GET_VALID_WIFI_USER;
    cmd[1] = user.nofValidUser;
    SendCommand(cmd, 2);

    t = (unsigned short)millis();
    ctrlSys.toResend.cmd = cmd[0];
    if( bNew ) ctrlSys.toResend.cntDwn = 2;
    // next resend time is set to one second later.
    ctrlSys.toResend.nextTime = ((unsigned char)((t >> NOF_SHIFT_TO_DELAY_ABOUT_1_SEC) + 1))& RESEND_COUNT_DWN_MISS_RECOVERY; // 0x1F;  // t &= 0x0000FC00;
}

//==========================================================================================
bool GetDoorlockState(void)
{

#ifdef SIMULATE_LOCK_WITH_LED
    if (digitalRead(STATUS_LED_GREEN) == 1) {
        bDoorClosed = true;
        Serial1.println("Door Locked");
    } else {
        bDoorClosed = false;
        Serial1.println("Door Unlocked");
    }
#else
    if (bDoorClosed == true) {
        Serial1.println("++Door Locked");
    } else {
        Serial1.println("++Door Unlocked");
    }
#endif
    return bDoorClosed;
}

//==========================================================================================
//door lock/unlock control page.
bool HandleDoor(void)
{
    unsigned char ix, ct;
    unsigned short ia;

    if (server.hasArg("DrCtrl")) {

        ia = GetLaonId(server.header("Cookie"));

        ct = (unsigned char)server.arg("DrCtrl").toInt();

        if (ct <= CMD_UNLOCK_DOOR_SERVER) {
        
            if (user.nofValidUser > 0) {

                if (ia < VALID_ADMIN_USER) { // general user
                    for (ix = 0; ix < user.nofValidUser; ix++) {
                        if (ia == user.paUser[ix]->cookie) break;
                    }

                    if (ix < user.nofValidUser) {
                        DoorLockControl((char)ct, (char)user.paUser[ix]->userWifiStatus);
                        // server.sendHeader("Location", "/");
                        // server.sendHeader("Cache-Control", "no-cache");
                        // server.send(307);
                        // 204: No Content. The server successfully processed the request and is not returning any content.
                        // 205: Reset Content: The server successfully processed the request, but is not returning any content. 
                        //      Unlike a 204 response, this response requires that the requester reset the document view.
                        server.send(204);

                    }  else {
                        ReadAndSendFile("/login.html", "text/html");

                        Serial1.println("No match LAONID");
                    }

                } else if ( ctrlSys.cookieResetCntDwn > 0 ) {
                    //TODO:: Jun. 1, 2020: it need to be set to proper wifi user id.
                    if( ia < VALID_SUPER_USER_I ) ix = FIRST_SPECIAL_USER__USR_ST;
                    else if (ia < VALID_SUPER_USER_II) ix = FIRST_POWER_USER__USR_ST;
                    else ix = FIRST_SUPER_USER__USR_ST;

                    DoorLockControl((char)ct, (char) ix);
                    server.send(204);

                } else { // ctrlSys.cookieResetCntDwn == 0
                    ReadAndSendFile("/login.html", "text/html");

                    Serial1.println("Login timeout");
                }
                
                return true;
            }
        }
        //delay(10);
    } else if (server.hasArg("AdmSysMg")) {

        HandleAdmin();
        return true;

    } else if (server.hasArg("LightCtrl")) {
        ia = (unsigned char) server.arg("LightCtrl").toInt();
        if (ia < CMD_ARG_RED_LIGHT_OFF) {
            LightOnOff(CMD_LIGHT_ON, ia);
            //   server.sendHeader("Location","/");
            //   server.sendHeader("Cache-Control","no-cache");
            //   server.send(307);
            server.send(204);
            return true;
        } else { //if (ia == CMD_ARG_LIGHT_OFF) {
            LightOnOff(CMD_LIGHT_OFF, ia);
            // server.sendHeader("Location","/");
            // server.sendHeader("Cache-Control","no-cache");
            // server.send(307);
            server.send(204);
            return true;
        }
        //delay(10);
    }

    Serial1.println("*");
    return false;
}




//==========================================================================================
// It is called when it receives a complete message from the Control Server device.
void ExecuteCmd(void)
{
    unsigned char r = 0, i, ix;
    char cmd[3];

    //if (S.ctrlStatus != CTRL_STS_IDLE) r = 1;

    if (rxCmdBuf[CMD_HEADER_2_IDX] == CMD_HEADER_2_CH) { // Command has been received.

        switch (rxCmdBuf[CMD_COMMAND_IDX]) {

        default:
            rxCmdBuf[CMD_COMMAND_IDX] = CMD_UNKNOWN;
            r = 0;
            break;

        case CMD_LOCK_DOOR:
        case CMD_LOCK_DOOR_SERVER:
            ///*if ((S.ctrlStatus == CTRL_STS_DOOR_OPEN) || (S.ctrlStatus == CTRL_STS_DOOR_OPEN_CNF)) {
            //r = 0;
            //}*/
            //LockDoor(rxCmdBuf[CMD_COMMAND_IDX], rxCmdBuf[CMD_ARG_1_IDX]);
            //break;

        case CMD_UNLOCK_DOOR:
        case CMD_UNLOCK_DOOR_SERVER:
            //r = 0;
            DoorLockControl(rxCmdBuf[CMD_COMMAND_IDX], rxCmdBuf[CMD_ARG_1_IDX]);
            break;

        case CMD_UPDATE_DOOR_STATUS:

            if (rxCmdBuf[CMD_ARG_1_IDX] > 0) {
                bDoorClosed = false;
                cmd[0] = 1;
            } else {
                // Jun. 4, 2020: Bug fix: added following 'if' in order to resolve the kept unlock state which 
                //               happened 'Light On' was sent while the door was being unlocked by 'Unlock' command.
                //               Whenever bDoorClosed is set to 'true', you must be carefull not to interfere locking the door,
                //               especially when ctrlSys.bLockEn == true.
                if (ctrlSys.bLockEn == false) bDoorClosed = true;
                cmd[0] = 0;
            }
            SendResponse(CMD_UPDATE_DOOR_STATUS, cmd, 1);
            break;

        case CMD_LIGHT_OFF:
            LightOnOff((char)rxCmdBuf[CMD_COMMAND_IDX], CMD_ARG_ALL_LIGHT_OFF);
            break;
        case CMD_LIGHT_ON:
            LightOnOff((char)rxCmdBuf[CMD_COMMAND_IDX], CMD_ARG_ALL_LIGHT_ON);
            break;
        case CMD_UPDATE_LIGHT_STATUS:
            if (rxCmdBuf[CMD_ARG_1_IDX] > 0) {
                bLightOn = true;
            } else {
                bLightOn = false;
            }
            break;

        case CMD_POLL: // 0x20 // command.
            
            //`>06F92062 
            //`<08F7.20 01 01 62  <== example for one valid WiFi user record available;
            //                         two paramenters: sys.status and user.nofValidWifiUser in order.
            if (ctrlSys.ctrlServerState == INIT__ST) {                
                if (rxCmdBuf[CMD_ARG_1_IDX] == IDLE__ST) {
                    ctrlSys.ctrlServerState = rxCmdBuf[CMD_ARG_1_IDX];

                    if (user.nofValidUser < rxCmdBuf[CMD_ARG_1_IDX + 1]) {
                        
                        for (i = 0; i < user.nofValidUser; i++) {
                            delete(user.paUser[i]);
                            user.paUser[i] = 0;
                        }
                        
                        user.nofValidUser = 0;

                        SendGetValidWifiUserCmd(true);
                    }
                }
            } else {
                
                if (rxCmdBuf[CMD_ARG_1_IDX] == IDLE__ST) {
                    if (user.nofValidUser != rxCmdBuf[CMD_ARG_1_IDX + 1]) {
                        for (i = 0; i < user.nofValidUser; i++) {
                            delete(user.paUser[i]);
                            user.paUser[i] = 0;
                        }

                        user.nofValidUser = 0;

                        SendGetValidWifiUserCmd(true);
                    } else {
                        //cmd[0] = CMD_POLL;
                        cmd[0] = RESULT_OK;
                        cmd[1] = user.nofValidUser;
                        SendResponse(CMD_POLL, cmd, 2);
                    }
                }
            }
            break;

        case CMD_ALERT_STATUS_CHANGE:

            //cmd[0] = CMD_ALERT_STATUS_CHANGE;
            cmd[0] = RESULT_NG;
            cmd[1] = rxCmdBuf[CMD_ARG_1_IDX];

            switch (rxCmdBuf[CMD_ARG_1_IDX]) {

            case ARG_ALERT_WIFI_REC_CHANGE:
                if (rxCmdBuf[CMD_MSG_LEN_IDX] == (CMD_RSP_ACK_NAK_IDX + RSP_LEN_ARG_ALERT_WIFI_REC_CHANGE)) {
                    if (rxCmdBuf[CMD_ARG_1_IDX + 1] == user.nofValidUser) { // the number of valid Wifi user matches.
                        if (rxCmdBuf[CMD_ARG_1_IDX + 2] < user.nofValidUser) { // the index to the modified WiFi user is valid one.

                            ix = rxCmdBuf[CMD_ARG_1_IDX + 2]; // the index to the modified Wifi user record.

                            if (user.paUser[ix]->userWifiStatus == rxCmdBuf[CMD_ARG_1_IDX + 3]) { // user Id is matches.

                                r = CMD_ARG_1_IDX + 4;
                                for (i = 0; i < SIZE_INITIAL; i++) {
                                    user.paUser[ix]->initial[i] = rxCmdBuf[r + i];
                                }

                                r += i;

                                for (i = 0; i < SIZE_PASSWORD; i++) {
                                    user.paUser[ix]->password[i] = rxCmdBuf[r + i];
                                }
                                cmd[0] = RESULT_OK;
                            }
                        }
                    }
                }
                break;

            case ARG_ALERT_WIFI_REC_ADD:
                if (rxCmdBuf[CMD_MSG_LEN_IDX] == (CMD_RSP_ACK_NAK_IDX + RSP_LEN_ARG_ALERT_WIFI_REC_ADD)) {
                    if (user.nofValidUser < rxCmdBuf[CMD_ARG_1_IDX + 1]) { // the number of valid WiFi user in the Control System is greater than one in this system.
                        if (user.nofValidUser < NOF_MAX_VALID_USER_IDX) {
                            ix = user.nofValidUser;
                            if (user.paUser[ix] == 0) user.paUser[ix] = new WifiUserRecord_t;
                            user.paUser[ix]->userWifiStatus = rxCmdBuf[CMD_ARG_1_IDX + 2];

                            r = CMD_ARG_1_IDX + 3;
                            for (i = 0; i < SIZE_INITIAL; i++) {
                                user.paUser[ix]->initial[i] = rxCmdBuf[r + i];
                            }

                            r += i;

                            for (i = 0; i < SIZE_PASSWORD; i++) {
                                user.paUser[ix]->password[i] = rxCmdBuf[r + i];
                            }

                            user.nofValidUser++;
                            cmd[0] = RESULT_OK;
                        }
                    }
                }
                break;

            case ARG_ALERT_WIFI_REC_DELETE:
                if (rxCmdBuf[CMD_MSG_LEN_IDX] == (CMD_RSP_ACK_NAK_IDX + RSP_LEN_ARG_ALERT_WIFI_REC_DELETE)) {
                    if (rxCmdBuf[CMD_ARG_1_IDX + 1] < user.nofValidUser) { // the number of valid Wifi user in the Control System is smaller.
                        if (rxCmdBuf[CMD_ARG_1_IDX + 2] < user.nofValidUser) { // the index to the WiFi user record to be deleted is valid one.

                            ix = rxCmdBuf[CMD_ARG_1_IDX + 2]; // the index to the Wifi user record to delete.

                            if (user.paUser[ix]->userWifiStatus == rxCmdBuf[CMD_ARG_1_IDX + 3]) { // user Id is matches.

                                delete(user.paUser[ix]);

                                i = ix + 1;
                                for (; i < user.nofValidUser; i++, ix++) {
                                    user.paUser[ix] = user.paUser[i];
                                }

                                user.nofValidUser--;
                                cmd[0] = RESULT_OK;
                            }
                        }
                    }
                }
                break;
            }

            //SendCommand(cmd, 3);
            SendResponse(CMD_ALERT_STATUS_CHANGE, cmd, 2);
            break;
        }







    } else if (rxCmdBuf[CMD_HEADER_2_IDX] == CMD_RSP_HEADER_2_CH) { // Response has been received.

        rxCmdReady = 0;

        if (lastCmd == rxCmdBuf[CMD_COMMAND_IDX]) {


            ///////////////////////
            switch (rxCmdBuf[CMD_COMMAND_IDX]) {

            default:
                break;

            case CMD_LOCK_DOOR:
            case CMD_LOCK_DOOR_SERVER:
                if (rxCmdBuf[CMD_ARG_1_IDX] > 0) {
                    // Jun. 4, 2020: Bug fix: added following 'if' in order to resolve the kept unlock state which 
                    //               happened 'Light On' was sent while the door was being unlocked by 'Unlock' command.
                    //               Whenever bDoorClosed is set to 'true', you must be carefull not to interfere locking
                    //               the door, especially when ctrlSys.bLockEn == true.
                    if (ctrlSys.bLockEn == false) bDoorClosed = true; // arg is ture (lock done)
                }  else bDoorClosed = false;
                break;

            case CMD_UNLOCK_DOOR:
            case CMD_UNLOCK_DOOR_SERVER:
                if (rxCmdBuf[CMD_ARG_1_IDX] > 0) bDoorClosed = false; // arg is true (unlock done).
                else if (ctrlSys.bLockEn == false) { // Jun. 4, 2020: Bug fix: added following 'if' in order to resolve 
                //               the kept unlock state which happened 'Light On' was sent while the door was being unlocked 
                //               by 'Unlock' command. Whenever bDoorClosed is set to 'true', you must be carefull not to 
                //               interfere locking the door, especially when ctrlSys.bLockEn == true.
                    bDoorClosed = true;
                }
                break;


            case CMD_LIGHT_OFF:
                if (rxCmdBuf[CMD_ARG_1_IDX] > 0) bLightOn = false; // arg is true (turn-off is done).
                else bLightOn = true;
                break;

            case CMD_LIGHT_ON:
                if (rxCmdBuf[CMD_ARG_1_IDX] > 0) bLightOn = true; // arg is true (turn-on is done).
                else bLightOn = false;
                break;
                
            case CMD_POLL:  // 0x20 // response received.
                // Flow of 3 valid WiFi user records example, when this WiFi server has all 3 records already:
                //`>06F9 20 42        <== to Control Server
                //'<08F7.20 01 03 40  <== from Control Server for this 'case' statement.

                // Flow of 3 valid WiFi user records, especially when this WiFi server doesn't have an record yet:
                //`>06F9 20 42        <== to Control Server
                //'<08F7.20 01 03 40  <== from Control Server for this 'case' statement.
                //                         two paramenters: sys.status and user.nofValidWifiUser in order.
                //`>07F8 93 00 CF     <== to Control Server to request the first valid wifi user record.
                //'<16E9.93 03 11 626967726D31 3131313131313131 F3 <== from Control Server.
                //`>07F8 93 01 CE
                //'<16E9.93 03 22 736D616C6C32 3232323232323232 D1 
                //`>07F8 93 02 CD
                //'<16E9.93 03 33 4C61756E6433 3333333333333333 DC 
                //`>07F8 93 03 CC
                //'<08F7.93 00 03 CE 

                
                if (rxCmdBuf[CMD_ARG_1_IDX] == IDLE__ST) {
                    ctrlSys.ctrlServerState = rxCmdBuf[CMD_ARG_1_IDX];

                    if (user.nofValidUser < rxCmdBuf[CMD_ARG_1_IDX + 1]) {
                        /*cmd[0] = CMD_GET_VALID_WIFI_USER;
                        cmd[1] = user.nofValidUser;
                        SendCommand(cmd, 2);*/
                        SendGetValidWifiUserCmd(true);
                    } else { 
                        if (user.nofValidUser > rxCmdBuf[CMD_ARG_1_IDX + 1]) {
                            for (i = rxCmdBuf[CMD_ARG_1_IDX + 1]; i < user.nofValidUser; i++) {
                                delete(user.paUser[i]);
                                user.paUser[i] = 0;
                            }

                            user.nofValidUser = rxCmdBuf[CMD_ARG_1_IDX + 1];
                        }
                        InitResendBuf();
                    }
                } else if (rxCmdBuf[CMD_ARG_1_IDX] == INIT__ST) {
                    SendPollCommand(true);
                }
                break;

            case CMD_GET_ROOM_USER_INFO: // received given room's user information; both valid and blocked users.
                if( uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd ) {
                    
                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX]) {

                        r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 2; // setting data length. -2 is for command and the first arg which is a room index.

                        for (i = 0, ix = CMD_ARG_1_IDX + 1; i < r; i++, ix++) {
                            uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                        }

                    } else if (rxCmdBuf[CMD_ARG_1_IDX] == 0) { // error
                        uSettingInfo.aSettingData[0] = rxCmdBuf[CMD_ARG_1_IDX];   // to send 0 in the place of room index in order to let the esp8266 client know there is an error.
                        uSettingInfo.aSettingData[1] = rxCmdBuf[CMD_ARG_1_IDX+1]; // error code.
                    }

                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // to let HandleXML know there is received data even if there is no valid/blocked user in the specified room where there is no user index.
                }
                break;

            case CMD_GET_USER_INFO: // received max 2 user records; no record, one or two records of one or mix of valid and blocked users. Each record from either or both of buffer and external EEPROM; userStatus, card/user ID, etc..
                //`>07F88621BB  `<27D88621213377C6DB17070700000000FF0FFF2D2D2D2D2D2DFFFFFFFFFFFFFFFF00000019
                //`>07F88622BA  `<27D88622220698C6DB17070700000000FF0FFF2D2D2D2D2D2DFFFFFFFFFFFFFFFF00000023

                if ( uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd ) {

                    //if ((uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX]) || (rxCmdBuf[CMD_COMMAND_IDX] == CMD_SCAN_USER_FOB)) {
                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX]) {

                        r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 2; // setting data length. -2 is for command and the first arg which is a room index.

                        for (i = 0, ix = CMD_ARG_1_IDX + 1; i < r; i++, ix++) {
                            uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                        }

                    } else if (rxCmdBuf[CMD_ARG_1_IDX] == 0) { // error
                        uSettingInfo.aSettingData[0] = rxCmdBuf[CMD_ARG_1_IDX];   // to send 0 in the place of room index in order to let the esp8266 client know there is an error.
                        uSettingInfo.aSettingData[1] = rxCmdBuf[CMD_ARG_1_IDX + 1]; // error code.
                    }
                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // replace the initial value 0 with the length in order to indicate the response has been received.
                }
                break;

                
            case CMD_SCAN_AND_SAVE_FOB:
                //`>07F88E21D3  `<08F78E0000D5  // in progress
                //`>07F88E21BB  `<27D88E21213377C6DB17070700000000FF0FFF2D2D2D2D2D2DFFFFFFFFFFFFFFFF00000019

            // TODO:: Feb. 20, 2020: the response to the CMD_SCAN_USER_FOB might be only User Fob Account data necessary; no User Wifi Account.
            //        Otherwise default User Wifi Account data can be a part of the response from the Door Control.
            case CMD_SCAN_USER_FOB:                
                //`>07F88F00D3  `<08F78F0000D5  // in progress
                //`>07F88F22BA  `<27D88F22220698C6DB17070700000000FF0FFF2D2D2D2D2D2DFFFFFFFFFFFFFFFF00000023

                if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd) {

                    //if ((uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX]) || (rxCmdBuf[CMD_COMMAND_IDX] == CMD_SCAN_USER_FOB)) {
                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX]) {

                        r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 2; // setting data length. -2 is for command and the first arg which is a room index.

                        for ( i = 0, ix = CMD_ARG_1_IDX + 1; i < r; i++, ix++) {
                            uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                        }

                    } else if (rxCmdBuf[CMD_COMMAND_IDX] == CMD_SCAN_AND_SAVE_FOB) {
                        if (rxCmdBuf[CMD_ARG_1_IDX] == WIFI_CLIENT_SCAN_ADD_REQ_IN_PROGRESS) { // error or in-progress
                            if (rxCmdBuf[CMD_ARG_1_IDX + 1] > 0) { // error
                                uSettingInfo.aSettingData[0] = rxCmdBuf[CMD_ARG_1_IDX];   // to send 0 in the place of room index in order to let the esp8266 client know there is an error.
                                uSettingInfo.aSettingData[1] = rxCmdBuf[CMD_ARG_1_IDX + 1]; // error code.
                            } else break; // in-progress. set no data received.
                        }
                    } else if (rxCmdBuf[CMD_ARG_1_IDX] == WIFI_CLIENT_REQ_IN_PROGRESS) { // error or in-progress

                        if (rxCmdBuf[CMD_ARG_1_IDX + 1] > 0) { // error
                            uSettingInfo.aSettingData[0] = rxCmdBuf[CMD_ARG_1_IDX];   // to send 0 in the place of room index in order to let the esp8266 client know there is an error.
                            uSettingInfo.aSettingData[1] = rxCmdBuf[CMD_ARG_1_IDX + 1]; // error code.
                        } else break; // in-progress. set no data received.
                    }
                    
                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // replace the initial value 0 with the length in order to indicate the response has been received.
                }
                break;

            case CMD_SAVE_FOB_USER:
                if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd) {

                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX + 1]) { //Oct. 9, 2019: added '+ 1' to get user ID.

                        if (rxCmdBuf[CMD_ARG_1_IDX] == WIFI_CLIENT_REQ_IN_PROGRESS) break;

                        r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 1; // setting data length. -1 is for command.

                        for ( i = 0, ix = CMD_ARG_1_IDX; i < r; i++, ix++) { // get all received arguments; argument only.
                            uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                        }
                    }
                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // replace the initial value 0 with the length in order to indicate the response has been received.
                }
                break;

            case CMD_FORMAT_FOB_SEC_CODE:
                if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd) {
                    if (rxCmdBuf[CMD_ARG_1_IDX] != WIFI_CLIENT_REQ_IN_PROGRESS) {
                        //if (rxCmdBuf[CMD_ARG_1_IDX] == RESULT_OK) {
                        //    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX + 1]) { //security code block index

                                r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 1; // setting data length. -1 is for command.

                                for ( i = 0, ix = CMD_ARG_1_IDX; i < r; i++, ix++) { // get all received arguments; argument only.
                                    uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                                }
                        //    } else {
                        //        uSettingInfo.aSettingData[0] = RESULT_OK;
                        //        uSettingInfo.aSettingData[1] = ERROR_IF_UNKNOWN_RSP_ARG;
                        //    }

                            

                        //} else {
                        //    uSettingInfo.aSettingData[0] = rxCmdBuf[CMD_ARG_1_IDX];
                        //    uSettingInfo.aSettingData[1] = rxCmdBuf[CMD_ARG_1_IDX + 1];
                        //}

                        uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // replace the initial value 0 with the length in order to indicate the response has been received.
                    }
                }
                break;

            case CMD_SAVE_WIFI_USER: // 0x92  rx reponse
                // `<09F6 92 01 11 00 C0
                if (uSettingInfo.dataHead[USET_INFO_CMD__HDDT_IDX] == lastCmd) {

                    if (uSettingInfo.dataHead[USET_INFO_ARG__HDDT_IDX] == rxCmdBuf[CMD_ARG_1_IDX + 1]) { //Oct. 9, 2019: added '+ 1' to get user ID.

                        //if (rxCmdBuf[CMD_ARG_1_IDX] == WIFI_CLIENT_REQ_IN_PROGRESS) break;

                        r = rxCmdBuf[CMD_MSG_LEN_IDX] - CMD_HEADER_FOOTER_SIZE - 1; // setting data length. -1 is for command.

                        for ( i = 0, ix = CMD_ARG_1_IDX; i < r; i++, ix++) { // get all received arguments; argument only.
                            uSettingInfo.aSettingData[i] = rxCmdBuf[ix];
                        }

                        if (rxCmdBuf[CMD_ARG_1_IDX + 2] == RESULT_OK) { // update valid wifi user info.
                            r = user.newUser.userWifiStatus & USR_ST__MASK_FULL_USERS_ID;
                            for (i = 0; i < user.nofValidUser; i++) {
                                if (user.paUser[i]->userWifiStatus == r) break;
                            }
                                                        
                            if ( (user.newUser.userWifiStatus & USR_ST__MASK_BLOCKED_USER) > 0) { // delete wifi user info.
                                if (i < user.nofValidUser) { // existing valid wifi user data.
                                    delete (user.paUser[i]);
                                    for (i++; i < user.nofValidUser; i++)
                                        user.paUser[i - 1] = user.paUser[i];
                                    user.nofValidUser--;
                                    user.paUser[user.nofValidUser] = 0;
                                }
                                //user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate key generation is not requested. // May 29, 2020: moved to HandleUserSettings().

                            } else { // update a valid wifi user info.

                                if (i == user.nofValidUser) { // brand new valid wifi user data.
                                    
                                    if (user.nofValidUser < NOF_MAX_VALID_USER_IDX) {
                                        user.paUser[i] = new WifiUserRecord_t;
                                        user.paUser[i]->userWifiStatus = user.newUser.userWifiStatus;

                                        user.nofValidUser++;
                                        //user.newUser.cookie = LOGIN_KEY_GEN_EN_REQ; // To indicate key generation is requested. // May 29, 2020: moved to HandleUserSettings().
                                    }
                                    
                                } // else // existing valid wifi user data.

                                if (user.nofValidUser <= NOF_MAX_VALID_USER_IDX) {
                                    for (r = 0; r < SIZE_INITIAL; r++) {
                                        user.paUser[i]->initial[r] = user.newUser.initial[r];
                                    }
                                    for (r = 0; r < SIZE_PASSWORD; r++) {
                                        user.paUser[i]->password[r] = user.newUser.password[r];
                                    }
                                }
                            }

                            //user.newUser.userWifiStatus = INVALID_USER_WIFI_STATUS;

                        } //else user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate key generation is not requested. // May 29, 2020: moved to HandleUserSettings().

                    } //else user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate key generation is not requested. // May 29, 2020: moved to HandleUserSettings().

                    uSettingInfo.dataHead[USET_INFO_LEN__HDDT_IDX] = rxCmdBuf[CMD_MSG_LEN_IDX]; // replace the initial value 0 with the length in order to indicate the response has been received.

                } //else user.newUser.cookie = LOGIN_KEY_GEN_OFF; // To indicate key generation is not requested. // May 29, 2020: moved to HandleUserSettings().
                break;

            case CMD_GET_VALID_WIFI_USER: // 0x93. Response.
                // An example of pares of a command and response when there are 3 valid wifi users.
                // These responses are sent by the Control Server; once it is received, this Wifi server will send
                // a command to request the next wifi user info.
                //   `>07F8 93 00 CF  `<17E8.93 00 03 11 626967310000 3131313131313131 D2  // a request and response of the first valid wifi user's info.
                //   `>07F8 93 01 CE  `<17E8.93 01 03 22 736D616C6C32 3232323232323232 D0  // a request and response of the 2nd valid wifi user's info.
                //   `>07F8 93 02 CD  `<17E8.93 02 03 33 4C61756E6433 3333333333333333 DA  // a request and response of the 3rd valid wifi user's info.
                //   `>07F8 93 03 CC  `<09F6.93 00 03 03 CB <== // a request of the 4th valid wifi user's info. received an indication to no more valid wifi users. 

                r = CMD_ARG_1_IDX;

                if (rxCmdBuf[CMD_MSG_LEN_IDX] == (CMD_RSP_ACK_NAK_IDX + RSP_LEN_CMD_GET_VALID_WIFI_USER)) {
                    
                    if (user.nofValidUser == rxCmdBuf[r++]) {
                        if (user.nofValidUser < rxCmdBuf[r++]) {
                            if (user.nofValidUser < NOF_MAX_VALID_USER_IDX) {
                                ix = user.nofValidUser;
                                if (user.paUser[ix] == 0) user.paUser[ix] = new WifiUserRecord_t;

                                user.paUser[ix]->userWifiStatus = rxCmdBuf[r++];

                                for (i = 0; i < SIZE_INITIAL; i++) {
                                    user.paUser[ix]->initial[i] = rxCmdBuf[r + i];
                                }

                                r += i;

                                for (i = 0; i < SIZE_PASSWORD; i++) {
                                    user.paUser[ix]->password[i] = rxCmdBuf[r + i];
                                }

                                user.nofValidUser++;
                                ////SendPollCommand();
                                //cmd[0] = CMD_GET_VALID_WIFI_USER;
                                //cmd[1] = user.nofValidUser;
                                //SendCommand(cmd, 2);

                                SendGetValidWifiUserCmd(true);
                            }
                        }
                    }
                } else if (rxCmdBuf[CMD_MSG_LEN_IDX] == (CMD_RSP_ACK_NAK_IDX + RSP_LEN_CMD_GET_VALID_WIFI_USER_DONE)) {
                    if (rxCmdBuf[r++] == RESULT_OK) {
                        if (user.nofValidUser == rxCmdBuf[r++]) {
                            if (user.nofValidUser == rxCmdBuf[r]) {
                                InitResendBuf();
                            }
                        }
                    }
                }
                break;
            }
        }

        r = 1;

    } else {  // else if (rxCmdBuf[CMD_HEADER_2_IDX] == CMD_RSP_HEADER_2_CH) { // Response has been received.
        r = 1;
        rxCmdReady = 0;
    }

    if (r == 0) {
        if (rxCmdBuf[CMD_COMMAND_IDX] != CMD_UNKNOWN) {
            lastCmd = rxCmdBuf[CMD_COMMAND_IDX];
        }

        rxCmdReady = 0;
    }
}

//==========================================================================================
void OnUartRxd(void)
{
    uint8_t byteRxd, bf;

    byteRxd = (uint8_t)Serial.read();

    if (rxCmdReady == 0) {
        switch (rxCmdIdx) {
        case CMD_HEADER_1_IDX:
            if (byteRxd == CMD_HEADER_1_CH) rxCmdBuf[rxCmdIdx++] = byteRxd;
            break;

        case CMD_HEADER_2_IDX:
            if ((byteRxd == CMD_HEADER_2_CH) || (byteRxd == CMD_RSP_HEADER_2_CH)) rxCmdBuf[rxCmdIdx++] = byteRxd;
            else rxCmdIdx = 0;
            break;

        case CMD_MSG_LEN_IDX:
            if (byteRxd <= NOF_BYTE_RX_CMD_BUF) rxCmdBuf[rxCmdIdx++] = byteRxd;
            else rxCmdIdx = 0;
            break;

        case CMD_MSG_LEN_INV_IDX:
            bf = ~byteRxd;
            if (bf == rxCmdBuf[CMD_MSG_LEN_IDX]) {
                rxCmdBuf[rxCmdIdx++] = byteRxd;
                //rxCmdLen = byteRxd;
            } else {
                rxCmdIdx = 0;
            }
            break;

            //case CMD_COMMAND_IDX:
        default:
            if (rxCmdIdx < CMD_COMMAND_IDX) {

                rxCmdIdx = 0;

            } else if (rxCmdIdx < rxCmdBuf[CMD_MSG_LEN_IDX]) {

                rxCmdBuf[rxCmdIdx++] = byteRxd;

            } else {
                rxCmdIdx = 0;
            }

            if (rxCmdIdx == rxCmdBuf[CMD_MSG_LEN_IDX]) {

                byteRxd = 0;

                for (signed i = rxCmdBuf[CMD_MSG_LEN_IDX] - 2; i >= 0; i--) {
                    byteRxd += rxCmdBuf[i];
                }

                byteRxd += rxCmdBuf[rxCmdBuf[CMD_MSG_LEN_IDX] - 1];
                if (byteRxd == 0xFF) {
                    rxCmdReady = 1;
                }

                rxCmdIdx = 0;
            }
            //< NOF_BYTE_RX_CMD_BUF) {
            break;
        }

        rxCmdBuf[rxCmdIdx] = 0; // Reset to zero either next available buffer or the first member depending on the rxCmdIdx value.

        rxTimeOut = millis();
        rxTimeOut += 10;
    }

}

//==========================================================================================
bool HandleUartMsg(void)
{
    OnUartRxd();

    if (rxCmdReady == 1) {
#ifdef DBG_PRINT_RX_IND_MSG
        Serial1.println("+Msg Received!!!");
#endif
        ExecuteCmd();
        return true;
    }
    /*else {
    Serial1.println("+");
    }*/

    return false;
}

//==========================================================================================
void SwitchRxBufIndex(void)
{
    if (rxDoneInd == RX_NOT_DONE_IND__SERVER) {
        rxDoneInd = rxBufIdx;
        //rxBuf[rxBufIdx][NOF_BYTE_ONE_RX_BUF] = 0;
        rxBufIdx = (rxBufIdx + 1) % 2;
        indexBuf = 0;

    }
}

//==========================================================================================
//==========================================================================================
