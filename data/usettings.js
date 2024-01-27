// shared with both SmHomeWifiWebServer.h and command.hpp
const MAX_NOF_USERS_PER_ID = 1; //Apr. 18, 2020: decreased to 1. //Mar. 7, 2020: decreased from 3 to 2.  // max number of rows for user records. you can increase it to view more user records.
const MAX_NOF_ROOM = 3;
// shared with both SmHomeWifiWebServer.h and command.hpp
const MAX_NOF_USERS_PER_ROOM = 3; //15; // Mar. 21, 2020: set it to 3, since the update countdown is supported for only 3 users per room.
const NOF_SEC_CODE_BLOCK = 4;
const NOF_SEC_UPDATE_OPT = 8;
const NOF_SEC_CODE_IDX_OPT = 6;


//==> Mar. 19, 2020:  NOTE: Do not change the order of definitions below !!!
//=> shared with access.hpp as slightly different name such as 'OFF___SC_UPDT_ARG__WIFI_CMD' for 'SC_UPDT_MOD__OFF'.
const SC_UPDT_MOD__OFF            = 0;
//const  SC_UPDT_MOD___ON         = 1;
//const  SC_UPDT_MOD___ONCE       = 2;
//const  SC_UPDT_MOD___WIFI_ONLY  = 3;
const SC_UPDT_MOD__FORCE_NOW      = 4;
//const SC_UPDT_MOD___FORCE_SYNC  = 5;
//const SC_UPDT_MOD___FAIL        = 6;
const SC_UPDT_MOD__NOT_APPLICABLE = 7;
const MASK_SC_UPDT_MOD___OPTIONS  = 0x07; //0x7F;
const MASK_SC_UPDT_MOD___WIFI_DOOR_OPENED_IND = 0x40;
const MASK_SC_UPDT_MOD___FAIL_IND  = 0x80;

//<=
//<==
const FIRST_SEC_UPDATE_OPT_TO_HIDE = 5;


const SEC_CODE_IDX_OPT_NA = 4;
const SEC_CODE_IDX_OPT_ALL = 5;
//==> it is shared with access.hpp and SmHomeWifiWebServer.h
//const INVALID_SECURITY_CODE_INDEX = 9;
const SECURITY_CODE_INDEX_FOR_ALL = 5; // == SEC_CODE_IDX_OPT_ALL

const SIZE_INITIAL = 6;
const MIN_SIZE_INITIAL = 2;
const SIZE_REG_DATE = 3;
const SIZE_PASSWORD = 8;
//<==

//==> Shared with both Control Server and SmHomeWifiWebServer.h.
const USR_ST__MASK_BLOCKED_USER     = 0x80;
//const VALID_USER_WIFI_STATUS__iEE   = 0x00;
//const BLOCKED_USER_WIFI_STATUS__iEE = 0x80;
//const INVALID_USER_WIFI_STATUS__iEE = 0xFF;
//<==

const CMD_GET_USERS = 1;
const CMD_READ_USERS = 10;
const CMD_SCAN_ADD = 15;
const CMD_SCAN = 20;
const CMD_SAVE_FOB = 25;
const CMD_FORMAT_FOB = 27;
const CMD_READ_MAC = 30;
const CMD_SAVE_WIFI = 35;

const NOF_FOB_TBL_COL = 8;
const NOF_WIFI_TBL_COL = 10;

var waitingRspId = 0;            
var maxTimerRptCnt;
var myRspTimer;
var xmlQueryCnt = 0;
var numRegUser = 0;             
            
//var fobUser = [[0, 0, false, 0, "mm/dd/yyyy", 32, 0, 0, false]];
var fobUser = new Array(5);
//var wifiUser = [[0, 0, false, "?", "mm/dd/yyyy", 32, 255, "n/a", 255, "n/a", false]]; // Room id, User id, valid user (true/false), user name, password, reg. date, host I, mac I, host II, mac II, delete.
var wifiUser = new Array(5);
//var numUserElement = [NOF_FOB_TBL_COL, NOF_WIFI_TBL_COL]; // the number of element of each fobUser and wifiUser respectively.
var ctrl = {
    lastRoomIx: 0,
    lastUserIx: 0,
    curReqVal: 0,
    del: false
};

let bKeyGenReq = false;

for (var i = 0; i < MAX_NOF_USERS_PER_ID; i++) {
    fobUser[i] = new Array(NOF_FOB_TBL_COL+1);
    wifiUser[i] = new Array(NOF_WIFI_TBL_COL+1);
}

fobUser[0] = [0, 0, false, 0, "2000-01-01", 32, 0, 0, false]; // room id, user id, valid user (true/false), fob s/n number, reg. date, security code, security update req/status, security update period (days), delete.
wifiUser[0] = [0, 0, false, "?", "?", "2000-01-01", 255, "00:00:00:00:00:00", 255, "FF:FF:FF:FF:FF:FF", false]; // Room id, User id, valid user (true/false), user name, password, reg. date, host I, mac I, host II, mac II, delete.

var $ = function(id) { return document.getElementById(id);}

var xmlHttp = new CreateXmlHttpObject();
var xmlResponse;

