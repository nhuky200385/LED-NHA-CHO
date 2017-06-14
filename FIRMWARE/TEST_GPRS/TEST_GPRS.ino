
#include <Arduino.h>

#include "SIM800_.h"

#define debugSerial Serial
#define displaySerial Serial1

#define GPRS_Using 1

CGPRS_SIM800 gprs;

#define time_get 10 //seconds

// Common string used
#define str_ok 		"OK"			//string to reduce stack usage
#define str_at		"AT"			//string to reduce stack usage
//
#define ctrlz 26 //Ascii character for ctr+z. End of a SMS.
#define cr    13 //Ascii character for carriage return. 
#define lf    10 //Ascii character for line feed. 
//
#define GETSMS_UNREAD_SMS 20
#define GETSMS_UNREAD_SMS_AUTH 21
#define GETSMS_OLD_SMS 22

#define MOBI 1
#define VIETTEL 2
#define VINA 3

byte Network_provider=3;
//define rtcmemory store address
#define wakeup_mode_addr 0
#define sensor_laststate_addr 1
#define wakeup_count_addr 2
byte wakeup_count;


#define StartUp "StartUp"
#define BusInfo "BusInfo"
#define End_BusInfo "End_Info"
#define BusConfig "BusConfig"
#define End_BusConfig "End_Config"
#define CheckRunning "Running?"
#define Running "OK"
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

uint8_t Comm_Infor=0;
uint8_t Comm_Error=0;
uint8_t Get_Error=0;
//
char* chipID = "ESP_xxxxxxxx"; //xx = Chip ID
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
//
const char* password_ap = "13245768";

const char *server_host ="n2k16.esy.es";
const uint16_t server_port = 80;

static const char* get_url = "http://bus.danang.gov.vn:1022/bus-services-api/BusInfo" ;
static const char* content = "content=mt%";
static const char* station_no = "2069";

static const char* datetime_url = "n2k16.esy.es/datetime.php";
static const char* datetime_host = "n2k16.esy.es";
static const char* datetime_arg = "/datetime.php";

char *sketch_name ="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.ino";//
char sketch_time[14];

char* APN = "m3-world";

int e_bytesRecv;
String header_time="Date: Wed, 05 Apr 2017 08:42:28 GMT";
//byte stat=0;
byte LED=13;
int PW_GSM_PIN=0;

bool GSM_started=false;
bool GSM_turnedON=false;
char smsbuffer[160];
char n[20];
char n1[20];
char com_buffer[600];
int  com_buffer_len;
//


//******************************************************************************************************
#define RST_NORMAL HIGH  //LOW,HIGH
#define LEVEL_ON LOW
#define PW_GSM_OFF LOW

unsigned long Startup_timestamp;
unsigned long lastSent_timeStamp;

bool AP_On=false;
bool GSM_ON=false;
bool Checkupdate=false;
bool isUpdate=false;
float voltage;
bool timer0_en;
bool firstScan;
bool RF_Disable;
bool detected_NetworkTime;
bool Get_condition;

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
  uint32_t unixTime;
  uint32_t lastSent2Cloud;
  uint32_t sleep_seconds; //seconds
  uint32_t lastGetDatetime;
  float Temperature_Max;
  //byte data[100];
} rtcData;
//http header

//******************************************************************************************************
void StartGSM()
{
	debugSerial.println("SIM800 STATRING");
   GSM_started=false;
  for (int i=0;i<3;i++) {
    debugSerial.print("Resetting...");
    if (!gprs.init()) return;
    debugSerial.println("OK");
    delay(3000);
	if (!gprs.getOperatorName()) break;
    debugSerial.print("Operator:");
    debugSerial.println(gprs.buffer);
	Network_provider = GetProvider();
	
    debugSerial.print("Setting up network...");
	if (Network_provider == MOBI) APN = "m-wap";
	else if (Network_provider == VIETTEL) APN = "v-internet";
	else if (Network_provider == VINA) APN = "m3-world";
    byte ret = gprs.setup(APN);
    if (ret == 0)
      break;
    debugSerial.print("Error code:");
    debugSerial.println(ret);
    debugSerial.println(gprs.buffer);
  }
  GSM_started=true;
  
  debugSerial.println("OK");
  delay(3000);  
  
  int ret = gprs.getSignalQuality();
  if (ret) {
     debugSerial.print("Signal:");
     debugSerial.print(ret);
     debugSerial.println("dB");
  }
  for (int i=0;i<3;i++) {
    if (gprs.httpInit()) break;
    debugSerial.println(gprs.buffer);
    gprs.httpUninit();
    delay(1000);
  }
}
byte GetProvider()
{
// AT+COPS?
// +COPS: 0,2,"45201"
// OK
byte ret=0;
//if (SendATCmdWaitResp("AT+COPS?",1500,500,str_ok,2))
{
	char *p_char,*p_char1;
	convert2upperChar(gprs.buffer);
	p_char = strstr((char *)gprs.buffer, "45201");
	p_char1 = strstr((char *)gprs.buffer, "MOBIF");
	if (p_char != NULL || p_char1 != NULL) ret=MOBI;
	else
	{
		p_char = strstr((char *)gprs.buffer, "45204");
		p_char1 = strstr((char *)gprs.buffer, "VIET");
		if (p_char != NULL || p_char1 != NULL) ret=VIETTEL;
		else
		{
			p_char = strstr((char *)gprs.buffer, "45202");
			p_char1 = strstr((char *)gprs.buffer, "VINA");
			if (p_char != NULL || p_char1 != NULL) ret=VINA;
		}
	}
}
return ret;
}

