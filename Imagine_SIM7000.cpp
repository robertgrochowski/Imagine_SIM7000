#include <Imagine_SIM7000.h>

static SoftwareSerial* SIMSerial;
static char buffer[BUFFER_SIZE];
static int LAST_CMD_TIME;

void Imagine_SIM7000::init(Stream &_stream)
{
	SIMSerial = &_stream;
	SIMSerial->begin(BAUDRATE);
	LAST_CMD_TIME = -1;
}
bool Imagine_SIM7000::checkAT()
{
	return tryCommand("AT\r\n","OK");
}

bool Imagine_SIM7000::checkSIM(const char* pin = "")
{
	if(strlen(pin) > 0)
	{
		sendCommand("AT+CPIN=");
		sendCommand(pin);
		return checkSendCommand("\r\n", "READY");
	}

	return tryCommand("AT+CPIN?\r\n","READY");
}
bool Imagine_SIM7000::prepareNetwork()
{
	if(!tryCommand("AT+CNMP=51\r\n","OK")) return false; 		delay(500);
	if(!tryCommand("AT+CMNB=3\r\n","OK")) return false; 		delay(500);
	if(!tryCommand("AT+CGATT=1\r\n","OK")) return false;   delay(500);
	if(!tryCommand("AT+SAPBR=3,1,\"APN\",\"plus\"\r\n","OK")) return false; delay(500);
	if(!tryCommand("AT+SAPBR=1,1\r\n","OK")) return false; delay(500);
	if(!tryCommand("AT+SAPBR=2,1\r\n","OK")) return false; delay(500);
	//if(!tryCommand("AT+CSTT=\"plus\"\r\n","OK")) return false; 	delay(100);
	//if(!tryCommand("AT+CIICR\r\n","OK")) return false; 			delay(100);
	//if(checkSendCommand("AT+CIFSR\r\n","ERROR")) return false; 	delay(100);

	return true;
}
bool Imagine_SIM7000::turnOFF()
{
	return checkSendCommand("AT+CPOWD=1\r\n","NORMAL POWER DOWN");
}
bool Imagine_SIM7000::prepareGPS()
{
	return tryCommand("AT+CGNSPWR=1\r\n","OK");
}
String Imagine_SIM7000::getGNSSinfo()
{
	sendCommand("AT+CGNSINF\r\n");
	cleanBuffer();
	readBuffer();
	
	Serial.print("RECEIVE<-\n{\n");
	Serial.print(buffer);
	Serial.print("}\n");
	
	char *start = strstr(buffer, ":") + 2; //ommitting : and space
	String data = String(*start);
	
	while(*(++start) != '\r'){
		data += *start;
	}
	
	return data;
}
bool Imagine_SIM7000::turnON(){
	
	pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);
    delay(2000);
    digitalWrite(12, LOW);
	
	delay(2000);
	
	return tryCommand("AT\r\n","OK"); 
}
bool Imagine_SIM7000::restart()
{
	if(tryCommand("AT\r\n","OK")) //module is ON
	{
		if(!turnOFF())
			Serial.println(F("ERROR: failed to turnOFF device... (isn't it off?)"));
		
		delay(1000);
	}
	return turnON();
}
bool Imagine_SIM7000::tryCommand(const char* cmd, const char* resp, unsigned int delayTime = 300, int attempts=3)
{
	//Serial.print(cmd);
	while(attempts > 0)
	{
		if(!checkSendCommand(cmd, resp))
		{
            attempts--;
            delay(delayTime);
        }
		else return true;
    }
	return false;
}
bool Imagine_SIM7000::HTTPconnect(const char* host, bool progmem=false)
{
	HTTPdisconnect();
	
	if(!tryCommand("AT+HTTPINIT\r\n","OK")) return false; delay(100);
	if(!tryCommand("AT+HTTPPARA=\"CID\",\"1\"\r\n","OK")) return false; delay(100);
	
	sendCommand("AT+HTTPPARA=\"URL\",\"");
	
	
	/*if(progmem) //todo
	{
		for (int k = 0; k < strlen_P(host); k++)
		{
			char myChar = pgm_read_byte_near(host + k);
			sendCommand(myChar);
		}
	} else*/

	sendCommand(host);
	
	return checkSendCommand("\"\r\n","OK");
}
bool Imagine_SIM7000::HTTPpost(String data)
{
	sendCommand("AT+HTTPDATA=");
    sendCommand(String(data.length()));
	
	if(!checkSendCommand(",10000\r\n","DOWNLOAD")) return false;
	
	delay(100);
	sendCommand(data);
	sendCommand("\r\n");
	delay(100);
	
	cleanBuffer();
	
    while(1){
        readBuffer();
        if(NULL != strstr(buffer,"OK")){
            break;
        }
        if(NULL != strstr(buffer,"ERROR")){
            return false;
        }
    }
    if(!checkSendCommand("AT+HTTPACTION=1\r\n","200,2\r\n", true, 20000, 10000)){
        return false;
    }
	
    sendCommand("AT+HTTPREAD\r\n");
	cleanBuffer();
	readBuffer();
	Serial.print(buffer);
	
	return true;

}
bool Imagine_SIM7000::HTTPdisconnect(){
	return checkSendCommand("AT+HTTPTERM\r\n","OK");
}