function CreateXmlHttpObject() {
    var xHttp;
    if(window.XMLHttpRequest){
        xHttp = new XMLHttpRequest();
    }else{
        xHttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    return xHttp;
}



var handleServerResponse = function() {
    if( xmlHttp.readyState == 4 && xmlHttp.status == 200 ){
        var message = "";
        if (xmlQueryCnt <= maxTimerRptCnt) {
            //if( xmlHttp.responseXML ) $('msgwin').innerHTML = "No responseXML found!";
            if (xmlHttp.responseXML) $('msgwin').textContent = "No responseXML found!";
                        
            xmlResponse = xmlHttp.responseXML;
                        
            xmldoc = xmlResponse.getElementsByTagName('rlt');
                        
            //if (xmldoc[0].innerHTML.toString() === 'ok') { // got rid of toString() since it doesn't work in IE.
            //if (xmldoc[0].firstChild.nodeValue === 'ok') {
            if (xmldoc[0].textContent === 'ok') {
                var cmd;
                var sel;
                var n, i, ln;

                xmldoc = xmlResponse.getElementsByTagName('cmd');
                //cmd = xmldoc[0].innerHTML.toString(); // got rid of toString() since it doesn't work in IE.
                cmd = xmldoc[0].textContent; //firstChild.nodeValue;

                switch (cmd) {
                    case '1':
                        sel = $('user');

                        //xmldoc = xmlResponse.getElementsByTagName('rlt');                                
                        //if( xmldoc.childNodes[0].innerHTML === 'ok' ) {                                
                        xmldoc = xmlResponse.getElementsByTagName('li');
                        n = xmldoc.length;

                        for (i = 0, j = 1; i < n; i++, j++) {
                            if (sel.length <= j) break;
                            ////sel.option[j].innerHTML = xmldoc[i].innerHTML;
                            //sel[j].innerHTML = xmldoc[i].innerHTML;
                            sel[j].textContent = xmldoc[i].textContent;
                            sel[j].value = xmldoc[i].textContent;
                        }

                        if (xmlQueryCnt < maxTimerRptCnt) xmlQueryCnt = maxTimerRptCnt;
                        //sel.style.backgroundColor = "yellow";
                        ////$('getUsers').disabled = false;
                        //sel.disabled = false;
                        setUserSelectOption( 0, false, "Yellow" );
                        //}

                        ShowResult("  Got the list of room users !", true);
                        break;

                    case '15': // scan & add
                        $('user')[ctrl.lastUserIx].textContent = ctrl.lastUserIx.toString();
                    case '10':
                    case '20': // scan
                        sel = $('fobTable');
                                                        
                        xmldoc = xmlResponse.getElementsByTagName('idx');
                        n = parseInt(xmldoc[0].textContent);

                        //if (ctrl.curReqVal === parseInt(xmldoc[0].innerHTML)) { // got rid of toString() since it doesn't work in IE.
                        if ( ctrl.curReqVal === n ) { // got rid of toString() since it doesn't work in IE.
                            xmldoc = xmlResponse.getElementsByTagName('len');

                            ////if ( parseInt(xmldoc[0].innerHTML, 16) > 0) {
                            //ln = parseInt(xmldoc[0].innerHTML.toString());
                            ln = parseInt(xmldoc[0].textContent);
                            if (ln > 0) {
                                populateTable();
                            }
                            //numRegUser = xmldoc.length;

                            //for (var i = 0, j = 1; i <= numRegUser; i++, j++) {
                            //    if (sel.length <= j) break;
                            //    sel.option[j].innerHTML = xmldoc[i].innerHTML;
                            //}
                        }

                        if (xmlQueryCnt < maxTimerRptCnt) xmlQueryCnt = maxTimerRptCnt;
                        //$('getUsers').disabled = false;
                        sel.disabled = false;

                        for (i = 1; i < sel.rows.length; i++) {
                            enableTableInput(1, i);
                        }

                        $('scanFob').disabled = false;
                        //$('saveFUser').disabled = false;

                        n = $('wifiTable').rows.length;
                        for (i = 1; i < n; i++) {
                            enableTableInput(2, i);
                        }

                        $('readMac').disabled = false;
                        $('saveWifi').disabled = false;
                        $('genKey').disabled = false;
                        //}
                        if ( cmd !== '10' ) {
                            xmldoc = xmlResponse.getElementsByTagName('usr');
                            n = parseInt(xmldoc[0].textContent);

                            if (ctrl.curReqVal !== (n & 0x7F)) {

                                if (cmd === '15') {
                                    message = " Existing";

                                    if (cmd === '15') message = " Not Saved ! &nbsp;&nbsp;" + message;

                                    if ((n & 0x80) > 0) {
                                        message += " blocked";
                                    }

                                    message += " user:  ";

                                    if (n < 0x20) message += "Big";
                                    else if (n > 0x30) message += "Laundry";
                                    else message += "Small";

                                    message += " Room No. ";

                                    i = n % 0x10;
                                    message += i.toString() + " !";
                                    ShowResult(message, false);
                                    ln = 0xFF;

                                    $('saveFUser').disabled = true;

                                } else {
                                    var ct;

                                    if (n < 0x80) ct = "";
                                    else ct = " (block)";

                                    $('room').selectedIndex = (n &= 0x7F) >> 4;
                                    n &= 0x0F;

                                    sel = $('user');
                                    if (sel.length < 16) {
                                        var opt;
                                        for (var i = sel.length; i <= MAX_NOF_USERS_PER_ROOM; i++) {
                                            opt = document.createElement("option");
                                            opt.text = i + " (x)";
                                            sel.add(opt);
                                        }
                                    }

                                    //for (i = 1; i < 5; i++) {
                                    //    if (sel.length <= i) break;
                                    //    ct = "<option value='" + i.toString() + "'>" + i.toString() + " (n/a)</option>";
                                    //    sel[i].textContent = ct;
                                    //}

                                    //xmldoc = xmlResponse.getElementsByTagName('vu');

                                    ct = n.toString() + ct;
                                    //if (xmldoc[0].textContent === 'n') {
                                    //    ct += " (block)";
                                    //}

                                    //ct = "<option value='" + n.toString() + "'>" + ct + "</option>";
                                    sel[n].textContent = ct;

                                    sel.selectedIndex = n;
                                    $('saveFUser').disabled = false;
                                    setButtonByIdx(1, false, true);
                                }
                            } else {
                                $('saveFUser').disabled = true;
                                //if (cmd === '20') {
                                //    setButtonByIdx(15, false, true);
                                //}
                            }
                        } else $('saveFUser').disabled = false;

                        if (ln == 0) {
                            ShowResult("  New user doesn't usually have any record. !", true);
                        } else if (ln < 0xFF){
                            if (cmd == '10') ShowResult("  Got the user info. !", true);
                            else if (cmd == '15') ShowResult("  Got the info. of scanned & added user !", true);
                            else ShowResult("  Got the scanned user info. !", true);

                            xmldoc = xmlResponse.getElementsByTagName('kge');
                            n = parseInt(xmldoc[0].textContent);
                            if (n == 0) {
                                bKeyGenReq = false;
                                $('genKey').value = "Enable Key Generation";
                            } else {
                                bKeyGenReq = true;
                                $('genKey').value = "Disable Key Generation";
                            }
                        }

                        $('room').disabled = false;

                        break;
                    //case '20':
                    //    break;
                    case '25': // save fob user
                        if (ctrl.del == true) {
                            if ($('fobTable').rows.length == 2) {
                                $('user')[ctrl.lastUserIx].textContent = ctrl.lastUserIx.toString() + " (new)";
                            }
                            ctrl.del = false;
                        }
                    case '35': // save wifi user
                    case '37': // generate key
                        xmldoc = xmlResponse.getElementsByTagName('len');
                        //i = parseInt((xmldoc[0].innerHTML.toString()));
                        i = parseInt((xmldoc[0].textContent));

                        xmldoc = xmlResponse.getElementsByTagName('sRlt');
                        n = xmldoc.length;
                        if (i == xmldoc.length) { // all rows were saved.

                            for (i = 0; i < n; i++) {
                                //if (xmldoc[i].innerHTML.toString() != 'ok') break;
                                if (xmldoc[i].textContent != 'ok') break;
                            }

                            if (i == n) { // all OK
                                n = $('fobTable').rows.length;
                                for (i = 1; i < n; i++) {
                                    enableTableInput(1, i);
                                    if (cmd === "25") {
                                        if ($('fobTable').rows[i].cells[7].children.del.checked === true) {                                            
                                             ClearTableRow(1, i);
                                        }
                                    }
                                }

                                n = $('wifiTable').rows.length;
                                for (i = 1; i < n; i++) {
                                    enableTableInput(2, i);

                                    if (cmd === "35") {
                                        if ($('wifiTable').rows[i].cells[9].children.del.checked === true) {
                                            ClearTableRow(2, i);
                                        }
                                    }
                                }

                                if (xmlQueryCnt < maxTimerRptCnt) xmlQueryCnt = maxTimerRptCnt;

                                if (cmd == '25') ShowResult("  Saved Fob user data!", true);
                                else if (cmd == '35') ShowResult("  Saved WiFi user data!", true);
                                else ShowResult("  Key Code Generation has been Activated!", true);
                                            
                            } else {
                                i++;
                                ShowResult("Failed with the row #" + i.toString(), false);                                
                            }

                            if (cmd != '25') {
                                xmldoc = xmlResponse.getElementsByTagName('kge');
                                n = parseInt((xmldoc[0].textContent));
                                if (n == 0) {
                                    bKeyGenReq = false;
                                    $('genKey').value = "Enable Key Generation";
                                } else {
                                    bKeyGenReq = true;
                                    $('genKey').value = "Disable Key Generation";
                                }
                            }
                        }

                        setButtonByIdx(1, false, false);
                        setButtonByIdx(15, true, true); //comment it out once it is supported.

                        $('room').disabled = false;
                        $('user').disabled = false;

                        break;

                    case '27': // format

                        setButtonByIdx(1, false, false);
                        sel = $('user');

                        //if ($('fFcID').value == INVALID_SECURITY_CODE_INDEX) {
                        //    $('fFsReq').value = 0;
                        //}

                        if ($('fFsReq').value === 'all') {
                            $('fFsReq').value = 0;
                            $('scIx').selectedIndex = 0;
                        }

                        if (sel.value.indexOf("new") === -1) {
                            setButtonByIdx(15, true, true);
                        }

                        if (xmlQueryCnt < maxTimerRptCnt) xmlQueryCnt = maxTimerRptCnt;
                        ShowResult("Formatted Fob security code block !", true);
                        break;

                    case '30':
                        break;
                    default:
                        break;
                }
            //} else if (xmldoc[0].innerHTML.toString() === 'ng') {
            } else if (xmldoc[0].textContent === 'ng') { // replaced innerHTML.toString() with firstChild.nodeValue for IE.
                            
                message = "   Failed !";
                xmlQueryCnt = maxTimerRptCnt;
                xmldoc = xmlResponse.getElementsByTagName('cmd');
                //cmd = xmldoc[0].innerHTML.toString();
                cmd = xmldoc[0].textContent;
                if ((cmd == '25') || (cmd == '35') || (cmd == '37')) {
                    setButtonByIdx(1, false, false);
                    setButtonByIdx(15, true, true); //comment it out once it is supported.

                    //$('room').disabled = false;
                    $('user').disabled = false;

                    if (cmd != '25') {
                        bKeyGenReq = false;
                        $('genKey').value = "Enable Key Generation";
                    }
                }

                $('room').disabled = false;

                ShowResult(message, false);

                xmldoc = xmlResponse.getElementsByTagName('err');
                            
                if (xmldoc == undefined) {
                    message = "Error Code: n/a";
                } else {
                    cmd = xmldoc[0].textContent;
                    message = "Error Code: " + cmd;
                }

            } else {
                message = "   please wait! count: " + xmlQueryCnt;
            }
        } else {
            message = "   please try it again !";
        }

        //$('msgwin').innerHTML = message;
        $('msgwin').textContent = message;

        ClearReqRespondTimer();
    }
}

function ClearReqRespondTimer() {
    if (xmlQueryCnt === maxTimerRptCnt) {
        clearInterval(myRspTimer);
        xmlQueryCnt = 10 + maxTimerRptCnt;
        setButtonByIdx(waitingRspId, false, true);
        waitingRspId = 0;
    }
}

var reqRespond = function() {
    if( xmlHttp.readyState === 0 || xmlHttp.readyState === 4 ){
        if (xmlQueryCnt < maxTimerRptCnt) {
            var cnt;
            var arg;
            xmlQueryCnt++;
            cnt = maxTimerRptCnt - xmlQueryCnt;
            arg = '/xml?USET=' + waitingRspId + '&IDX=' + ctrl.curReqVal + '&CDN=' + cnt; //.toString(16).toUpperCase();
            xmlHttp.open('PUT',arg,true);
            xmlHttp.onreadystatechange = handleServerResponse;
            xmlHttp.send(null);
            
        } else {
            ClearReqRespondTimer();
        }
    }
}

var startReqRespond = function(cmd, tInt) {
            
    if (waitingRspId === 0) {
        if (cmd > 0) {
            waitingRspId = cmd;
            myRspTimer = setInterval(reqRespond, tInt);
            return 0;
        }
    } else $('msgwin').textContent = "There is a pending command " + waitingRspId;

    return 1;
}

var restartReqResponse = function (cmd, tIntv, rpt) {
    var r;
    maxTimerRptCnt = rpt;
    xmlQueryCnt = 0;

    r = startReqRespond(cmd, tIntv);

    if (r == 0) id = setButtonByIdx(cmd, true, true);
    else id = setButtonByIdx(cmd, false, true);
}
            
var sendCommand = function(src) {
    var usr;
    var cnt = 0;
    var ut = $("utable");
                
    usr = "s";
    for( var i = 1; i < ut.rows.length; ) { // -1 due to submit
                        
        if(ut.rows[i].cells[0].firstChild.checked == true ) {
            usr += ut.rows[i].cells[0].firstChild.value + "-" + ut.rows[i].cells[1].firstChild.value;
            cnt++;
            if(1 < ut.rows[i].cells[1].childNodes.length) {
                for(var j = 1; j < ut.rows[i].cells[1].childNodes.length; j++ ) {
                    usr += "," + ut.rows[i].cells[1].childNodes[j].value;
                    cnt++;
                }
            }

            if( ++i < ut.rows.length) {
                usr += ".";
            }
        } else { i++; }
        //$('msgwin').innerHTML += cb.checked.toString();
    }
    //$("usubmit").name = usr;
    $("uReq").value = usr + ". ";
    $("formch").submit();
    restartReqResponse(cnt);
}

            
var addNewFobCell = function (row, rowIdx, col, val, bChecked) {                
    //var row;
    var cell;
    var inp;
    var sel;
    var opt;
                
    if ((col > 0) && (col < NOF_FOB_TBL_COL)) {
        if( col === 5 ) {                    
            sel = document.createElement('select');
        } else {
            inp = document.createElement('input');
        }
    }
    //row = ut.insertRow(rowIdx);
    //for (var j = 0; j < nofCol; j++) {
        cell = row.insertCell(col);
        switch ( col ) {
        case 0:
            if (bChecked === true) {
                cell.textContent = val; //firstChild.nodeValue
            } else {
                cell.textContent = val + " / ?";
            }
            break;
                        
        case 1:
            inp.name = 'vu';
        case 7:
            if (col == 7) inp.name = 'del';

            inp.type = 'checkbox';                        
            //inp.value = val;
            inp.checked = bChecked;
            //inp.onclick = 'handleClick(this)';
            cell.appendChild(inp);
            break;
                        
        case 2:
            if (bChecked === true) {
                inp.value = val;
            } else {
                //inp.value = '?';
                inp.value = '?';
                //inp.style = 'width:6em;';
            }
            inp.name = 'fn';
        
        case 4:
            if (col == 4) {
                inp.name = 'cd';
                inp.value = val;
            }

        case 6:
            if (col == 6) {
                inp.name = 'sup';
                inp.value = val;
            }

            inp.type = 'text';

            inp.disabled = true;
            
            cell.appendChild(inp);
            break;
                        
        case 3:
            var inp2 = document.createElement('input');
            inp.type = 'checkbox';
            inp.name = 'rd';
            inp.checked = false;
            inp.onclick = function () { setToday() };
                        
            cell.appendChild(inp);
            cell.appendChild(document.createTextNode("\n")); //necessary because of newline between two inputs in the fobTable HTML lower this page.
                        
            inp2.type = 'date'; // IE doesn't support it.
            inp2.name = 'dt';
            if (bChecked == true) inp2.value = val;
            inp2.disabled = true;                        
            cell.appendChild(inp2);                        
            break;

        case 5:
            var sid;
            var sud = [ 'Off', 'On', 'Once', 'Now', 'Sync', 'Done', 'Fail', 'n/a' ];
            sel.id = 'sud' + rowIdx.toString();
            sel.class = "cell";
            sel.name = 'su';
            sel.disabled = true;
            sel.onchange = function () {scUpdateSelected(this)};
            cell.appendChild(sel);
            sid = $(sel.id);
            for( var i = 0; i < NOF_SEC_UPDATE_OPT; i++ ) {
                opt = document.createElement('option');
                if (i == 0) {
                    opt.selected = true;
                } else if (i >= FIRST_SEC_UPDATE_OPT_TO_HIDE) {
                    opt.hidden = true;
                }
                opt.text = sud[i];
                sid.add(opt);
            }

            break;
        default:
            cell.textContent = val;
            break;
        }
    //}
    ////var cell1 = row.insertCell(0);
    //// cell1.innerHTML = "NEW CELL1";
    //$("utable").rows[rowIdx].cells[0].firstChild.checked = true;
}

var addNewWifiCell = function (row, rowIdx, col, val, bChecked) {
    //var row;
    var cell;
    var inp;
    var sel;
    var opt;

    if ((col > 0) && (col < NOF_WIFI_TBL_COL)) {
        inp = document.createElement('input');
    }
    //row = ut.insertRow(rowIdx);
    //for (var j = 0; j < nofCol; j++) {
    cell = row.insertCell(col);
    switch (col) {
        case 0:
            if (bChecked === true) {
                cell.textContent = val;
            } else {
                cell.textContent = val + " / ?";
            }
            break;

        case 1:
            inp.name = 'vu';
        case 9:
            if (col === 9) inp.name = 'del';

            inp.type = 'checkbox';
            //inp.value = val;
            inp.checked = bChecked;
            //inp.onclick = 'handleClick(this)';
            cell.appendChild(inp);
            break;

        case 2:
            if (bChecked === true) {
                inp.value = val;
            } else {
                //inp.value = '?';
                inp.value = '?';
                //inp.style = 'width:6em;';
            }
            inp.name = 'un';

        case 5:
        case 6:
        case 7:
        case 8:
            inp.type = 'text';
            //inp.id = 'wf' + col.toString();
            //                        inp.value = val;
            //inp.style = 'font-size:40px;';
            inp.disabled = true;
            if (col != 2) {
                //inp.value = val;
                inp.value = val;
                
                if ((col % 2) === 1) {
                    inp.name = 'hs';
                } else {
                    inp.name = 'mc';
                }
                            
                //inp.style = 'width:2em;';
            }
            cell.appendChild(inp);

            if ($('wifiMore').checked === false) {                            
                if (col != 2) {
                    $('wifiTable').rows[rowIdx].cells[col].hidden = true;
                }
            }
            break;

        case 3:
            //var inp2 = document.createElement('input');
            //inp.type = 'checkbox';
            //if (bChecked == true) inp.value = val;
            //inp.checked = false;
            //inp.onclick = setToday;

            //cell.appendChild(inp);
            //cell.appendChild(document.createTextNode("\n")); //necessary because of newline between two inputs in the fobTable HTML lower this page.

            //inp2.type = 'date';
            //inp2.name = 'd';
            //inp2.disabled = true;
            //cell.appendChild(inp2);

            //cell.innerHTML = "<input type='password' class='three' name='p1' value='????????' onclick='handleClick(this)' disabled=''><br\><input type='password' class='three' name='p2' value='????????' onclick='handleClick(this)' disabled=''>";
            cell.innerHTML = "<input type='password' name='p1' value='" + val + "' onclick='handleClick(this)' disabled=''><br\><input type='password' name='p2' value='" + val + "' onclick='handleClick(this)' disabled=''>";

            break;

        case 4:
            var inp2 = document.createElement('input');
            inp.type = 'checkbox';
            inp.name = 'rd';           

            inp.checked = false;
            inp.onclick = function () { setToday() };

            cell.appendChild(inp);
            cell.appendChild(document.createTextNode("\n")); //necessary because of newline between two inputs in the fobTable HTML lower this page.

            inp2.type = 'date';  // IE doesn't support it.                        
            inp2.name = 'dt';
            if (bChecked == true) inp2.value = val;
            inp2.disabled = true;
            cell.appendChild(inp2);
            break;

        //case 5:
        //    var sid;
        //    var sud = ['Off', 'On', 'Done', 'Fail', 'n/a'];
        //    sel.id = 'sud' + rowIdx.toString();
        //    sel.class = "cell";
        //    sel.disabled = true;
        //    cell.appendChild(sel);
        //    sid = $(sel.id);
        //    for (var i = 0; i < 5; i++) {
        //        opt = document.createElement('option');
        //        if (i == 0) {
        //            opt.selected = true;
        //        }
        //        opt.text = sud[i];
        //        sid.add(opt);
        //    }

        //    break;
        default:
            cell.textContent = val;
            break;
    }
    //}
    ////var cell1 = row.insertCell(0);
    //// cell1.innerHTML = "NEW CELL1";
    //$("utable").rows[rowIdx].cells[0].firstChild.checked = true;
}


////
var ClearTableRow = function (tblIdx, rowIdx) {
    var id;

    if (tblIdx == 1) {
        id = $("fobTable");

        if (rowIdx < id.rows.length) {
            var i = rowIdx - 1;
            //var tds = id.rows[rowIdx].querySelector("td");
            //var tds = id.querySelectorAll("tr")[rowIdx];
            var tds = id.rows[rowIdx].querySelectorAll("td");

            if ((ctrl.lastRoomIx === 0) || (ctrl.lastUserIx === 0)) {
                tds[0].textContent = fobUser[i][0] + " / ?";
            } else {
                tds[0].textContent = ctrl.lastRoomIx.toString() + " / " + ctrl.lastUserIx.toString();
            }

            tds[1].children.vu.checked = false;
            tds[2].children.fn.value = "?"; //tds[2].childNodes[0].defaultValue;
            //tds[3].childNodes[0].checked = false;
            tds[3].children.rd.checked = false;
            //tds[3].childNodes[2].value = tds[3].childNodes[2].defaultValue;
            tds[3].children.dt.value = tds[3].children[1].defaultValue;
            tds[4].children.cd.value = tds[4].childNodes[0].defaultValue;
            tds[5].children.su.selectedIndex = 0;
            tds[6].children.sup.value = tds[6].childNodes[0].defaultValue;
            tds[7].children.del.checked = false;
            
        }

    } else {
        id = $("wifiTable");

        if (rowIdx < id.rows.length) {
            var i = rowIdx - 1;
            var tds = id.rows[rowIdx].querySelectorAll("td");
            
            if ((ctrl.lastRoomIx === 0) || (ctrl.lastUserIx === 0)) {
                tds[0].textContent = wifiUser[i][0] + " / ?";
            } else {
                tds[0].textContent = ctrl.lastRoomIx.toString() + " / " + ctrl.lastUserIx.toString();
            }

            tds[1].children.vu.checked = false; // valid
            tds[2].children.un.value = '?';     // name
            //tds[2].childNodes[0].value = tds[2].childNodes[0].defaultValue;
            //tds[3].childNodes[0].value = '????'; //pw
            tds[3].children.p1.value = '????';   //pw
            //tds[3].childNodes[3].value = '????'; //pw
            tds[3].children.p2.value = '????';   //pw
            //tds[4].childNodes[0].checked = false;        //reg. date
            tds[4].children.rd.checked = false;  //reg. date
            //tds[4].childNodes[2].value = tds[4].childNodes[2].defaultValue;
            tds[4].children.dt.value = tds[4].children[1].defaultValue;

            tds[5].children.hs.value = 255; //host I
            tds[6].children.mc.value = '00:00:00:00:00:00'; //mac I
            tds[7].children.hs.value = 255; //host II
            tds[8].children.mc.value = 'FF:FF:FF:FF:FF:FF'; //mac II
            tds[9].children.del.checked = false; //delete
        }
    }

}
            
////
var updateTableRow = function (tblIdx, rowIdx) {
    var id;

    if (tblIdx == 1) {
        id = $("fobTable");

        if (rowIdx < id.rows.length) {
            var i = rowIdx - 1;
                        
            if ((fobUser[i][0] > 0) || (waitingRspId === CMD_SCAN)) {
                var su;
                var tds = id.rows[rowIdx].querySelectorAll("td");

                //id.rows[rowIdx].cells[0].textContent = fobUser[i][0] + " / " + fobUser[i][1];
                //id.rows[rowIdx].cells[1].children.vu.checked = fobUser[i][2];
                
                tds[0].textContent = fobUser[i][0] + " / " + fobUser[i][1];
                tds[1].children.vu.checked = fobUser[i][2];
                tds[2].children.fn.value = fobUser[i][3];
                //tds[2].childNodes[0].value = tds[2].childNodes[0].defaultValue;
                //tds[3].childNodes[0].checked = false;
                tds[3].children.rd.checked = false;
                //tds[3].childNodes[2].value = fobUser[i][4];

                if (fobUser[i][4] == undefined) {
                    tds[3].children.dt.value = tds[3].children.dt.defaultValue;
                } else {
                    tds[3].children.dt.value = fobUser[i][4];
                }

                tds[4].children.cd.value = fobUser[i][5];
                tds[5].children.su.selectedIndex = fobUser[i][7];
                tds[6].children.sup.value = fobUser[i][6];
                tds[7].children.del.checked = fobUser[i][8];

            } else ClearTableRow(tblIdx, rowIdx);
        }

    } else {
        id = $("wifiTable");

        if (rowIdx < id.rows.length) {
            var i = rowIdx - 1;

            if ((wifiUser[i][0] > 0) || (waitingRspId === CMD_SCAN)) {

                var tds = id.rows[rowIdx].querySelectorAll("td");

                //id.rows[rowIdx].cells[0].textContent = wifiUser[i][0] + " / " + wifiUser[i][1];
                //id.rows[rowIdx].cells[1].children.vu.checked = wifiUser[i][2]; // valid                

                tds[0].textContent = wifiUser[i][0] + " / " + wifiUser[i][1];
                tds[1].children.vu.checked = wifiUser[i][2]; // valid
                tds[2].children.un.value = wifiUser[i][3];   // name
                //tds[2].childNodes[0].value = tds[2].childNodes[0].defaultValue;
                //tds[3].childNodes[0].value = wifiUser[i][4]; //pw
                tds[3].children.p1.value = wifiUser[i][4];     //pw
                //tds[3].childNodes[3].value = wifiUser[i][4]; //pw
                tds[3].children.p2.value = wifiUser[i][4];     //pw
                //tds[4].childNodes[0].checked = false;        //reg. date
                tds[4].children.rd.checked = false;            //reg. date
                //tds[4].childNodes[2].value = wifiUser[i][5];

                if (wifiUser[i][5] == undefined) {
                    tds[4].children.dt.value = tds[4].children.dt.defaultValue;
                } else {
                    tds[4].children.dt.value = wifiUser[i][5];
                }
                tds[5].children.hs.value = wifiUser[i][6]; //host I
                tds[6].children.mc.value = wifiUser[i][7]; //mac I
                tds[7].children.hs.value = wifiUser[i][8]; //host II
                tds[8].children.mc.value = wifiUser[i][9]; //mac II
                tds[9].children.del.checked = wifiUser[i][10]; //delete

            } else ClearTableRow(tblIdx, rowIdx);
        }
    }

}

////
function populateTable() {
    //if( xmlHttp.readyState == 4 && xmlHttp.status == 200 ) {
    var ft = $("fobTable");
    var wt = $("wifiTable");
    var row, rowW; //var cell;
    var len, rLen;
    var val = 0;
    var vTmp;
    var bChk;
    var doc;
    var bAdd, bAddW;
    var ix;
    var n;
    
    //xmlResponse = xmlHttp.responseXML;
    doc = xmlResponse.getElementsByTagName('len');
    //len += parseInt(xmldoc[0].innerHTML.toString());
    len = parseInt(doc[0].textContent);
                    
    if (len > 0) {
        rLen = len + 1;

        doc = xmlResponse.getElementsByTagName("rec");
        n = doc.length;

        //ut.rows[0].cells[0].firstChild.checked = true;

        if (n > MAX_NOF_USERS_PER_ID) {
            $('msgwin').textContent += (n - MAX_NOF_USERS_PER_ID) + "\nrows are short.";
            n = MAX_NOF_USERS_PER_ID;
        }
        
        for (var i = 1, ix = 0; i <= n; i++, ix++) {

            if (ft.rows.length <= i) {
                row = ft.insertRow(i);
                bAdd = true;
            } else {
                bAdd = false;
            }

            if (wt.rows.length <= i) {
                rowW = wt.insertRow(i);
                bAddW = true;
            } else {
                bAddW = false;
            }

            //// fob table + 1st col. of wifi.
            for (var j = 0; j < 8; j++) {

                bChk = false;

                switch (j) {

                case 0:
                    if (waitingRspId === CMD_SCAN) {
                        doc = xmlResponse.getElementsByTagName('usr');                                        
                        val = parseInt(doc[0].textContent);
                    } else {
                        doc = xmlResponse.getElementsByTagName('idx');
                        //val = parseInt(doc[0].innerHTML.toString());
                        val = parseInt(doc[0].textContent);
                    }

                    fobUser[ix][0] = ((val & 0x7F) >> 4);
                    fobUser[ix][1] = (val & 0x0F);
                        
                    if (bAdd === true) {
                        vTmp = fobUser[ix][0] + " / " + fobUser[ix][1];
                        addNewFobCell(row, i, j, vTmp, true); //addNewFobCell(row, rowIdx, col, val, bChecked);
                    }
                    break;

                case 1:
                    doc = xmlResponse.getElementsByTagName('vu');
                    //if (doc[ix].innerHTML.toString() === 'y') fobUser[ix][2] = true;
                    if (doc[ix].textContent === 'y') fobUser[ix][2] = true;
                    else fobUser[ix][2] = false;

                    if (bAdd === true) {
                        addNewFobCell(row, i, j, 0, fobUser[ix][2]); //addNewFobCell(row, rowIdx, col, val, bChecked);
                    }
                    break;

                case 2:
                    doc = xmlResponse.getElementsByTagName('cid');
                    //fobUser[ix][3] = doc[ix].innerHTML.toUpperCase();                                    
                    fobUser[ix][3] = doc[ix].textContent.toUpperCase();
                    bChk = true;
                    break;

                case 3:
                    doc = xmlResponse.getElementsByTagName('rdtf');
                    //val = parseInt(doc[ix].innerHTML, 16); //BCD value
                    val = parseInt(doc[ix].textContent, 16); //BCD value
                                    
                    //fobUser[ix][4] = ((val >> 16) + 2000) + "-" + ((val >> 8) & 0xFF) + "-" + (val & 0xFF);
                    len = val >> 16;
                    len = ((len >> 4) * 10) + (len & 0x0F) + 2000;
                    vTmp = len + "-";

                    if ((len >= 2000) && (len < 2100)) {

                        len = ((val >> 8) & 0xFF);
                        len = ((len >> 4) * 10) + (len & 0x0F);
                        if (len < 10) vTmp += "0";
                        vTmp += len + "-";

                        if ((len > 0) && (len < 13)) {

                            len = (val & 0xFF);
                            len = ((len >> 4) * 10) + (len & 0x0F);
                            if (len < 10) vTmp += "0";
                            vTmp += len;

                            if ((len > 0) && (len < 32)) bChk = true;
                        }
                    }

                    if ( bChk === false ) {
                        $('msgwin').textContent += vTmp;
                        //fobUser[ix][4] = "2000-01-02";
                        fobUser[ix][4] = undefined;
                        //bChk = true;
                    } else {
                        fobUser[ix][4] = vTmp;
                        //bChk = false;
                    }
                    break;

                case 4:
                    doc = xmlResponse.getElementsByTagName('scd');
                    ////fobUser[ix][5] = doc[ix].innerHTML.toString();
                    //val = parseInt(doc[ix].innerHTML, 16);
                    val = parseInt(doc[ix].textContent, 16);
                    fobUser[ix][5] = val.toString();                                    
                    break;

                case 5:
                    doc = xmlResponse.getElementsByTagName('ucyc');
                    //fobUser[ix][6] = parseInt(doc[ix].innerHTML.toString());
                    fobUser[ix][6] = parseInt(doc[ix].textContent);
                    break;

                case 6:
                    doc = xmlResponse.getElementsByTagName('ud');
                    //val = parseInt(doc[ix].innerHTML.toString());
                    val = parseInt(doc[ix].textContent);
                    //fobUser[ix][7] = (val < FIRST_SEC_UPDATE_OPT_TO_HIDE) ? val : SC_UPDT_MOD__NOT_APPLICABLE;
                    
                    if ((val & MASK_SC_UPDT_MOD___OPTIONS) < SC_UPDT_MOD__FORCE_NOW ) {

                        $('saveFUser').value = 'Save Fob User';

                        if ( val >= SC_UPDT_MOD__FORCE_NOW) {

                            vTmp = "";  
                            if ((val & MASK_SC_UPDT_MOD___FAIL_IND) > 0) {
                                vTmp = "'Security Code Update' was failed before !";
                            }
                            if ((val & MASK_SC_UPDT_MOD___WIFI_DOOR_OPENED_IND) > 0) {
                                vTmp += "  Door Opend by WiFi Client!";
                            }

                            ShowTblMsg(vTmp, false);

                            val = val & MASK_SC_UPDT_MOD___OPTIONS;
                        }
                    } else {
                        val = SC_UPDT_MOD__NOT_APPLICABLE;
                        ShowTblMsg("'Security Update' option is not valid.", false);
                    }

                    fobUser[ix][7] =  val;
                    break;

                case 7:
                    fobUser[ix][8] = 0;
                    break;

                default:
                    j = 0;
                    bAdd = true;
                    break;

                }

                if (bAdd === true) {
                    if (j > 1) {
                        addNewFobCell(row, i, j, fobUser[ix][j + 1], bChk); //addNewFobCell(row, rowIdx, col, val, bChecked);
                    }
                }                            
            }

            if (bAdd === false) {
                updateTableRow(1, i);
            }

            doc = xmlResponse.getElementsByTagName('ixsc');
            //$('fFsReq').value = doc[0].innerHTML.toString();
            val = parseInt(doc[0].textContent);
            if (((val == SEC_CODE_IDX_OPT_NA) || (val >= NOF_SEC_CODE_IDX_OPT) || isNaN(val))) {
                //$('fFsReq').value = 'n/a';
                if( ix == 0 ) $('scIx').selectedIndex = SEC_CODE_IDX_OPT_NA;
                if ((waitingRspId === CMD_SCAN_ADD) || (waitingRspId === CMD_SCAN)) {
                    setButtonByIdx(27, false, true);
                }
            } else {
                //$('fFsReq').value = doc[0].textContent;
                if( ix == 0 ) $('scIx').selectedIndex = parseInt(doc[0].textContent);
                setButtonByIdx(27, true, true);
            }
            




            //// WiFi table from 2nd col.
            for (var j = 0; j < 10; j++) {

                bChk = false;

                switch (j) {
                    case 0:
                        wifiUser[ix][0] = fobUser[ix][0];
                        wifiUser[ix][1] = fobUser[ix][1];

                        if (bAddW === true) {
                            vTmp = wifiUser[ix][0] + " / " + wifiUser[ix][1];
                            addNewWifiCell(rowW, i, j, vTmp, true); //addNewFobCell(row, rowIdx, col, val, bChecked);
                        }
                        break;

                    case 1:
                        doc = xmlResponse.getElementsByTagName('uwst');
                        v = parseInt(doc[ix].textContent, 16);

                        if ((v & USR_ST__MASK_BLOCKED_USER) === 0 ) wifiUser[ix][2] = true;
                        else wifiUser[ix][2] = false;

                        if (bAddW === true) {
                            addNewWifiCell(rowW, i, j, 0, wifiUser[ix][2]); //addNewFobCell(row, rowIdx, col, val, bChecked);
                        }
                        break;

                    case 2:
                        doc = xmlResponse.getElementsByTagName('name');                        
                        wifiUser[ix][3] = doc[ix].textContent;
                        bChk = true;                         
                        break;

                    case 3:
                        doc = xmlResponse.getElementsByTagName('pwd');                        
                        wifiUser[ix][4] = doc[ix].textContent;
                        break;

                    case 4:
                        doc = xmlResponse.getElementsByTagName('rdtw');
                        //val = parseInt(doc[ix].innerHTML, 16); //BCD value
                        val = parseInt(doc[ix].textContent, 16); //BCD value

                        //fobUser[ix][4] = ((val >> 16) + 2000) + "-" + ((val >> 8) & 0xFF) + "-" + (val & 0xFF);
                        len = val >> 16;
                        len = ((len >> 4) * 10) + (len & 0x0F) + 2000;
                        vTmp = len + "-";

                        if ((len >= 2000) && (len < 2100)) {

                            len = ((val >> 8) & 0xFF);
                            len = ((len >> 4) * 10) + (len & 0x0F);
                            if (len < 10) vTmp += "0";
                            vTmp += len + "-";

                            if ((len > 0) && (len < 13)) {

                                len = (val & 0xFF);
                                len = ((len >> 4) * 10) + (len & 0x0F);
                                if (len < 10) vTmp += "0";
                                vTmp += len;

                                if ((len > 0) && (len < 32)) bChk = true;
                            }
                        }

                        if (bChk === false) {
                            $('msgwin').textContent += vTmp;
                            //wifiUser[ix][5] = "2000-01-02";
                            wifiUser[ix][5] = undefined;
                            //bChk = true;
                        } else {
                            wifiUser[ix][5] = vTmp;
                            //bChk = false;
                        }
                        break;

                    

                    case 5:
                    case 7:
                        wifiUser[ix][j+1] = "255"; //TODO:: Mar. 2, 2020: To Be Implemented.
                        break;

                    case 6:
                    case 8:
                        wifiUser[ix][j+1] = "00:00:00:00:00:00"; //TODO:: Mar. 2, 2020: To Be Implemented.
                        break;

                    case 9:
                        wifiUser[ix][10] = 0;
                        break;

                    default:
                        j = 0;
                        bAddW = true;
                        break;

                }

                if (bAddW === true) {
                    if (j > 1) {
                        addNewWifiCell(rowW, i, j, wifiUser[ix][j + 1], bChk); //addNewFobCell(row, rowIdx, col, val, bChecked);                                    
                    }
                }

            }

            
            if (bAddW === false) {                
                updateTableRow(2, i);
            }

            enableTableInput(1, i);
            enableTableInput(2, i); //TODO:: uncomment once wifi account populated.
        }

        if (ft.rows.length > rLen) for (i = rLen; ft.rows.length > rLen; i++) ft.deleteRow(rLen);
        if (wt.rows.length > rLen) for (i = rLen; wt.rows.length > rLen; i++) wt.deleteRow(rLen);
    }
    $("scanFob").disabled = false;
    //}
}

var viewMoreCol = function (cb) {
    var tb = $("wifiTable");
    for( var i = 0; i < tb.rows.length; i++ ) {
        for( var j = 5; j < (tb.rows[i].cells.length - 1); j++ ) {
            tb.rows[i].cells[j].hidden = !cb.checked;
        }
    }
}

//var addNewHost = function () {
var addNewTable = function () {
    var ft = $("fobTable");
    var row; //var cell;
    var ix;

    ix = ft.rows.length;
    row = ft.insertRow(ix);

    for (var j = 0; j < NOF_FOB_TBL_COL; j++) {
        addNewFobCell(row, ix, j, 0, false);
    }

    //ft.rows[ix].cells[0].firstChild.checked = true;
}

var handleClick = function (cb) {                
    //updateCheckbox(cb.checked, 0);
}
           
function ResetUserBuff(tblIdx) {
    if (tblIdx === 1) {
        for (i = 0; i < MAX_NOF_USERS_PER_ID; i++) {
            //fobUser.pop();
            fobUser[i][0] = 0;
            fobUser[i][1] = 0;
            fobUser[i][2] = false;
            fobUser[i][3] = 0;
            fobUser[i][4] = "2000-01-01";
            fobUser[i][5] = 0;
            fobUser[i][6] = 0;
            fobUser[i][7] = 0;
            fobUser[i][8] = false;
        }
    } else {
        for (i = 0; i < MAX_NOF_USERS_PER_ID; i++) {
            wifiUser[i][0] = 0;
            wifiUser[i][1] = 0;
            wifiUser[i][2] = false;
            wifiUser[i][3] = "?";
            wifiUser[i][4] = "?";
            wifiUser[i][5] = "2000-01-01";
            wifiUser[i][6] = 255;
            wifiUser[i][7] = "00:00:00:00:00:00";
            wifiUser[i][8] = 255;
            wifiUser[i][9] = "FF:FF:FF:FF:FF:FF";
            wifiUser[i][10] = false;
        }
    }
}
            
var disableAllTableInputs = function (tblIdx) {
    var tb, tds;
                
    if( tblIdx == 1 ) tb = $("fobTable");
    else tb = $("wifiTable");
    
    for (var i = 1; i < tb.rows.length; i++) {
        tds = tb.rows[i].querySelectorAll("td");

        tds[1].children.vu.disabled = true;
                    
                    
        if( tblIdx == 1 ) {
            //tds[1].children.vu.disabled = true;
            tds[2].children.fn.disabled = true;
            tds[3].children.rd.disabled = true;
            tds[3].children.dt.disabled = true;
            tds[4].children.cd.disabled = true;
            tds[5].children.su.disabled = true;
            tds[6].children.sup.disabled = true;
            tds[7].children.del.disabled = true;
            //tds[3].children.d.disabled = true;
            //tds[4].children.cd.disabled = true;
                        
        } else {
            //tds[1].children.vu.disabled = true;
            tds[2].children.un.disabled = true;
            tds[3].children.p1.disabled = true;
            tds[3].children.p2.disabled = true;
            tds[4].children.rd.disabled = true;
            tds[4].children.dt.disabled = true;
            tds[5].children.hs.disabled = true;
            tds[6].children.mc.disabled = true;
            tds[7].children.hs.disabled = true;
            tds[8].children.mc.disabled = true;
            tds[9].children.del.disabled = true;
        }
    }
}
            
var enableTableInput = function ( tblIdx, row ) {
    var tb, tds;
                
                
    if (tblIdx == 1) {
        tb = $("fobTable");
        tds = tb.rows[row].querySelectorAll("td");

        if (row < tb.rows.length) {
            tds[1].children.vu.disabled = false;
            tds[2].children.fn.disabled = false;
            tds[3].children.rd.disabled = false;
            tds[3].children.dt.disabled = false;
            tds[4].children.cd.disabled = false;
            tds[5].children.su.disabled = false;
            tds[6].children.sup.disabled = false;
            tds[7].children.del.disabled = false;                        
        } else return false;

    } else {
        tb = $("wifiTable");
        tds = tb.rows[row].querySelectorAll("td");

        if (row < tb.rows.length) {
            tds[1].children.vu.disabled = false;
            tds[2].children.un.disabled = false;
            tds[3].children.p1.disabled = false;
            tds[3].children.p2.disabled = false;
            tds[4].children.rd.disabled = false;
            tds[4].children.dt.disabled = false;
            tds[5].children.hs.disabled = false;
            tds[6].children.mc.disabled = false;
            tds[7].children.hs.disabled = false;
            tds[8].children.mc.disabled = false;
            tds[9].children.del.disabled = false;
        } else return false;
    }
                    
    return true;
                    
                
}
                    
var setUserSelectOption = function(ix, disable, bkColor) {
    var id = $('user');
    id.disabled = disable;
    id.selectedIndex = ix;
    id.style.backgroundColor = bkColor;
                
    /*id = $('user2');
    id.disabled = disable;
    id.selectedIndex = ix;
    id.style.backgroundColor = bkColor;*/
}
            
// called whenever newly selected room is different from previous one.
var roomSelected = function(me) {
                
    if( me.selectedIndex == 0 ) {
        disableAllTableInputs(1);
        disableAllTableInputs(2);

        setButtonByIdx(1, true, false);                    
        //DbgTest();
    } else if( me.selectedIndex <= 4 ) {
        var idF = $('fobTable');
        var idA = $('wifiTable');
        var i;
                    
        i = idF.rows.length;
                
        for( ; i > 2; i-- ) {
            idF.deleteRow((i-1));
        }
                    
        ResetUserBuff(1);
        ResetUserBuff(2);

        ctrl.lastRoomIx = 0;
        ctrl.lastUserIx = 0;
        ctrl.curReqVal = 0;

        idF.rows[1].cells[0].innerHTML = me.selectedIndex + " / ?";
        idA.rows[1].cells[0].innerHTML = me.selectedIndex + " / ?";
                    
        //TODO:: restore it after you implement populating wifiUser arrary.
        //idA.rows[i+1].cells[0].innerHTML = me.selectedIndex + " / " + "?";
                        
        /*$('fobTable').rows[1].cells[0].innerHTML = me.selectedIndex + " / 1";
        $('fobTable').rows[2].cells[0].innerHTML = me.selectedIndex + " / 2";
        $('wifiTable').rows[1].cells[0].innerHTML = me.selectedIndex + " / 1";
        $('wifiTable').rows[2].cells[0].innerHTML = me.selectedIndex + " / 2";*/

        $('getUsers').disabled = false;
        setButtonByIdx(10, true, false);
    }
                
    setUserSelectOption( 0, true, "White" );
                
    updateTableRow(1, 1);
    updateTableRow(2, 1);

    if (ctrl.lastRoomIx != 0) {
        //$('wifiTable').disabled = b; // it doesn't work. need some other ways to resolve.
        disableAllTableInputs(1);
        disableAllTableInputs(2);
    }

    //ctrl.lastRoomIx = me.selectedIndex;
}




// called whenever newly selected user is different from previous one.
var userSelected = function(me) {                
    var b = false;
                
                
    //ctrl.lastUserIx
    if (me.selectedIndex == 0) {
        disableAllTableInputs(1);
        disableAllTableInputs(2);

        setButtonByIdx(10, true, false);

        ctrl.lastUserIx = 0;
        ctrl.curReqVal = 0;

    //} else if(isNaN(me.value)) { // new user has been selected
    //} else if (isNaN(me.options[me.selectedIndex].innerText)) { // new user has been selected                    
    } else if (me.value.toString().indexOf("new") !== -1) { // new user has been selected
        var len = $('fobTable').rows.length;
                    
        ctrl.lastUserIx = me.selectedIndex;
        ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;

        for (var i = 1; i < len; i++) {
            //fobUser[i - 1][0] = 0;
            fobUser[i - 1][0] = ctrl.lastRoomIx;
            fobUser[i - 1][1] = ctrl.lastUserIx;
            fobUser[i - 1][2] = true;
            updateTableRow(1, i);
            enableTableInput(1, i);
        }

        len = $('wifiTable').rows.length;

        for (var i = 1; i < len; i++) {
            //wifiUser[i - 1][0] = 0;
            wifiUser[i - 1][0] = ctrl.lastRoomIx;
            wifiUser[i - 1][1] = ctrl.lastUserIx;
            wifiUser[i - 1][2] = true;
            updateTableRow(2, i);
            enableTableInput(2, i);
        }

        //setButtonByIdx(20, false, false); // scanFob
        setButtonByIdx(15, false, true); // scanAdd
        setButtonByIdx(25, true, false); // save

    } else {
        disableAllTableInputs(1);
        disableAllTableInputs(2);

        setButtonByIdx(15, true, false);
        setButtonByIdx(10, false, true); // readUser
        //setButtonByIdx(20, false, true); // scanFob

        ctrl.lastUserIx = 0;
        ctrl.curReqVal = 0;
    }
    ResetUserBuff(1);
    ResetUserBuff(2);                
}
            

var getRoomUsers = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    if (waitingRspId == 0) {
        if ($('room').selectedIndex !== 0) {
            var sel = $("user");
            //var fm = $('fGetusers');
            var req = $("rReq");
            ctrl.lastRoomIx = $('room').selectedIndex;
            ctrl.curReqVal = ctrl.lastRoomIx;
            req.value = ctrl.curReqVal;
            ctrl.lastUserIx = 0;

            //req.name = ctrl.lastRoomIx;
            //$("rReq").value = ctrl.lastRoomIx;

            $("fGetusers").submit();
            restartReqResponse(cmd, 1000, 3);
            if (sel.length <= MAX_NOF_USERS_PER_ROOM) {
                var opt;
                for (var i = sel.length; i <= MAX_NOF_USERS_PER_ROOM; i++) {
                    opt = document.createElement("option");
                    opt.text = i + " (x)";
                    sel.add(opt);
                }
            }

            setUserSelectOption(0, true, "White");
            setButtonByIdx(10, true, false);
        } else {
            ShowResult("Please select a room !", false);
        }
    } else ShowBusyMessage();
}

