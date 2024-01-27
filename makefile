# My makefile
##SMHOME_DIR = /cygdrive/p/project/esp8266ex/smHomeWifiWebServer
#SMHOME_DIR = p:/project/esp8266ex/smHomeWifiWebServer
#SKETCH = $(SMHOME_DIR)/smHomeMain.ino \
#	 $(SMHOME_DIR)/smHomeAdmin.ino \
#	 $(SMHOME_DIR)/smHomeUartDoorCtrl.ino \
#	 $(SMHOME_DIR)/smHomeWifiWebServer.ino

#SKETCH = /cygdrive/p/project/esp8266ex/smHomeWifiWebServer/
SKETCH = p:/project/esp8266ex/smHomeWifiWebServer/
MAIN_NAME = smhWServer

#set VERBOSE to 1 in order to get the compile verbose.
#VERBOSE = 1


#UPLOAD_PORT = /dev/ttyUSB1
#in Cygwin COM1 is ttyS0
UPLOAD_PORT = /dev/ttyS3
#BOARD = nodemcuv2
OTA_ADDR=192.168.43.78

#include /cygdrive/p/makeEspArduino/makeEspArduino.mk
#include p:/makeEspArduino/makeEspArduino.mk
include p:/project/esp8266ex/smHomeWifiWebServer/makeEspArduino.mk
#include P:/project/esp8266ex/smHomeWifiWebServer/make/makeEspArduino.mk
#include /cygdrive/p/project/esp8266ex/smHomeWifiWebServer/make/makeEspArduino.mk
