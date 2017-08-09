// compiled path C:\Users\LT-N2K\AppData\Local\Temp
//ChipID
//Cong vien hung vuong: BUS_376F07 //BUS_06D780
//DD Chung cu 4A Chu Huy Man: BUS_06DB02
//Ben xe Bui Duong Lich: BUS_06D780  // BUS_3773DC
#include <memory>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <WiFiUdp.h>
#include <ESP8266httpUpdate.h>

#include "PubSubClient.h"

#include <SPI.h>
#include <Ethernet2.h>

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
unsigned int udpPort = 11000;  // local port to listen on

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
const uint8_t Ethernet_CS_pin=15;

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

//#define WIFI_AP_En true

#define from_Ethernet 0
#define from_WIFI 1
//uint8_t Interface=0;


// #define time_getInfor 15 //seconds
// #define time_checkconfig 300 //seconds


#define Set_EthIP "EthIP=" // EthIP=192.168.1.10
#define Set_Wifi "Wifi="  //Wifi=3G,1234567890
#define Set_Interface "Interface=" //Interface=0 (E) or Interface=1 (W)
#define Set_busStop "BusStop="
#define Set_Default "Default=1"
#define Set_Host "Host=" //Host=abc,Port=123,Infor_arg=akdj,Config_arg=add,Route_arg=xxx
#define Set_Port "Port="
#define Set_Infor_arg "Infor_arg="
#define Set_Config_arg "Config_arg="
#define Set_Route_arg "Route_arg="
//
#define Set_time_getInfor "TgetInfor="
#define Set_time_checkconfig "TcheckConfig="
#define Show_Config "Config?"
#define Show_Status "Status?"
#define Set_Restart "Restart"
#define whois "???"
#define StartUp "StartUp"
#define Begin_BusInfo "BusInfo"
#define End_BusInfo "End_Info"
#define Begin_BusConfig "BusConfig"
#define End_BusConfig "End_Config"
#define Begin_BusRoute "BusRoute"
#define End_BusRoute "End_Route"
#define Begin_BusStop "BusStopInfor"
#define End_BusStop "End_BusStop"
#define CheckRunning "Running?"
#define Running "RUNNING"
#define Idle "IDLE"
#define Begin_SetTime "Set_Time"
#define End_SetTime "End_Time"
#define Set_Brightness "Set_Brightness="
#define Set_Reset_Display "Reset_Display"
//#define End_Brightness "End_Brightness"
#define Set_LostConnection "Set_Connection=0"
#define Set_display "DISPLAY="
//#define End_Connection "End_Connection"
#define CheckSum_Fail "CheckSum_Fail"
#define CheckSize_Fail "CheckSize_Fail"
#define cm_updatefromserver "ServerUpdate"
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

String command_topic = ""; //xem trong setup()
String status_topic = "";
String broadcast_topic = "/BusStop";
String topic = "";
String payload = "";

uint32_t mqtt_timestamp;

String mqttServerName = "m20.cloudmqtt.com";
int    mqttport = 14409;
String mqttuser =  "test";
String mqttpass =  "test";
// String mqttServerName = "n2k.homenet.org";
// int    mqttport = 1883;
// String mqttuser =  "";
// String mqttpass =  "";
//WiFiClient wmqttclient;
EthernetClient emqttclient;
PubSubClient psclient(emqttclient, mqttServerName, mqttport); // for cloud broker - by hostname

uint8_t Comm_Infor=0;
uint8_t Comm_Error=0;
uint8_t Get_Error=0;
uint8_t Reset_Ethernet_Count=0;
//

// cho Ben xe Bui Duong Lich
// String danhsachxe[12] = {"43B-03508","43B-03568","43B-03587","43A-00355"};
// const int gioxuatben[] = {545,615,635,645,715,735,755,815,845,915,945,1015,1030,1115,1215,1315,1330,1400,1430,1500,1515,1545,1615,1635,1655,1715,1735,1755,1815,1830,1850,1930};
// const char* config_bdl = "[{\"c1\":\"2\",\"c2\":\"3\",\"c3\":\"1\",\"c4\":\"2\",\"c5\":\"3\",\"c6\":\"1\",\"c7\":\"2\",\"c8\":\"3\",\"c9\":\"1\",\"c10\":\"2\",\"c11\":\"3\",\"c12\":\"1\",\"c13\":\"2\",\"c14\":\"3\",\"c15\":\"1\",\"c16\":\"2\",\"s1\":\"%tentram%chaychu\",\"s2\":\"1498582141\",\"s3\":\"Tuyến\",\"s4\":\"%kieu1%chaychu\",\"s5\":\"Giờ đi \",\"s6\":\"\",\"s7\":\"\",\"s8\":\"\",\"s9\":\"\",\"s10\":\"\",\"s11\":\"\",\"s12\":\"\",\"s13\":\"\",\"s14\":\"\",\"s15\":\"\",\"s16\":\"\",\"ConfigTime\":1234}]";
// const char* route_TMF1 = "[{\"busRouteId\":\"2101\",\"routeNo\":\"TMF1\",\"routeName\":\"Bùi Dương Lịch - Xuân Diệu - CV 29-3\",\"distance\":\"19\",\"tripsPerDay\":\"20\",\"firstDepartureTime\":\"06:00 AM\",\"lastDepartureTime\":\"08:45 PM\",\"frequency\":\"30\",\"color\":\"1f3ff2\",\"bendi\":\"Bến xe Bùi Dương Lịch\",\"benden\":\"Công viên 29/3\",\"gia\":\"0\",\"Luotdi\":\"Bùi Dương Lịch – Dương Vân Nga – Khúc Hạo – Hồ Hán Thương – Chu Huy Mân – Cầu Thuận Phước – Đường 3/2 – Bãi đỗ xe Xuân Diệu – Đường 3/2 – Trần Phú – Lý Tự Trọng – Nguyễn Thị Minh Khai – Lê Duẩn – Ngã ba Cai Lang\",\"Luotve\":\"Ngã ba Cai Lang – Lý Thái Tổ - Hùng Vương – Chi lăng – Lê Duẩn – Phan Châu Trinh – Hùng Vương – Bạch Đằng – Đường 3/2 - Bãi đỗ xe Xuân Diệu – Đường 3/2 - Cầu Thuận Phước – Chu Huy Mân – Hồ Hán Thương – Khúc Hạo – Dương Vân Nga – Bùi Dương Lịch\"}]";
int last_index =-1;
//
char* chipID = "BUS_xxxxxxxx"; //xx = Chip ID
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
//
const char* password_ap = "13245768";

byte Update_from = 0;
const char *host_update = "n2k16.esy.es";
const char *firmware_server_name ="http://n2k16.esy.es/bus/";
const char *check4update = "check4update.php";
const char *folder_update = "/bus/";

char *sketch_name ="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.ino";//
char sketch_time[14];

byte stat=0;
byte LED=13;

char busStopName[200];

char tempbuffer[160];
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
unsigned long lastSent_timeStamp;
unsigned long lastCheckConfig_timeStamp;

bool PW_On=false;
bool AP_On=false;
bool Checkupdate=false;
bool isUpdate=false;
float voltage;
bool timer0_en;
bool firstScan;
bool RF_Disable;
bool bGet_condition,bGet_config;
bool bDateTime_OK = false;
bool send_report;
bool all_no_change;

uint32_t unixTime_sent;
uint32_t Unixtime_GMT7;
uint32_t time_bk;
uint32_t last_infor_OK_ts;
bool bLostconnection;
uint32_t last_check_update;
uint32_t last_Get_DateTime_ts;
uint32_t setup_wifi_timestamp,setup_eth_timestamp;
uint32_t auto_sleep_ts;
uint32_t lastGet_Datetime;
uint32_t unixTime_report;

uint8_t display_state;
enum
{
	isNot_response,
	isStartUp,
	isIdle,
	isRunning,
};
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
  char departure_arg[100];
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
	bool isOutbound;
}bus_struct;
typedef struct {
	char car_no[12];
	uint16_t Time;
}departure_struct;
#define departure_Max 3
typedef struct {
	char route_no[12];
	bool route_sent;
	departure_struct departure[departure_Max];
	uint8_t car_count;
}route_struct;
bool BusStopName_sent = false;
bool Config_sent = false;
bool bisNew_Config = false;
uint32_t last_ConfigTime = 0;

#define Bus_Max 10
int Bus_count;
bus_struct Bus[Bus_Max];
//byte Bus_sort[Bus_Max];
route_struct Route[Bus_Max];

void Read_Config(bool bdefault = false);
uint8_t Send_BusConfig(bool isfrom_buffer = false);
void printConfig(bool ismqttPub = false);
void Process_Com_buffer(bool isfrommqtt = false);