var readOneUser = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    if (waitingRspId == 0) {
        if ($('room').selectedIndex !== 0) {
            //var b;
            var sel = $('user');
            if (sel.selectedIndex !== 0) {
                var req = $("rUReq");
                var ft = $('fobTable');
                var wt = $('wifiTable');

                ctrl.lastUserIx = sel.selectedIndex;
                ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;
                req.value = ctrl.curReqVal;

                disableAllTableInputs(1);
                disableAllTableInputs(2);

                //b = enableTableInput(1, sel.selectedIndex);
                //if (b == true) b = false;
                //else b = true;
                //$('scanFob').disabled = b;
                //$('saveFUser').disabled = b;

                //b = enableTableInput(2, sel.selectedIndex);
                //if (b == true) b = false;
                //else b = true;
                ////$('readMac').disabled = b;                
                //$('saveWifi').disabled = b;
                $("fReadUser").submit();
                // clear all rows; reset all rows to its default initial value.
                for (var i = 1; i < ft.rows.length; i++) {
                    fobUser[i - 1][0] = 0;
                    updateTableRow(1, i);
                    if (i >= MAX_NOF_USERS_PER_ID) break;
                }
                for (var i = 1; i < wt.rows.length; i++) {
                    wifiUser[i - 1][0] = 0;
                    updateTableRow(2, i);
                    if (i >= MAX_NOF_USERS_PER_ID) break;
                }
                restartReqResponse(cmd, 1500, 3);
            } else {
                ShowResult("Please select a user !", false);
            }
        } else {
            ShowResult("Please select a room !", false);
        }
    } else ShowBusyMessage();
}

