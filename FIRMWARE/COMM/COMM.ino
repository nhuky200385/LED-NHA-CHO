#include <memory>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>


#include <SPI.h>
#include <Ethernet.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

//IPAddress ip(192, 168, 1, 177);
EthernetClient Eclient;
int e_bytesRecv;
String header_time="Date: Wed, 05 Apr 2017 08:42:28 GMT";

#include <OneWire.h>
#include <DallasTemperature.h>

#define Ethernet_RESET_PIN 0

WiFiServer wServer(23);
WiFiClient wificlient;

SoftwareSerial swSer(5,4, false, 256); //RX, TX

#define wClient wificlient
#define displaySerial swSer
#define debugSerial Serial1

#ifdef debugSerial
	#ifdef wClient
		#define DEBUG_SERIAL(...) {debugSerial.printf( __VA_ARGS__ ); if (wClient && wClient.connected()) wClient.printf( __VA_ARGS__ );}
	#else
		#define DEBUG_SERIAL(...) debugSerial.printf( __VA_ARGS__ );
	#endif
#endif

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL(...)
#endif

#define WIFI_AP_En true

#define from_Ethernet 0
#define from_WIFI 1
//uint8_t Interface=0;


#define time_get 15 //seconds


#define Set_EthIP "EthIP="
#define Set_Wifi "Wifi="
#define Set_Interface "Interface="
#define Set_busStop "BusStop="
#define Set_Default "Default=1"
#define Get_Config "Config?"
#define StartUp "StartUp"
#define BusInfo "BusInfo"
#define End_BusInfo "End_Info"
#define BusConfig "BusConfig"
#define End_BusConfig "End_Config"
#define CheckRunning "Running?"
#define Running "DISPLAY"
#define Idle "Idle"
#define Set_Time "SetTime="
#define End_Time "End_Time"
#define Set_Brightness "Set_Brightness="
#define End_Brightness "End_Brightness"
#define CheckSum_Fail "CheckSum_Fail"
#define CheckSize_Fail "CheckSize_Fail"

#define isBusIdle 0
#define isBusInfo 1
#define isBusConfig 2
#define isCheckRunning 3
#define isSet_Time 4
#define isBusRoute 5


#define DISPLAY_RESET 16
// Data wire is plugged into port 12 on the Arduino
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;
int DeviceCount;
float current_Temp;

uint8_t Comm_Infor=0;
uint8_t Comm_Error=0;
uint8_t Get_Error=0;
//
char* chipID = "BUS_xxxxxxxx"; //xx = Chip ID
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
//
const char* password_ap = "13245768";


char *sketch_name ="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.ino";//
char sketch_time[14];

byte stat=0;
byte LED=13;

char tempbuffer[160];
char n[20];
char n1[20];
char com_buffer[600];
int  com_buffer_len;
char data_buffer[6000];
//

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
HTTPClient http;

//******************************************************************************************************
#define RST_NORMAL HIGH  //LOW,HIGH
#define LEVEL_ON LOW

unsigned long Startup_timestamp;
unsigned long lastGet_timeStamp;

bool PW_On=false;
bool AP_On=false;
bool Checkupdate=false;
bool isUpdate=false;
float voltage;
bool timer0_en;
bool firstScan;
bool RF_Disable;
bool detected_NetworkTime;
bool bGet_condition,bGet_config;

uint32_t unixTime_send;
uint32_t Unixtime_GMT7;
uint32_t time_bk;
#define SECONDS_FROM_1970_TO_2000 946684800
const uint8_t daysInMonth [] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
struct {
	uint8_t year;//yOff;
	uint8_t month;//m;
	uint8_t day;//d;
	uint8_t hour;//hh;
	uint8_t minute;//mm;
	uint8_t second;//ss;
}rtc;

struct {
  uint32_t crc32;
  char host[100];
  char infor_arg[100];
  char config_arg[100];
  char route_arg[100];
  char BusStopNo[12]; //he so cua 4
  uint32_t port;
  IPAddress ethIP; //4 bytes
  uint32_t Interface;
  char WiFi_Pass[20];
  char WiFi_Name[20];
} EEData;

void Read_Config(bool bdefault = false);


