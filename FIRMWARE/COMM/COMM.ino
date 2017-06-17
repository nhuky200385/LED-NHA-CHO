// compiled path C:\Users\LT-N2K\AppData\Local\Temp

#include <memory>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiUdp.h>


#include <SPI.h>
#include <Ethernet.h>

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
unsigned int udpPort = 11000;  // local port to listen on

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


// #define time_getInfor 15 //seconds
// #define time_checkconfig 300 //seconds


#define Set_EthIP "EthIP="
#define Set_Wifi "Wifi="
#define Set_Interface "Interface="
#define Set_busStop "BusStop="
#define Set_Default "Default=1"
#define Set_Host "Host=" //Host=abc,Port=123,Infor_arg=akdj,Config_arg=add,Route_arg=xxx
#define Set_Port "Post="
#define Set_Infor_arg "Infor_arg="
#define Set_Config_arg "Config_arg="
#define Set_Route_arg "Route_arg="
#define Set_time_getInfor "TgetInfor="
#define Set_time_checkconfig "TcheckConfig="
#define Show_Config "Config?"
#define StartUp "StartUp"
#define Begin_BusInfo "BusInfo"
#define End_BusInfo "End_Info"
#define Begin_BusConfig "BusConfig"
#define End_BusConfig "End_Config"
#define Begin_BusRoute "BusRoute"
#define End_BusRoute "End_Route"
#define CheckRunning "Running?"
#define Running "RUNNING"
#define Idle "IDLE"
#define Begin_SetTime "Set_Time"
#define End_SetTime "End_Time"
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
unsigned long lastCheckConfig_timeStamp;

bool PW_On=false;
bool AP_On=false;
bool Checkupdate=false;
bool isUpdate=false;
float voltage;
bool timer0_en;
bool firstScan;
bool RF_Disable;
bool bDisplay_isRunning;
bool bGet_condition,bGet_config;
bool bDateTime_OK = false;

uint32_t unixTime_sent;
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
  uint32_t time_getInfor; //in seconds
  uint32_t time_checkconfig; //seconds
  char WiFi_Pass[20];
  char WiFi_Name[20];
} EEData;
typedef struct {
	uint8_t display_index;
	char route_no[12],car_no[12],time_arrival[6],passenger[10];
	bool changed;
	bool visible;
	bool route_sent;
}bus_struct;
bool Config_sent = false;
bool bisNew_Config = false;
uint32_t last_ConfigTime = 0;

#define Bus_Max 10
int Bus_count;
bus_struct Bus[Bus_Max];
byte Bus_sort[Bus_Max];

