#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#define CommSerial Serial2
#define debugSerial Serial

#define Set_busStop "BusStop="
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
#define Set_display "DISPLAY="
#define Set_Reset_Display "Reset_Display"
#define Begin_SetTime "Set_Time"
#define End_SetTime "End_Time"
#define Set_Brightness "Set_Brightness="
#define End_Brightness "End_Brightness"
#define CheckSum_Fail "CheckSum_Fail"
#define CheckSize_Fail "CheckSize_Fail"
#define Set_LostConnection "Set_Connection=0"
#define Set_Frame "Frame="
//#define demo "TMF_demo="
#define Set_Line "Line=" //Line=x,y x=line id, y=color

#define isBusIdle 0
#define isBusInfo 1
#define isBusConfig 2
#define isCheckRunning 3
#define isSet_Time 4
#define isSet_Brightness 5
#define isBusRoute 6
#define isBusStop 7

uint8_t State=1;

uint8_t Comm_Infor=0;
uint32_t timeout;
int sec=0,minute=0,hour=0;
bool swap_businfo=false,flash=false,b=false, swap_station;
uint32_t last_scroll_ts;
const uint32_t scroll_ts = 40;
const uint32_t businfo_swap_ts = 10000; //10s
uint32_t flash_ts,brightness_ts,businfo_ts,station_ts;
uint8_t businfo_index=0;
uint32_t last_update_infor_ts;
const uint32_t update_info_timeout = 300000; //5M
uint32_t last_ConfigTime = 0;

#include "Adafruit_GFX.h"
#include "RGBmatrixPanel_7.h"
#include "MyFont/TimeNewRoman11VN.h"
#include "MyFont/TimeNewRoman11.h"
#include "MyFont/Tahoma_8.h"
#include "MyFont/Tahoma_8VN.h"
#include "MyFont/Tahoma_8B.h"
#include "MyFont/Tahoma_8BVN.h"
#include "MyFont/TahomaNumber_10B.h"
#include "MyFont/Tahoma_12B.h"
#include "MyFont/Tahoma_12BVN.h"
#include "MyFont/Tahoma_12.h"
#include "MyFont/Tahoma_12VN.h"
#include "MyFont/Tahoma_14.h"
#include "MyFont/Tahoma_14VN.h"
#include "MyFont/VNFonts.h"
//#include "Config.h"

#define LED  13
#define OE_74HC245  12
#define OE  A0 //25
#define LAT A1 //26
#define CLK A2 //27
#define A   A3
#define B   A4
#define C   A5
#define D   A6

extern char _end;
extern "C" char *sbrk(int i);

char serial_buffer[200];
char comm_buffer[6000];
int comm_count;

int16_t x_scroll=-127,y_scroll=0; //
bool pause;
uint8_t auto_brightness;
bool auto_adjust_brightness=true;
bool bstartup;
bool bDateTime_OK = false;
uint8_t route_infor_Stype=1;
uint8_t route_infor_Step=0;
enum
{
	s_Nothing,
	s_tentuyen_biensoxe,  	//1
	s_tentuyen,				//2
	s_biensoxe,				//3
	s_diemdau_diemcuoi,		//4
};
//Hien thi thong tin chung
char disp_infor[1003];
bool bisCommonInfor;
char waitData[]={"ĐANG KẾT NỐI ĐẾN SERVER, VUI LÒNG CHỜ!"};
char lostConnection[]={"MẤT KẾT NỐI ĐẾN SERVER"};
char nobus[]={"HIỆN TẠI KHÔNG CÓ XE NÀO SẮP ĐẾN NHÀ CHỜ NÀY "};
char bendi[] = {"Đ.đầu"};
char benden[] = {"Đ.cuối"};

char tmf[] = {"TMF\1"};

uint32_t Unixtime_GMT7;
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

/* typedef struct{
	char text[30];
}header_struct; */
// #define Header_Max 5
// header_struct Header[Header_Max];
char busStopNo[12];
char busStopName[200];
char title_text[200];
char time_text[6];
char route_code_text[30];
char route_info_text[30];
char time_arrival_text[30];

/* typedef struct {
	char route_no[10],car_no[10],time_arrival[10];
	uint16_t passenger_present, passenger_down, passenger_up;
}bus_struct; */
#define Bus_Max 10
int Bus_count;
bus_struct Bus[Bus_Max];
bus_struct bus_temp;
//
#define Route_Max 10
Route_struct Route[Route_Max];
int route_index=0;
// typedef struct {
	// int16_t x,y,w,h;
	// uint8_t color,align;
	// const GFXfont *f,*fvn;
	// char* text;
	// bool isScroll;
	//int16_t yOffsetPlus;
// }frame;
#define Frame_Max 16
int Frame_count;
frame_struct Frame[Frame_Max];

//luu thong tin noi dung cau hinh
configInfor_struct configInfor[Frame_Max];

#define Line_Max 12
int Line_count;
//uint16_t x0,y0,x1,y1,color
line_struct Line[]
{
	{0,16,127,16,YELLOW},
	{0,30,127,30,GREEN},
	 {0,45,127,45,GREEN},
	 {0,60,127,60,GREEN},
	{0,75,127,75,YELLOW},
	//{43,31,43,70,GREEN},
	//{95,31,95,70,GREEN},
	{0,0,0,0,BLACK},
	{0,0,0,0,BLACK},
	{0,0,0,0,BLACK},
	{0,0,0,0,BLACK},
	{0,0,0,0,BLACK},
};
	
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

enum
{
	ftitle,
	ftime,
	froutecode,
	froute_info,
	ftime_arrival,
	frow1_c1,frow1_c2,frow1_c3,
	frow2_c1,frow2_c2,frow2_c3,
	frow3_c1,frow3_c2,frow3_c3,	
	froute_index,
	finfor,
};