void SketchInfor2tempbuffer()
{
	IPAddress ip = WiFi.localIP();
	if (WiFi.status() != WL_CONNECTED) ip = WiFi.softAPIP();
	sprintf(tempbuffer,"chipID=%s\nSketch=%s\nCompiled on=%s %s\nWiFi IP: %d.%d.%d.%d\n", chipID,String(sketch_name).c_str(),__DATE__,__TIME__,ip[0],ip[1],ip[2],ip[3]);	
}
void printConfig(bool ismqttPub)
{
	sprintf(tempbuffer,"host=%s\n",EEData.host);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"port=%d\n",EEData.port);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"infor_arg=%s\n",EEData.infor_arg);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"config_arg=%s\n",EEData.config_arg);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"route_arg=%s\n",EEData.route_arg);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"departure_arg=%s\n",EEData.departure_arg);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"BusStopNo=%s\n",EEData.BusStopNo);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);	
	sprintf(tempbuffer,"EthernetIP=%d.%d.%d.%d\n",EEData.ethIP[0],EEData.ethIP[1],EEData.ethIP[2],EEData.ethIP[3]);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"%s%s\n",Set_Interface,EEData.Interface == from_Ethernet ? "Ethernet" : "WIFI");
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"%s%s,%s\n",Set_Wifi,EEData.WiFi_Name,EEData.WiFi_Pass);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"time_getInfor=%ds\n",EEData.time_getInfor);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"time_checkconfig=%ds\n",EEData.time_checkconfig);
    if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
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
		//http://bus.danang.gov.vn:1022/bus-services-api/BusStation?StationId=2907
		//http://bus.danang.gov.vn:1022/bus-services-api/BusStation?StationId=2907&Route=
		//http://bus.danang.gov.vn:1022/bus-services-api/BusStation?Config=1
		DEBUG_SERIAL("Set to default Config\n");
		EEData.ethIP = {0,0,0,0}; //DHCP
		EEData.Interface=from_Ethernet;
		EEData.time_getInfor = 20; //s
		EEData.time_checkconfig = 300; //5M
		memcpy(&EEData.WiFi_Name[0],"3G\0",sizeof(EEData.WiFi_Name));
		memcpy(&EEData.WiFi_Pass[0],"1234567890\0",sizeof(EEData.WiFi_Pass));
		memcpy(&EEData.host[0],"bus.danang.gov.vn\0",sizeof(EEData.host));
		memcpy(&EEData.infor_arg[0],"/bus-services-api/BusStation?StationId=\0",sizeof(EEData.infor_arg));
		memcpy(&EEData.config_arg[0],"/bus-services-api/BusStation?Config=1\0",sizeof(EEData.config_arg));
		memcpy(&EEData.route_arg[0],"/bus-services-api/getBusRoute?content=\0",sizeof(EEData.route_arg));
		memcpy(&EEData.departure_arg[0],"&Route=\0",sizeof(EEData.departure_arg));
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
	setup_wifi_timestamp = millis();
	if (EEData.WiFi_Name[0] != 0)
	{
		WiFi.disconnect();
	 WiFi.mode(WIFI_AP_STA);
	 int count =20;
	  //
	  WiFi.begin(EEData.WiFi_Name, EEData.WiFi_Pass);
	  WiFi.softAP(chipID, password_ap,1,1); // ssid, password, channel, hide
	 count =20;
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		DEBUG_SERIAL(".");
		if (count--<=0) break;
		wdt_reset();
	  }
	  if (WiFi.status() != WL_CONNECTED) WiFi.softAP(chipID, password_ap,2,0);
	}
	if (EEData.WiFi_Name[0] == 0 || WiFi.status() != WL_CONNECTED)
	{
		WiFi.mode(WIFI_STA);
		WiFi.softAP(chipID, password_ap,2,0);
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
#ifdef wClient
	wServer.begin();
	wServer.setNoDelay(true);
#endif
	 }
