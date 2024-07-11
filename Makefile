PACKAGE=bgnet
WEB_IMAGES=$(wildcard src/*.svg)

BGBSPD_BUILD_DIR?=../bgbspd

include $(BGBSPD_BUILD_DIR)/main.make