bool Imagine_SIM7000::checkSendCommand(const char* cmd, const char* resp, bool isLastChar = false, unsigned int timeout = DEFAULT_TIMEOUT, unsigned int lastchartimeout = DEFAULT_LAST_CHAR_TIMEOUT)
{
    cleanBuffer();
    sendCommand(cmd);
	LAST_CMD_TIME = -1;
	
	if(isLastChar) LAST_CMD_TIME = readBufferTill(resp, timeout, lastchartimeout);
	else readBuffer(timeout, lastchartimeout);
	
	if(LAST_CMD_TIME != -1)
		Serial.printf("Time:{%d}\n", LAST_CMD_TIME);
	
	Serial.print(F("RECEIVE<-\n{\n"));
	Serial.print(buffer);
	Serial.print(F("}\n"));
	
	return strstr(buffer, resp) != NULL;
}

//Returns elapsed time in ms
int Imagine_SIM7000::readBufferTill(const char* lastChar, unsigned int timeout = DEFAULT_TIMEOUT, unsigned int lastchartimeout = DEFAULT_LAST_CHAR_TIMEOUT)
{
	int i = 0;
    unsigned long timerStart = millis();
    unsigned long prevChar = 0;
	
    while(1)
	{
        while(SIMSerial->available())
		{
            if(i < BUFFER_SIZE) 
                buffer[i++] = SIMSerial->read();
			else SIMSerial->read(); //release serial //TODO: add debug warning
			
			prevChar = millis();
        }
		
		if(strstr(buffer, lastChar) != NULL)
			break;
    
		if((unsigned long) (millis() - timerStart) > timeout)
			break;
		
		if(((unsigned long) (millis() - prevChar) > lastchartimeout) && (prevChar != 0))
            break;
    }
    SIMSerial->flush();
    return millis() - timerStart;
}

int Imagine_SIM7000::readBuffer(unsigned int timeout = DEFAULT_TIMEOUT, unsigned int lastchartimeout = DEFAULT_LAST_CHAR_TIMEOUT)
{
    int i = 0;
    unsigned long timerStart = millis();
    unsigned long prevChar = 0;
	
    while(1)
	{
        while(SIMSerial->available())
		{
            if(i < BUFFER_SIZE) 
                buffer[i++] = SIMSerial->read();
			else SIMSerial->read(); //release serial //TODO: add debug warning
			
			prevChar = millis();
        }
    
		if((unsigned long) (millis() - timerStart) > timeout)
			break;
		
		if(((unsigned long) (millis() - prevChar) > lastchartimeout) && (prevChar != 0))
            break;
    }
    SIMSerial->flush();
    return i;
}
void Imagine_SIM7000::cleanBuffer()
{
    memset(buffer, 0, sizeof(buffer));
}
void Imagine_SIM7000::sendCommand(String aaa)
{
	//Serial.printf("SEND->\n{\n%s}\n", cmd.c_str());
	Serial.print("SENDs->\n{\n");
	Serial.print(aaa);
	Serial.print("}\n");
	SIMSerial->print(aaa);
}
void Imagine_SIM7000::sendCommand(const char* cmd)
{
	Serial.printf("SEND->\n{\n%s}\n", cmd);
	SIMSerial->write(cmd);
}