void printConfig()
{
	DEBUG_SERIAL("host=%s\n",EEData.host);
	DEBUG_SERIAL("port=%d\n",EEData.port);
	DEBUG_SERIAL("infor_arg=%s\n",EEData.infor_arg);
	DEBUG_SERIAL("config_arg=%s\n",EEData.config_arg);
	DEBUG_SERIAL("route_arg=%s\n",EEData.route_arg);
	DEBUG_SERIAL("BusStopNo=%s\n",EEData.BusStopNo);	
	DEBUG_SERIAL("EthernetIP=%d.%d.%d.%d\n",EEData.ethIP[0],EEData.ethIP[1],EEData.ethIP[2],EEData.ethIP[3]);
	DEBUG_SERIAL("%s%s\n",Set_Interface,EEData.Interface == from_Ethernet ? "Ethernet" : "WIFI");
	DEBUG_SERIAL("%s%s,%s\n",Set_Wifi,EEData.WiFi_Name,EEData.WiFi_Pass);
}
void Read_Config(bool bdefault)
{
	wdt_reset();
	if (bdefault) 
	{
		EEData.host[0]=0;
		DEBUG_SERIAL("Reset to Default Config\n");
	}
	else
	{
		EEPROM.get(0,EEData);
		uint32_t crcOfData = calculateCRC32(((uint8_t*) &EEData) + 4, sizeof(EEData) - 4);
		if (crcOfData != EEData.crc32) {
		  DEBUG_SERIAL("Read_Config failed\n");

		  EEData.host[0]=0;
		}
		else {
		  DEBUG_SERIAL("Read_Config ok\n");
		}
	}
	if (EEData.host[0] == 0)
	{
		DEBUG_SERIAL("Set to default Config\n");
		EEData.ethIP = {192,168,1,10};
		EEData.Interface=from_Ethernet;
		memcpy(&EEData.WiFi_Name[0],"3G\0",sizeof(EEData.WiFi_Name));
		memcpy(&EEData.WiFi_Pass[0],"1234567890\0",sizeof(EEData.WiFi_Pass));
		memcpy(&EEData.host[0],"bus.danang.gov.vn\0",sizeof(EEData.host));
		memcpy(&EEData.infor_arg[0],"/bus-services-api/BusInfo?content=mt%20\0",sizeof(EEData.infor_arg));
		memcpy(&EEData.config_arg[0],"/bus-services-api/getConfigLed\0",sizeof(EEData.config_arg));
		memcpy(&EEData.route_arg[0],"/bus-services-api/getBusRoute?content=\0",sizeof(EEData.route_arg));
		memcpy(&EEData.BusStopNo[0],"0\0",sizeof(EEData.BusStopNo));
		EEData.port =1022;
		Save_Config();
	}
	printConfig();
}
void Save_Config()
{
	DEBUG_SERIAL("Save_Config\n");
	EEData.crc32 = calculateCRC32(((uint8_t*) &EEData) + 4, sizeof(EEData) - 4);
	EEPROM.put(0,EEData);
	EEPROM.commit();
	delay(100);
}
void SetupWifi()
{
	if (WIFI_AP_En)
	{
	 WiFi.mode(WIFI_AP_STA);
	 int count =20;
	  //
	  WiFi.begin(EEData.WiFi_Name, EEData.WiFi_Pass); //"3G","1234567890"
	  WiFi.softAP(chipID, password_ap);
	 count =20;
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		DEBUG_SERIAL(".");
		if (count--<=0) break;
		wdt_reset();
	  }
	}
	if (!WIFI_AP_En || WiFi.status() != WL_CONNECTED)
	{
		WiFi.mode(WIFI_STA);
		WiFi.softAP(chipID, password_ap);
	}
	  //
	  IPAddress ip = WiFi.localIP();
	  if (WiFi.status() != WL_CONNECTED) ip = WiFi.softAPIP();
	  DEBUG_SERIAL("\n");
	  DEBUG_SERIAL("WiFi IP: %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
	  //debugSerial.println(WiFi.localIP());
	  
	 httpUpdater.setup(&httpServer, update_path, update_username, update_password);
	 httpServer.begin();
}
void StartEthernet()
{
	digitalWrite(Ethernet_RESET_PIN,LOW);
	delay(500);
	digitalWrite(Ethernet_RESET_PIN,HIGH);
	delay(500);
	// start the Ethernet connection:
  /* if (Ethernet.begin(mac) == 0) {
    debugSerial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } */
 
  // start the Ethernet connection and the server:
  // IPAddress gateway = EEData.ethIP;
  // gateway[3] = 1;
  // IPAddress subnet(255, 255, 255, 0);
  // DEBUG_SERIAL("gateway=%d.%d.%d.%d\n",gateway[0],gateway[1],gateway[2],gateway[3]);  
  // Ethernet.begin(mac, EEData.ethIP,gateway,subnet);
  Ethernet.begin(mac, EEData.ethIP);
  debugSerial.print("Ethernet Shield IP ");
  debugSerial.println(Ethernet.localIP());
}
void setup() {
  //khoi tao truyen thong  
  //debugSerial.begin(115200);//115200
  displaySerial.begin(19200);
  Serial1.begin(115200);
  Serial.begin(115200);
  displaySerial.setTimeout(500);
  //
  DEBUG_SERIAL("StartUp\n");
  //
  
  ESP.wdtDisable();
  ESP.wdtEnable(20000); //ESP.wdtEnable(20000); //20s
  EEPROM.begin(1024);
  //
  sprintf(chipID, "BUS_%06X", ESP.getChipId());
  DEBUG_SERIAL("chipID=%s\n",String(chipID).c_str());

  display_Running_Sketch();
  
  pinMode(A0,INPUT);
  // pinMode(4,INPUT);
  // pinMode(5,INPUT);
  pinMode(Ethernet_RESET_PIN,OUTPUT);
  //digitalWrite(SIM800_RESET_PIN,GPRS_Using);
  //digitalWrite(SIM800_RESET_PIN,0);
  //
  DEBUG_SERIAL("ResetReason: %s\n",ESP.getResetReason().c_str());
  //Unixtime_GMT7 = 0;
  //External System  Power on
  if (ESP.getResetReason().indexOf("Power on")>=0) PW_On=true;
  rtc.year=1;
  rtc.month = 1;
  rtc.day = 1;
  rtc.hour = 0;
  rtc.minute = 0;
  rtc.second = 0;
  //
  Unixtime_GMT7 = GetUnixTime_fromrtc();
  Now();
  //
  Read_Config();
  //
  SetupWifi();
  //
  SetupTempsensor();
  firstScan=true;
  //
  //if (EEData.Interface == from_Ethernet) 
	  StartEthernet();
  
#ifdef wClient
  wServer.begin();
  wServer.setNoDelay(true);
#endif
  //
  if (PW_On) Reset_display();
  delay(3000);
  Startup_timestamp = millis();

}
//******************************************************************************************************

