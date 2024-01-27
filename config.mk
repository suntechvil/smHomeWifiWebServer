# config.mk
THIS_DIR := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
#ROOT := $(THIS_DIR)/..
ROOT :=p:/project/esp8266ex/smHomeWifiWebServer

LIBS = $(ESP_LIBS)/ArduinoOTA \
  $(ESP_LIBS)/DNSServer \
  $(ESP_LIBS)/ESP8266mDNS \
  $(ESP_LIBS)/ESP8266WebServer \
  $(ESP_LIBS)/ESP8266WiFi \
  $(ROOT)/libraries/ESP8266-ping
#  p:/project/esp8266ex/smHomeWifiWebServer/libraries/ESP8266-ping

UPLOAD_SPEED = 115200