void Init_bus()
{
	//
	int size = sizeof(configInfor[0].text);
	for (int i =0;i<Frame_Max;i++)
	{
		memset(configInfor[i].text,0,size);
	}
	Bus_count = 0;
}
void Process_Serialbuffer(Stream& inStream)
{
	char *p;
	if (strstr(serial_buffer, Set_Reset_Display))
	{
		debugSerial.println(Set_Reset_Display);
		Reset_all_Data();
	}
	else if (strstr(serial_buffer, Set_display))
	{
		p = strstr(serial_buffer, Set_display) + sizeof(Set_display) - 1;
		State = atoi(p);
		inStream.println(State==1 ? Running : Idle);
	}
	else if (strstr(serial_buffer, Set_Brightness))
	{
		p = strstr(serial_buffer, Set_Brightness) + sizeof(Set_Brightness) - 1;
		int f = atoi(p);		
		if (f<=16 && f>0)
		{
			inStream.print("Set_brightness=");
			inStream.println(matrix.Set_brightness(f));
			auto_adjust_brightness =false;
		}
		else if (f==0) auto_adjust_brightness = true;
		inStream.print("Auto Set_brightness=");
		inStream.println(auto_adjust_brightness);
	}
	/* else if (strstr(serial_buffer, Set_Frame))
	{
		p = strstr(serial_buffer, Set_Frame) + sizeof(Set_Frame) - 1;
		Frame_Stype = atoi(p);
		inStream.print(Set_Frame);
		inStream.println(Frame_Stype);
		Frame_Config();
	} */
	/* else if (strstr(serial_buffer, demo))
	{
		p = strstr(serial_buffer, demo) + sizeof(demo) - 1;
		TMF_demo = atoi(p);
		inStream.print(demo);
		inStream.println(TMF_demo);
		bstartup = true;
	} */
	else if (strstr(serial_buffer, Set_Line))
	{
		p = strstr(serial_buffer, Set_Line) + sizeof(Set_Line) - 1;
		int line_id = atoi(p);
		p = strchr(p, ',');
		if (p==NULL) return;
		p +=1;
		int color = atoi(p);
		Line[line_id].color = color;
		inStream.print(Set_Line); inStream.print(line_id);  inStream.print(",");
		inStream.println(color);
		matrix.drawLine(Line[line_id].x0,Line[line_id].y0,Line[line_id].x1,Line[line_id].y1,Line[line_id].color);
	}
	else if (strstr(serial_buffer, Set_LostConnection))
	{
		if (Bus_count>0)
		{
			Bus_count = 0;
			Frame[finfor].text = &lostConnection[0];
			Frame[finfor].changed = text_change;
			ClearBusInfor();
		}
	}
}
void CheckSerial()
{
if (debugSerial.available())
{
	int chars=debugSerial.readBytesUntil('\n', serial_buffer, sizeof(serial_buffer));	
	if (serial_buffer[0]=='p') pause = !pause;
	serial_buffer[chars] = 0;
    Process_Serialbuffer(debugSerial);
  while (debugSerial.available()) debugSerial.read(); //clear serial serial_buffer
}
}
void Frame_Config()
{
	Frame[ftitle] = {1,0,88,14,YELLOW,LEFT,&Tahoma_12B,&Tahoma_12BVN,&title_text[0],false,-4,true}; //0 &Tahoma_12BVN
	Frame[ftime] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&time_text[0],false,-1}; //1
	//
	Frame[froutecode] = {0,16,42,15,RED,LEFT,&Tahoma_8B,&Tahoma_8BVN,&route_code_text[0],false,0}; //2
	Frame[froute_info] = {42,16,45,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,&route_info_text[0],false,0}; //3
	Frame[ftime_arrival] = {87,16,42,15,YELLOW,RIGHT,&Tahoma_8B,&Tahoma_8BVN,&time_arrival_text[0],false,0}; //4
	//
	Frame[frow1_c1] = {1,31,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[0].route_no[0],false,-1}; //5
	Frame[frow1_c2] = {23,31,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],true,-1,true,true}; //6
	Frame[frow1_c3] = {96,31,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[0].time_arrival[0],false,-1}; //7
	//
	Frame[frow2_c1] = {1,46,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[1].route_no[0],false,-1}; //8
	Frame[frow2_c2] = {23,46,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],true,-1,true,true}; //9
	Frame[frow2_c3] = {96,46,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[1].time_arrival[0],false,-1}; //10
	//
	Frame[frow3_c1] = {1,61,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[2].route_no[0],false,-1}; //11
	Frame[frow3_c2] = {23,61,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],true,-1,true,true}; //12
	Frame[frow3_c3] = {96,61,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[2].time_arrival[0],false,-1}; //13
	
	Frame[froute_index] = {0,76,0,24,RED,CENTER,&Tahoma_12B,&Tahoma_12BVN,&Bus[0].route_no[0],false,-1,false}; //14
	Frame[finfor] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&waitData[0],true,0,true}; //15
	//22 -> 0		
			
	station_ts = millis();
	swap_station = false;
	
	for (int i = 0;i<Frame_Max;i++)
	{
		Frame[i].changed = all_change;
		//if (Frame[i].syncScroll) debugSerial.println(i);
	}
}
void Redraw()
{
	int16_t scroll_max=0,current_sc_len=0;
	bool isSync_Changed = false;
	for (int i=0;i<Frame_Max;i++)
	{
		if (Frame[i].w == 0) continue;
		if (Frame[i].isScroll && Frame[i].syncScroll)
		{			
			// debugSerial.print("changed: ");
			// debugSerial.print(Frame[i].changed,HEX); debugSerial.print(", ");
			// debugSerial.print(Frame[i].changed & text_change); debugSerial.print(", ");
			if ((Frame[i].changed & text_change) >0) {isSync_Changed = true; break;}
		}
	}
	if (isSync_Changed)
	{
		for (int i=0;i<Frame_Max;i++)
		{
			if (Frame[i].w == 0) continue;
			if (Frame[i].isScroll && Frame[i].syncScroll)
			{
				Frame[i].changed |= text_change;
				// Frame[i].scroll_index = 0;
				// Frame[i].scroll_pixel = 0;
				// Frame[i].scroll_position = 0;
				// Frame[i].scroll_length = 0;
			}
		}
		scroll_max = 0;
		debugSerial.println("isSync_Changed");
	}
	for (int i=0;i<Frame_Max;i++)
	{
		if (Frame[i].w == 0) continue;
		if (Frame[i].isScroll)
		{
			if (Frame[i].changed & text_change) //neu text thay doi
			{
				Frame[i].scroll_x = Frame[i].w + Frame[i].x;// - 1;
				if (Frame[i].syncScroll)
				{
					//kiem tra do dai text max truoc do
					if (scroll_max < Frame[i].scroll_length) scroll_max = Frame[i].scroll_length;
					Frame[i].scroll_length = 0;
					//tinh lai text lengh cho frame thay doi
					matrix.print_Rect_scroll(&Frame[i]);
					if (scroll_max < Frame[i].scroll_length) scroll_max = Frame[i].scroll_length;
				}
			}
		}
		if (!Frame[i].isScroll && Frame[i].changed != no_change) matrix.print_Rect(&Frame[i]);
		Frame[i].changed = no_change;
	}
	if (isSync_Changed)
	{
		for (int i=0;i<Frame_Max;i++)
		{
			if (Frame[i].isScroll && Frame[i].syncScroll)
			{
				Frame[i].scroll_length = scroll_max;
			}
		}
	}
	if (last_ConfigTime > 0)
	{
		for (int i=0;i<Line_Max;i++)
		{
			if (Line[i].color == BLACK) break;
			matrix.drawLine(Line[i].x0,Line[i].y0,Line[i].x1,Line[i].y1,Line[i].color);
		}
	}
	matrix.swapBuffers(true);
	
	// debugSerial.print("Position: ");
	// debugSerial.print(Frame[6].scroll_position); debugSerial.print(", ");
	// debugSerial.print(Frame[9].scroll_position); debugSerial.print(", ");
	// debugSerial.print(Frame[12].scroll_position); debugSerial.println();
	// debugSerial.print("scroll_max: ");
	// debugSerial.println(scroll_max);
	// debugSerial.print("scroll_length: ");
	// debugSerial.print(Frame[6].scroll_length); debugSerial.print(", ");
	// debugSerial.print(Frame[9].scroll_length); debugSerial.print(", ");
	// debugSerial.print(Frame[12].scroll_length); debugSerial.println();
}
void setup() {

	debugSerial.begin(115200);
	CommSerial.begin(19200);
	debugSerial.println("Startup");
	pinMode(OE_74HC245,OUTPUT);
	digitalWrite(OE_74HC245,LOW); //LOW HIGH
	pinMode(LED,OUTPUT);
	digitalWrite(LED,LOW);
	//startTimer1(TC2, 0, TC6_IRQn);
	matrix.begin();
	matrix.setTextWrap(false);
	matrix.fillScreen(BLACK);
	matrix.Set_brightness(2);
	
	Init_bus();
	Frame_Config();
	
	delay(500);
	check_mem_free();
	
	bstartup = true;
	//
	  rtc.year=1;
	  rtc.month = 1;
	  rtc.day = 1;
	  rtc.hour = 0;
	  rtc.minute = 0;
	  rtc.second = 0;
	  //
	  Unixtime_GMT7 = GetUnixTime_fromrtc();
	  //
	  last_update_infor_ts = millis();
}