void loop() {
	wdt_reset();
	httpServer.handleClient();
	
	CheckSerial();
#ifdef wClient
	Wifi_Server();
#endif
	//
	bGet_condition=false; 
  uint32_t time = Now();
  if (time%10 == 0 && time != time_bk)
  {
	  time_bk = time;
	  DEBUG_SERIAL("%02d:%02d:%02d\n",rtc.hour,rtc.minute,rtc.second);	  
	  if (Check_display_Running()>=6) Reset_display();
  }
  //if (time%time_get==0 || firstScan) bGet_condition=true;//5m
  if (millis()-lastGet_timeStamp>(time_get*1000) || firstScan) 
  {
	   bGet_condition = true;
  }
  if (bGet_condition)
 {
	 if (EEData.Interface == from_Ethernet) 
	 {
		 int retry=3;
		while (Comm_Infor == 0) 
		{
			GET_Infor_from_Ethernet();
			// GET_Config_from_Ethernet();
			// GET_Route_from_Ethernet("2");
			if (--retry<=0) break;
			delay(1000);
		}
		if (Get_Error>=3)
		 {
			 Get_Error=0;
			 DEBUG_SERIAL("Reset Ethernet Shield\n");
			 StartEthernet();
		 }
	 }
	 else if (EEData.Interface == from_WIFI) 
	 {
		int retry=3;
		while (Comm_Infor == 0) 
		{
			GET_Infor_from_Wifi();
			// GET_Config_from_Wifi();
			// GET_Route_from_Wifi("2");
			if (--retry<=0) break;
			delay(1000);
		}
		if (Get_Error>=3)
		 {
			 Get_Error=0;
			 //DEBUG_SERIAL("Reconnect to WiFi\n");
			 //SetupWifi();
		 }
	 }
	 lastGet_timeStamp = millis();	 
	 Send_DateTime();
	 if (Comm_Infor == isBusInfo) Send_BusInfor();
	 if (Comm_Infor>0) Comm_Infor=0;
}
 DebugFromDisplay();
 //
 firstScan=false;
}
void DebugFromDisplay()
{	int n = 0;
	if (displaySerial.available())
	{
		n=displaySerial.readBytesUntil('\n', tempbuffer, sizeof(tempbuffer));	
		tempbuffer[n++] = '\n';
		tempbuffer[n] = 0;
	}
	if (n>0) DEBUG_SERIAL(tempbuffer);
}
bool Get_DateTimefrom_buffer()
{
	bool b=false;
	uint32_t u32 = (millis()- u32)/2;
	// YOUR IP:116.103.225.215<br>UNIXTIME:1478294079<br>DATETIME:2016-11-04 21:14:39
	do{
		char *p;char *p1;
		if (!(p=strstr(data_buffer,"UNIXTIME"))) break;
		if (!(p = strchr(p, ':'))) break;
		if (!(p1 = strchr(p, '<'))) break;
		*p1=0;
		Unixtime_GMT7 = strtoul(++p,NULL,10);
		Unixtime_GMT7 = Unixtime_GMT7 -(uint32_t)(millis()/1000) + (uint32_t)(u32/1000);
		Now();
		//Set datetime
		//"02/03/18,09:54:28+40"
		DEBUG_SERIAL("DateTime: %02d/%02d/%02d %02d:%02d:%02d\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
		b = true;
		break;
	}while(0);
	return b;
}
void CheckSerial()
{
if (Serial.available())
{
	int chars=Serial.readBytesUntil('\n', com_buffer, sizeof(com_buffer));	
	com_buffer[chars] = 0;
    Process_Com_buffer();
  while (Serial.available()) Serial.read(); //clear serial
}
}
void Wifi_Server()
{
  //check if there are any new clients
  if (wServer.hasClient()){
	wdt_reset();
	wificlient = wServer.available();
	DEBUG_SERIAL("wificlient hasClient");
    }
	if (wificlient && wificlient.available())
	{
		com_buffer_len = wificlient.available();
		wificlient.readBytes(com_buffer,com_buffer_len);
		com_buffer[com_buffer_len] =0;
		Process_Com_buffer();
	}
}
void Process_Com_buffer()
{
	char* p,*p1;
		if (strstr(com_buffer,Set_busStop))
		{
			p = strstr(com_buffer,Set_busStop) + sizeof(Set_busStop) - 1;
			p1 = strchr(p,'\r');
			if (p1) *p1 = 0;
			memcpy(EEData.BusStopNo,p,sizeof(EEData.BusStopNo));
			lastGet_timeStamp = 0;
			DEBUG_SERIAL("%s%s\n",Set_busStop,EEData.BusStopNo);
			Save_Config();
		}
		else if (strstr(com_buffer, Set_Interface))
		{
			p = strstr(com_buffer, Set_Interface) + sizeof(Set_Interface) - 1;
			EEData.Interface = atoi(p);
			lastGet_timeStamp = 0;
			DEBUG_SERIAL("%s%s\n",Set_Interface,EEData.Interface == from_Ethernet ? "Ethernet" : "WIFI");
			Save_Config();
		}
		else if (strstr(com_buffer,Set_Wifi))
		{
			p = strstr(com_buffer,Set_Wifi) + sizeof(Set_Wifi) - 1;
			p1 = strchr(p,',');
			if (p1==NULL) return;
			*p1 = 0;
			memcpy(EEData.WiFi_Name,p,sizeof(EEData.WiFi_Name));
			p = p1 + 1;
			p1 = strchr(p,'\r');
			if (p1) *p1 = 0;
			memcpy(EEData.WiFi_Pass,p,sizeof(EEData.WiFi_Pass));
			DEBUG_SERIAL("%s%s,%s\n",Set_Wifi,EEData.WiFi_Name,EEData.WiFi_Pass);
			Save_Config();
			SetupWifi();
		}
		else if (strstr(com_buffer, Set_EthIP))
		{
			//EthIP=192.168.1.10
			p = strstr(com_buffer, Set_EthIP) + sizeof(Set_EthIP) - 1;
			IPAddress ip;
			ip[0] = atoi(p);
			p1 = strchr(p,'.');
			if (p1==NULL) return; p=p1;
			ip[1] = atoi(p);
			p1 = strchr(p,'.');
			if (p1==NULL) return; p=p1;
			ip[2] = atoi(p);
			p1 = strchr(p,'.');
			if (p1==NULL) return; p=p1;
			ip[3] = atoi(p);
			EEData.ethIP = ip;			
			DEBUG_SERIAL("%s%d.%d.%d.%d\n",Set_EthIP,EEData.ethIP[0],EEData.ethIP[1],EEData.ethIP[2],EEData.ethIP[3]);
			Save_Config();
			StartEthernet();
		}
		else if (strstr(com_buffer, Set_Default))
		{			
			Read_Config(true);
		}
		else if (strstr(com_buffer, Get_Config))
		{			
			printConfig();
		}
		else
		{
			displaySerial.write(com_buffer,com_buffer_len);
		}
}
void GET_Config_from_Ethernet()
{
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //Eclient.println("GET /bus-services-api/BusInfo?content=mt%2068 HTTP/1.1");
	DEBUG_SERIAL("GET %s HTTP/1.1\n",EEData.config_arg);
	Eclient.printf("GET %s HTTP/1.1\n",EEData.config_arg);
    Eclient.print("Host: ");
	Eclient.println(EEData.host);
    Eclient.println("Connection: close");
    Eclient.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(Eclient,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		Eclient.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusConfig;
		//Send_BusInfor();
		}
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  Eclient.stop();
}
void GET_Infor_from_Ethernet()
{
  if (EEData.BusStopNo[1] == 0 && EEData.BusStopNo[0] == '0' )
  {
	 DEBUG_SERIAL("Not yet config BusStop=%s\n",EEData.BusStopNo);
  }
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //Eclient.println("GET /bus-services-api/BusInfo?content=mt%2068 HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo);
	Eclient.printf("GET %s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo);
    Eclient.print("Host: ");
	Eclient.println(EEData.host);
    Eclient.println("Connection: close");
    Eclient.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(Eclient,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		Eclient.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusInfo;
		//Send_BusInfor();
		}
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  Eclient.stop();
}
void GET_Route_from_Ethernet(char* routerID)
{
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //Eclient.println("GET /bus-services-api/BusInfo?content=mt%2068 HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.route_arg,routerID);
	Eclient.printf("GET %s%s HTTP/1.1\n",EEData.route_arg,routerID);
    Eclient.print("Host: ");
	Eclient.println(EEData.host);
    Eclient.println("Connection: close");
    Eclient.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(Eclient,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		Eclient.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusRoute;
		}
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  Eclient.stop();
}
int handleHeaderResponse(EthernetClient cli,const unsigned long getTimeout)
{
    //String transferEncoding;
    int _returnCode = -1;
    int _size = -1;
    unsigned long lastDataTime = millis();
	e_bytesRecv=0;
	header_time = "";
    while(1) {
		wdt_reset();
        size_t len = cli.available();
        if(len > 0) {
            String headerLine = cli.readStringUntil('\n');
            headerLine.trim(); // remove \r
			DEBUG_SERIAL("%s\n",headerLine.c_str());
            lastDataTime = millis();

            ////DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] RX: '%s'\n", headerLine.c_str());
			// int temp=headerLine.indexOf("HTTP/1.");
			// if (temp>0)
			// {
				// headerLine=headerLine.substring(temp);
			// }
            if(headerLine.startsWith("HTTP/1.")) {
                _returnCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
				//DEBUG_SERIAL("ReturnCode: %d\n", _returnCode);
            } else if(headerLine.indexOf(':')) {
                String headerName = headerLine.substring(0, headerLine.indexOf(':'));
                String headerValue = headerLine.substring(headerLine.indexOf(':') + 1);
                headerValue.trim();

                if(headerName.equalsIgnoreCase("Content-Length")) {
                    _size = headerValue.toInt();
					e_bytesRecv = _size;
                }
				else if(headerName.equalsIgnoreCase("Date")) {
                    header_time = headerValue;	
					//DEBUG_SERIAL("%s\n",header_time.c_str());					
                }

               

                // if(headerName.equalsIgnoreCase("Transfer-Encoding")) {
                    // transferEncoding = headerValue;
                // }
				/* if (header_count<Headers_Max && headerName != ""){
					Headers[header_count].name=headerName;
					Headers[header_count].value=headerValue;
					header_count++;
					DEBUG_SERIAL("Header %d name=%s, value=%s\n",header_count,headerName.c_str(),headerValue.c_str());
                } */
            }

            if(headerLine == "") {
                DEBUG_SERIAL("[handleHeaderResponse] code: %d\n", _returnCode);

                if(_size > 0) {
                    DEBUG_SERIAL("[handleHeaderResponse] size: %d\n", _size);
                }
				return _returnCode;
            }

        } else {
            if((millis() - lastDataTime) > getTimeout) {
				_returnCode=-1;
                return _returnCode;
            }
            delay(0);
        }
    }
    return _returnCode;
}
void GET_Config_from_Wifi()
{
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return;
	}
	WiFiClient client1;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (client1.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //client1.println("GET /bus-services-api/getConfigLed HTTP/1.1");
	DEBUG_SERIAL("GET %s HTTP/1.1\n",EEData.config_arg);
	client1.printf("GET %s HTTP/1.1\n",EEData.config_arg);
    client1.print("Host: ");
	client1.println(EEData.host);
    client1.println("Connection: close");
    client1.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(client1,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		client1.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusConfig;
		}
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  client1.stop();
}
void GET_Infor_from_Wifi()
{
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return;
	}
	if (EEData.BusStopNo[1] == 0 && EEData.BusStopNo[0] == '0' )
  {
	 DEBUG_SERIAL("Not yet config BusStop=%s\n",EEData.BusStopNo);
  }
	WiFiClient client1;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (client1.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //client1.println("GET /bus-services-api/BusInfo?content=mt%2068 HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo);
	client1.printf("GET %s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo);
    client1.print("Host: ");
	client1.println(EEData.host);
    client1.println("Connection: close");
    client1.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(client1,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		client1.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusInfo;
		//Send_BusInfor();
		}
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  client1.stop();
}
void GET_Route_from_Wifi(char *routerID)
{
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return;
	}
	WiFiClient client1;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (client1.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //client1.println("GET /bus-services-api/getConfigLed HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.route_arg,routerID);
	client1.printf("GET %s%s HTTP/1.1\n",EEData.route_arg,routerID);
    client1.print("Host: ");
	client1.println(EEData.host);
    client1.println("Connection: close");
    client1.println();
	int dl=30;
	e_bytesRecv = 0;
	if (handleHeaderResponse(client1,7000) == 200)
	{
		if (e_bytesRecv>0)
		{
		client1.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusRoute;
		}
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  client1.stop();
}
int handleHeaderResponse(WiFiClient cli,const unsigned long getTimeout)
{
    //String transferEncoding;
    int _returnCode = -1;
    int _size = -1;
    unsigned long lastDataTime = millis();
	e_bytesRecv=0;
	header_time = "";
    while(1) {
		wdt_reset();
        size_t len = cli.available();
        if(len > 0) {
            String headerLine = cli.readStringUntil('\n');
            headerLine.trim(); // remove \r
			DEBUG_SERIAL("%s\n",headerLine.c_str());
            lastDataTime = millis();

            ////DEBUG_HTTPCLIENT("[HTTP-Client][handleHeaderResponse] RX: '%s'\n", headerLine.c_str());
			// int temp=headerLine.indexOf("HTTP/1.");
			// if (temp>0)
			// {
				// headerLine=headerLine.substring(temp);
			// }
            if(headerLine.startsWith("HTTP/1.")) {
                _returnCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
				//DEBUG_SERIAL("ReturnCode: %d\n", _returnCode);
            } else if(headerLine.indexOf(':')) {
                String headerName = headerLine.substring(0, headerLine.indexOf(':'));
                String headerValue = headerLine.substring(headerLine.indexOf(':') + 1);
                headerValue.trim();

                if(headerName.equalsIgnoreCase("Content-Length")) {
                    _size = headerValue.toInt();
					e_bytesRecv = _size;
                }
				else if(headerName.equalsIgnoreCase("Date")) {
                    header_time = headerValue;	
					//DEBUG_SERIAL("%s\n",header_time.c_str());					
                }

               

                // if(headerName.equalsIgnoreCase("Transfer-Encoding")) {
                    // transferEncoding = headerValue;
                // }
				/* if (header_count<Headers_Max && headerName != ""){
					Headers[header_count].name=headerName;
					Headers[header_count].value=headerValue;
					header_count++;
					DEBUG_SERIAL("Header %d name=%s, value=%s\n",header_count,headerName.c_str(),headerValue.c_str());
                } */
            }

            if(headerLine == "") {
                DEBUG_SERIAL("[handleHeaderResponse] code: %d\n", _returnCode);

                if(_size > 0) {
                    DEBUG_SERIAL("[handleHeaderResponse] size: %d\n", _size);
                }
				return _returnCode;
            }

        } else {
            if((millis() - lastDataTime) > getTimeout) {
				_returnCode=-1;
                return _returnCode;
            }
            delay(0);
        }
    }
    return _returnCode;
}
bool GetTime_fromHeader()
{
	bool bok=false;
	//header_time=Wed, 05 Apr 2017 08:42:28 GMT"
	char* p;
	char buf[30];
	uint8_t yOff, mo, d, hh, mm, ss;
	header_time.toCharArray(buf,sizeof(buf));
	do{
	p = strchr(buf,',') + 2;
	if (p == NULL) break;
	d = atoi(p);
	p = strchr(p,' ') + 1;
	if (p == NULL) break;
	switch (*p) {
        case 'J': mo = *(p+1) == 'a' ? 1 : mo = *(p+2) == 'n' ? 6 : 7; break;
        case 'F': mo = 2; break;
        case 'A': mo = *(p+2) == 'r' ? 4 : 8; break;
        case 'M': mo = *(p+2) == 'r' ? 3 : 5; break;
        case 'S': mo = 9; break;
        case 'O': mo = 10; break;
        case 'N': mo = 11; break;
        case 'D': mo = 12; break;
    }
	p = strchr(p,' ') + 1;
	yOff = atoi(p) - 2000;
	p = strchr(p,' ') + 1;
	hh = atoi(p);
	p = strchr(p,':') + 1;
	mm = atoi(p);
	p = strchr(p,':') + 1;
	ss = atoi(p);
	if (mo <=1 && d<=1 && hh ==0 ) break;
	  else if (yOff>99 || mo>12 || mo==0 || d==0 || d>31 || hh>23 || mm>59 || ss>59) break;
	  else
	  {
		  rtc.year=yOff;
		  rtc.month = mo;
		  rtc.day = d;
		  rtc.hour = hh;
		  rtc.minute = mm;
		  rtc.second = ss;
		  //
		  Unixtime_GMT7 = GetUnixTime_fromrtc()-(millis()/1000) + 25200;
		  Now();
		  DEBUG_SERIAL("DateTime: %02d/%02d/%02d %02d:%02d:%02d\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
		  bok = true;
		  break;
	  }
	}
	while(0);
	return bok;
}
uint8_t Check_display_Running()
{
	uint32_t t;
	uint32_t timeout = 2000;
	int n = 0;
	int retry=2;
	bool ret=false;
  while(retry-->0)
  {
	  n=0;
	  t = millis();
	purgedisplaySerial();
	displaySerial.print(CheckRunning);
	  do {
		  wdt_reset();
		if (displaySerial.available()) {
			wdt_reset();
		  char c = displaySerial.read();
		  t = millis();
		  if (n >= sizeof(tempbuffer) - 1) {
			// buffer full, discard first half
			n = sizeof(tempbuffer) / 2 - 1;
			memcpy(tempbuffer, tempbuffer + sizeof(tempbuffer) / 2, n);
		  }
		  tempbuffer[n++] = c;
		  tempbuffer[n] = 0;		  
		  if (strstr(tempbuffer, Running)) {
		   DEBUG_SERIAL("[DISPLAY] OK\n");
		   ret = true;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, Idle)) {
		   DEBUG_SERIAL("[DISPLAY] Idle\n");
		   ret = true;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, StartUp)) {
		   DEBUG_SERIAL("[DISPLAY] OK\n");
		   lastGet_timeStamp = 0;
		   ret = true;
		   Comm_Error = 0;
		   break;
		  }
		}
	  } while (millis() - t < timeout);
	  if (ret) break;
	  else DEBUG_SERIAL(tempbuffer);
  }
  if (!ret) Comm_Error += 1;
  return Comm_Error;
}
void Reset_display()
{
	DEBUG_SERIAL("[RESET DISPLAY]\n");
	pinMode(DISPLAY_RESET,OUTPUT);
	digitalWrite(DISPLAY_RESET,LOW);
	wdt_reset();
	delay(1000);
	digitalWrite(DISPLAY_RESET,HIGH);
	pinMode(DISPLAY_RESET,INPUT);
	Comm_Error = 0;
}
uint8_t Send_BusInfor()
{
	if (Comm_Infor != isBusInfo) return 0;
	Comm_Infor = isBusIdle;
	char tempbuffer[100];	
	uint32_t t;
	uint32_t timeout = 2000;
  int n = 0;
  int retry=3;
  int step=0;
  char *p,*p1;
  //
	p = strchr(data_buffer,'[');
	p1 =strchr(data_buffer,']');
  if (p==NULL || p1==NULL)
  {
	  DEBUG_SERIAL("%s Not OK\n",BusInfo);
	 return 0;
  }
  int size = (p1 - p) + 1;
  byte bcc = BCC(p,size);
  DEBUG_SERIAL("Send %s\n",BusInfo);
  while(retry-->0)
  {
	  n=0;
	  step=0;
	  t = millis();
	purgedisplaySerial();
	displaySerial.print(BusInfo);
	
	  do {
		  wdt_reset();
		if (displaySerial.available()) {
			wdt_reset();
		  char c = displaySerial.read();
		  t = millis();
		  if (n >= sizeof(tempbuffer) - 1) {
			// buffer full, discard first half
			n = sizeof(tempbuffer) / 2 - 1;
			memcpy(tempbuffer, tempbuffer + sizeof(tempbuffer) / 2, n);
		  }
		  tempbuffer[n++] = c;
		  tempbuffer[n] = 0;		  
		  if (strstr(tempbuffer, BusInfo)) {
		   step =1;
		   break;
		  }
		}
	  } while (millis() - t < timeout);
	  // if (step == 1) break;
	  // else DEBUG_SERIAL(tempbuffer);
  if (step == 1)
  {
	DEBUG_SERIAL("size=%d,BCC=%d,%s%s\n",size,bcc,Set_busStop,EEData.BusStopNo);
	displaySerial.printf("size=%d,BCC=%d,%s%s,",size,bcc,Set_busStop,EEData.BusStopNo);
	//p = &data_buffer[0];
	int id=0;	
	while (id<size)
	{
		while (displaySerial.available()) displaySerial.read();
		p = &data_buffer[id]; id +=200;
		char c = data_buffer[id];
		data_buffer[id]=0;
		displaySerial.print(p);
		data_buffer[id]=c;
		int dl = 500;
		/* while (displaySerial.available() == 0) 
		{
			delay(1);
			if (dl--<=0) break;
		} */
		delay(50);
		wdt_reset();
	}
	//displaySerial.print(data_buffer);
	//DEBUG_SERIAL(data_buffer); DEBUG_SERIAL("\n");
	wdt_reset();
	//displaySerial.flush();
	//delay(10);
	step = 2;
	//
	n=0;
	t = millis();
	displaySerial.println(End_BusInfo);
	  do {
		  wdt_reset();
		if (displaySerial.available()) {
			wdt_reset();
		  char c = displaySerial.read();
		  t = millis();
		  if (n >= sizeof(tempbuffer) - 1) {
			// buffer full, discard first half
			n = sizeof(tempbuffer) / 2 - 1;
			memcpy(tempbuffer, tempbuffer + sizeof(tempbuffer) / 2, n);
		  }
		  tempbuffer[n++] = c;
		  tempbuffer[n] = 0;		  
		  if (strstr(tempbuffer, End_BusInfo)) {
		   step = 3;		   
		   DEBUG_SERIAL("Send %s OK\n",BusInfo);
		   break;
		  }
		  else if (strstr(tempbuffer, CheckSize_Fail)) {
		   DEBUG_SERIAL("%s\n",CheckSize_Fail);
		   break;
		  }
		  else if (strstr(tempbuffer, CheckSum_Fail)) {
		   DEBUG_SERIAL("%s\n",CheckSum_Fail);
		   break;
		  }
		}
	  } while (millis() - t < timeout);
  }
  if (step == 3) break;
  DEBUG_SERIAL("Sync %s timeout\n",BusInfo);
  if (retry>0)
  {
	  delay(100);
	  DEBUG_SERIAL("retry Send %s\n",BusInfo);
  }
 }
  //DEBUG_SERIAL("step=%d\n",step);
  return step;
}
void Send_DateTime()
{
	uint32_t t = Now();
	// DEBUG_SERIAL("%s%02d:%02d:%02d%s\n",String(Set_Time).c_str(),rtc.hour,rtc.minute,rtc.second,String(End_Time).c_str());
	// displaySerial.printf("%s%02d:%02d:%02d%s",String(Set_Time).c_str(),rtc.hour,rtc.minute,rtc.second,String(End_Time).c_str());
	DEBUG_SERIAL("%s%lu%s\n",String(Set_Time).c_str(),t,String(End_Time).c_str());
	displaySerial.printf("%s%lu%s",String(Set_Time).c_str(),t,String(End_Time).c_str());
}
void convert2upperChar(char *ch)
{
  // char *ch;
  // ch = bufsmsbuffer;
  while (*ch) // exits on terminating NULL
  {
    if (( *ch >= 'a' ) && ( *ch <= 'z' ))  *ch &= 0xDF;
	// if (removeCRLF)
	// {
		// if ((*ch == '\r') || (*ch == '\n')) {*ch=0;break;}
	// }
	ch+=1;
  }
}
void String2buffer(char *buf,String s)
{
  memset(buf,0,sizeof(buf));
  s.toCharArray(buf,sizeof(buf));
}
String getFlashTime()
{
	if (sketch_time[0]!=0) return String(sketch_time);
	char* date = __DATE__;
	char* time = __TIME__;
    uint8_t y,mo,d,h,m,s;
	y= conv2d(date + 9);
	// if (t<10) s += "0";
	// s += String(t);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	
    switch (date[0]) {
        case 'J': mo = date[1] == 'a' ? 1 : mo = date[2] == 'n' ? 6 : 7; break;
        case 'F': mo = 2; break;
        case 'A': mo = date[2] == 'r' ? 4 : 8; break;
        case 'M': mo = date[2] == 'r' ? 3 : 5; break;
        case 'S': mo = 9; break;
        case 'O': mo = 10; break;
        case 'N': mo = 11; break;
        case 'D': mo = 12; break;
    }
    d = conv2d(date + 4); //d
	
    h = conv2d(time);
	m = conv2d(time + 3);
	s = conv2d(time + 6);
	if (s>=40)
	{
		s=s-40;
		m +=1;
		if (m==60 && h<59) h +=1;
	}
	else s +=20;
	sprintf(sketch_time,"%02d%02d%02d%02d%02d%02d",y,mo,d,h,m,s);	
	return String(sketch_time);
}
uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
// displays at startup the Sketch running in the Arduino
void display_Running_Sketch (void){
  String the_path = __FILE__;
  int slash_loc = the_path.lastIndexOf('\\');
  if (slash_loc<=0) slash_loc = the_path.lastIndexOf('/');
  String the_ino_name = the_path.substring(slash_loc+1);
  //Serial.println(the_ino_name);
  the_ino_name.toCharArray(sketch_name,50);
  // int dot_loc = the_ino_name.lastIndexOf('.');
  // String the_sketchname = the_ino_name.substring(0, dot_loc);

  DEBUG_SERIAL("Sketch=%s\n",String(sketch_name).c_str());
  DEBUG_SERIAL("Compiled on: %s %s\n",__DATE__,__TIME__);
	getFlashTime();
}
void SetupTempsensor()
{
	 sensors.begin();  
  DeviceCount=sensors.getDeviceCount();
  if (DeviceCount<=0){
	 DEBUG_SERIAL("No Temp sensor found\n");
  }
  else{
	  DEBUG_SERIAL("Found %d sensor\n",DeviceCount);
	  sensors.getAddress(insideThermometer, 0);
	  sensors.setResolution(insideThermometer, 10);
	  sensors.getResolution(insideThermometer);
	  current_Temp=getTemperature();
  }
}
float getTemperature()
{
	if (DeviceCount==0) return -1.0f;
	sensors.requestTemperatures();
	delay(250);
	float tempC = sensors.getTempC(insideThermometer);
	DEBUG_SERIAL("Temp=%s\n",String(tempC,1).c_str());
  return tempC;  
}
uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}
void DateTime (uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

    rtc.second = t % 60;
    t /= 60;
    rtc.minute = t % 60;
    t /= 60;
    rtc.hour = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (rtc.year = 0; ; ++rtc.year) {
        leap = rtc.year % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (rtc.month = 1; ; ++rtc.month) {
        uint8_t daysPerMonth = daysInMonth[rtc.month - 1];
        if (leap && rtc.month == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    rtc.day = days + 1;
}
uint32_t Now()
{
	uint32_t t = (millis()/1000 + Unixtime_GMT7);
	DateTime(t);
	return t;
}
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += daysInMonth[i - 1];
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}
uint32_t GetUnixTime_fromrtc()
{
	uint32_t t;
  uint16_t days = date2days(rtc.year, rtc.month, rtc.day);
  t = time2long(days, rtc.hour, rtc.minute, rtc.second);
  t += SECONDS_FROM_1970_TO_2000;  // seconds from 1970 to 2000  
  return t;
}
void purgedisplaySerial()
{
  while (displaySerial.available()) displaySerial.read();
}
byte BCC(char* from, int len)
{
	byte bcc=0;
	while(len-->0)
	{
		bcc ^= *from++;
	}
	return bcc;
}