void setup() {
  //khoi tao truyen thong  
  debugSerial.begin(115200);//115200
  displaySerial.begin(9600);
  //
  //DEBUG_SERIAL("StartUp\n");
  //

  pinMode(SIM800_RESET_PIN,OUTPUT);
  digitalWrite(SIM800_RESET_PIN,GPRS_Using);
  //

  Startup_timestamp = millis();

}
//******************************************************************************************************

void loop() {
	Get_condition=false; 
 //kiem tra thoi gian thuc
 // if (Unixtime_GMT7==0)
 // {
	 // if (GPRS_Using) GetDateTime_fromGSM();
	 // else Get_Time_fromEthernetShield();
 // }
  uint32_t time = Now();
  if (time%10 == 0 && time != time_bk)
  {
	  time_bk = time;
	  //DEBUG_SERIAL("%02d:%02d:%02d\n",rtc.hour,rtc.minute,rtc.second);	  
	 // if (Check_display_Running()>=6) Reset_display();
  }
  //if (time%time_get==0 || firstScan) Get_condition=true;//5m
  if (millis()-lastSent_timeStamp>(time_get*1000) || firstScan) 
  {
	   Get_condition = true;
  }
  if (Get_condition)
 {
	 if (GPRS_Using)
	 {
	unixTime_send = time;
	//current_Temp=getTemperature();

	 //check status STATE: IP INITIAL
	 //if (!gprs.sendCommand("AT+CIPSTATUS", 2000,"IP INITIAL")) GSM_started=false;
	 if (gprs.sendCommand("AT+SAPBR=2,1", 2000,"0.0.0.0")>0) GSM_started=false;
	 if (!GSM_started) StartGSM();
	 if (GET_fromServer() == 0)
	 {
		 //delay(1000);
		 //GET_fromServer();
		 // AT+SAPBR=2,1
		// +SAPBR: 1,1,"10.63.8.206"
		// OK


		// if (gprs.sendCommand("AT+SAPBR=2,1", 2000,"0.0.0.0")>0)
		// {
			// StartGSM();
			// GET_fromServer();
		// }
	 }
	 if (Get_Error>=3)
	 {
		 //DEBUG_SERIAL("Reset GSM Modem\n");
		 Get_Error=0;
		 gprs.sleep(true);
		 delay(1000);
		 gprs.sleep(false);
		 GSM_started = false;
		 StartGSM();
	 }
	// if (Comm_Infor == isBusInfo) Send_BusInfor();
	 //
	 GetDateTime_fromGSM();
	 }
	
	 lastSent_timeStamp = millis();
	 
	// Send_DateTime();
 }
 //
 firstScan=false;
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
void String2buffer(String s)
{
  memset(smsbuffer,0,sizeof(smsbuffer));
  s.toCharArray(smsbuffer,sizeof(smsbuffer));
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

int GET_fromServer()
{
	int b=-1;
	Comm_Infor=isBusIdle;
	if (!GSM_started) return b;// StartGSM();	
  String s=String(content) + String(station_no);
  debugSerial.print("Requesting ");
  debugSerial.print(get_url);
  debugSerial.print('?');
  //
  debugSerial.println(s);
  //
  gprs.sendCommand("AT+HTTPPARA?", 1000);
  //
  s.toCharArray(com_buffer,sizeof(com_buffer));
  gprs.httpConnect(get_url,&com_buffer[0]);
  //count++;
  int dl=30;
  while (gprs.httpIsConnected() == 0) {
    // can do something here while waiting
	
    debugSerial.write('.');
    for (byte n = 0; n < 25 && !gprs.available(); n++) {
      delay(10);
    }
	if (dl--<=0) break;
  }
  if (gprs.httpState == HTTP_ERROR) {
    debugSerial.println("Connect error");
	//debugSerial.println(gprs.buffer);
    //errors++;
    //delay(3000);
	if (strstr(gprs.buffer, "ERROR")) b=0;
	else Get_Error +=1;
    return b;
  }
  Get_Error = 0;
  debugSerial.println();
  gprs.httpRead();
  int ret;
  dl=50;
  while ((ret = gprs.httpIsRead()) == 0) {
    // can do something here while waiting
	
	debugSerial.write('.');
    for (byte n = 0; n < 25 && !gprs.available(); n++) {
      delay(10);
    }
	if (dl--<=0) break;
  }
  if (gprs.httpState == HTTP_ERROR) {
    debugSerial.println("Read error");
    //errors++;
    //delay(3000);
    return b;
  }
  // now we have received payload
  // debugSerial.print("[Payload]\n");
  // debugSerial.println(gprs.buffer);
  b = gprs.buffer_count();
  Comm_Infor = isBusInfo;
  // if (strstr(gprs.buffer, "routeName"))
  // {
	  // Comm_Infor = isBusInfo;
  // }  
  //
  return b;
}
float mapint2float(int x, int in_min, int in_max, float out_min, float out_max)
{
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
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
bool GetDateTime_fromGSM()
{
	int retry_count=2;
	bool ret=false;
	uint8_t yOff, m, d, hh, mm, ss;
	if (!GSM_started) StartGSM();
	if (!GSM_started) goto _exit;
	if (Network_provider == VIETTEL) goto _exit;
	retry:
	if (gprs.sendCommand("AT+CCLK?", 2000)) do {
    char *p;
	//+CCLK: "16/10/15,15:09:55+28"
    if (!(p = strchr(gprs.buffer, ':'))) break;
    if (!(p = strchr(p, '"'))) break;    
    yOff = atoi(++p);
    if (!(p = strchr(p, '/'))) break;
    m = atoi(++p);
    if (!(p = strchr(p, '/'))) break;
    d = atoi(++p);
    if (!(p = strchr(p, ','))) break;
    hh = atoi(++p);
    if (!(p = strchr(p, ':'))) break;
    mm = atoi(++p);
    if (!(p = strchr(p, ':'))) break;
    ss = atoi(++p);
	
	if (m <=1 && d<=1 && hh ==0 ) break;
	  else if (yOff>99 || m>12 || m==0 || d==0 || d>31 || hh>23 || mm>59 || ss>59) break;
	  else
	  {
		  rtc.year=yOff;
		  rtc.month = m;
		  rtc.day = d;
		  rtc.hour = hh;
		  rtc.minute = mm;
		  rtc.second = ss;
		  //
		  Unixtime_GMT7 = GetUnixTime_fromrtc()-(millis()/1000);// + 25200;
		  Now();
		  //DEBUG_SERIAL("DateTime: %02d/%02d/%02d %02d:%02d:%02d\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
		  ret = true;
		  goto _exit;
	  }
  } while(0);
  if (retry_count-->0) goto retry;
  _exit:
  if (!ret) ret = Get_http_Time();
  if (ret) rtcData.lastGetDatetime = Now();
  return ret;
}
bool Get_http_Time()
{
	bool b=false;
	gprs.httpConnect(datetime_url,NULL);
  //count++;
  int dl=30;
  while (gprs.httpIsConnected() == 0) {
    // can do something here while waiting
    debugSerial.write('.');
    for (byte n = 0; n < 25 && !gprs.available(); n++) {
      delay(10);
    }
	if (dl--<=0) break;
  }
  uint32_t u32 = millis();
  if (gprs.httpState == HTTP_ERROR) {
    debugSerial.println("Connect error");
    //errors++;
    //delay(3000);
    return b;
  }
  debugSerial.println();
  gprs.httpRead();
  int ret;
  dl=30;
  while ((ret = gprs.httpIsRead()) == 0) {
    // can do something here while waiting
	debugSerial.write('.');
    for (byte n = 0; n < 25 && !gprs.available(); n++) {
      delay(10);
    }
	if (dl--<=0) break;
  }
  if (gprs.httpState == HTTP_ERROR) {
    debugSerial.println("Read error");
    //errors++;
    //delay(3000);
    return b;
  }
  // now we have received payload
  debugSerial.print("[Payload]");
  debugSerial.println(gprs.buffer);
  b= Get_DateTimefrom_buffer();
	return b;
}

bool Get_DateTimefrom_buffer()
{
	bool b=false;
	uint32_t u32 = (millis()- u32)/2;
	// YOUR IP:116.103.225.215<br>UNIXTIME:1478294079<br>DATETIME:2016-11-04 21:14:39
	do{
		char *p;char *p1;
		if (!(p=strstr(gprs.buffer,"UNIXTIME"))) break;
		if (!(p = strchr(p, ':'))) break;
		if (!(p1 = strchr(p, '<'))) break;
		*p1=0;
		Unixtime_GMT7 = strtoul(++p,NULL,10);
		Unixtime_GMT7 = Unixtime_GMT7 -(uint32_t)(millis()/1000) + (uint32_t)(u32/1000);
		Now();
		//Set datetime
		//"02/03/18,09:54:28+40"
		//DEBUG_SERIAL("DateTime: %02d/%02d/%02d %02d:%02d:%02d\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
		b = true;
		if (GPRS_Using) SetGSM_DateTime();
		break;
	}while(0);
	return b;
}
bool SetGSM_DateTime()
{
	char dt[21];
	sprintf(dt,"%02d/%02d/%02d,%02d:%02d:%02d+00\n",rtc.year,rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
	dt[20] = 0;
	//DEBUG_SERIAL(dt);
	SIM_SERIAL.print("AT+CCLK=\"");
	SIM_SERIAL.print(dt);
	SIM_SERIAL.println("\"");
	gprs.sendCommand(0);
	debugSerial.println(gprs.buffer);
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
void Get_from_GPRS()
{
	
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