var scanAndAdd = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    if (waitingRspId == 0) {
        //sendCommand();

        var sel = $('user');
        var u = sel[ctrl.lastUserIx].textContent; //sel[j].textContent

        if (u.indexOf("new") !== -1) { // new user has been selected
            var req = $("fSAReq");

            ctrl.lastUserIx = sel.selectedIndex;
            ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;
            req.value = ctrl.curReqVal;

            disableAllTableInputs(1);
            disableAllTableInputs(2);

            $('fScanAdd').submit();
            restartReqResponse(cmd, 1500, 5);
        } else {
            ShowResult("Please select a new user ID!", false);
        }
    } else ShowBusyMessage();
}

var scanUserFob = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    if (waitingRspId === 0) {
        //var sel = $('user');
        //if ((ctrl.lastRoomIx === 0) || (ctrl.lastUserIx === 0)) {

        //}
                    
        $("sFReq").value = ctrl.curReqVal = 0;

        //var u = sel[ctrl.lastUserIx].textContent; //sel[j].textContent

        //if (u.indexOf("new") !== -1) { // new user has been selected
        //    var req = $("sFReq");
                        
        //    ctrl.lastUserIx = sel.selectedIndex;
        //    ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;
        //    req.value = ctrl.curReqVal;

            disableAllTableInputs(1);
            disableAllTableInputs(2);

            $('fScanfob').submit();
            restartReqResponse(cmd, 1000, 8);
        //} else {
        //    ShowResult("Please select a new user ID!", false);
        //}
        ctrl.lastRoomIx = $('room').selectedIndex = 0;
        ctrl.lastUserIx = 0;
        setUserSelectOption( 0, true, "White" );
        setButtonByIdx(10, true, false);
    } else ShowBusyMessage();
}