void Read_Config(bool bdefault = false);
uint8_t Send_BusConfig(bool isfrom_buffer = false);


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
	DEBUG_SERIAL("time_getInfor=%ds\n",EEData.time_getInfor);
	DEBUG_SERIAL("time_checkconfig=%ds\n",EEData.time_checkconfig);
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
		EEData.time_getInfor = 20; //s
		EEData.time_checkconfig = 300; //5M
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
	  WiFi.begin(EEData.WiFi_Name, EEData.WiFi_Pass);
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
	 //
	 udp.begin(udpPort);
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
  mac[sizeof(mac)-1] = ESP.getChipId() % 200;

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
  if (bDateTime_OK && (time < unixTime_sent || time - unixTime_sent >300))
  {	  
	  if (Send_DateTime()) unixTime_sent = time;
	  else unixTime_sent = time-290;  //resend after 10 s;
  }
  if (firstScan || (time%10 == 0 && time != time_bk))
  {
	  time_bk = time;
	  DEBUG_SERIAL("%02d:%02d:%02d\n",rtc.hour,rtc.minute,rtc.second);	  
	  if (Check_display_Running()>=6) Reset_display();
  }
  if (millis()-lastGet_timeStamp>(EEData.time_getInfor*1000) || firstScan) 
  {
	   if (bDisplay_isRunning) bGet_condition = true;
  }
  if (bGet_condition)
 {
	 int retry=3;
		while (Comm_Infor == 0) 
		{
			Get_Infor();
			if (--retry<=0) break;
			delay(1000);
		}
		if (Get_Error>=3 && EEData.Interface == from_Ethernet)
		 {
			 Get_Error=0;
			 DEBUG_SERIAL("Reset Ethernet Shield\n");
			 StartEthernet();
		 }	 
	 //Send_DateTime();
	 while (Comm_Infor == isBusInfo) {
		 Get_BusInfor_from_buffer();
		 //Send config
		 if (!Config_sent)
		 {
			if (Send_BusConfig() == 3) 
			{
				Config_sent = true;
				bisNew_Config = false;
				lastCheckConfig_timeStamp = millis();
			}
		 }
		 if (!Config_sent) break; //loi ko gui dc
		 //Send Route Infomation
		 int i=0;
		 for (i=0;i<Bus_count;i++)
		 {
			if (!Bus[i].route_sent)
			{
				if (Send_BusRoute(&Bus[i].route_no[0]) == 3) Bus[i].route_sent = true;
				else break;
			}
		 }
		 if (i<Bus_count) break; //loi ko gui dc
		 //Send_BusInfor();
		 String s="[";
		 for (int k=0;k<Bus_count;k++)
		 {
			 int i = Bus_sort[k];
			//if (Bus[i].changed || 
			if(Bus[i].visible)
			{
				s += "{";
				// char *p2 = &Bus[i].route_no[0];
				// DEBUG_SERIAL("Bus %d: ",i);
				// while (*p2>0) DEBUG_SERIAL("%2X ",*p2++);
				// DEBUG_SERIAL("\n");
				//sprintf(data_buffer,"[index=%d,route_no=%s,car_no=%s,time=%s,passenger=%s,visible=%d,]\0",Bus[i].display_index,Bus[i].route_no,Bus[i].car_no,Bus[i].time_arrival,Bus[i].passenger,Bus[i].visible);
				//s += "index=" + String(Bus[i].display_index);
				s += "route_no=" + String(Bus[i].route_no);
				if (Bus[i].changed == false) s += ",no_change";
				else{
				s += ",car_no=" + String(Bus[i].car_no);
				s += ",time=" + String(Bus[i].time_arrival);
				s += ",passenger=" + String(Bus[i].passenger);
				//s += ",visible=" + String(Bus[i].visible);
				}
				s += ",}";
				//DEBUG_SERIAL("%s\n",data_buffer);
				//Send_data2display(Begin_BusInfo,End_BusInfo);
				
			}
		 }
		s += "]";
		s.toCharArray(data_buffer,sizeof(data_buffer)-1);
		//DEBUG_SERIAL("%s\n",data_buffer);
		if (Send_data2display(Begin_BusInfo,End_BusInfo))
		{
			for (int k=0;k<Bus_count;k++) Bus[k].changed = false;
		}
		Comm_Infor=0;
	 }
	 if (Comm_Infor>0) Comm_Infor=0;
	 lastGet_timeStamp = millis();
}
//check config
if (bDisplay_isRunning && millis()-lastCheckConfig_timeStamp>(EEData.time_checkconfig*1000)) 
  {
	Get_Config();
	if (bisNew_Config) Send_BusConfig(true); //from buffer
	lastCheckConfig_timeStamp = millis(); 
  }
 DebugFromDisplay();
 //
 Check_udp_packet();
 //
 firstScan=false;
}
void DebugFromDisplay()
{	int n = 0;
	tempbuffer[n] = 0;
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
void Check_udp_packet()
{
	int packetSize = udp.parsePacket();
  if (packetSize) {
    IPAddress remoteIp = udp.remoteIP();
	DEBUG_SERIAL("UDP from: %d.%d.%d.%d, port %d\n",remoteIp[0],remoteIp[1],remoteIp[2],remoteIp[3],udp.remotePort());

    // read the packet into packetBufffer
    int len = udp.read(tempbuffer, sizeof(tempbuffer));
    if (len > 0) {
      tempbuffer[len+1] = 0;
	  wdt_reset();
    }
    DEBUG_SERIAL("%s\n",String(tempbuffer).c_str());
		//Send to back udp server	 
	if (strstr(tempbuffer,"???") != NULL)
	{
		IPAddress ip = WiFi.localIP();
	    if (WiFi.status() != WL_CONNECTED) ip = WiFi.softAPIP();
		sprintf(tempbuffer,"chipID=%s\nSketch=%s\nCompiled on=%s %s\nWiFi IP: %d.%d.%d.%d\n", chipID,String(sketch_name).c_str(),__DATE__,__TIME__,ip[0],ip[1],ip[2],ip[3]);
		udp.beginPacket(remoteIp, udp.remotePort());
		udp.print(tempbuffer);
		udp.endPacket();
	}
  }
}
void Process_Com_buffer()
{
	char* p,*p1;
	//loai \r\n neu co
	p = strchr(com_buffer,'\r');
	if (p) *p = 0;
	p = strchr(com_buffer,'\n');
	if (p) *p = 0;
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
		else if (strstr(com_buffer,Set_Host))
		{
			do
			{
				p = strstr(com_buffer,Set_Host) + sizeof(Set_Host) - 1;
				p1 = strchr(p,',');
				if (p1) *p1 = 0;
				memcpy(EEData.host,p,sizeof(EEData.host));
				if (p1) *p1 = ','; else break;
				p = p1 + 1;
				p = strstr(com_buffer,Set_Port);
				if (p==NULL) break;
				p+= sizeof(Set_Port) - 1;
				EEData.port = atoi(p);
				p = strstr(com_buffer,Set_Infor_arg);
				if (p==NULL) break;
				p += sizeof(Set_Infor_arg) - 1;
				p1 = strchr(p,',');
				if (p1) *p1 = 0;
				memcpy(EEData.infor_arg,p,sizeof(EEData.infor_arg));
				if (p1) *p1 = ','; else break;
				p = strstr(com_buffer,Set_Config_arg);
				if (p==NULL) break;
				p += sizeof(Set_Config_arg) - 1;
				p1 = strchr(p,',');
				if (p1) *p1 = 0;
				memcpy(EEData.config_arg,p,sizeof(EEData.config_arg));
				if (p1) *p1 = ','; else break;
				p = strstr(com_buffer,Set_Route_arg);
				if (p==NULL) break;
				p += sizeof(Set_Route_arg) - 1;
				p1 = strchr(p,',');
				if (p1) *p1 = 0;
				memcpy(EEData.route_arg,p,sizeof(EEData.route_arg));
				break;			
			}while (true);
			Save_Config();
			printConfig();
		}
		else if (strstr(com_buffer, Set_time_getInfor))
		{
			p = strstr(com_buffer, Set_time_getInfor) + sizeof(Set_time_getInfor) - 1;
			EEData.time_getInfor = atoi(p);
			DEBUG_SERIAL("%s%ds\n",Set_time_getInfor,EEData.time_getInfor);
			Save_Config();
		}
		else if (strstr(com_buffer, Set_time_checkconfig))
		{
			p = strstr(com_buffer, Set_time_checkconfig) + sizeof(Set_time_checkconfig) - 1;
			EEData.time_checkconfig = atoi(p);
			DEBUG_SERIAL("%s%ds\n",Set_time_checkconfig,EEData.time_checkconfig);
			Save_Config();
		}
		else if (strstr(com_buffer, Set_Default))
		{			
			Read_Config(true);
		}
		else if (strstr(com_buffer, Show_Config))
		{			
			printConfig();
		}
		else
		{
			displaySerial.write(com_buffer,com_buffer_len);
		}
}
void Reset_Bus()
{
	/* typedef struct {
		uint8_t display_index;
		char route_no[12],car_no[12],time_arrival[6],passenger[10];
		bool changed;
		bool visible;
		bool route_sent;
	}bus_struct; */
	for (int i=0;i<Bus_Max;i++)
	{
		// Bus[i].route_no[0] = 0;
		// Bus[i].car_no[0] = 0;
		// Bus[i].time_arrival[0] = 0;
		// Bus[i].passenger[0] = 0;
		Bus[i].changed = false;
		Bus[i].visible = false;
		Bus[i].route_sent = false;
		Bus[i].display_index = 0;
		//
		memset(Bus[i].route_no,0,sizeof(Bus[i].route_no)-1);
		memset(Bus[i].car_no,0,sizeof(Bus[i].car_no)-1);
		memset(Bus[i].time_arrival,0,sizeof(Bus[i].time_arrival)-1);
		memset(Bus[i].passenger,0,sizeof(Bus[i].passenger)-1);
	}
	Bus_count = 0;
	unixTime_sent = 0;
	Config_sent = false;
	last_ConfigTime = 0;
}
bool Get_Config()
{
	bool b = false;
	bisNew_Config = false;
	if (EEData.Interface == from_Ethernet) b= GET_Config_from_Ethernet();
	else if (EEData.Interface == from_WIFI) b= GET_Config_from_Wifi();
	if (b)
	{
		//"ConfigTime":1497078249}
		char *p = strstr(data_buffer,"ConfigTime");
		if (p)
		{
			p = strchr(p,':') + 1;
			uint32_t ts = atol(p);
			if (ts != last_ConfigTime)
			{
				bisNew_Config = true;				
				DEBUG_SERIAL("New Config %lu != %lu\n",last_ConfigTime,ts);
				last_ConfigTime = ts;
			}
		}
	}
	return b;
}
bool Get_Infor()
{
	if (bDateTime_OK && EEData.BusStopNo[1] == 0 && EEData.BusStopNo[0] == '0')
	  {
		 DEBUG_SERIAL("Not yet config BusStop=%s\n",EEData.BusStopNo);
		 return false;
	  }
	if (EEData.Interface == from_Ethernet) return GET_Infor_from_Ethernet();
	else if (EEData.Interface == from_WIFI) return GET_Infor_from_Wifi();
	else return false;
}
bool Get_Route(char *routeNo)
{
	if (EEData.Interface == from_Ethernet) return GET_Route_from_Ethernet(routeNo);
	else if (EEData.Interface == from_WIFI) return GET_Route_from_Wifi(routeNo);
	else return false;
}
bool GET_Config_from_Ethernet()
{
	data_buffer[0] = 0;
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
	}else e_bytesRecv = 0;
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
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_Infor_from_Ethernet()
{
	data_buffer[0] = 0;
  
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
	}else e_bytesRecv = 0;
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
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_Route_from_Ethernet(char* routeNo)
{
	data_buffer[0] = 0;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //Eclient.println("GET /bus-services-api/BusInfo?content=mt%2068 HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.route_arg,routeNo);
	Eclient.printf("GET %s%s HTTP/1.1\n",EEData.route_arg,routeNo);
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
	}else e_bytesRecv = 0;
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  Eclient.stop();
  if (e_bytesRecv>0) return true;
  else return false;
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
			//DEBUG_SERIAL("%s\n",headerLine.c_str());
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
bool GET_Config_from_Wifi()
{
	data_buffer[0] = 0;
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return false;
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
	}else e_bytesRecv = 0;
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
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_Infor_from_Wifi()
{
	data_buffer[0] = 0;
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return false;
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
	}else e_bytesRecv = 0;
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
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_Route_from_Wifi(char *routeNo)
{
	data_buffer[0] = 0;
	if  (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected\n");
		Get_Error +=1;
		return false;
	}
	WiFiClient client1;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (client1.connect(EEData.host, EEData.port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //client1.println("GET /bus-services-api/getConfigLed HTTP/1.1");
	DEBUG_SERIAL("GET %s%s HTTP/1.1\n",EEData.route_arg,routeNo);
	client1.printf("GET %s%s HTTP/1.1\n",EEData.route_arg,routeNo);
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
	else e_bytesRecv = 0;
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	Get_Error +=1;
  }
  client1.stop();
  if (e_bytesRecv>0) return true;
  else return false;
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
			//DEBUG_SERIAL("%s\n",headerLine.c_str());
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
		  bDateTime_OK = true;
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
	bDisplay_isRunning = false;
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
		   DEBUG_SERIAL("[DISPLAY] %s\n",Running);
		   ret = true;
		   bDisplay_isRunning = true;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, Idle)) {
		   DEBUG_SERIAL("[DISPLAY] %s\n",Idle);
		   ret = true;
		   bDisplay_isRunning = false;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, StartUp)) {
		   DEBUG_SERIAL("[DISPLAY] %s\n", StartUp);
		   bDisplay_isRunning = true;
		   lastGet_timeStamp = 0;
		   ret = true;
		   Comm_Error = 0;
		   Reset_Bus();
		   break;
		  }
		}
	  } while (millis() - t < timeout);
	  if (ret) break;
	  else DEBUG_SERIAL(tempbuffer);
  }
  if (!ret) Comm_Error += 1;
  if (n==0) DEBUG_SERIAL("[DISPLAY] NOT Response\n");
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
	return Send_data2display(&Begin_BusInfo[0],&End_BusInfo[0]);
}
uint8_t Send_data2display(const char* begin,const char* end)
{
  uint32_t t;
  uint32_t timeout = 2000;
  int n = 0;
  int retry=2;
  int step=0;
  char *p,*p1;
  //
	p = strchr(data_buffer,'[');
	p1 =strchr(data_buffer,']');
  if (p==NULL || p1==NULL)
  {
	  DEBUG_SERIAL("%s Not OK\n",begin);
	 return 0;
  }
  int size = (p1 - p) + 1;
  byte bcc = BCC(p,size);
  DEBUG_SERIAL("Send %s\n",begin);
  while(retry-->0)
  {
	  n=0;
	  step=0;
	  t = millis();
	purgedisplaySerial();
	displaySerial.print(begin);
	
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
		  if (strstr(tempbuffer, begin)) {
		   step =1;
		   break;
		  }
		}
	  } while (millis() - t < timeout);
	  // if (step == 1) break;
	  // else DEBUG_SERIAL(tempbuffer);
  if (step == 1)
  {
	DEBUG_SERIAL("size=%d,BCC=%d\n",size,bcc);
	displaySerial.printf("size=%d,BCC=%d,",size,bcc);
	//p = &data_buffer[0];
	int id=0;	
	while (id<size)
	{
		while (displaySerial.available()) displaySerial.read();
		p = &data_buffer[id]; id +=200;
		if (id>sizeof(data_buffer)-1) id = sizeof(data_buffer)-1;
		
			char c = data_buffer[id];
			data_buffer[id]=0;
			displaySerial.print(p);
			data_buffer[id]=c;
			if (id<size) delay(50);
		
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
	displaySerial.println(end);
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
		  if (strstr(tempbuffer, end)) {
		   step = 3;		   
		   DEBUG_SERIAL("Send %s OK\n",begin);
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
  DEBUG_SERIAL("Step=%d, Send %s Failed\n",step,begin);
  DEBUG_SERIAL("tempbuffer=%s\n",tempbuffer);
  if (retry>0)
  {
	  delay(100);
	  DEBUG_SERIAL("retry Send %s\n",begin);
  }
 }
  //DEBUG_SERIAL("step=%d\n",step);
  return step;
}
uint8_t Send_BusConfig(bool isfrom_buffer)
{
	if (!isfrom_buffer && Get_Config()) return Send_data2display(&Begin_BusConfig[0],&End_BusConfig[0]);
	else if (isfrom_buffer) return Send_data2display(&Begin_BusConfig[0],&End_BusConfig[0]);
	else return 0;
}
uint8_t Send_BusRoute(char *routeNo)
{
	if (Get_Route(routeNo)) return Send_data2display(&Begin_BusRoute[0],&End_BusRoute[0]);
	else return 0;
}
bool Send_DateTime()
{
	uint32_t t = Now();
	sprintf(data_buffer,"[%lu]",t);	
	//DEBUG_SERIAL("%s\n",data_buffer);
	if (Send_data2display(&Begin_SetTime[0],&End_SetTime[0])==3) return true;
	return false;
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
void Get_BusInfor_from_buffer()
{

	char* p,*p1,*p_endline;
	bool bok=false;
	uint8_t disp_index = 0;
	//
	for (int i=0;i<Bus_count;i++)
	{
		Bus[i].visible = false;
		//Bus[i].changed = false; //Set = false if sent ok
	}
	p = strstr(data_buffer,"[{");
	while(p){
		bus_struct bus_temp;
		p1 = p + 1;
		p_endline = strstr(p1,"},");
		if (p_endline == NULL) p_endline = strstr(p1,"}]");
		if (p_endline == NULL) break;
		p = strstr(p1,"matuyen");
		if (p==NULL) break;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.route_no[0],p1,sizeof(bus_temp.route_no)-1);
		*p = '"';
		//
		/* p1 = p + 1;
		p = strstr(p1,"tentuyen");
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.bus_name[0],p1,sizeof(bus_temp.bus_name)-1);
		debugSerial.println(bus_temp.bus_name);
		int name_len = p-p1; */
		//
		p1 = p + 1;
		p = strstr(p1,"biensoxe");
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.car_no[0],p1,sizeof(bus_temp.car_no)-1);
		*p = '"';
		//
		p1 = p + 1;
		p = strstr(p1,"soluonghanhkhach");
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 1;
		p = strchr(p1,',');
		*p = 0;
		memcpy(&bus_temp.passenger[0],p1,sizeof(bus_temp.passenger)-1);
		*p = ',';
		//
		p1 = p + 1;
		p = strstr(p1,"thoigianden"); //"thoigianden":"07:48:00"
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.time_arrival[0],p1,sizeof(bus_temp.time_arrival)-1);
		*p = '"';
		//
		/* p1 = p + 1;
		p = strstr(p1,"Benden"); //"Benden":"Bến chợ Hàn"
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.station_to[0],p1,sizeof(bus_temp.station_to)-1);
		debugSerial.println(bus_temp.station_to);
		//
		p1 = p + 1;
		p = strstr(p1,"Bendi"); //"Bendi":"Bến Kim Liên"
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&bus_temp.station_from[0],p1,sizeof(bus_temp.station_from)-1);
		debugSerial.println(bus_temp.station_from); */
		//
		
		p = p_endline;
		//
		bool exist = false;
		int id = 0;
		for (id=0;id<Bus_count;id++)
		{
			if((Bus[id].route_no[0] == 0) || Compare2array(bus_temp.route_no,Bus[id].route_no))
			{
				exist = true;
				Bus[id].visible = true;
				if(disp_index != Bus[id].display_index) {DEBUG_SERIAL("display_index changed\n");Bus[id].changed = true; break;}
				if(Compare2array(bus_temp.car_no,Bus[id].car_no) == false) {DEBUG_SERIAL("car_no changed\n");Bus[id].changed = true; break;}
				if(Compare2array(bus_temp.time_arrival,Bus[id].time_arrival) == false) {DEBUG_SERIAL("time_arrival changed\n");Bus[id].changed = true; break;}
				break;
			}
		}
		if (!exist)
		{
			id = Bus_count++;
			Bus[id].changed = true;
			Bus[id].visible = true;
			DEBUG_SERIAL("New Bus, Bus_count=%d\n",Bus_count);
		}
		if (Bus[id].changed)
		{
			Bus_sort[disp_index] = id;
			Bus[id].display_index = disp_index;
			memcpy(&Bus[id].route_no[0],bus_temp.route_no,sizeof(Bus[id].route_no)-1);
			memcpy(&Bus[id].car_no[0],bus_temp.car_no,sizeof(Bus[id].car_no)-1);
			memcpy(&Bus[id].time_arrival[0],bus_temp.time_arrival,sizeof(Bus[id].time_arrival)-1);
			memcpy(&Bus[id].passenger[0],bus_temp.passenger,sizeof(Bus[id].passenger)-1);
		}
		DEBUG_SERIAL("Bus %d: index=%d, route_no=%s, car_no=%s, time_arrival=%s, changed=%d, visible=%d\n",id,Bus[id].display_index,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].changed,Bus[id].visible);
		
		disp_index+=1;
		if (Bus_count >= Bus_Max) break;
	};
}
bool Compare2array(char* p1,char* p2)
{
	bool ret = true;
	while (*p1 != NULL)
	{
		if (*p1 != *p2) {ret = false; break;}
		p1++; p2++;
	}
	if (ret && *p2 != NULL) ret = false;
	return ret;
}
