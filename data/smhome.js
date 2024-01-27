var myStatusTimer;
var maxStatusQueryCnt = 0;

var xmlHttp= new CreateXmlHttpObject();

var $ = function(id) {
    return document.getElementById(id); 
}

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
        if( maxStatusQueryCnt < 10 ){
            xmlResponse = xmlHttp.responseXML;
            xmldoc = xmlResponse.getElementsByTagName('response');
            message = xmldoc[0].firstChild.nodeValue;
        } else { message = "   please click on Lock Status button !";}

        $('doorst').innerHTML = message;
    }
}

var checkStatus = function() {
    if( xmlHttp.readyState == 0 || xmlHttp.readyState == 4 ){
        if( maxStatusQueryCnt < 10 ){
            xmlHttp.open('PUT','/xml',true);
            xmlHttp.onreadystatechange = handleServerResponse;
            xmlHttp.send(null);
            maxStatusQueryCnt++;
        } else if( maxStatusQueryCnt == 10 ){
            clearInterval(myStatusTimer);
            maxStatusQueryCnt = 100;
            $('lstat').disabled = false;
        }
    }
}

var startStatusCheck = function() {
    myStatusTimer = setInterval( function() {checkStatus();}, 3000);
}

var restartCheckStatus = function() {
    $('lstat').disabled = true;
    maxStatusQueryCnt = 0;
    startStatusCheck();
    $('doorst').innerHTML = "   please wait !";
}