var formatFobSecuirty = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    var ft = $('fobTable');
    var rq = $('fFsReq');
    initMsgWindow();

                
    if (ft.rows[1].cells[2].children.fn.value.length !== 8) {
        ShowResult("\"Fob No.\" must be 8 characters !", false);

    } else if (waitingRspId == 0) {        
        //var six = $('fFsReq');
        var v;

        //v = parseInt(six.value);
        v = $('scIx').selectedIndex;
        //if (isNaN(v)) {
        if (v == SEC_CODE_IDX_OPT_ALL) {
            rq.value = 'all';
            //v = INVALID_SECURITY_CODE_INDEX;
        } else if (v >= SEC_CODE_IDX_OPT_NA) {
            //six.value = six.value.toLowerCase();
            //if ( six.value === 'n/a') {
            //    v = INVALID_SECURITY_CODE_INDEX;
            //} else v = 0xFF;
            rq.value = 'n/a';
            v = 0xFF;

        } else rq.value = v.toString();
                    
        if (v !== 0xFF) {

            ctrl.curReqVal = v;

            var sec = $("fFcID");

            //var wt = $('wifiTable');                    

            sec.value = ft.rows[1].cells[2].children.fn.value;


            $('fFormatSecurity').submit();
            restartReqResponse(cmd, 1500, 5);

        } else {
            ShowResult("\"Security Index\" must be 0, 1, 2, 3, or 'all' !", false);
        }

    } else ShowBusyMessage();
}



