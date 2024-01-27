################################################################################
###  Laon Smart Home System  ###################################################
################################################################################

* Laon Smart Home System consists of following sub-systems: 
    - RFID sub-system, 
    - Laon wifi (ESP8266) server sub-system,
    - Light control sub-system,
    - Laon control server sub-system

* Version control for Laon Smart Home System:
    h.i.v.r      h: hardware version number: increamented whenever there is hardware changes.
                i: system integration version number: increamented whenever system interface is changed.
                    This number is independent from the change in 'h'.
                    It increases, e.g, whenever any interface command value is changed/added.
                    Each sub-systems can work together when these first 2 versions are equal.
                v: sub-system version number: increamented when there is any major code change in its sub-system.
                r: sub-system revision number: increamented when there is any minor change in its sub-system.

* The Router for Demos:
    - Router: Netgear WNR100
    - SSID:LaonMakers
    - Password: laonmakers
    - ESP8266 ip: 192.168.0.2
    - Router Config URL: http://www.routerlogin.net
    - Router Login ID/PW: admin password

* Access Point Selection Guide:
   ---------------------------------------------------------------------------------------
   |          GPIO16            |                      Access Point                      |
   ---------------------------------------------------------------------------------------
   |  Open (Internal Pull-down) |  LaonMakers, [YOUR HOME ROUTER SSID]                   |
   |  Vcc                       | 'Toronto Public Libruary', 'Second Cup', TimHortons    |
   ---------------------------------------------------------------------------------------
   

#################################################################################
###  Laon WiFi (ESP8266) Server Sub-system  #####################################
#################################################################################
  * Note: 
    You need separate library folders which is SmartDoorWifiWebServer_hal_lib.git to build executable image.
    This library repository hasn't been uploaded to GitHub yet because there is no enough free space.

  * New log-in flow: (Apr. 26, 2020)
           version 3 (May 9, 2020)
 ________             _____________                                                      _____________
| Control|           | WiFi Server |                                                    | WiFi Client |
| Server |            '''''''''''''                                                      '''''''''''''
 ''''''''                            Wifi Server URL (192.168.0.26)             
                            <-----------------------------------------------------------------
                                     "/login" for redirection
                            ----------------------------------------------------------------->
                       
                                     "/login"
                            <-----------------------------------------------------------------
                                     login.html                                            <==(May 16, 2020: moved the contents in login.js into the html file.)
                            ----------------------------------------------------------------->  get information about user ID and user name from its local storage
                                                                                                to be ready for login. List users with radio buttons on its page.
 
                             the number of valid users (which Wifi client has in its local storage) via Http XML.
                            <-----------------------------------------------------------------
                             ( requesting both temporary session ID (TSID) and 2 login code group indics )
                             
                                     TSID & 2 login code group indics via Http XML.
                            ----------------------------------------------------------------->  find login code with the TSID and 2 indics from its local storage.

                              TSID, 2 login code group indics, and either {login code & user ID} if match user found or
                              TSID and { user name and password } if not found.
                            <-----------------------------------------------------------------  selecting one radio buttons to specify a user, typing password in,
                              ( submit by clicking on 'Submit' button )                          and clicking on 'Submit' button to get the password verified.
                                                                                                 When either the password is correct or New user is selected, login request is submitted.
                              
                            One of following data depending on the wifi user validation results
                            and configurations are sent with ESPSESSIONID if login is successful:
                              1) 'logincodegen.html' if the wifi client is set to receive login code
                              generation web page and also found a valid user 
                              ( either login code and user ID matches or both name and password
                              are valid.), or 
                              2) '/' if both login code and user ID are valid to get it eventually redirected to 'doorctrl.html',
                              3) Otherwise redirection dir '/login' to end up repeating the login procedure.
                            ----------------------------------------------------------------->
                            
                            
                            
                            1) 'logincodegen.html'
                            ----------------------------------------------------------------->
                                WIFI_CMD_LOGIN_CODEGEN_PARAM_REQ via Http XML.
                            <-----------------------------------------------------------------
                            
                                user ID, Name, Password, ESPSESSIONID (== previous TSID & 0x7FFFF if the user is not an super user I/II, or either 'TSID | 0x8000' or 'TSID | 0xC000' for a super user I and II respectively)
                            -----------------------------------------------------------------> populate associated buffers with the parameters.
                                
                            <-----------------------------------------------------------------
                                ( Log out )
                            
                            
                            2) "/" for redirection with a cookie in "ESPSESSIONID"
                            ----------------------------------------------------------------->
                             If ID and Password match with a valid wifi user, it checks if log-in key code generation is required by checking newUser.cookie.
                             If the key code generation was requested, it send logincodegen.html.
                             But in this case is when there is no login code generation request.
                             Therefore it sends both cookie and the redirect directory ("/") provided 
                             the ID, Password, and the Log-in Key Code matched with a valid user.)
                             root dir ("/") and a new cookie is sent.
                             
                       
                                     "/"
                            <----------------------------------------------------------------- send the redirection directory since the log-in was succeeded in previous flow.
                                     doorctrl.html
                            ----------------------------------------------------------------->
                       
                                     "/?cmd=1&DrCtrl=3" or "/?cmd=1&DrCtrl=4"
        <---------------    <----------------------------------------------------------------- "DrCtrl=3" for 'Unlock' or "DrCtrl=4" for 'Get Unlocked'
         (Send a command to get the Door Control Server to unlock the door.)
                                     
                            ----------------------------------------------------------------->
                            

                            3) login.html & login.js with the cookie "ESPSESSIONID=0"
                            ----------------------------------------------------------------->
                             ( redirection dir "/login" is sent; it get the client go back to
                               login page through next a few more data traffic below. )
                                     '/login'
                            <-----------------------------------------------------------------
                            
                              login.html & login.js with the cookie "ESPSESSIONID=0"
                            ----------------------------------------------------------------->