void loop() {
	digitalWrite(OE_74HC245,State==1 ? LOW : HIGH);
	long ml = millis();
	if (brightness_ts==0 || (ml - brightness_ts) >=30000 ) //30s
	{
		// debugSerial.print("Position: ");
		// debugSerial.print(Frame[6].scroll_position); debugSerial.print(", ");
		// debugSerial.print(Frame[9].scroll_position); debugSerial.print(", ");
		// debugSerial.print(Frame[12].scroll_position); debugSerial.println();
		
		brightness_ts = ml;
		//debugSerial.print("A7="); debugSerial.println(analogRead(A7));
		float f=0;
		for (int i=0;i<20;i++)
		{
			f += analogRead(A7);
		}
		f /= 20.0f;
		CommSerial.print("brightness="); CommSerial.println(f);
		//auto_brightness = map((uint16_t)f,0,1023,16,0);
		//Tu dong dieu chinh do sang luc moi bat nguon
		if (Bus_count==0 && ml<300000) //5m
		{
			auto_brightness = 16;
		}
		else
		{
			//Tu dong dieu chinh do sang theo thoi gian
		if (bDateTime_OK)
		{
			if (rtc.hour >= 19 && rtc.hour < 6) auto_brightness = 2;
			else if (rtc.hour >= 18 && rtc.hour < 19) auto_brightness = 5;
			else if (rtc.hour >= 7 && rtc.hour < 18) auto_brightness = 16;
			else if (rtc.hour >= 6 && rtc.hour < 7) auto_brightness = 6;
		}
		}		
		//
		if (auto_brightness < 1) auto_brightness = 1;
		if (auto_adjust_brightness && auto_brightness != matrix.Get_brightness())
		{
			debugSerial.print("Set_brightness=");
			debugSerial.println(matrix.Set_brightness(auto_brightness));
		}
	}
	//Tat hien thi luc 22h den 5h //SET FROM COMM
	// if (bDateTime_OK)
	// {
		// if (rtc.hour >= 22 || rtc.hour < 5) State = 0;
		// if (rtc.hour ==5 && rtc.minute < 5) State =1;
	// }
	if (ml - last_update_infor_ts > update_info_timeout)
	{
		if (Bus_count>0)
		{
			Bus_count = 0;
			debugSerial.println(F("BusInfor Timeout"));
			Frame[finfor].text = &lostConnection[0];
			Frame[finfor].changed = text_change;
			ClearBusInfor();
		}
	}
	Scroll_process();
	// if (b) 
	// {
		// debugSerial.println(millis()-ml);
		// b=false;
	// }
	
  CheckSerial();
  CheckComm();
  //if (CommAvailable()) debugSerial.write(CommSerial.read());
}
int CommAvailable()
{
	//Scroll_process();
	return CommSerial.available();
}
void Scroll_process()
{
	char *p;
	uint32_t ml = millis();
	if (ml - businfo_ts >= businfo_swap_ts)
	{
		businfo_ts = ml;
		if (Bus_count > 3)
		{
			if (Bus_count - businfo_index > 3) businfo_index += 3;
			else businfo_index = 0;	
			int index = frow1_c1;
			
			for (int i=0;i<3;i++)
			{
				Frame[index].text = &Bus[businfo_index + i].route_no[0]; Frame[index++].changed = text_change;//matrix.print_Rect(&Frame[index++]);
				Frame[index].text = &Bus[businfo_index + i].bus_name[0]; Frame[index++].changed = text_change;//matrix.print_Rect(&Frame[index++]);
				Frame[index].text = &Bus[businfo_index + i].time_arrival[0]; Frame[index++].changed = text_change;//matrix.print_Rect(&Frame[index++]);
			}
			Redraw();
			//Frame[3].text = &bendi[0];
			//matrix.print_Rect(&Frame[3]);
			station_ts = ml;
			swap_station=false;
		}
	}

	if (ml - station_ts >= 10000)
	{
		station_ts = ml;
		if (route_infor_Stype == s_diemdau_diemcuoi)
		{
			route_infor_Step = 1- route_infor_Step;
			//Diem dau - diem cuoi
			if (route_infor_Step==0) memcpy(route_info_text,bendi,sizeof(bendi));
			else  memcpy(route_info_text,benden,sizeof(benden));
			Frame[froute_info].changed = text_change;
			for (int i=0;i<3;i++)
			{
				int id = businfo_index + i;
				if (route_infor_Step==0)
				{
					if(Bus[id].isOutbound) p = &Route[Bus[businfo_index + i].route_index].from[0];
					else p = &Route[Bus[businfo_index + i].route_index].to[0];
				}
				else
				{
					if(Bus[id].isOutbound) p = &Route[Bus[businfo_index + i].route_index].to[0];
					else p = &Route[Bus[businfo_index + i].route_index].from[0];
				}
				memcpy(Bus[id].bus_name,p,sizeof(Bus[id].bus_name)-1);
			}
			Frame[frow1_c2].changed = text_change;
			Frame[frow2_c2].changed = text_change;
			Frame[frow3_c2].changed = text_change;
			Redraw();

		}
	}
	if (bDateTime_OK && ml - flash_ts >= 500)
	{
		b = true;
		flash_ts= ml;
		if (flash) Now();
		if (flash) Frame[ftime].text[2] = ' ';
		else Frame[ftime].text[2] = ':';
		flash = !flash;
		Frame[ftime].text[0] = rtc.hour/10 + 0x30;
		Frame[ftime].text[1] = rtc.hour%10 + 0x30;
		Frame[ftime].text[3] = rtc.minute/10 + 0x30;
		Frame[ftime].text[4] = rtc.minute%10 + 0x30;

		matrix.print_Rect(&Frame[ftime]);
		//matrix.swapBuffers(true);
		//debugSerial.println(analogRead(A7));
	}
	if (ml - last_scroll_ts >= scroll_ts)
	{
		for (int i=0;i<Frame_Max;i++)
		{
			if (Frame[i].w == 0) continue;			
			if (Frame[i].isScroll) matrix.print_Rect_scroll(&Frame[i]);
			/* if (i==finfor)
			{
				debugSerial.print("index="); debugSerial.print(Frame[i].scroll_index);
				debugSerial.print(",pixel="); debugSerial.print(Frame[i].scroll_pixel);
				debugSerial.print(",scroll_x="); debugSerial.print(Frame[i].scroll_x);
				debugSerial.println();
			} */
		}
		if (last_ConfigTime > 0)
		{
			for (int i=0;i<Line_Max;i++)
			{
				if (Line[i].color == BLACK) break;
				matrix.drawLine(Line[i].x0,Line[i].y0,Line[i].x1,Line[i].y1,Line[i].color);
			}
		}
		matrix.swapBuffers(true);
		last_scroll_ts = ml;
	}
}
void CheckComm()
{
	Comm_Infor = isBusIdle;
	comm_count = 0;	
	uint32_t timeout = 1000;
	CommSerial.setTimeout(200);
	if(CommAvailable()) {
		uint32_t t = millis();
		do {
			if(CommAvailable()){
				/* char c=CommSerial.read();
				t = millis();
				if (comm_count >= sizeof(comm_buffer) - 1) {
					// buffer full, discard first half
					comm_count = sizeof(comm_buffer) / 2 - 1;
					memcpy(comm_buffer, comm_buffer + sizeof(comm_buffer) / 2, comm_count);
				  }
				comm_buffer[comm_count++] = c; */
				//if (c == '\n') break;
				int size = CommSerial.available();
				if (size>0)
				{
					CommSerial.readBytes(&comm_buffer[comm_count],size);
					comm_count += size;
					comm_buffer[comm_count] = 0;
				}
				if (strstr(comm_buffer, CheckRunning)) {
					Comm_Infor = isCheckRunning;
					break;
				}
				else if (strstr(comm_buffer, Begin_BusInfo)) {
					debugSerial.print("<-"); debugSerial.println(Begin_BusInfo);
					Comm_Infor = isBusInfo;
					CommSerial.println(Begin_BusInfo);
					break;
				}
				else if (strstr(comm_buffer, Begin_BusConfig)) {
					debugSerial.print("<-"); debugSerial.println(Begin_BusConfig);
					Comm_Infor = isBusConfig;
					CommSerial.println(Begin_BusConfig);
					break;
				}
				else if (strstr(comm_buffer, Begin_BusRoute)) {
					debugSerial.print("<-"); debugSerial.println(Begin_BusRoute);
					Comm_Infor = isBusRoute;
					CommSerial.println(Begin_BusRoute);
					break;
				}
				else if (strstr(comm_buffer, Begin_SetTime)) {
					debugSerial.print("<-"); debugSerial.println(Begin_SetTime);
					Comm_Infor = isSet_Time;
					CommSerial.println(Begin_SetTime);
					break;
				}
				else if (strstr(comm_buffer, Begin_BusStop)) {
					debugSerial.print("<-"); debugSerial.println(Begin_BusStop);
					Comm_Infor = isBusStop;
					CommSerial.println(Begin_BusStop);
					break;
				}
				else if (strstr(comm_buffer, Set_Reset_Display))
				{
					Comm_Infor = 0;
					Reset_all_Data();
				}
				/* else if (strstr(comm_buffer, Set_Brightness)) {
					debugSerial.print("<-"); debugSerial.println(Set_Brightness);
					Comm_Infor = isSet_Brightness;
					break;
				}	 */	
			}
		}while (millis()-t < timeout);
	}
	if (comm_count>0)
	{
		debugSerial.println(comm_buffer);
	}
	if (Comm_Infor == isCheckRunning)
	{
		if (bstartup) {CommSerial.println(StartUp); bstartup = false;}
		else 
		{
			CommSerial.println(State==1 ? Running : Idle);
		}
		debugSerial.print("->"); debugSerial.println(State==1 ? Running : Idle);
	}
	else if (Comm_Infor == isBusInfo) 
	{
		if (Get_datafromServer(Begin_BusInfo,End_BusInfo)) Get_BusInfor_from_buffer();
	}
	else if (Comm_Infor == isBusConfig) 
	{
		if (Get_datafromServer(Begin_BusConfig,End_BusConfig)) Get_BusConfig_from_buffer();
	}
	else if (Comm_Infor == isBusRoute)
	{
		if (Get_datafromServer(Begin_BusRoute,End_BusRoute)) Get_Route_from_buffer();
	}
	else if (Comm_Infor == isSet_Time) 
	{
		if (Get_datafromServer(Begin_SetTime,End_SetTime)) Get_DateTime_from_buffer();
	}
	else if (Comm_Infor == isBusStop) 
	{
		if (Get_datafromServer(Begin_BusStop,End_BusStop)) Get_BusStopName();
	}
	else if (Comm_Infor == isSet_Brightness) Get_Brightness();
	else
	{
		memcpy(serial_buffer,comm_buffer,comm_count);
		serial_buffer[comm_count] = 0;
		Process_Serialbuffer(CommSerial);
	}
}
bool Get_datafromServer(const char* begin,const char* end_data)
{
	bool bok=false;
	if (Comm_Infor == isBusIdle) return bok;
	Comm_Infor = isBusIdle;
	comm_count = 0;
	char* p,*p1,*p_endline;
	uint32_t chartimeout = 2000;
	uint32_t t = millis();
	comm_buffer[comm_count] = 0;
	CommSerial.setTimeout(2000);
	do
	{
		if(CommAvailable()){
			comm_count = CommSerial.readBytesUntil('\r', comm_buffer, sizeof(comm_buffer)-1);
			comm_buffer[comm_count] = 0;
			//
			if (strstr(comm_buffer, begin))
			{
				comm_count = 0;
				comm_buffer[comm_count] = 0;
				CommSerial.println(begin);
			}
			if (strstr(comm_buffer, end_data)) {
				p = strchr(comm_buffer,'[');
				p1 =strchr(comm_buffer,']');
			  if (p==NULL || p1==NULL) break;
			  int sizercv = (p1 - p) + 1;
			  p1 = strstr(comm_buffer,"size=");
			  if (p1==NULL) break;
			  p1 += 5;
			  int size = atoi(p1);
			  p1 = strstr(comm_buffer,"BCC=");
			  if (p1==NULL) break;
			  p1 += 4;
			  int bcc = atoi(p1);
			  int bccrcv = BCC(p,sizercv);
			  if (size != sizercv)
			  {
				CommSerial.println(CheckSize_Fail);
				debugSerial.print(CheckSize_Fail);
				debugSerial.print("="); debugSerial.println(sizercv);
				break;
			  }
			  if (bcc != bccrcv)
			  {
				debugSerial.println(CheckSum_Fail);
				CommSerial.print(CheckSum_Fail);
				debugSerial.print("="); debugSerial.println(bccrcv);
				break;
			  }
			  while(CommSerial.available()) CommSerial.read();
			  delay(10);
				bok = true;
				CommSerial.println(end_data);
				debugSerial.print("OK ");
				debugSerial.println(end_data);
				break;
			}
		}
	}while (millis()-t < chartimeout);
	
	// if (comm_count>0)
	// {
		// debugSerial.println(comm_buffer);
	// }
	return bok;
}
void Get_BusInfor_from_buffer()
{
	char*p, *p1,*p_endline;
	
	bool bno_change;
	int id;
	last_update_infor_ts = millis();
	//
	p1 = strstr(comm_buffer,"[{");
	int Bus_count_temp = 0;
	while (p1)
	{
		bno_change = false;
		id = Bus_count_temp;
		Bus[id].changed = no_change;
		p_endline = strstr(p1,"}");
		*p_endline = 0;
		if (p_endline == NULL) break;
		// p = strstr(p1,"index=");
		// if (p == NULL) return;
		// p1 = strchr(p,'=') + 1;
		// bus_temp.display_index = atoi(p1);
		//
		p = strstr(p1,"no_change");
		if (p) {bno_change = true; goto next_;}
		p = strstr(p1,"route_no=");
		if (p == NULL) break;
		p1 = strchr(p,'=') + 1;
		p = strchr(p1,',');
		*p = 0;
		memcpy(&bus_temp.route_no[0],p1,sizeof(bus_temp.route_no)-1);
		*p = ',';
		//
		p = strstr(p1,"car_no=");
		if (p == NULL) return;
		p1 = strchr(p,'=') + 1;
		p = strchr(p1,',');
		*p = 0;
		memcpy(&bus_temp.car_no[0],p1,sizeof(bus_temp.car_no)-1);
		*p = ',';
		//
		p = strstr(p1,"time=");
		if (p == NULL) return;
		p1 = strchr(p,'=') + 1;
		p = strchr(p1,',');
		*p = 0;
		memcpy(&bus_temp.time_arrival[0],p1,sizeof(bus_temp.time_arrival)-1);
		*p = ',';
		//
		p = strstr(p1,"passenger=");
		if (p == NULL) return;
		p1 = strchr(p,'=') + 1;
		p = strchr(p1,',');
		*p = 0;
		memcpy(&bus_temp.passenger[0],p1,sizeof(bus_temp.passenger)-1);
		*p = ',';
		//
		p = strstr(p1,"outbound");
		if (p) bus_temp.isOutbound = true;
		else bus_temp.isOutbound = false;
		//
		//if(Compare2array(bus_temp.route_no,Bus[id].route_no))
		if(Compare2array(bus_temp.car_no,Bus[id].car_no))
		{
			//if(Compare2array(bus_temp.car_no,Bus[id].car_no) == false) {debugSerial.println("car_no changed"); Bus[id].changed |= car_no_change;}
			if(Compare2array(bus_temp.time_arrival,Bus[id].time_arrival) == false) {debugSerial.println("time_arrival changed"); Bus[id].changed |= time_change;}
			if(Compare2array(bus_temp.passenger,Bus[id].passenger) == false) {debugSerial.println("passenger changed"); Bus[id].changed |= passenger_change;}
			//if (bus_temp.visible != Bus[id].visible) {debugSerial.println("visible changed"); Bus[id].changed |= visible_change;}
			memcpy(&Bus[id].car_no[0],bus_temp.car_no,sizeof(Bus[id].car_no)-1);
			memcpy(&Bus[id].time_arrival[0],bus_temp.time_arrival,sizeof(Bus[id].time_arrival)-1);
			memcpy(&Bus[id].passenger[0],bus_temp.passenger,sizeof(Bus[id].passenger)-1);
			Bus[id].isOutbound = bus_temp.isOutbound;
			if(Bus[id].bus_name[0]==0) // || (Bus[id].changed & car_no_change)>0)
			{
				Fill_BusName(id);
				Bus[id].changed |= text_change;
			}
		}
		else
		{
			debugSerial.println("Bus Index changed");
			Bus[id].changed = all_change;
			Bus[id].visible = bus_temp.visible;
			Bus[id].display_index = bus_temp.display_index;
			memcpy(&Bus[id].route_no[0],bus_temp.route_no,sizeof(Bus[id].route_no)-1);
			memcpy(&Bus[id].car_no[0],bus_temp.car_no,sizeof(Bus[id].car_no)-1);
			memcpy(&Bus[id].time_arrival[0],bus_temp.time_arrival,sizeof(Bus[id].time_arrival)-1);
			memcpy(&Bus[id].passenger[0],bus_temp.passenger,sizeof(Bus[id].passenger)-1);
			Bus[id].route_index = Find_RouteIndex(id);
			debugSerial.print("route_index="); debugSerial.println(Bus[id].route_index);			
			Fill_BusName(id);
		}
		sprintf(serial_buffer,"Bus %d:route_no=%s,car_no=%s,time=%s,passenger=%s,bus_name=%s",id,Bus[id].route_no,Bus[id].car_no,Bus[id].time_arrival,Bus[id].passenger,Bus[id].bus_name);
		debugSerial.println(serial_buffer);
		//
next_:
		if (bno_change)
		{
			sprintf(serial_buffer,"Bus %d:route_no=%s,no_change",id,Bus[id].route_no);
			debugSerial.println(serial_buffer);
		}
		Bus_count_temp += 1;
		*p_endline = '}';
		p1 = p_endline + 1;
	}
	Bus_count = Bus_count_temp;
	for (int i=Bus_count;i<Bus_Max;i++)
	{
		if (Bus[i].route_no[0] == 0) continue;
		memset(&Bus[i].route_no[0],0,sizeof(Bus[0].route_no));
		memset(&Bus[i].car_no[0],0,sizeof(Bus[0].car_no));
		memset(&Bus[i].bus_name[0],0,sizeof(Bus[0].bus_name));
		memset(&Bus[i].time_arrival[0],0,sizeof(Bus[0].time_arrival));
		Bus[i].changed = all_change;
	}
	//
	int index = frow1_c1;
	if (Bus_count<=3) businfo_index = 0;
	for (int i=0;i<3;i++)
	{
		Frame[index].text = &Bus[businfo_index + i].route_no[0]; Frame[index++].changed = Bus[businfo_index + i].changed;
		Frame[index].text = &Bus[businfo_index + i].bus_name[0]; Frame[index++].changed = (Bus[businfo_index + i].changed & text_change) > 0 ? text_change : no_change;
		Frame[index].text = &Bus[businfo_index + i].time_arrival[0]; Frame[index++].changed = (Bus[businfo_index + i].changed & time_change) > 0 ? text_change : no_change;
		//debugSerial.println(Bus[businfo_index + i].changed,BIN);
	}
	if (bisCommonInfor == false)
	{
		if (Bus_count == 0)
		{
			Frame[finfor].text = &nobus[0];
			if (Frame[froute_index].w >0)
			{
				Frame[froute_index].w = 0;
				Frame[finfor].x = Frame[froute_index].w;
				Frame[finfor].w = 128 - Frame[froute_index].w;
				Frame[finfor].changed |= text_change;
			}
		}
		else{
			if (Frame[froute_index].w == 0)
			{
				Frame[froute_index].w = 22;
				Frame[finfor].x = Frame[froute_index].w;
				Frame[finfor].w = 128 - Frame[froute_index].w;
				Frame[froute_index].changed = text_change;
			}
			//debugSerial.print("Bus[0].changed="); debugSerial.println(Bus[0].changed);
			if (Bus[0].changed == all_change)
			{				
				Frame[froute_index].text = &Bus[0].route_no[0]; //&tmf[0];//
				Frame[froute_index].changed = text_change;
				//change font to fix text len
				if (Bus[0].route_no[2]>0 && Bus[0].route_no[1]>0)
				{
					Frame[froute_index].f = &Tahoma_8B;
					Frame[froute_index].fvn = &Tahoma_8BVN;
					Frame[froute_index].yOffsetPlus = 2;
				}
				else {
					Frame[froute_index].f = &Tahoma_12B; 
					Frame[froute_index].fvn = &Tahoma_12BVN;
					Frame[froute_index].yOffsetPlus = -1;
					}
				//
				if (Bus[0].route_index>=0) 
				{					
					Frame[finfor].text = &Route[Bus[0].route_index].infor[0];
					Frame[finfor].changed |= text_change;
				}
				else Frame[finfor].text = NULL;
			}
		}
	}
	Redraw();
}
int Find_RouteIndex(int id)
{
	int route_selected=-1;
	for (int i=0;i<Route_Max;i++)
	{
		if(Route[i].route_no[0] == 0) break;
		if(Compare2array(Route[i].route_no,Bus[id].route_no))
		{
			route_selected = i;
			break;
		}
	}
	return route_selected;
}
//return route index
void Fill_BusName(int id)
{
	char*p,*p1;;
	int i = Bus[id].route_index;
	if (Bus[id].route_index<0) memcpy(Bus[id].bus_name,Bus[id].car_no,sizeof(Bus[id].car_no)-1);
	else
	{
		if (route_infor_Stype == s_Nothing)
		{
			Bus[id].bus_name[0] = 0;
		}
		else if (route_infor_Stype == s_tentuyen_biensoxe)
		{
			//Ten tram - BS xe
			memcpy(Bus[id].bus_name,Route[i].routeName,sizeof(Bus[id].bus_name)-1);
			p = &Bus[id].bus_name[0];
			p1 = strchr(p,'\0');
			memcpy(p1," - BS Xe: ",10); p1 += 10;
			memcpy(p1,Bus[id].car_no,sizeof(Bus[id].car_no));
			Bus[id].bus_name[sizeof(Bus[id].bus_name)-1] = 0;
		}
		else if (route_infor_Stype == s_tentuyen)
		{
			memcpy(Bus[id].bus_name,Route[i].routeName,sizeof(Bus[id].bus_name)-1);
		}
		else if (route_infor_Stype == s_biensoxe)
		{
			memcpy(Bus[id].bus_name,Bus[id].car_no,sizeof(Bus[id].bus_name)-1);
		}
		else if (route_infor_Stype == s_diemdau_diemcuoi)
		{
			//Diem dau - diem cuoi
			if (route_infor_Step==0)
			{
				if(Bus[id].isOutbound) p = &Route[i].from[0];
				else p = &Route[i].to[0];
			}
			else
			{
				if(Bus[id].isOutbound) p = &Route[i].to[0];
				else p = &Route[i].from[0];
			}
			if (Bus[id].changed == all_change) memcpy(Bus[id].bus_name,p,sizeof(Bus[id].bus_name)-1);

		}
	}
}
void Get_BusConfig_from_buffer()
{
	char *p,*p1;
	bool bisNew_Config = false;
	char buf[10];
	Comm_Infor = isBusIdle;
	//[{"c1":"1","c2":"1","c3":"2","c4":"3","c5":"2","c6":"1","c7":"2","c8":"1","c9":"2","c10":"1","c11":"1","s1":"TMF BUS","s2":"1497573007","s3":"Tuyến","s4":"Giờ đến","s11":"","ConfigTime":1497078249}]
		p = strstr(comm_buffer,"ConfigTime");
		debugSerial.println(comm_buffer);
		if (p)
		{
			p = strchr(p,':') + 1;
			uint32_t ts = atol(p);
			debugSerial.print(F("ConfigTime=")); debugSerial.println(ts);
			if (ts != last_ConfigTime)
			{
				bisNew_Config = true;				
				debugSerial.println(F("Recieved new Config"));
				//debugSerial.flush();
				last_ConfigTime = ts;
			}
		}
		if (bisNew_Config) //return;
		{
			for (int i=0;i<Frame_Max;i++)
			{
				// check_mem_free();
				// debugSerial.println("begin");
				// debugSerial.flush();
				sprintf(buf,"\"c%d\"",i+1);
				// debugSerial.println(buf);
				// debugSerial.flush();
				p = strstr(comm_buffer,buf);
				if (p==NULL) continue;
				p1 = strchr(p,':') + 2;
				int cl = atoi(p1);
				if (Frame[i].color != cl)
				{
					Frame[i].color = cl;
					Frame[i].changed |= color_change;
				}
				//
				configInfor[i].text[0] = 0;
				sprintf(buf,"\"s%d\"",i+1);
				p = strstr(comm_buffer,buf);
				if (p==NULL) continue;
				p1 = strchr(p,':') + 2;
				p = strchr(p1,'"');
				if (p==NULL) continue;
				*p = 0;
				memcpy(configInfor[i].text,p1,p-p1+1);
				if (i == finfor)
				{
					if (*p1 == 0) {
						if (bisCommonInfor)
						{
							int route_id = Find_RouteIndex(0);
							if (route_id>=0) 
							{
								Frame[finfor].text = &Route[route_id].infor[0];
								Frame[finfor].changed |= text_change;
								Frame[froute_index].changed |= text_change;
							}
							else Frame[finfor].text = NULL;
						}
						disp_infor[0] = 0; 
						bisCommonInfor = false;
						}
					else //if (Compare2array(p1,&disp_infor[0]) == false)
					{
						debugSerial.println(F("bisCommonInfor"));
						bisCommonInfor =  true;
						memcpy(&disp_infor[0],p1,sizeof(disp_infor)-1);
						Frame[finfor].text = &disp_infor[0];
						Frame[finfor].changed |= text_change;						
						debugSerial.println(disp_infor);
					}
				}
				// else if (i<frow1_c1 && i != froute_info)
				// {
					// if (Compare2array(p1,Frame[i].text) == false)
					// {
						// memcpy(Frame[i].text,p1,p-p1+1);
						// Frame[i].changed |= text_change;
					// }
				// }
				*p = '"';
			}
			
		}
		//
		Process_Config();
		//neu khong hien thi tuyen gan den hoac hien thi thong tin chung thi hien thi thong tin Full width
		/* if (bisCommonInfor || Frame[froute_index].color == BLACK)
		{
			Frame[froute_index].w = 0;
			Frame[finfor].x = Frame[froute_index].w;
			Frame[finfor].w = 128 - Frame[froute_index].w;
		}
		else
		{
			Frame[froute_index].w = 22;
			Frame[finfor].x = Frame[froute_index].w;
			Frame[finfor].w = 128 - Frame[froute_index].w;
		} */
		if(Frame[froute_index].color == BLACK)
		{
			Frame[froute_index].w = 0;
			Frame[finfor].x = Frame[froute_index].w;
			Frame[finfor].w = 128 - Frame[froute_index].w;
		}
		Frame[froute_index].changed = all_change;
		Frame[finfor].changed = all_change;
		Redraw();
}
void Process_Config()
{
	char *p,*p1,*p2;
	
	memcpy(route_code_text,configInfor[froutecode].text,sizeof(route_code_text)-1);
	memcpy(time_arrival_text,configInfor[ftime_arrival].text,sizeof(time_arrival_text)-1);
	Frame[froutecode].changed |= text_change;
	Frame[ftime_arrival].changed |= text_change;
	do{
		p = &configInfor[ftitle].text[0];
		convert2upperChar(p);
		p2 = strstr(configInfor[ftitle].text,"%MATRAM");
		p1 = &busStopNo[0];
		if (p2==NULL) 
		{
			p2 = strstr(configInfor[ftitle].text,"%TENTRAM");
			p1 = &busStopName[0];
		}
		if (p2)
		{
			*p2 = 0;
			p2 = p2 + 1;
			//p2 = strchr(p2,'%');
			//if (p2) p2 += 1;
			// p2 = strstr(p2,"%CHAYCHU");
			// if (p2) Frame[ftitle].isScroll = true;
			// else Frame[ftitle].isScroll = false;
			//convert2upperChar(p1);
			// if (*p==0) sprintf(title_text,"%s%s",p,p1);
			// else sprintf(title_text,"%s%s%s",p,p,p1);
			if (*p==0) sprintf(title_text,"%s",p1);
			else sprintf(title_text,"%s%s",p,p1);
			//memcpy(title_text,busStopName,sizeof(title_text)-1);
			debugSerial.print("title=");
			debugSerial.println(title_text);
		}
		else
		{
			memcpy(title_text,configInfor[ftitle].text,sizeof(title_text)-1);
		}		
		p2 = strstr(p2,"%CHAYCHU");
		if (p2) Frame[ftitle].isScroll = true;
		else Frame[ftitle].isScroll = false;
		break;
		
	}while(true);
	//Frame[ftitle].isScroll = true;
	Frame[ftitle].changed |= text_change;
	//
	convert2upperChar(&configInfor[froute_info].text[0]);
	//debugSerial.println(configInfor[froute_info].text);
	p = strstr(configInfor[froute_info].text,"%KIEU");
	if (p)
	{
		p += 5;
		uint8_t stype = atoi(p);
		debugSerial.print("infor_Stype="); debugSerial.println(stype);		
		//if (route_infor_Stype==0) route_infor_Stype = 1;
		if (stype != route_infor_Stype)
		{
			route_infor_Stype = stype;
			for(int i=0;i<Bus_count;i++)
			{
				Bus[i].changed = all_change;
				Fill_BusName(i);
			}
			if (route_infor_Stype != s_diemdau_diemcuoi)
			{
				route_info_text[0] = 0;
				Frame[finfor].changed = text_change;
			}
		}
	}
	p = strstr(configInfor[froute_info].text,"%CHAYCHU");
	bool iscroll = false;
	if (p) iscroll = true;
	Frame[frow1_c2].isScroll = iscroll; Frame[frow1_c2].changed = all_change;
	Frame[frow2_c2].isScroll = iscroll; Frame[frow2_c2].changed = all_change;
	Frame[frow3_c2].isScroll = iscroll; Frame[frow3_c2].changed = all_change;

}
void Get_Route_from_buffer()
{
	char*p, *p1,*p2;
	//char temprouteno[12];
	int index = 0;
	//"routeNo":"2","routeName":"Kim Liên - Chợ Hàn",
	p = strstr(comm_buffer,"routeNo");
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	//memcpy(&temprouteno[0],p1,sizeof(temprouteno)-1);
	//Find route index
	for (int i = 0; i<Route_Max;i++)
	{
		if (Route[i].route_no[0] == 0 || Compare2array(&Route[i].route_no[0],p1) )
		{
			index = i;			
			break;
		}
	}
	if (index == Route_Max) 
	{
		debugSerial.println(F("Over Route"));
		return;
	}
	//debugSerial.print("index="); debugSerial.println(index);
	memcpy(&Route[index].route_no[0],p1,sizeof(Route[index].route_no)-1);
	debugSerial.print(F("route_no=")); debugSerial.println(Route[index].route_no);
	//
	*p = '"';
	p1 = p + 1;
	p = strstr(p1,"routeName");
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Route[index].routeName[0],p1,sizeof(Route[index].routeName)-1);
	//debugSerial.print("routeName="); debugSerial.println(Route[index].routeName);
	//
	*p = '"';
	p1 = p + 1;
	p = strstr(p1,"bendi");
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Route[index].from[0],p1,sizeof(Route[index].from)-1);
	//debugSerial.print("bendi="); debugSerial.println(Route[index].from);
	//
	*p = '"';
	p1 = p + 1;
	p = strstr(p1,"benden");
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Route[index].to[0],p1,sizeof(Route[index].to)-1);
	//debugSerial.print("benden="); debugSerial.println(Route[index].to);
	//
	*p = '"';
	p1 = p + 1;
	p = strstr(p1,"\"Luotdi"); //"Luotdi":"274 Nguyễn Văn Cừ (Hòa Hiệp Bắc) – 
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	//p1 = p;
	p = strchr(p1,'"');
	*p = 0;
	//sprintf(&Route[index].infor[0],"LƯỢT ĐI: %s",p1);
	p2 = p1;
	//*p = '"';
	p1 = p + 1;
	p = strstr(p1,"\"Luotve"); //"Luotve":"Bạch Đằng (đối diện Chợ Hàn)
	if (p==NULL) return;
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	sprintf(&Route[index].infor[0],"TUYẾN %s - LƯỢT ĐI:  %s, LƯỢT VỀ:  %s.",Route[index].route_no,p2,p1);
	//memcpy(&Route[index].infor[0],p1,sizeof(Route[index].infor)-1);
	//debugSerial.print("lotrinh="); debugSerial.println(Route[index].infor);
}
void Get_DateTime_from_buffer()
{
	char *p = strchr(comm_buffer,'[');
	if (p == NULL) return;
	p +=1;
	uint32_t u32 = atol(p);
	if (u32 == 0) return;
	Unixtime_GMT7 = u32 - millis()/1000;
	Now();
	//debugSerial.println("Time OK");
	sprintf(comm_buffer,"Rcv Time: %d/%02d/%02d %02d:%02d:%02d",rtc.year, rtc.month,rtc.day,rtc.hour,rtc.minute,rtc.second);
	debugSerial.println(comm_buffer);
	bDateTime_OK = true;
}
void Get_BusStopName()
{
	char *p,*p1;
	Comm_Infor = isBusIdle;
	//debugSerial.println(comm_buffer);
	p = strstr(comm_buffer,"BusStopNo=");
	if (p == NULL) return;
	p1 = strchr(p,'=') + 1;
	p = strchr(p1,';');
	*p = 0;
	memcpy(busStopNo,p1,p - p1 + 1);
	*p = ';';
	//debugSerial.println(busStopNo);
	p1 = p + 1;
	p = strstr(comm_buffer,"busStopName=");
	if (p == NULL) return;
	p1 = strchr(p,'=') + 1;
	p = strchr(p1,';');
	*p = 0;
	if (!Compare2array(busStopName,p1))
	{
		memcpy(busStopName,p1,p - p1 + 1);
		//debugSerial.println(busStopName);
		Process_Config();
	}
}
void Get_Brightness()
{
	if (Comm_Infor != isSet_Brightness) return;
	Comm_Infor = isBusIdle;
	comm_count = 0;
	uint32_t chartimeout = 1000;
	uint32_t t = millis();
	char *p;
	bool bok=false;
	do
	{
		if(CommAvailable()){
			char c=CommSerial.read();
			t = millis();
			if (comm_count >= sizeof(comm_buffer) - 1) {
				// buffer full, discard first half
				debugSerial.println(F("buffer full"));
				comm_count = sizeof(comm_buffer) / 2 - 1;
				memcpy(comm_buffer, comm_buffer + sizeof(comm_buffer) / 2, comm_count);
			  }
			comm_buffer[comm_count++] = c;
			comm_buffer[comm_count] = 0;
			
			if (strstr(comm_buffer, End_Brightness)) {
				bok = true;
				//CommSerial.println(End_Time);
				int f = atoi(comm_buffer);		
				if (f<=16 && f>=0)
				{
					debugSerial.print(F("Set_brightness="));
					debugSerial.println(matrix.Set_brightness(f));
				}
				break;
			}
		}
	}while (millis()-t < chartimeout);	
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
unsigned long ram_used=0;
void check_mem_free()
{

char *ramstart=(char *)0x20070000;
char *ramend=(char *)0x20088000;
   char *heapend=sbrk(0);
   register char * stack_ptr asm ("sp");
   struct mallinfo mi=mallinfo();
   printf("Dynamic ram used: %lu\n",mi.uordblks);
   // if (mi.uordblks>=ram_used) printf("Dynamic ram increased: %lu\n",ram_used-mi.uordblks);
   // else printf("Dynamic ram Decreased: %lu\n",mi.uordblks-ram_used);
   // ram_used=mi.uordblks;
   // printf("Program static ram used %lu\n",&_end - ramstart); 
   //printf("Stack ram used %lu\n",ramend - stack_ptr); 
   //printf("My guess at free mem: %lu\n",stack_ptr - heapend + mi.fordblks);
   
   /* int* ptr_i;
   int i = 0xaa;
   ptr_i = &i;
   
   printf("Size of int: %lu, Size of uint: %lu\n", sizeof(int), sizeof(unsigned int));
   printf("Size of char: %lu, Size of uchar: %lu\n", sizeof(char), sizeof(unsigned char));
   printf("Size of short: %lu, Size of ushort: %lu\n", sizeof(short), sizeof(unsigned short));
   printf("Size of float: %lu, Size of double: %lu\n", sizeof(float), sizeof(double));
   printf("Size of long: %lu, Size of longlong: %lu\n", sizeof(long), sizeof(long long));
   printf("Size of pointer (ptr_i): %lu\n", sizeof(ptr_i));
   printf("Addr of pointer (ptr_i): 0x%x\n", ptr_i);
   printf("Value of i: 0x%x\n", *ptr_i); */
}
/* 
void ShowMemory(void)
{
	struct mallinfo mi=mallinfo();

	char *heapend=sbrk(0);
	register char * stack_ptr asm("sp");

	printf("    arena=%lu\n",mi.arena);
	printf("  ordblks=%lu\n",mi.ordblks);
	printf(" uordblks=%lu\n",mi.uordblks);
	printf(" fordblks=%lu\n",mi.fordblks);
	printf(" keepcost=%lu\n",mi.keepcost);
	
	printf("RAM Start %lx\n", (unsigned long)ramstart);
	printf("Data/Bss end %lx\n", (unsigned long)&_end);
	printf("Heap End %lx\n", (unsigned long)heapend);
	printf("Stack Ptr %lx\n",(unsigned long)stack_ptr);
	printf("RAM End %lx\n", (unsigned long)ramend);

	printf("Heap RAM Used: %lu\n",mi.uordblks);
	printf("Program RAM Used %lu\n",&_end - ramstart);
	printf("Stack RAM Used %lu\n",ramend - stack_ptr);

	printf("Estimated Free RAM: %lu\n\n",stack_ptr - heapend + mi.fordblks);
} */
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
void ClearBusInfor()
{
	route_index =0;
	for (int i=Bus_count;i<Bus_Max;i++)
	{
		memcpy(&Bus[i].route_no[0],"\0",sizeof(Bus[0].route_no)-1);
		memcpy(&Bus[i].car_no[0],"\0",sizeof(Bus[0].car_no)-1);
		memcpy(&Bus[i].bus_name[0],"\0",sizeof(Bus[0].bus_name)-1);
		memcpy(&Bus[i].time_arrival[0],"\0",sizeof(Bus[0].time_arrival)-1);
	}
	for (int i = frow1_c1 ; i<=froute_index;i++) Frame[i].changed = all_change;
	//
	Redraw();
}
void Reset_all_Data()
{
	Init_bus();
	Frame_Config();
	for (int i=0;i<Route_Max;i++)
	{
		memset(Route[i].route_no,0,sizeof(Route[i].route_no));
		memset(Route[i].routeName,0,sizeof(Route[i].routeName));
		memset(Route[i].from,0,sizeof(Route[i].from));
		memset(Route[i].to,0,sizeof(Route[i].to));
		memset(Route[i].infor,0,sizeof(Route[i].infor));
	}
	for (int i = 0;i<Frame_Max;i++)
	{
		Frame[i].changed = all_change;
	}
	ClearBusInfor();
	last_ConfigTime = 0;
	bstartup = true;
}
void convert2upperChar(char *ch)
{
  while (*ch) // exits on terminating NULL
  {
    if (( *ch >= 'a' ) && ( *ch <= 'z' ))  *ch &= 0xDF;
	ch+=1;
  }
}