void StartEthernet()
{
	setup_eth_timestamp = millis();
	if (Reset_Ethernet_Count++>=3) 
	{
		DEBUG_SERIAL("Reset_Ethernet_Count=%d ->return\n",Reset_Ethernet_Count);
		if (Reset_Ethernet_Count>13) Reset_Ethernet_Count=0;
		return;
	}
	digitalWrite(Ethernet_RESET_PIN,LOW);
	delay(500);
	digitalWrite(Ethernet_RESET_PIN,HIGH);
	delay(500);
	Ethernet.init(Ethernet_CS_pin);
	// start the Ethernet connection:
	if (EEData.ethIP[0]==0)
	{
		DEBUG_SERIAL("Configure Ethernet using DHCP ... ");
	  if (Ethernet.begin(mac) == 0) {
		DEBUG_SERIAL("Failed\n");
		//set static ip
		IPAddress ipstatic(192,168,1,177);
		IPAddress gateway = ipstatic;
		gateway[3] = 1;
		IPAddress subnet(255, 255, 255, 0);
		DEBUG_SERIAL("gateway=%d.%d.%d.%d\n",gateway[0],gateway[1],gateway[2],gateway[3]);  
		Ethernet.begin(mac, ipstatic,gateway,subnet);
	  }
	  else
	  {
		  DEBUG_SERIAL("success\n");
		  Reset_Ethernet_Count = 0;
	  }
	}
	else
	{
	  DEBUG_SERIAL("Configure Ethernet static IP\n");
	  // start the Ethernet connection and the server:
	  IPAddress gateway = EEData.ethIP;
	  gateway[3] = 1;
	  IPAddress subnet(255, 255, 255, 0);
	  DEBUG_SERIAL("gateway=%d.%d.%d.%d\n",gateway[0],gateway[1],gateway[2],gateway[3]);
	  Ethernet.begin(mac, EEData.ethIP,gateway,subnet);
	  Ethernet.begin(mac, EEData.ethIP);
	}
	IPAddress ip = Ethernet.localIP();
   DEBUG_SERIAL("Ethernet IP: %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);

}
void printStatus(bool ismqttPub)
{
	IPAddress ip = WiFi.localIP();
	if (WiFi.status() != WL_CONNECTED) ip = WiFi.softAPIP();
	sprintf(tempbuffer,"WiFi IP: %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
	if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	ip = Ethernet.localIP();
	sprintf(tempbuffer,"Ethernet IP: %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
	if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
	sprintf(tempbuffer,"Bus_count=%d\n",Bus_count);
	if (!ismqttPub) {DEBUG_SERIAL(tempbuffer);} else pubStatus(tempbuffer);
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
  
  command_topic = "/BusStop/" + String(chipID) + "/control";
  status_topic = "/BusStop/" + String(chipID) + "/status";

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
  StartEthernet();
  SetupWifi();
  //
  SetupTempsensor();
  firstScan=true;
  //
  delay(3000);
  Startup_timestamp = millis();
 Reset_display();
}
//******************************************************************************************************

void loop() {
	if (bDateTime_OK)
	{
		if (rtc.hour >= 22 || rtc.hour < 5) if (display_state == isRunning) Set_DisplayState(0);
		if (rtc.hour ==5 && rtc.minute < 5) if (display_state == isIdle) ESP.restart();//Set_DisplayState(1);
		//tu dong tat hien thi neu ko co xe nao sap xuat ben
		if (Bus_count == 0 && display_state == isRunning && rtc.hour >= 20)
		{
			if (auto_sleep_ts==0) auto_sleep_ts = millis();
			if (millis() - auto_sleep_ts > 300000) Set_DisplayState(0);
		}
		else if (Bus_count>0 && auto_sleep_ts>0) auto_sleep_ts = 0;
	}
	if (firstScan)
	{
		Check_display_Running();
		if (display_state == isNot_response) Reset_display();
	}
	wdt_reset();
	httpServer.handleClient();
	
	CheckSerial();
#ifdef wClient
	Wifi_Server();
#endif
	//
	bGet_condition=false; 
  uint32_t time = Now();
  //auto check4update
  if (bDateTime_OK && rtc.minute == 0 && rtc.second<=2) 
  {
	  if (millis() - last_check_update > 5000) 
	  {
		  Update_from = EEData.Interface;
		  Update_Firmware_fromServer();
	  }
  }
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
	  if (display_state == isStartUp) bGet_condition = true;
	  //CheckWifi_connection();  khong co wifi
	  Check_Ethernet_connection();
  }
  if ((millis()-lastGet_timeStamp)>(EEData.time_getInfor*1000) || firstScan) 
  {
	   if (display_state == isRunning || bDateTime_OK == false || (millis()-lastGet_timeStamp > 3600000)) //1h
	   {
		   bGet_condition = true;
		//DEBUG_SERIAL("bGet_condition=1\n");
	   }
  }
  
  if (bGet_condition)
 {
	 int retry=3;
	 Comm_Infor=0;
		while (Comm_Infor == 0) 
		{
			if (bDateTime_OK && EEData.BusStopNo[1] == 0 && EEData.BusStopNo[0] == '0')
			  {
				 DEBUG_SERIAL("Not yet config BusStop=%s\n",EEData.BusStopNo);
				 break;
			  }
			if (Get_Infor()) {last_infor_OK_ts = millis(); bLostconnection =false; break;}
			if (--retry<=0) break;
			delay(1000);
		}
		if (Get_Error>=3 && EEData.Interface == from_Ethernet)
		 {
			 Get_Error=0;
			 DEBUG_SERIAL("Reset Ethernet Shield\n");
			 StartEthernet();
		 }	 
		 else if (Get_Error>=3) Get_Error=0;
	 //Send_DateTime();
	 while (Comm_Infor == isBusInfo) {
		 Get_Route_of_Station();
		 if (Get_departure()) Fill_BusInfor();
		 //Get_BusInfor_from_buffer();
		 //
		 if (BusStopName_sent == false)
		 {
			 if (Send_BusStopName()==3) BusStopName_sent = true;
		 }
		 //Send config
		 if (!Config_sent)
		 {
			if (Send_BusStopName()==3)
			{
				BusStopName_sent = true;
				if (Send_BusConfig() == 3) 
				{
					Config_sent = true;
					bisNew_Config = false;
					lastCheckConfig_timeStamp = millis();
				}
			}
		 }
		 if (!Config_sent) break; //loi ko gui dc
		 //Send Route Infomation
		 int i=0;
		 for (i=0;i<Bus_Max;i++)
		 {
			if (Route[i].route_no[0]==0) break;
			if (!Route[i].route_sent)
			{
				if (Send_BusRoute(&Route[i].route_no[0]) == 3) Route[i].route_sent = true;
				else break;
			}
		 }
		 if (Route[i].route_no[0] > 0) break; //loi ko gui dc
		 //Send_BusInfor();
		 all_no_change=true;
		 String s="[";
		 for (int i=0;i<Bus_count;i++)
		 {
			 //int i = Bus_sort[k];
			//if (Bus[i].changed || 
			//if(Bus[i].visible)
			{
				s += "{";
				// char *p2 = &Bus[i].route_no[0];
				// DEBUG_SERIAL("Bus %d: ",i);
				// while (*p2>0) DEBUG_SERIAL("%2X ",*p2++);
				// DEBUG_SERIAL("\n");
				//sprintf(data_buffer,"[index=%d,route_no=%s,car_no=%s,time=%s,passenger=%s,visible=%d,]\0",Bus[i].display_index,Bus[i].route_no,Bus[i].car_no,Bus[i].time_arrival,Bus[i].passenger,Bus[i].visible);
				s += "id=" + String(Bus[i].display_index);
				s += ",route_no=" + String(Bus[i].route_no);
				if (Bus[i].changed == false) s += ",no_change";
				else{
				s += ",car_no=" + String(Bus[i].car_no);
				s += ",time=" + String(Bus[i].time_arrival);
				s += ",passenger=" + String(Bus[i].passenger);
				if (Bus[i].isOutbound) s += ",outbound";
				//s += ",visible=" + String(Bus[i].visible);
				all_no_change = false;
				}
				s += ",}";
				//DEBUG_SERIAL("%s\n",data_buffer);
				//Send_data2display(Begin_BusInfo,End_BusInfo);
				
			}
		 }
		s += "]";
		s.toCharArray(data_buffer,sizeof(data_buffer)-1);
		DEBUG_SERIAL("BusInfo=%s\n",data_buffer);
		if (Bus_count==0) all_no_change = false;
		if (all_no_change == false || (millis()-lastSent_timeStamp)>=60000) //1M
		{
			if (Send_data2display(Begin_BusInfo,End_BusInfo))
			{
				lastSent_timeStamp = millis();
				for (int k=0;k<Bus_count;k++) Bus[k].changed = false;
			}
			send_report = true;
		}
		Comm_Infor=0;
	 }
	 if (Comm_Infor>0) Comm_Infor=0;
	 lastGet_timeStamp = millis();
	  if (send_report) //60s
	 {
		 unixTime_report = time;
		 if (Report(&data_buffer[0])) sprintf(tempbuffer,"Send Report Done");
		 else sprintf(tempbuffer,"Send Report Fail");
		 pubStatus(tempbuffer);
		 send_report = false;
	 }
}
// check connection
if (bLostconnection == false && millis() - last_infor_OK_ts > 120000) //2M
{
	bLostconnection = true;
	//displaySerial.println(Set_LostConnection);
}
//check config
if ((display_state == isRunning) && (bisNew_Config || (millis()-lastCheckConfig_timeStamp) >(EEData.time_checkconfig*1000)))
  {
	if (BusStopName_sent)
	{
		Get_Config();
		if (bisNew_Config) 
		{
			Send_BusConfig(true); //from buffer
		}
		lastCheckConfig_timeStamp = millis();
	}
  }
 DebugFromDisplay();
 //
 Check_udp_packet();
 //
 firstScan=false;
 
 Process_MQTT();
}
void Check_Ethernet_connection()
{	
	if(millis() - setup_eth_timestamp > 43200000) StartEthernet();
}
void CheckWifi_connection()
{
	if (EEData.WiFi_Name[0] == 0) return;
	if (WiFi.status() != WL_CONNECTED)
	{
		if(millis() - setup_wifi_timestamp > 1800000) SetupWifi(); //30 phut = 30*60000
	}
	else if(millis() - setup_wifi_timestamp > 43200000) SetupWifi(); //12h
}
void Set_DisplayState(int state)
{
	displaySerial.print(Set_display);
	displaySerial.println(state);
}
void DebugFromDisplay()
{
	int n = 0;
	tempbuffer[n] = 0;
	data_buffer[0] =0;
	if (displaySerial.available())
	{
		n=displaySerial.readBytesUntil('\n', tempbuffer, sizeof(tempbuffer));	
		tempbuffer[n++] = '\n';
		tempbuffer[n] = 0;
		sprintf(data_buffer,"[D] %s",tempbuffer);
	}
	if (n>2) DEBUG_SERIAL(data_buffer);
	if (n>5) pubStatus(data_buffer);
	data_buffer[0] =0;
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
		last_Get_DateTime_ts = millis();
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
	DEBUG_SERIAL(com_buffer);
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
	DEBUG_SERIAL("wificlient connected\n");
    }
	if (wificlient && wificlient.available())
	{
		com_buffer_len = wificlient.available();
		wificlient.readBytes(com_buffer,com_buffer_len);
		com_buffer[com_buffer_len] =0;
		DEBUG_SERIAL("%s\n",com_buffer);
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
      tempbuffer[len] = 0;
	  wdt_reset();
    }
    DEBUG_SERIAL("%s\n",String(tempbuffer).c_str());
		//Send to back udp server	 
	if (strstr(tempbuffer,"???") != NULL)
	{
		SketchInfor2tempbuffer();
		udp.beginPacket(remoteIp, udp.remotePort());
		udp.print(tempbuffer);
		udp.endPacket();
	}
  }
}
void Process_Com_buffer(bool isfrommqtt)
{
	char* p,*p1;
	//loai \r\n neu co
	p = strchr(com_buffer,'\r');
	if (p) *p = 0;
	p = strchr(com_buffer,'\n');
	tempbuffer[0] = 0;
	if (p) *p = 0;
		if (strstr(com_buffer,Set_busStop))
		{
			p = strstr(com_buffer,Set_busStop) + sizeof(Set_busStop) - 1;
			p1 = strchr(p,'\r');
			if (p1) *p1 = 0;
			memcpy(EEData.BusStopNo,p,sizeof(EEData.BusStopNo));
			lastGet_timeStamp = 0;
			sprintf(tempbuffer,"%s%s\n",Set_busStop,EEData.BusStopNo);
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
			//
			Reset_display();
			busStopName[0] = 0;
		}
		else if (strstr(com_buffer, Set_Interface))
		{
			p = strstr(com_buffer, Set_Interface) + sizeof(Set_Interface) - 1;
			if (*p == 'E')
			{
				EEData.Interface = from_Ethernet;
			}
			else if(*p == 'W')
			{
				EEData.Interface = from_WIFI;
			}
			else EEData.Interface = atoi(p);
			lastGet_timeStamp = 0;
			sprintf(tempbuffer,"%s%s\n",Set_Interface,EEData.Interface == from_Ethernet ? "Ethernet" : "WIFI");
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
		}
		else if (strstr(com_buffer,Set_Wifi))
		{
			//Wifi=3G,1234567890
			p = strstr(com_buffer,Set_Wifi) + sizeof(Set_Wifi) - 1;
			p1 = strchr(p,',');
			if (p1==NULL) return;
			*p1 = 0;
			if (p==p1) 
			{
				EEData.WiFi_Name[0] = 0;
				//DEBUG_SERIAL("NULL");
			}
			else memcpy(EEData.WiFi_Name,p,sizeof(EEData.WiFi_Name));
			p = p1 + 1;
			p1 = strchr(p,'\r');
			if (p1) *p1 = 0;
			memcpy(EEData.WiFi_Pass,p,sizeof(EEData.WiFi_Pass));
			sprintf(tempbuffer,"%s%s,%s\n",Set_Wifi,EEData.WiFi_Name,EEData.WiFi_Pass);
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
			SetupWifi();
		}
		else if (strstr(com_buffer, Set_EthIP))
		{
			//EthIP=192.168.1.10
			p = strstr(com_buffer, Set_EthIP) + sizeof(Set_EthIP) - 1;
			IPAddress ip;
			ip[0] = atoi(p);
			if (ip[0] > 0)
			{
				p1 = strchr(p,'.');
				if (p1==NULL) return; p=p1 + 1;
				ip[1] = atoi(p);
				p1 = strchr(p,'.');
				if (p1==NULL) return; p=p1 + 1;
				ip[2] = atoi(p);
				p1 = strchr(p,'.');
				if (p1==NULL) return; p=p1 + 1;
				ip[3] = atoi(p);				
			}
			EEData.ethIP = ip;			
			sprintf(tempbuffer,"%s%d.%d.%d.%d\n",Set_EthIP,EEData.ethIP[0],EEData.ethIP[1],EEData.ethIP[2],EEData.ethIP[3]);
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
			StartEthernet();
		}
		else if (strstr(com_buffer,Set_Host))
		{
			do
			{
				bool next_available = false;
				//
				p = strstr(com_buffer,Set_Host) + sizeof(Set_Host) - 1;
				p1 = strchr(p,',');
				next_available = false;
				if (p1) {*p1 = 0; next_available = true;}
				memcpy(EEData.host,p,sizeof(EEData.host));
				if (next_available) *p1 = ','; else break;
				p = p1 + 1;
				p = strstr(com_buffer,Set_Port);
				if (p==NULL) break;
				p+= sizeof(Set_Port) - 1;
				EEData.port = atoi(p);
				p = strstr(com_buffer,Set_Infor_arg);
				if (p==NULL) break;
				p += sizeof(Set_Infor_arg) - 1;
				p1 = strchr(p,',');
				next_available = false;
				if (p1) {*p1 = 0; next_available = true;}
				memcpy(EEData.infor_arg,p,sizeof(EEData.infor_arg));
				if (next_available) *p1 = ','; else break;
				p = strstr(com_buffer,Set_Config_arg);
				if (p==NULL) break;
				p += sizeof(Set_Config_arg) - 1;
				p1 = strchr(p,',');
				next_available = false;
				if (p1) {*p1 = 0; next_available = true;}
				memcpy(EEData.config_arg,p,sizeof(EEData.config_arg));
				if (next_available) *p1 = ','; else break;
				p = strstr(com_buffer,Set_Route_arg);
				if (p==NULL) break;
				p += sizeof(Set_Route_arg) - 1;
				p1 = strchr(p,',');
				next_available = false;
				if (p1) {*p1 = 0; next_available = true;}
				memcpy(EEData.route_arg,p,sizeof(EEData.route_arg));
				break;			
			}while (true);
			Save_Config();
			printConfig();
			if (isfrommqtt) printConfig(true);
			tempbuffer[0] = 0;
		}
		else if (strstr(com_buffer, Set_time_getInfor))
		{
			p = strstr(com_buffer, Set_time_getInfor) + sizeof(Set_time_getInfor) - 1;
			EEData.time_getInfor = atoi(p);
			sprintf(tempbuffer,"%s%ds\n",Set_time_getInfor,EEData.time_getInfor);
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
		}
		else if (strstr(com_buffer, Set_time_checkconfig))
		{
			p = strstr(com_buffer, Set_time_checkconfig) + sizeof(Set_time_checkconfig) - 1;
			EEData.time_checkconfig = atoi(p);
			sprintf(tempbuffer,"%s%ds\n",Set_time_checkconfig,EEData.time_checkconfig);
			DEBUG_SERIAL(tempbuffer);
			Save_Config();
		}
		else if (strstr(com_buffer, Set_Default))
		{			
			Read_Config(true);
			if (isfrommqtt) printConfig(true);
			tempbuffer[0] = 0;
		}
		else if (strstr(com_buffer, Show_Config))
		{
			printConfig(isfrommqtt);
			tempbuffer[0] = 0;
		}
		else if (strstr(com_buffer, Show_Status))
		{
			printStatus(isfrommqtt);
			tempbuffer[0] = 0;
		}
		else if (strstr(com_buffer, cm_updatefromserver))
		{			
			//=1
			p = strstr(com_buffer, cm_updatefromserver) + sizeof(cm_updatefromserver);
			if (*p>0)
			{
				Update_from = atoi(p);				
			}
			else Update_from = EEData.Interface;
			DEBUG_SERIAL("\nCheck Update from %s\n",Update_from == from_Ethernet ? "Ethernet" : "WIFI");
			Update_Firmware_fromServer();
			tempbuffer[0] = 0;
		}
		else if (strstr(com_buffer,"???") != NULL)
		{
			SketchInfor2tempbuffer();
			DEBUG_SERIAL(tempbuffer);
		}
		else if (strstr(com_buffer,Set_Reset_Display) != NULL)
		{
			Reset_display();			
		}
		else if (strstr(com_buffer,Set_Restart) != NULL)
		{
			sprintf(tempbuffer,"Restarting\n");
			DEBUG_SERIAL(tempbuffer);
			pubStatus(tempbuffer);
			delay(500);
			ESP.restart();
		}
		else
		{
			displaySerial.write(com_buffer,com_buffer_len);
			tempbuffer[0] = 0;
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
		//
		memset(Route[i].route_no,0,sizeof(Route[i].route_no)-1);
		for (int k=0;k<departure_Max;k++)
		{
			Route[i].departure[k].Time = 0;
			Route[i].departure[k].car_no[0] = 0;
		}
		Route[i].route_sent = false;
	}
	Bus_count = 0;
	unixTime_sent = 0;
	Config_sent = false;
	last_ConfigTime = 0;
	lastGet_timeStamp = 0;
	BusStopName_sent = false;
	Reset_Ethernet_Count = 0;
	lastCheckConfig_timeStamp = millis();
	last_index = -1;
}
bool Get_Config()
{
	bool b = false;
	bisNew_Config = false;
	Comm_Infor = 0;
	// if (millis() - last_Get_DateTime_ts > 3600000)
	// {
		// if (WiFi.status() == WL_CONNECTED) GET_Config_from_Wifi();
		// else GET_Config_from_Ethernet();
	// }
	if (EEData.Interface == from_Ethernet) b= GET_Config_from_Ethernet();
	else if (EEData.Interface == from_WIFI) b= GET_Config_from_Wifi();
	//memcpy(data_buffer,config_bdl,sizeof(data_buffer)-1);
	//b = true;
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
	Comm_Infor = 0;
	// if (!bDateTime_OK)
	// {
		// if (WiFi.status() == WL_CONNECTED) GET_Infor_from_Wifi();
		// else GET_Infor_from_Ethernet();
	// }
	if (EEData.Interface == from_Ethernet) return GET_Infor_from_Ethernet();
	else if (EEData.Interface == from_WIFI) return GET_Infor_from_Wifi();
	else return false;
	/* int now_hmm = rtc.hour * 100 + rtc.minute;
	DEBUG_SERIAL("now_hmm=%d\n",now_hmm);
	int len = sizeof(gioxuatben)/sizeof(gioxuatben[0]);
	int i = 0;
	int id = 0;
	for (i=0;i<len;i++)
	{
		if (now_hmm < gioxuatben[i]) break;
	}
	if (last_index != i)
	{
		DEBUG_SERIAL("index=%d\n",i);
		last_index = i;
		Bus_count = 0;
		int count = len - i;
		if (count>=1)
		{
			Bus[Bus_count].display_index = Bus_count;
			memcpy(&Bus[Bus_count].route_no[0],"TMF1",sizeof(Bus[0].route_no)-1);
			danhsachxe[i%4].toCharArray(Bus[Bus_count].car_no,sizeof(Bus[0].car_no)-1);
			sprintf(Bus[Bus_count].time_arrival,"%02d:%02d",gioxuatben[i]/100,gioxuatben[i]%100);
			memcpy(&Bus[Bus_count].passenger[0],"0",sizeof(Bus[0].passenger)-1);
			
			Bus[Bus_count].changed = true;
			Bus[Bus_count].visible = true;	
			id = Bus_count;
			DEBUG_SERIAL("index=%d, route_no=%s, car_no=%s, time_arrival=%s, changed=%d, visible=%d\n",id,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].changed,Bus[id].visible);
			i+=1;
			Bus_count += 1;
			memcpy(&Route[0].route_no[0],"TMF1",sizeof(Route[0].route_no)-1);
		}
		if (count>=2)
		{
			Bus[Bus_count].display_index = Bus_count;
			memcpy(&Bus[Bus_count].route_no[0],"TMF1",sizeof(Bus[0].route_no)-1);
			danhsachxe[i%4].toCharArray(Bus[Bus_count].car_no,sizeof(Bus[0].car_no)-1);
			sprintf(Bus[Bus_count].time_arrival,"%02d:%02d",gioxuatben[i]/100,gioxuatben[i]%100);
			memcpy(&Bus[Bus_count].passenger[0],"0",sizeof(Bus[0].passenger)-1);
			Bus[Bus_count].changed = true;
			Bus[Bus_count].visible = true;
			id = Bus_count;
			DEBUG_SERIAL("index=%d, route_no=%s, car_no=%s, time_arrival=%s, changed=%d, visible=%d\n",id,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].changed,Bus[id].visible);
			i+=1;
			Bus_count += 1;
		}
		if (count>=3)
		{
			Bus[Bus_count].display_index = Bus_count;
			memcpy(&Bus[Bus_count].route_no[0],"TMF1",sizeof(Bus[0].route_no)-1);
			danhsachxe[i%4].toCharArray(Bus[Bus_count].car_no,sizeof(Bus[0].car_no)-1);
			sprintf(Bus[Bus_count].time_arrival,"%02d:%02d",gioxuatben[i]/100,gioxuatben[i]%100);
			memcpy(&Bus[Bus_count].passenger[0],"0",sizeof(Bus[0].passenger)-1);
			
			Bus[Bus_count].changed = true;
			Bus[Bus_count].visible = true;
			id = Bus_count;
			DEBUG_SERIAL("index=%d, route_no=%s, car_no=%s, time_arrival=%s, changed=%d, visible=%d\n",id,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].changed,Bus[id].visible);
			i+=1;
			Bus_count += 1;
		}
		DEBUG_SERIAL("Bus_count=%d\n",Bus_count);
	}
	return true; */
}
bool Get_Route(char *routeNo)
{	
	Comm_Infor = 0;
	if (EEData.Interface == from_Ethernet) return GET_Route_from_Ethernet(routeNo);
	else if (EEData.Interface == from_WIFI) return GET_Route_from_Wifi(routeNo);
	else return false;
}
bool Get_departure()
{
	int id = 0;
	bool bok = false;
	bool bupdate = false;
	int now_hmm = rtc.hour * 100 + rtc.minute;
	while (Route[id].route_no[0]>0)
	{
		wdt_reset();
		if (Route[id].departure[0].Time>=now_hmm)
		{
			id += 1;
			continue;
		}
		if (EEData.Interface == from_Ethernet) bok = GET_departure_from_Ethernet(Route[id].route_no);
		else if (EEData.Interface == from_WIFI) bok = GET_departure_from_Wifi(Route[id].route_no);
		if (!bok) break;
		Get_departure_from_buffer(id);
		id += 1;
		bupdate = true;
	}
	return bupdate;
}
bool GET_Config_from_Ethernet()
{
	data_buffer[0] = 0;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
	  wdt_reset();
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
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
	  wdt_reset();
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
	int rescode = handleHeaderResponse(Eclient,7000);
	if (rescode == 200)
	{
		if (e_bytesRecv>0)
		{
		Eclient.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusInfo;
		//Send_BusInfor();
		}
		else
		{
			sprintf(tempbuffer,"Conneced but No data response\n");
			DEBUG_SERIAL(tempbuffer);
			pubStatus(tempbuffer);
		}
	}
	else 
	{
		sprintf(tempbuffer,"ReturnCode=%d\n",rescode);
		DEBUG_SERIAL(tempbuffer);
		pubStatus(tempbuffer);
		e_bytesRecv = 0;
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
	  wdt_reset();
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
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
	Get_Error +=1;
  }
  Eclient.stop();
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_departure_from_Ethernet(char* routeNo)
{
	data_buffer[0] = 0;
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
  if (Eclient.connect(EEData.host, EEData.port)) {
	  wdt_reset();
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:
    //Eclient.println("GET /bus-services-api/BusStation?StationId=2907&Route= HTTP/1.1");
	DEBUG_SERIAL("GET %s%s%s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo,EEData.departure_arg,routeNo);
	Eclient.printf("GET %s%s%s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo,EEData.departure_arg,routeNo);
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
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
    //DEBUG_SERIAL("connection failed\n");
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
	int rescode = handleHeaderResponse(client1,7000);
	if (rescode == 200)
	{
		if (e_bytesRecv>0)
		{
		client1.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		debugSerial.println(data_buffer);
		Comm_Infor = isBusInfo;
		//Send_BusInfor();
		}
		else
		{
			sprintf(tempbuffer,"Conneced but No data response\n");
			DEBUG_SERIAL(tempbuffer);
			pubStatus(tempbuffer);
		}
	}
	else 
	{
		sprintf(tempbuffer,"ReturnCode=%d\n",rescode);
		DEBUG_SERIAL(tempbuffer);
		pubStatus(tempbuffer);
		e_bytesRecv = 0;
	}
	if (header_time != "")
	{
		//DEBUG_SERIAL("%s\n",header_time.c_str());
		GetTime_fromHeader();
	}
  }
  else {
    DEBUG_SERIAL("connection failed\n");
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
	Get_Error +=1;
  }
  client1.stop();
  if (e_bytesRecv>0) return true;
  else return false;
}
bool GET_departure_from_Wifi(char *routeNo)
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
    //client1.println("GET /bus-services-api/BusStation?StationId=2907&Route= HTTP/1.1");
	DEBUG_SERIAL("GET %s%s%s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo,EEData.departure_arg,routeNo);
	client1.printf("GET %s%s%s%s HTTP/1.1\n",EEData.infor_arg,EEData.BusStopNo,EEData.departure_arg,routeNo);
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
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
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
                DEBUG_SERIAL("[Response] code: %d\n", _returnCode);

                if(_size > 0) {
                    DEBUG_SERIAL("[Response] size: %d\n", _size);
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
	
	if (bDateTime_OK && (millis() - lastGet_Datetime < 600000)) return bDateTime_OK;
	
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
		  lastGet_Datetime = millis();
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
		   DEBUG_SERIAL("[DISPLAY] %s\n",Running);
		   pubStatus_addtime(tempbuffer);
		   ret = true;
		   if (display_state == isIdle) Reset_Bus();
		   display_state = isRunning;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, Idle)) {
		   DEBUG_SERIAL("[DISPLAY] %s\n",Idle);
		   pubStatus_addtime(tempbuffer);
		   ret = true;
		   display_state = isIdle;
		   Comm_Error = 0;
		   break;
		  }
		  if (strstr(tempbuffer, StartUp)) {
		   DEBUG_SERIAL("[DISPLAY] %s\n", StartUp);
		   pubStatus_addtime(tempbuffer);
		   display_state = isStartUp;
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
  if (n==0) 
  {
	  //DEBUG_SERIAL("[DISPLAY] NOT Response\n");
	sprintf(tempbuffer,"[DISPLAY] NOT Response\n");
	display_state = isNot_response;
    DEBUG_SERIAL(tempbuffer);
	pubStatus_addtime(tempbuffer);
	  if (millis()<10000) Comm_Error = 10; //Reset display
  }
  return Comm_Error;
}
void Reset_display()
{
	sprintf(tempbuffer,"[%s]\n",Set_Reset_Display);
	DEBUG_SERIAL(tempbuffer);
	displaySerial.println(Set_Reset_Display);
	pinMode(DISPLAY_RESET,OUTPUT);
	digitalWrite(DISPLAY_RESET,LOW);
	wdt_reset();
	delay(1000);
	digitalWrite(DISPLAY_RESET,HIGH);
	pinMode(DISPLAY_RESET,INPUT);
	Comm_Error = 0;
	wdt_reset();
	
	int recheck = 7;
	display_state = isNot_response;
	while (display_state == isNot_response)
	{
		if (recheck--<=0) break;
		wdt_reset();
		delay(1000);
		Check_display_Running();
	}
	Reset_Bus();
	auto_sleep_ts==0;
}
uint8_t Send_BusStopName()
{
	sprintf(data_buffer,"[BusStationNo=%s;busStopName=%s;]",EEData.BusStopNo,busStopName);	
	return Send_data2display(&Begin_BusStop[0],&End_BusStop[0]);
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
	  DEBUG_SERIAL("%s Invalid\n",begin);
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
	//DEBUG_SERIAL("size=%d,BCC=%d,%s\n",size,bcc,data_buffer);
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
		   //DEBUG_SERIAL("Send %s OK\n",begin);)
		   sprintf(tempbuffer,"Send %s Done\n",begin);
		   DEBUG_SERIAL(tempbuffer);
		   pubStatus(tempbuffer);
		   break;
		  }
		  else if (strstr(tempbuffer, CheckSize_Fail)) {
		   //DEBUG_SERIAL("%s\n",CheckSize_Fail);
		   sprintf(tempbuffer,"%s\n",CheckSize_Fail);
		   DEBUG_SERIAL(tempbuffer);
		   pubStatus(tempbuffer);
		   break;
		  }
		  else if (strstr(tempbuffer, CheckSum_Fail)) {
		   //DEBUG_SERIAL("%s\n",CheckSum_Fail);
		   sprintf(tempbuffer,"%s\n",CheckSum_Fail);
		   DEBUG_SERIAL(tempbuffer);
		   pubStatus(tempbuffer);
		   break;
		  }
		}
	  } while (millis() - t < timeout);
  }
  if (step == 3) break;
  sprintf(tempbuffer,"Step=%d, Send %s Failed\n",step,begin);
  //DEBUG_SERIAL("Step=%d, Send %s Failed\n",step,begin);
  DEBUG_SERIAL(tempbuffer);
  pubStatus(tempbuffer);
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
void Get_Route_of_Station()
{

// [{"Tentram":"Bến xe Bùi Dương Lịch","capnhatcauhinh":1499245412,"matuyen":"9","tentuyen":"Thọ Quang - Quế Sơn"},
// {"Tentram":"Bến xe Bùi Dương Lịch","capnhatcauhinh":1499245412,"matuyen":"2","tentuyen":"Kim Liên - Chợ Hàn"},
// {"Tentram":"Bến xe Bùi Dương Lịch","capnhatcauhinh":1499245412,"matuyen":"TMF1","tentuyen":"Bùi Dương Lịch - Xuân Diệu - CV 29-3"}]
	char* p,*p1,*p_endline;
	//kiem tra cap nhat cau hinh
	p = strstr(data_buffer,"capnhatcauhinh");
	if (p)
	{
		p1 = strchr(p,':') + 2;
		uint32_t ts = atol(p1);
		if (ts != last_ConfigTime)
		{
			bisNew_Config = true;
			DEBUG_SERIAL("New_Config\n");
		}
	}
	//Ten tram
	p = strstr(data_buffer,"Tentram");
	if (p)
	{
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		if (busStopName[0] == 0 || Compare2array(busStopName,p1) == false)
		{
			memcpy(&busStopName[0],p1,sizeof(busStopName)-1);
			DEBUG_SERIAL("busStopName=%s\n",busStopName);
			BusStopName_sent = false;
		}
		*p = '"';
	}
	//
	p = strstr(data_buffer,"[{");
	int id = 0;
	while(p){
		//
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
		if (Compare2array(p1,Route[id].route_no)==false)
		{
			memcpy(&Route[id].route_no[0],p1,sizeof(Route[id].route_no)-1);
			Route[id].route_sent = false;
		}
		*p = '"';
		id +=1;
		p = p_endline;
	}
	while (Route[id].route_no[0]>0)
	{
		Route[id].route_no[0] = 0;
		if (++id>=Bus_Max) break;
	}
}
void Get_departure_from_buffer(int route_id)
{
//[{"matuyen":"TMF1","tentuyen":"Bùi Dương Lịch - Xuân Diệu - CV 29-3"}],[{"biensoxe":"43B-03508","gioxuatben":"5h45"},{"biensoxe":"43B-03568","gioxuatben":"6h15"},{"biensoxe":"43B-03587","gioxuatben":"6h35"}]
	char* p,*p1,*p_endline;
	char car_no[12];
	p = strstr(data_buffer,",[{");
	int index = 0;
	int now_hmm = rtc.hour * 100 + rtc.minute;
	int read_hmm = 0;
	bool isEnd_data = false;
	memset(car_no,0,sizeof(car_no));
	while(p){
		wdt_reset();
		
		p1 = p + 1;
		p_endline = strstr(p1,"},");
		if (p_endline == NULL) {p_endline = strstr(p1,"}]"); }
		if (p_endline == NULL) break;
		p = strstr(p1,"biensoxe");
		if (p==NULL) break;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(car_no,p1,sizeof(car_no)-1);
		*p = '"';
		//
		p1 = p + 1;
		p = strstr(p1,"gioxuatben");
		if (p==NULL) continue;
		if (p > p_endline) {p = p_endline; continue;};
		p1 = strchr(p,':') + 2;
		//DEBUG_SERIAL("%s\n",p1);
		read_hmm = atoi(p1) * 100;
		p = strchr(p1,'h');
		if (p==NULL) p = strchr(p1,'H'); 
		if (p==NULL) {p = p_endline; continue;};
		p1 = p + 1;
		
		read_hmm += atoi(p1);
		if (read_hmm<now_hmm) {p = p_endline; continue;};
		memcpy(Route[route_id].departure[index].car_no,car_no,sizeof(car_no));
		Route[route_id].departure[index].Time = read_hmm;
		//DEBUG_SERIAL("car_no=%s,time_arrival=%d\n",car_no,read_hmm);
		if (++index>=departure_Max) break;
	}
	Route[route_id].car_count = index;
	//DEBUG_SERIAL("return index=%d\n",index);
	while (index<departure_Max)
	{
		Route[route_id].departure[index].car_no[0] = 0;
		Route[route_id].departure[index].Time = 0;
		index += 1;
	}	
}
void Fill_BusInfor()
{
	DEBUG_SERIAL("Fill_BusInfor\n");
	int count =0;
	uint8_t disp_index = 0;
	char time_arrival[6];
	int total_car = 0;
	int id = 0;
	wdt_reset();
	while (Route[id].route_no[0]>0) {total_car += Route[id].car_count; id += 1;}
	while (disp_index<3 && disp_index<total_car)
	{
		id = 0;
		
		while (Route[id].route_no[0]>0)
		{
			if (Route[id].departure[count].car_no[0] == 0) continue;
			int gioxuatben = Route[id].departure[count].Time;
			sprintf(time_arrival,"%02d:%02d\0",gioxuatben/100,gioxuatben%100);
			//DEBUG_SERIAL("gioxuatben=%d\n",gioxuatben);
			if(Compare2array(Route[id].departure[count].car_no,Bus[disp_index].car_no)==false || Compare2array(time_arrival,Bus[disp_index].time_arrival) == false)			
			{
				Bus[disp_index].visible = true;
				Bus[disp_index].display_index = disp_index;
				Bus[disp_index].changed = true;
				memcpy(Bus[disp_index].route_no,Route[id].route_no,sizeof(Bus[disp_index].route_no));
				memcpy(Bus[disp_index].car_no,Route[id].departure[count].car_no,sizeof(Bus[disp_index].car_no)-1);
				memcpy(Bus[disp_index].time_arrival,time_arrival,sizeof(time_arrival));
				Bus[disp_index].passenger[0] = '0';
				DEBUG_SERIAL("New car=%d,%s,%s\n",disp_index,Bus[disp_index].car_no,Bus[disp_index].time_arrival);
			}
			//else DEBUG_SERIAL("Nochange, car=%s,%s\n",Bus[disp_index].car_no,Bus[disp_index].time_arrival);
			id += 1;
			disp_index += 1;
		}
		count += 1;
	}
	for (int i=disp_index;i<Bus_count;i++)
	{
		memset(Bus[i].route_no,0,sizeof(Bus[i].route_no));
		memset(Bus[i].car_no,0,sizeof(Bus[i].car_no));
		memset(Bus[i].time_arrival,0,sizeof(Bus[i].time_arrival));
		memset(Bus[i].passenger,0,sizeof(Bus[i].passenger));
		Bus[i].changed = false;
		Bus[i].visible = false;
	}
	Bus_count = disp_index;
	DEBUG_SERIAL("Bus_count=%d\n",Bus_count);
	delay(500);
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
	//kiem tra cap nhat cau hinh
	p = strstr(data_buffer,"capnhatcauhinh");
	if (p)
	{
		p1 = strchr(p,':') + 1;
		uint32_t ts = atol(p1);
		if (ts != last_ConfigTime)
		{
			bisNew_Config = true;
			DEBUG_SERIAL("New_Config\n");
		}
	}
	//Ten tram
	p = strstr(data_buffer,"Tentram");
	if (p)
	{
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		if (busStopName[0] == 0 || Compare2array(busStopName,p1) == false)
		{
			memcpy(&busStopName[0],p1,sizeof(busStopName)-1);
			DEBUG_SERIAL("busStopName=%s\n",busStopName);
			BusStopName_sent = false;
		}
		*p = '"';
	}
	//
	p = strstr(data_buffer,"[{");
	while(p){
		bus_struct bus_temp;		
		//
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
		bus_temp.route_no[sizeof(bus_temp.route_no)-1] = 0;
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
		bus_temp.car_no[sizeof(bus_temp.car_no)-1] = 0;
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
		bus_temp.passenger[sizeof(bus_temp.passenger)-1] = 0;
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
		bus_temp.time_arrival[sizeof(bus_temp.time_arrival)-1] = 0;
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
		//inbound - outbound
		p = strstr(p1,"outbound");
		if (p && p < p_endline) bus_temp.isOutbound = true;
		else bus_temp.isOutbound = false;
		p = p_endline;
		//
		bool exist = false;
		bool car_repeat = false;
		int id = 0;
		/* for (id=0;id<Bus_count;id++)
		{
			if((Bus[id].route_no[0] == 0) || Compare2array(bus_temp.car_no,Bus[id].car_no))
			{
				if (id<disp_index)
				{
					car_repeat = true;
					DEBUG_SERIAL("Car No repeated=%s",bus_temp.car_no);
					break;
				}
				exist = true;
				Bus[id].visible = true;
				if(disp_index != Bus[id].display_index) {DEBUG_SERIAL("display_index changed\n");Bus[id].changed = true; break;}
				//if(Compare2array(bus_temp.car_no,Bus[id].car_no) == false) {DEBUG_SERIAL("car_no changed\n");Bus[id].changed = true; break;}
				if(Compare2array(bus_temp.time_arrival,Bus[id].time_arrival) == false) {DEBUG_SERIAL("time_arrival changed\n");Bus[id].changed = true; break;}
				break;
			}
		} */
		//kiem tra lap bien so xe
		for (id=0;id<disp_index;id++)
		{
			if(Compare2array(bus_temp.car_no,Bus[id].car_no))
			{
				car_repeat = true;
				DEBUG_SERIAL("Car No repeated=%s\n",bus_temp.car_no);
				break;
			}
		}
		//neu lap biensoxe -> bo qua
		if (car_repeat) {p = p_endline; continue;}
		//kiem tra xem bus index da dung chua
		id = disp_index;
		if (Compare2array(bus_temp.car_no,Bus[id].car_no))
		{
			exist = true;
			Bus[id].visible = true;
			if(Compare2array(bus_temp.time_arrival,Bus[id].time_arrival) == false) {DEBUG_SERIAL("time_arrival changed\n");Bus[id].changed = true; break;}
		}
		if (!exist)
		{
			id = disp_index;//Bus_count++;
			Bus[id].changed = true;
			Bus[id].visible = true;
			//DEBUG_SERIAL("New Bus, Bus_count=%d\n",Bus_count);
			for (int i=0;i<Bus_Max;i++)
			{
				//neu da co route -> bo qua
				if(Compare2array(bus_temp.route_no,Route[i].route_no)) break;
				//neu chua co thi them moi
				if (Route[i].route_no[0]==0)
				{
					memcpy(&Route[i].route_no[0],bus_temp.route_no,sizeof(Route[i].route_no));
					Route[i].route_sent = false;
					break;
				}
			}
		}
		if (Bus[id].changed)
		{
			//Bus_sort[disp_index] = id;
			Bus[id].display_index = disp_index;
			memcpy(&Bus[id].route_no[0],bus_temp.route_no,sizeof(Bus[id].route_no));
			memcpy(&Bus[id].car_no[0],bus_temp.car_no,sizeof(Bus[id].car_no));
			memcpy(&Bus[id].time_arrival[0],bus_temp.time_arrival,sizeof(Bus[id].time_arrival));
			memcpy(&Bus[id].passenger[0],bus_temp.passenger,sizeof(Bus[id].passenger));
			Bus[id].isOutbound = bus_temp.isOutbound;
		}
		DEBUG_SERIAL("Bus %d: index=%d, route_no=%s, car_no=%s, time_arrival=%s, changed=%d, visible=%d\n",id,Bus[id].display_index,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].changed,Bus[id].visible);
		sprintf(tempbuffer,"index=%d, route_no=%s, car_no=%s, time_arrival=%s",Bus[id].display_index,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival);
		pubStatus(tempbuffer);
		disp_index+=1;
		if (disp_index >= Bus_Max) break;
	};
	//xoa cac bus cu neu co
	for (int i=disp_index;i<Bus_count;i++)
	{
		memset(Bus[i].route_no,0,sizeof(Bus[i].route_no));
		memset(Bus[i].car_no,0,sizeof(Bus[i].car_no));
		memset(Bus[i].time_arrival,0,sizeof(Bus[i].time_arrival));
		memset(Bus[i].passenger,0,sizeof(Bus[i].passenger));
	}
	Bus_count = disp_index;
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
void Update_Firmware_fromServer()
{
	if (Update_from == from_Ethernet) return Update_Firmware_fromEthernet();
	//
	if (WiFi.status() != WL_CONNECTED)
	{
		DEBUG_SERIAL("Wifi Not connected -> Check from Ethernet\n");
		return Update_Firmware_fromEthernet();
	}
	last_check_update = millis();
	String url = String(firmware_server_name) + String(check4update);
	String sketch_time = getFlashTime();
	wdt_reset();
		if (ESPhttpUpdate.Check_new_Update(url,sketch_name,sketch_time))
		{
			wdt_reset();
			sprintf(tempbuffer,"New firmware found, updating...\n");
			pubStatus(tempbuffer);
			DEBUG_SERIAL(tempbuffer);
			
			String update_url = String(firmware_server_name) + "bin/" + String(sketch_name) + ".bin";
			t_httpUpdate_return ret = ESPhttpUpdate.update(update_url);
			if (ret!=HTTP_UPDATE_OK)
			{
				DEBUG_SERIAL("Update fail....resetting\n");
				ESP.reset();
			}
		}
		else
		{
			sprintf(tempbuffer,"No Update found\n");
			pubStatus(tempbuffer);
			DEBUG_SERIAL(tempbuffer);
		}
}
void Update_Firmware_fromEthernet()
{
	bool update_available=false;
	last_check_update = millis();
	String url = "/bus/" + String(check4update);
	String sketch_time = getFlashTime();
	//check update
	data_buffer[0] = 0;
	if (Eclient.connect(host_update, 80)) {
	  wdt_reset();
		DEBUG_SERIAL("connected\n");
		DEBUG_SERIAL("GET %s%s HTTP/1.0\n",folder_update,check4update);
		Eclient.printf("GET %s%s HTTP/1.0\n",folder_update,check4update);
		Eclient.print("Host: ");	Eclient.println(host_update);
		Eclient.println("User-Agent: ESP8266-http-Update");
		Eclient.print("x-ESP8266-sketch-size: ");	Eclient.println(String(ESP.getSketchSize()));
		Eclient.print("x-ESP8266-sketch-name: ");	Eclient.println(sketch_name);
		Eclient.print("x-ESP8266-Flash-time: ");	Eclient.println(getFlashTime());
		Eclient.println("Connection: close");
		Eclient.println();
		//
		e_bytesRecv = 0;
		if (handleHeaderResponse(Eclient,7000) == 200)
		{
			update_available = true;
			wdt_reset();
			// sprintf(tempbuffer,"New firmware found, updating...\n");
			// pubStatus(tempbuffer);
			// DEBUG_SERIAL(tempbuffer);
		}
		else
		{
			sprintf(tempbuffer,"No Update found\n");
			pubStatus(tempbuffer);
			DEBUG_SERIAL(tempbuffer);
		}
		e_bytesRecv = Eclient.available();
		if (e_bytesRecv>0)
		{
			Eclient.readBytes(data_buffer,e_bytesRecv);
			data_buffer[e_bytesRecv] = 0;
			DEBUG_SERIAL("%s\n",data_buffer);
		}
	}
	else {
		DEBUG_SERIAL("connection failed to %s\n",host_update);
		sprintf(tempbuffer,"connection failed to %s\n",host_update);
		DEBUG_SERIAL(tempbuffer);
		pubStatus(tempbuffer);
  } 
	Eclient.stop();
  //get Update
  if (update_available==false) return;
  //
  data_buffer[0] = 0;
  if (Eclient.connect(host_update, 80)) { //bin/" + String(sketch_name) + ".bin";
	  wdt_reset();
		DEBUG_SERIAL("connected\n");
		DEBUG_SERIAL("GET %sbin/%s.bin HTTP/1.0\n",folder_update,sketch_name);
		Eclient.printf("GET %sbin/%s.bin HTTP/1.0\n",folder_update,sketch_name);
		Eclient.print("Host: ");	Eclient.println(host_update);
		Eclient.println("User-Agent: ESP8266-http-Update");
		// Eclient.print("x-ESP8266-sketch-size: ");	Eclient.println(String(ESP.getSketchSize()));
		// Eclient.print("x-ESP8266-sketch-name: ");	Eclient.println(sketch_name);
		// Eclient.print("x-ESP8266-Flash-time: ");	Eclient.println(getFlashTime());
		Eclient.println("Connection: close");
		Eclient.println();
		//
		e_bytesRecv = 0;
		if (handleHeaderResponse(Eclient,7000) == 200)
		{
			update_available = true;
			wdt_reset();
			sprintf(tempbuffer,"New firmware found, updating...\n");
			pubStatus(tempbuffer);
			DEBUG_SERIAL(tempbuffer);
		}
		else
		{
			e_bytesRecv = Eclient.available();
			if (e_bytesRecv>0)
			{
				Eclient.readBytes(data_buffer,e_bytesRecv);
				data_buffer[e_bytesRecv] = 0;
				//DEBUG_SERIAL("%s\n",data_buffer);
			}
			sprintf(tempbuffer,"Error On Update: %s\n",data_buffer);
			pubStatus(tempbuffer);
			DEBUG_SERIAL(tempbuffer);
		}
		if (update_available && e_bytesRecv>1000)
		{
			if (runUpdate(Eclient,e_bytesRecv,"",U_FLASH))
			 {
				 DEBUG_SERIAL("Update OK Restarting...\n");
				 ESP.restart();
			 }
			 else DEBUG_SERIAL("Update error\n");
		}
		
	}
	else {
		DEBUG_SERIAL("connection failed to %s\n",host_update);
		sprintf(tempbuffer,"connection failed to %s\n",host_update);
		DEBUG_SERIAL(tempbuffer);
		pubStatus(tempbuffer);
	} 
	Eclient.stop();
}
bool runUpdate(Stream& in, uint32_t size, String md5, int command)
{


    if(!Update.begin(size, command)) {
        //DEBUG_SERIAL("[httpUpdate] Update.begin failed! (%s)\n", error.c_str());
        return false;
    }

    if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
            return false;
        }
    }

    if(Update.writeStream(in) != size) {
       
        return false;
    }

    if(!Update.end()) {
        
        return false;
    }

    return true;
}
bool Report(char* rp_data)
{
	//DEBUG_SERIAL("connect to %s:%s\n",EEData.host,EEData.port);
	bool isPost = true;
	char* rp_host = "n2k16.esy.es";
	int rp_port = 80;
	char* rp_arg = "/bus/addbus.php";
	sprintf(com_buffer,"pw=bus_infor&table=%s&unixTime=%lu&StationID=%s&StationName=%s&Data=%s",chipID,unixTime_report,EEData.BusStopNo,busStopName,rp_data);
	//
	data_buffer[0] = 0;
  if (Eclient.connect(rp_host, rp_port)) {
    DEBUG_SERIAL("connected\n");
    // Make a HTTP request:	
	if (isPost){
		DEBUG_SERIAL("POST %s HTTP/1.1\n",rp_arg);
		Eclient.printf("POST %s HTTP/1.1\n",rp_arg);
	}
	else
	{
		DEBUG_SERIAL("GET %s?%s HTTP/1.1\n",rp_arg,com_buffer);
		Eclient.printf("GET %s?%s HTTP/1.1\n",rp_arg,com_buffer);
	}
    Eclient.print("Host: ");
	Eclient.println(rp_host);
    Eclient.println("Connection: close");
	if (isPost){
		Eclient.println("Content-Type: text/html; charset=UTF-8;");
		Eclient.print("Content-Length: ");
		Eclient.println(String(com_buffer).length());
		Eclient.println();	
		Eclient.print(com_buffer);
	}
	Eclient.println();	
	int dl=30;
	e_bytesRecv = 0;
	int httpcode = handleHeaderResponse(Eclient,7000);
	if (httpcode > 0)
	{
		if (e_bytesRecv>0)
		{
		Eclient.readBytes(data_buffer,e_bytesRecv);
		data_buffer[e_bytesRecv] = 0;
		//debugSerial.println(data_buffer);
		}		
		else
		{
			wdt_reset();
			e_bytesRecv = Eclient.readBytesUntil('#',com_buffer,200);
			com_buffer[e_bytesRecv] = 0;
			//debugSerial.println(com_buffer);
		}
	}else e_bytesRecv = 0;
	
  }
  else {
	sprintf(tempbuffer,"connection failed\n");
   DEBUG_SERIAL(tempbuffer);
   pubStatus(tempbuffer);
  }
  DEBUG_SERIAL("%s\n",data_buffer);   
  Eclient.stop();
  if (strstr(data_buffer,"GOOD")) return true;
  else 
  {
	  pubStatus(data_buffer);
	  return false;	
  }
}
void Process_MQTT()
{
	#ifdef wmqttclient
	if (WiFi.status() != WL_CONNECTED) return;
	#endif
	
	{

    if (!psclient.connected() && ((millis() - mqtt_timestamp>20000) || mqtt_timestamp==0)) {

      bool success;
	  wdt_reset();
      if (mqttuser.length() > 0) {
        success = psclient.connect( MQTT::Connect( String(chipID) ).set_auth(mqttuser, mqttpass) );
      } else {
        success = psclient.connect( String(chipID) );
      }
	  mqtt_timestamp=millis();
      if (success) {
		DEBUG_SERIAL("Connected to %s\n",mqttServerName.c_str());
        psclient.set_callback(onMessageArrived);

        psclient.subscribe(command_topic);
		psclient.subscribe(broadcast_topic);
		//
		SketchInfor2tempbuffer();
		pubStatus(tempbuffer);
		pubStatus(busStopName);

      } else {

        //debugSerial.println("Connect to MQTT server: FAIL");
        //delay(1000);
      }
    }

    if (psclient.connected()) {
      psclient.loop();
    }
  }
  yield();
}
void pubStatus_addtime( char* payload) {
	String s = String(rtc.hour) + ":" + String(rtc.minute) + ":";
	s+= String(rtc.second) + "->" + String(payload);
	pubStatus(s);
}
void pubStatus( char* payload) {
	pubStatus(String(payload));
}
void pubStatus( String payload) {

if (!psclient.connected()) return;
  if (psclient.publish(status_topic, payload)) {
    //debugSerial.println("Publish new status for " + t + ", value: " + payload);
  } else {
    //debugSerial.println("Publish new status for " + t + " FAIL!");
  }
}

void onMessageArrived(const MQTT::Publish& sub) {

  topic = sub.topic();
  payload = sub.payload_string();
  //DEBUG_SERIAL("topic=%s\npayload=%s\n",topic.c_str(),payload.c_str());
  DEBUG_SERIAL("from MQTT payload=%s\n",payload.c_str());
  
  if (topic == broadcast_topic && payload == whois)
  {
	  SketchInfor2tempbuffer();
  }
  else if (topic == command_topic)
  {
  payload.toCharArray(com_buffer, sizeof(com_buffer)-1);
  Process_Com_buffer(true);
  }
  else return;
  if (tempbuffer[0] > 0) pubStatus(tempbuffer);
  /* if (payload.equalsIgnoreCase(String(cm_updatefromserver)))
	{
		pubStatus("Check update from Server");
		Update_Firmware_fromServer();
	}
	else if (payload.equalsIgnoreCase(String(Show_Config)))
	{
		printConfig(true);
	}
	else if (payload.equalsIgnoreCase("???"))
	{
		SketchInfor2tempbuffer();
		pubStatus(tempbuffer);
	}
	else
	{
		pubStatus(payload);
	} */
}