var saveFobUser = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    var b = CheckDataValidation(1, false, cmd);

    if (b === false) {
        $('msgwin').innerHTML += "Failure in data validation !";

    } else if (waitingRspId == 0) {
        var b;
        var rm = $('room');
        var sel = $('user');
        var req = $("fSFUReq");
        var ft = $('fobTable');
        //var wt = $('wifiTable');
        var arg;
        var dt, n, len;

        //if ((ctrl.lastUserIx > 0) && (ctrl.lastRoomIx > 0)) {

            setButtonByIdx(1, true, false);

            disableAllTableInputs(1);
            disableAllTableInputs(2);

            sel.disabled = true;
            rm.disabled = true;

            ctrl.lastRoomIx = rm.selectedIndex;
            ctrl.lastUserIx = sel.selectedIndex;
            ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;
            //req.value = ctrl.curReqVal;

            len = ft.rows.length;
            for (n = 1; n < len; n++) {
                if (ft.rows[n].cells[5].children.su.value.toString().indexOf('Now') != -1) {
                    break;
                } else if (ft.rows[n].cells[5].children.su.value.toString().indexOf('Sync') != -1) {
                    break;
                }
            }

            if (n == len) {
                n = 1;
                arg = (len - 1).toString(16).toUpperCase() + ";"; // num records.
            } else { // only the row with either Now or Sync.
                len = n + 1;
                arg = "1;"; // num records.
            }

            
            arg += ctrl.curReqVal.toString(16).toUpperCase(); // user id

            for (var i = n; i < len; i++) {
                            
                if (ft.rows[i].cells[1].children.vu.checked === true) { // valid flag
                    arg += ":1;";
                } else {
                    arg += ":0;";
                }
                            
                arg += ft.rows[i].cells[2].children.fn.value.toUpperCase() + ";"; //fob

                //reg date
                dt = ft.rows[i].cells[3].children.dt.value.toString().split("-");
                if (dt.length === 3) {                                
                    n = parseInt(dt[0]); //BCD date. DoorControl device will undersatnd it as HEX value rather than decimal.
                    if (n >= 2000) {
                        n -= 2000;
                        if (n > 99) n = 0;
                    } else n = 0;

                    if (n < 10) arg += "0";
                    arg += n.toString(); //(16).toUpperCase();

                    n = parseInt(dt[1]);
                    if (n < 10) arg += "0";
                    arg += n.toString(); //(16).toUpperCase();

                    n = parseInt(dt[2]);
                    if (n < 10) arg += "0";
                    arg += n.toString() + ";"; //(16).toUpperCase() + ";";
                } else {
                    arg += "000103;";
                }

                //n = parseInt($('fFsReq').value);
                n = $('scIx').selectedIndex;
                // no need because CheckDataValidation() filtered it out.
                //if( n >= SEC_CODE_IDX_OPT_NA ) n = INVALID_SECURITY_CODE_INDEX; // for one digit, but indicating not valid. valid value: 0 ~ 3.
                
                arg += n.toString() + ";"; // sec. code idx

                n = parseInt(ft.rows[i].cells[4].children.cd.value);
                arg += n.toString(16).toUpperCase().padStart(4, '0') + ";"; // sec. code
                arg += ft.rows[i].cells[5].children.su.selectedIndex.toString() + ";";
                n = parseInt(ft.rows[i].cells[6].children.sup.value);
                arg += n.toString(16).toUpperCase(); // update cycle

                if (ft.rows[i].cells[7].children.del.checked === true) { // delete flag
                    arg += ";1";
                    ctrl.del = true;
                } else {
                    arg += ";0";
                    ctrl.del = false;
                }
            }
                        
            req.value = arg;
            //b = enableTableInput(1, sel.selectedIndex);
            //if (b == true) b = false;
            //else b = true;
            //$('scanFob').disabled = b;
            //$('saveFUser').disabled = b;

            //b = enableTableInput(2, sel.selectedIndex);
            //if (b == true) b = false;
            //else b = true;
            ////$('readMac').disabled = b;                
            //$('saveWifi').disabled = b;
            //$("fReadUser").submit();
            //// clear all rows; reset all rows to its default initial value.
            //for (var i = 1; i < ft.rows.length; i++) {
            //    fobUser[i - 1][0] = 0;
            //    updateTableRow(1, i);
            //}
            //for (var i = 1; i < wt.rows.length; i++) {
            //    wifiUser[i - 1][0] = 0;
            //    updateTableRow(2, i);
            //}
            $('fSaveFobUser').submit();
            restartReqResponse(cmd, 1500, 10);
        //}
    } else ShowBusyMessage();
}



