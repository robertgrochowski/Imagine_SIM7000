#include <Arduino.h>
#include "SoftwareSerial.h"
#include <Wire.h>
#include <stdio.h>

#define PIN_TX     7
#define PIN_RX     8
#define BAUDRATE 19200
#define BUFFER_SIZE 255
#define DEFAULT_TIMEOUT 3000
#define DEFAULT_LAST_CHAR_TIMEOUT 1500

#define DEBUG 1

class Imagine_SIM7000 {
	
public:
	void init(Stream &_stream);
	bool checkAT();
	bool checkSIM();
	bool restart();
	bool tryCommand(const char* cmd, const char* resp, unsigned int delayTime = 300, int attempts=3);
	bool checkSendCommand(const char* cmd, const char* resp, unsigned int timeout = DEFAULT_TIMEOUT, unsigned int lastchartimeout = DEFAULT_LAST_CHAR_TIMEOUT);
	int readBuffer(unsigned int timeout = DEFAULT_TIMEOUT, unsigned int lastchartimeout = DEFAULT_LAST_CHAR_TIMEOUT);
	void cleanBuffer();
	bool prepareNetwork();
	void sendCommand(const char* cmd);
	void send_Command(String cmd);
	bool turnON();
	bool turnOFF();
	bool HTTPpost(String data);
	bool HTTPconnect(const char* host, bool progmem=false);
	bool HTTPdisconnect();
	
	
};