// called whenever 'Security Update' is changed.
var scUpdateSelected = function (me) {
    var btn = $('saveFUser');
    //ctrl.lastUserIx
    if (me.value.toString().indexOf('Now') != -1) {
        btn.value = 'Update Security Code';
    } else if (me.value.toString().indexOf('Sync') != -1) {
        btn.value = 'Sync Security Code';
    } else {
        btn.value = 'Save Fob User';
    }
}



var readUserMac = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

}




var saveWifiUserOrKeyGen = function (e, cmd) {
    if (!e) {
        e = window.event; // for IE
        e.returnValue = false;
    } else {
        e.preventDefault();
    }

    initMsgWindow();

    var b = CheckDataValidation(2, false, cmd);

    if (b === false) {
        $('msgwin').innerHTML += "Failure in data validation !";

    } else if (waitingRspId == 0) {
        var b;
        var rm = $('room');
        var sel = $('user');
        var req;// = $("fWifiReq");
        //var ft = $('fobTable');
        var wt = $('wifiTable');
        var arg;
        var dt, n, i, j, k;

        //if ((ctrl.lastUserIx > 0) && (ctrl.lastRoomIx > 0)) {

            setButtonByIdx(1, true, false);

            disableAllTableInputs(1);
            disableAllTableInputs(2);

            sel.disabled = true;
            rm.disabled = true;

            ctrl.lastRoomIx = rm.selectedIndex;
            ctrl.lastUserIx = sel.selectedIndex;

            ctrl.curReqVal = (ctrl.lastRoomIx << 4) + ctrl.lastUserIx;
            //req.value = ctrl.curReqVal;
            arg = (wt.rows.length - 1).toString(16).toUpperCase() + ";"; // num records.
            arg += ctrl.curReqVal.toString(16).toUpperCase(); // user id

            for (i = 1; i < wt.rows.length; i++) {
                            
                if (wt.rows[i].cells[1].children.vu.checked === true) { // valid flag
                    arg += ":1;";
                } else {
                    arg += ":0;";
                }
                
                arg += $('fobTable').rows[i].cells[2].children.fn.value.toUpperCase() + ";"; //fob                
                arg += wt.rows[i].cells[2].children.un.value + ";"; //user name
                arg += wt.rows[i].cells[3].children.p1.value + ";";
                //arg += wt.rows[i].cells[3].children.p2.value + ";";

                //reg date
                dt = wt.rows[i].cells[4].children.dt.value.toString().split("-");
                if (dt.length === 3) {                                
                    n = parseInt(dt[0]); //BCD date. DoorControl device will undersatnd it as HEX value rather than decimal.
                    if (n >= 2000) {
                        n -= 2000;
                        if (n > 99) n = 0;
                    } else n = 0;

                    if (n < 10) arg += "0";
                    arg += n.toString(); //(16).toUpperCase();

                    n = parseInt(dt[1]);
                    if (n < 10) arg += "0";
                    arg += n.toString(); //(16).toUpperCase();

                    n = parseInt(dt[2]);
                    if (n < 10) arg += "0";
                    arg += n.toString() + ";"; //(16).toUpperCase() + ";";
                } else {
                    arg += "000103;";
                }

                if (wt.rows[i].cells[5].children.hs.value.toUpperCase() === 'N/A') {
                    arg += "FF;";
                } else {
                    n = parseInt(wt.rows[i].cells[5].children.hs.value);
                    arg += n.toString(16).padStart(2, "0") + ";";
                }

                dt = wt.rows[i].cells[6].children.mc.value.toString().split(":");
                if (dt.length > 0) {
                    if (dt[0].toUpperCase() === 'N/A') {
                        arg += "000000000000";
                    } else {
                        for (j = 0, k = 0; j < dt.length; j++) {
                            arg += parseInt(dt[k++], 16).toString(16).padStart(2, "0").toUpperCase(); // make sure it is upper case.
                        }
                    }
                } else arg += "FFFFFFFFFFFF";
                            
                //arg += ";";

                //if (wt.rows[i].cells[7].children.hs.value.toUpperCase() === 'N/A') {
                //    arg += "FF;";
                //} else {
                //    n = parseInt(wt.rows[i].cells[7].children.hs.value);
                //    arg += n.toString(16).padStart(2, "0") + ";";
                //}
                            
                //dt = wt.rows[i].cells[8].children.mc.value.toString().split(":");
                //if (dt.length > 0) {
                //    if (dt[0].toUpperCase() === 'N/A') {
                //        arg += "000000000000";
                //    } else {
                //        for (j = 0, k = 0; j < dt.length; j++) {
                //            arg += parseInt(dt[k++], 16).toString(16).padStart(2, "0").toUpperCase(); // make sure it is upper case.
                //        }
                //    }
                //} else arg += "FFFFFFFFFFFF";

                if (wt.rows[i].cells[9].children.del.checked === true) { // delete flag
                    arg += ";1";
                } else {
                    arg += ";0";
                }
            }

            if (cmd === 35) {
                req = $("fWifiReq");
                req.value = arg;
                //b = enableTableInput(1, sel.selectedIndex);
                //if (b == true) b = false;
                //else b = true;
                //$('scanFob').disabled = b;
                //$('saveFUser').disabled = b;

                //b = enableTableInput(2, sel.selectedIndex);
                //if (b == true) b = false;
                //else b = true;
                ////$('readMac').disabled = b;                
                //$('saveWifi').disabled = b;
                //$("fReadUser").submit();
                //// clear all rows; reset all rows to its default initial value.
                //for (var i = 1; i < wt.rows.length; i++) {
                //    fobUser[i - 1][0] = 0;
                //    updateTableRow(1, i);
                //}
                //for (var i = 1; i < wt.rows.length; i++) {
                //    wifiUser[i - 1][0] = 0;
                //    updateTableRow(2, i);
                //}
                $('fSaveWifi').submit();
            } else {
                req = $("fKeyGenReq");

                if (bKeyGenReq) arg += "-0";
                else arg += "-1";

                req.value = arg;
                $('fKeyGen').submit();        
            }
            restartReqResponse(cmd, 1500, 5);
        //}
    } else ShowBusyMessage();
}




//var setToday = function (cb) {
function setToday() {
    //var sel = $('user').selectedIndex;
    var cell = this.event.currentTarget.parentNode;
    var tid = cell.parentNode.parentNode.parentNode.id;                
    var tbl = $(tid);
    var r = cell.parentNode.rowIndex;
    var c = cell.cellIndex;
    var dt = new Date();

    ////cb.nextElementSibling.value = dt.toLocaleDateString();
    ////cb.nextElementSibling.value = dt.getFullYear() + "-" + dt.getMonth() + "-" + dt.getDate();
    //cb.nextElementSibling.value = dt.getFullYear() + "-" + ((dt.getMonth()<9)? "0": "") + (dt.getMonth()+1) + "-" + ((dt.getDate()<10)?"0":"") + dt.getDate();
                
    if( tbl.rows[r].cells[c].children.rd.checked == true ) {
        tbl.rows[r].cells[c].children.dt.value = dt.getFullYear() + "-" + ((dt.getMonth() < 9) ? "0" : "") + (dt.getMonth() + 1) + "-" + ((dt.getDate() < 10) ? "0" : "") + dt.getDate();
    } else {
        tbl.rows[r].cells[c].children.dt.value = tbl.rows[r].cells[c].children.dt.defaultValue;
    }
                
}
            
// Do NOT change the order of 'case' unless you know what you are doing.
function setButtonByIdx(idx, bDis, bBrk) {
    switch(idx) {
    case 1:
        $("getUsers").disabled = bDis;
        //if (bBrk == false) break;
        if (bBrk == true) break;
    case 10:
        $("readUser").disabled = bDis;
        //if (bBrk == false) break;
        if (bBrk == true) break;
    case 20:
        if (idx === 20) {
            $("scanFob").disabled = bDis;
            //if (bBrk == false) break;
            if (bBrk == true) break;
        }
    case 15:
        $("scanAdd").disabled = bDis;
        //if (bBrk == false) break;
        if (bBrk == true) break;
    case 25:
        $("saveFUser").disabled = bDis;
        //if (bBrk == false) break;
            if (bBrk == true) break;
    case 27:
        $("updateFSc").disabled = bDis;
        //if (bBrk == false) break;
        if (bBrk == true) break;
    case 30:
        $("readMac").disabled = bDis;
        //if (bBrk == false) break;
        if (bBrk == true) break;
    case 35:
        $("saveWifi").disabled = bDis;
        if (bBrk == true) break;
    case 37:
        $("genKey").disabled = bDis;
        break;
    default:
        idx = 0;
        break;
    }
    return idx;
}

function CheckDataValidation(tbl, bFormat, cmd) {
    //var msg = $('resultMsg');
                
    var v;
    var bVd = true;                   

    if ($('room').selectedIndex === 0) {
        ShowResult("Please select a room !", false);
        return false;
    }

    if ($('user').selectedIndex === 0) {
        ShowResult("Please select a user !", false);
        return false;
    }

    if (tbl === 1) {
        var ft;
        //var ml = $('fFsReq');
        //v = parseInt(ml.value); // security code block index.
        v = $('scIx').selectedIndex; // security code block index.

        if ( ((v >= SEC_CODE_IDX_OPT_ALL) && (bFormat === false)) || (v == SEC_CODE_IDX_OPT_NA)) {
            var m = "\"Security Index\" is not a valid number. It must 0, 1, 2, ";
            if( bFormat == true ) m += "3, or all.";
            else m += "or 3.";

            ShowResult(m, false);
            return false;
        }        

        ft = $('fobTable');

        for (var i = 1; i < ft.rows.length; i++) {

            if (ft.rows[i].cells[2].children.fn.value.length !== 8) { //fob
                ShowResult("The length of Fob ID number in 'Fob No.' must be 8 characters.", false);
                return false;
            }


            //reg date                            
            //ft.rows[i].cells[3].childNodes[2].value = id.rows[rowIdx].cells[3].childNodes[2].defaultValue;

            v = parseInt(ft.rows[i].cells[4].children.cd.value);
            if ((isNaN(v)) || (v > 65535)) {
                ShowResult("'Security Code' is not a valid number.", false);
                return false;
            }

            //ft.rows[i].cells[5].childNodes[0].selectedIndex = 0; //update
            v = parseInt(ft.rows[i].cells[6].children.sup.value);
            if ((isNaN(v)) || (v > 255)) {
                ShowResult("'Security Update Period' is not a valid number.", false);
                return false;
            }
        }

    } else {

        var wt = $('wifiTable');
        v = wt.rows[1].cells[2].children.un.value.length;
        if ( ( v < MIN_SIZE_INITIAL) || ( v > SIZE_INITIAL) ) {
            ShowResult("The length of 'User Name' must be 2 to 6 characters !", false);
            return false;
        }

        if (wt.rows[1].cells[3].children.p1.value !== wt.rows[1].cells[3].children.p2.value) {
            ShowResult("The password is different !", false);
            return false;
        }

        if (wt.rows[1].cells[3].children.p1.value.length !== SIZE_PASSWORD) {
            ShowResult("The password must be 8 characters !", false);
            return false;
        }

        if (cmd === 37) {
            if (wt.rows[1].cells[1].children.vu.checked === false) { // invalid flag
                ShowResult("The user must be a valid WiFi user !", false);
                return false;
            }

            if (wt.rows[1].cells[9].children.del.checked === true) { // delete flag
                ShowResult("The user must be a valid WiFi user !", false);
                return false;
            }
        }
    }
                        
    return true;
}

function ShowResult(msg, pass) {
    var r = $('resultMsg');
    if (pass == true) {
        r.style.color = 'green';
    } else {
        r.style.color = 'red';
    }
    r.innerHTML = msg;

    //$('msgwin').style.fontcolor('red');
    //$('resultMsg').innerHTML = "<span style='color:red'>Update Failed!</span>";
}

function ShowTblMsg(msg, pass) {
    var m = $('tblMsg');
    if (pass == true) {
        m.style.color = 'green';
    } else {
        m.style.color = 'red';
    }
    m.innerHTML = '( ' + msg + ' )';
}

function ShowBusyMessage() {
    $('msgwin').innerHTML = "Busy waiting for response of earlier request.";
}

function initMsgWindow() {
    ShowResult("", false);
    $('tblMsg').innerHTML = "";
    $('msgwin').innerHTML = "";
}

