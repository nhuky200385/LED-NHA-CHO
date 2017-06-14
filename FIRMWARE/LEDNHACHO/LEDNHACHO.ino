#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#define CommSerial Serial2
#define debugSerial Serial

#define Set_busStop "BusStop="
#define StartUp "StartUp"
#define BusInfo "BusInfo"
#define End_BusInfo "End_Info"
#define BusConfig "BusConfig"
#define End_BusConfig "End_Config"
#define CheckRunning "Running?"
#define Running "DISPLAY"
#define Idle "Idle"
#define Set_display "DISPLAY="
#define Set_Time "SetTime="
#define End_Time "End_Time"
#define Set_Brightness "Set_Brightness="
#define End_Brightness "End_Brightness"
#define CheckSum_Fail "CheckSum_Fail"
#define CheckSize_Fail "CheckSize_Fail"
#define Set_Frame "Frame="
#define demo "TMF_demo="
#define Set_Line "Line=" //Line=x,y x=line id, y=color

#define isBusIdle 0
#define isBusInfo 1
#define isBusConfig 2
#define isCheckRunning 3
#define isSet_Time 4
#define isSet_Brightness 5

uint8_t State=1;
uint8_t TMF_demo=0;

uint8_t Comm_Infor=0;
uint32_t timeout;
int sec=0,minute=0,hour=0;
bool swap_businfo=false,flash=false,b=false, swap_station;
uint32_t last_scroll_ts;
const uint32_t scroll_ts = 50;
const uint32_t businfo_swap_ts = 10000; //10s
uint32_t flash_ts,brightness_ts,businfo_ts,station_ts;
uint8_t businfo_index=0;
const uint8_t frame_infor_index=5; //
uint32_t last_update_infor_ts;
const uint32_t update_info_timeout = 300000; //5M

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

const char loi[]={"SỰ CỐ MẤT KẾT NỐI ĐẾN SERVER-XIN LỖI QUÝ KHÁCH VÌ SỰ BẤT TIỆN NÀY"};
const char nobus[]={"HIỆN TẠI KHÔNG CÓ XE NÀO SẮP ĐẾN NHÀ CHỜ NÀY "};
const char tmf_bdl[]={"TMF-BDL: ĐIỂM ĐẦU - BÙI DƯƠNG LỊCH - DƯƠNG VÂN NGA - KHÚC HẠO - HỒ HÁN THƯƠNG - CHU HUY MÂN - CẦU THUẬN PHƯỚC - ĐƯỜNG 3/2 - BÃI ĐỖ XE XUÂN DIỆU - ĐƯỜNG 3/2 - TRẦN PHÚ - LÝ TỰ TRỌNG - NGUYỄN THỊ MINH KHAI - LÊ DUẨN - NGÃ BA CAI LANG - LÝ THÁI TỔ - HÙNG VƯƠNG - CHI LĂNG - LÊ DUẨN - PHAN CHÂU TRINH - HÙNG VƯƠNG - BẠCH ĐẰNG - ĐƯỜNG 3/2 - BÃI ĐỖ XE XUÂN DIỆU - ĐƯỜNG 3/2 - CẦU THUẬN PHƯỚC - CHU HUY MÂN - HỒ HÁN THƯƠNG - KHÚC HẠO - DƯƠNG VÂN NGA - BÙI DƯƠNG LỊCH"};
const char tmf_xd[]={"TMF-XD: BÃI ĐỖ XE XUÂN DIỆU - ĐƯỜNG 3/2 - TRẦN PHÚ - LÝ TỰ TRỌNG - NGUYỄN THỊ MINH KHAI - LÊ DUẨN - NGÃ BA CAI LANG - LÝ THÁI TỔ - HÙNG VƯƠNG - CHI LĂNG - LÊ DUẨN - PHAN CHÂU TRINH - HÙNG VƯƠNG - BẠCH ĐẰNG - ĐƯỜNG 3/2 - BÃI ĐỖ XE XUÂN DIỆU"};
const char tuyen_7[]={"Tuyến 7: Chiều đi: Trạm xe buýt Xuân Diệu - Đường 3/2 - Đống Đa - Lê Lợi - Lý Tự Trọng - Trần Phú - Quang Trung - Lê Lợi - Phan Châu Trinh - Trưng Nữ Vương - Núi Thành - Tiểu La - Lê Thanh Nghị - CMT8 - Ông Ích Đường - Cầu Cầm Lệ - Phạm Hùng - Trạm xe buýt Phạm Hùng;    Chiều về: Trạm xe buýt Phạm Hùng - Phạm Hùng - Cầu Cẩm Lệ - Ông Ích Đường - CMT8 - Lê Thanh Nghị - Tiểu La - Núi Thành- Trưng Nữ Vương - Hoàng Diệu - Thái Phiên - Nguyễn Chí Thanh - Hùng Vương - Bạch Đằng - Đường 3/2 - Trạm xe buýt Xuân Diệu"};
const char tuyen_2[]={"Tuyến 2: Chiều đi: 274 Nguyễn Văn Cừ (Hòa Hiệp Bắc) - Nguyễn Lương Bằng (KCN Hòa Khánh) - Tôn Đức Thắng (Bến xe Trung tâm) - Điện Biên Phủ -Nguyễn Đức Trung - Trần Cao Vân - Hà Khê - Nguyễn Tất Thành - Ông Ích Khiêm - Lê Duẩn - Chi Lăng- Hùng Vương - Trần Phú - Trần Quốc Toản - Bạch Đằng (Chợ Hàn);   Chiều về: Bạch Đằng (đối diện Chợ Hàn) - Hùng Vương - Chi Lăng - Lê Duẩn - Ông Ích Khiêm - Nguyễn Tất Thành - Hà Khê - Trần Cao Vân - Nguyễn Đức Trung - Điện Biên Phủ - Tôn Đức Thắng (Bến xe Trung tâm) - Nguyễn Lương Bằng (KCN Hòa Khánh) - 274 Nguyễn Văn Cừ (Hòa Hiệp Bắc)"};
const char tuyen_9[]={"Tuyến 9: Chiều đi: Ngã 3 đường Nguyễn Phan Vinh - Hoàng Sa (điểm đầu tuyến) - Nguyễn Phan Vinh - Ngô Quyền - Cầu Rồng - Bạch Đằng - Lý Tự Trọng - Thanh Sơn - Ông Ích Khiêm - Nguyễn Tất Thành - Hà Khê - Hà Huy Tập - Trường Trinh - Quốc lộ 1A - Ngã Ba Hương An - Đường DT611 - Thị Trấn Đông Phú - Phan Chu Trinh - Bến xe Quế Sơn;   Chiều về: Bến xe Quế Sơn - Phan Chu Trinh - Đường ĐT 611 - Ngã Ba Hương An - Quốc lộ 1A - Trường Chinh - Hà Huy Tập - Hà Khê - Nguyễn Tất Thành - Ông Ích Khiêm - Thanh Sơn - Lý Tự Trọng - Nguyễn Thị Minh Khai - Hùng Vương - Trần Phú - Cầu Rồng - Ngô Quyền - Nguyễn Phan Vinh - Ngã 3 đường Nguyễn Phan Vinh - Hoàng Sa"};

char bendi[] = {"Đ.đầu\0"};
char benden[] = {"Đ.cuối\0"};

void Frame_Config(bool refresh = true);

char busStop_name[100];

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
header_struct Header[5];

/* typedef struct {
	char route_no[10],car_no[10],time_arrival[10];
	uint16_t passenger_present, passenger_down, passenger_up;
}bus_struct; */
#define Bus_Max 10
int Bus_count;
bus_struct Bus[Bus_Max];
//
#define Infor_Max 10
infor_struct Infor[Infor_Max];
int infor_index=0;
// typedef struct {
	// int16_t x,y,w,h;
	// uint8_t color,align;
	// const GFXfont *f,*fvn;
	// char* text;
	// bool isScroll;
	//int16_t yOffsetPlus;
// }frame;
#define Frame_Max 20
int Frame_count;
uint8_t Frame_Stype=5;
frame_struct Frame[20];
/* {
	{1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}, //0
	{91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}, //1
	//
	{1,16,44,16,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}, //2
	{45,16,44,16,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[3].text[0],false,0}, //3 &Header[3].text[0]
	{88,16,40,16,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}, //4
	//
	{1,34,44,8,RED,CENTER,&Tahoma_8,NULL,&Bus[0].route_no[0],false,-4}, //5
	{45,34,52,8,GREEN,LEFT,&Tahoma_8,NULL,&Bus[0].car_no[0],false,-4}, //6
	{97,34,31,8,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-4}, //7
	//
	{1,47,44,8,RED,CENTER,&Tahoma_8,NULL,&Bus[1].route_no[0],false,-4}, //8
	{45,47,52,8,GREEN,LEFT,&Tahoma_8,NULL,&Bus[1].car_no[0],false,-4}, //9
	{97,47,31,8,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-4}, //10
	//
	{1,60,44,8,RED,CENTER,&Tahoma_8,NULL,&Bus[2].route_no[0],false,-4}, //11
	{45,60,52,8,GREEN,LEFT,&Tahoma_8,NULL,&Bus[2].car_no[0],false,-4}, //12
	{97,60,31,8,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-4}, //13
	//
	{0,72,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,1}, //14
	//{0,72,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&tmf_bdl[0],true,0},
	{0,0,0,0,RED,LEFT,NULL,NULL,NULL,false,0},
	{0,0,0,0,RED,LEFT,NULL,NULL,NULL,false,0},
	{0,0,0,0,RED,LEFT,NULL,NULL,NULL,false,0},
	{0,0,0,0,RED,LEFT,NULL,NULL,NULL,false,0},
	{0,0,0,0,RED,LEFT,NULL,NULL,NULL,false,0},
}; */
//frame_struct *Frame_scroll;
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

void Init_bus()
{	
// header
// char tieude[] = "TMF BUS";
// char tuyen[] = "Tuyến";
// char xe[] = "BS.Xe";
// char time_arrival[] = "Giờ đến";
// char time_[]="12:00"; 
	memcpy(&busStop_name[0],"TMF BUS",sizeof(busStop_name)-1);
	memcpy(&Header[0].text[0],"TMF BUS",sizeof(Header[0].text)-1);
	memcpy(&Header[1].text[0],"00:00",sizeof(Header[0].text)-1);
	memcpy(&Header[2].text[0],"Tuyến",sizeof(Header[0].text)-1);
	memcpy(&Header[3].text[0],"BS.Xe",sizeof(Header[0].text)-1);
	memcpy(&Header[4].text[0],"Giờđến",sizeof(Header[0].text)-1);
	//
	Bus_count = 0;
	//
	//Bus[Bus_count].infor_index = 0;
/* 	memcpy(&Bus[Bus_count].route_no[0],"TMF\0",sizeof(Bus[0].route_no)-1);
	memcpy(&Bus[Bus_count].bus_name[0],"Nại Hiên Đông - Công Viên 29/3\0",sizeof(Bus[0].bus_name)-1);
	memcpy(&Bus[Bus_count].station_from[0],"Nại Hiên Đông\0",sizeof(Bus[0].station_from)-1);
	memcpy(&Bus[Bus_count].station_to[0],"Công Viên 29/3\0",sizeof(Bus[0].station_to)-1);
	memcpy(&Bus[Bus_count].car_no[0],"43H12345\0",sizeof(Bus[0].car_no)-1);
	memcpy(&Bus[Bus_count].time_arrival[0],"10:20\0",sizeof(Bus[0].time_arrival)-1);
	Bus_count+=1; */
	/* //
	Bus[Bus_count].infor_index = 1;
	memcpy(&Bus[Bus_count].route_no[0],"TMF-XD",sizeof(Bus[0].route_no)-1);
	memcpy(&Bus[Bus_count].car_no[0],"43H56789",sizeof(Bus[0].car_no)-1);
	memcpy(&Bus[Bus_count].time_arrival[0],"10:30",sizeof(Bus[0].time_arrival)-1);
	Bus[Bus_count].state = change_route_no;
	Bus_count+=1;
	//
	Bus[Bus_count].infor_index = 0;
	memcpy(&Bus[Bus_count].route_no[0],"08",sizeof(Bus[0].route_no)-1);
	memcpy(&Bus[Bus_count].car_no[0],"43H50009",sizeof(Bus[0].car_no)-1);
	memcpy(&Bus[Bus_count].time_arrival[0],"23:34",sizeof(Bus[0].time_arrival)-1);
	Bus[Bus_count].state = change_route_no;
	Bus_count+=1;
	//
	Bus[Bus_count].infor_index = 1;
	memcpy(&Bus[Bus_count].route_no[0],"12",sizeof(Bus[0].route_no)-1);
	memcpy(&Bus[Bus_count].car_no[0],"43H00087",sizeof(Bus[0].car_no)-1);
	memcpy(&Bus[Bus_count].time_arrival[0],"09:56",sizeof(Bus[0].time_arrival)-1);
	Bus[Bus_count].state = change_route_no;
	Bus_count+=1; */
	//
	int id =0;
	int infor_len = sizeof(Infor[infor_index].text)-3; //tieng viet co the chiem den 3 bytes
	memcpy(&Infor[id++].route_no[0]," ",sizeof(Infor[infor_index].route_no)-1);
	memcpy(&Infor[id++].route_no[0]," ",sizeof(Infor[infor_index].route_no)-1);
	memcpy(&Infor[id++].route_no[0],"TMF",sizeof(Infor[infor_index].route_no)-1);
	memcpy(&Infor[id++].route_no[0],"7",sizeof(Infor[infor_index].route_no)-1);
	memcpy(&Infor[id++].route_no[0],"2",sizeof(Infor[infor_index].route_no)-1);
	memcpy(&Infor[id++].route_no[0],"9",sizeof(Infor[infor_index].route_no)-1);
	//
	id =0;
	memcpy(&Infor[id++].text[0],&loi[0],infor_len);
	memcpy(&Infor[id++].text[0],&nobus[0],infor_len);
	memcpy(&Infor[id++].text[0],&tmf_bdl[0],infor_len);
	memcpy(&Infor[id++].text[0],&tuyen_7[0],infor_len);
	memcpy(&Infor[id++].text[0],&tuyen_2[0],infor_len);
	memcpy(&Infor[id++].text[0],&tuyen_9[0],infor_len);
	
	for (int i=0;i<Infor_Max;i++) Infor[i].text[infor_len] =0;
}
void Process_Serialbuffer(Stream& inStream)
{
	char *p;
	if (strstr(serial_buffer, Set_display))
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
	else if (strstr(serial_buffer, Set_Frame))
	{
		p = strstr(serial_buffer, Set_Frame) + sizeof(Set_Frame) - 1;
		Frame_Stype = atoi(p);
		inStream.print(Set_Frame);
		inStream.println(Frame_Stype);
		Frame_Config();
	}
	else if (strstr(serial_buffer, demo))
	{
		p = strstr(serial_buffer, demo) + sizeof(demo) - 1;
		TMF_demo = atoi(p);
		inStream.print(demo);
		inStream.println(TMF_demo);
		bstartup = true;
	}
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
void Frame_Config(bool refresh)
{
	int id = 0;
	switch(Frame_Stype)
	{
		case 0:
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[3].text[0],false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].car_no[0],false,-1,display_car_no}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].car_no[0],false,-1,display_car_no}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].car_no[0],false,-1,display_car_no}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 1:
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,NULL,false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],true,-1,display_bus_name}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],true,-1,display_bus_name}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],true,-1,display_bus_name}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 2:
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,NULL,false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],false,-1,display_bus_name}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],false,-1,display_bus_name}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],false,-1,display_bus_name}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 31:
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,NULL,false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],true,-1,display_bus_name}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],false,-1,display_bus_name}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],false,-1,display_bus_name}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 32: //tuong tu 3 nhung chay chu o dong so 2
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,NULL,false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],false,-1,display_bus_name}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],true,-1,display_bus_name}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],false,-1,display_bus_name}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 33: //tuong tu 3 nhung chay chu o dong so 2
			Frame[id++] = {1,0,93,14,YELLOW,LEFT,&Tahoma_12B,NULL,&Header[0].text[0],false,-4}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {1,16,44,15,RED,LEFT,&Tahoma_8,&Tahoma_8VN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {45,15,44,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,NULL,false,0}; //3
			Frame[id++] = {88,15,40,15,YELLOW,RIGHT,&Tahoma_8,&Tahoma_8VN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],false,-1,display_bus_name}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],false,-1,display_bus_name}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8,&Tahoma_8VN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],true,-1,display_bus_name}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {0,76,128,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0}; //14
		break;
		case 4:
			Frame[id++] = {1,0,90,14,YELLOW,LEFT,&Tahoma_12B,&Tahoma_12BVN,&busStop_name[0],false,-4,0,true}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {0,16,42,15,RED,LEFT,&Tahoma_8B,&Tahoma_8BVN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {42,16,46,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,&bendi[0],false,0}; //3
			Frame[id++] = {87,16,42,15,YELLOW,RIGHT,&Tahoma_8B,&Tahoma_8BVN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,74,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[0].station_from[0],false,-1,display_station}; //6
			Frame[id++] = {97,31,31,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,74,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[1].station_from[0],false,-1,display_station}; //9
			Frame[id++] = {97,46,31,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,74,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[2].station_from[0],false,-1,display_station}; //12
			Frame[id++] = {97,61,31,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {22,76,106,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0,0,true}; //14
			else id++;
			Frame[id++] = {0,76,22,24,RED,CENTER,&Tahoma_12B,&Tahoma_12BVN,&Bus[0].route_no[0],false,-1,0,false}; //15
			
			station_ts = millis();
			swap_station = false;
		break;
		case 5:
			Frame[id++] = {1,0,88,14,YELLOW,LEFT,&Tahoma_12B,&Tahoma_12BVN,&busStop_name[0],false,-4,0,true}; //0
			//Frame[id++] = {1,0,88,14,YELLOW,LEFT,&Tahoma_12,&Tahoma_12VN,&busStop_name[0],true,-3,0,true,true}; //0
			Frame[id++] = {91,0,37,14,YELLOW,LEFT,&TahomaNumber_10B,NULL,&Header[1].text[0],false,-1}; //1
			//
			Frame[id++] = {0,16,42,15,RED,LEFT,&Tahoma_8B,&Tahoma_8BVN,&Header[2].text[0],false,0}; //2
			Frame[id++] = {42,16,46,15,GREEN,CENTER,&Tahoma_8B,&Tahoma_8BVN,NULL,false,0}; //3
			Frame[id++] = {87,16,42,15,YELLOW,RIGHT,&Tahoma_8B,&Tahoma_8BVN,&Header[4].text[0],false,0}; //4
			//
			Frame[id++] = {1,31,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[0].route_no[0],false,-1}; //5
			Frame[id++] = {23,31,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[0].bus_name[0],true,-1,0,true,true}; //6
			Frame[id++] = {96,31,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[0].time_arrival[0],false,-1}; //7
			//
			Frame[id++] = {1,46,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[1].route_no[0],false,-1}; //8
			Frame[id++] = {23,46,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[1].bus_name[0],true,-1,0,true,true}; //9
			Frame[id++] = {96,46,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[1].time_arrival[0],false,-1}; //10
			//
			Frame[id++] = {1,61,22,15,RED,CENTER,&Tahoma_8B,&Tahoma_8BVN,&Bus[2].route_no[0],false,-1}; //11
			Frame[id++] = {23,61,73,15,GREEN,LEFT,&Tahoma_8,&Tahoma_8VN,&Bus[2].bus_name[0],true,-1,0,true,true}; //12
			Frame[id++] = {96,61,32,15,YELLOW,RIGHT,&Tahoma_8B,NULL,&Bus[2].time_arrival[0],false,-1}; //13
			//
			if (refresh) Frame[id++] = {22,76,106,24,GREEN,LEFT,&Tahoma_12,&Tahoma_12VN,&Infor[infor_index].text[0],true,0,0,true}; //14
			else id++;
			Frame[id++] = {0,76,22,24,RED,CENTER,&Tahoma_12B,&Tahoma_12BVN,&Bus[0].route_no[0],false,-1,0,false}; //15
			
			station_ts = millis();
			swap_station = false;
		break;
	}
	//if (refresh == false)  return;
	int16_t scroll_max=0;
	for (int i=0;i<Frame_Max;i++)
	{
		if (Frame[i].w == 0) break;		
		if (Frame[i].isScroll && i!=14)
		{
			Frame[i].scroll_x = Frame[i].w + Frame[i].x;// - 1;
			if (Frame[i].syncScroll)
			{
				Frame[i].scroll_length = 0;
				matrix.print_Rect_scroll(&Frame[i]);
				if (scroll_max < Frame[i].scroll_length) scroll_max = Frame[i].scroll_length;
			}
		}
		if (!Frame[i].isScroll) matrix.print_Rect(&Frame[i]);
	}
	for (int i=0;i<Frame_Max;i++)
	{
		if (Frame[i].isScroll && Frame[i].syncScroll && i!=14)
		{
			Frame[i].scroll_length = scroll_max;
		}
	}
	for (int i=0;i<Line_Max;i++)
	{
		if (Line[i].color == BLACK) break;
		matrix.drawLine(Line[i].x0,Line[i].y0,Line[i].x1,Line[i].y1,Line[i].color);
	}
	matrix.swapBuffers(true);
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
	
	debugSerial.print("tmf_bdl=");
	debugSerial.println(sizeof(tmf_bdl));
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
	if (ml - brightness_ts >=5000 )
	{
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
		if (rtc.hour >= 19 && rtc.hour < 6) auto_brightness = 2;
		else if (rtc.hour >= 17 && rtc.hour < 19) auto_brightness = 5;
		else if (rtc.hour >= 7 && rtc.hour < 17) auto_brightness = 16;
		else if (rtc.hour >= 6 && rtc.hour < 7) auto_brightness = 6;
		}
		//Tat hien thi luc 22h den 4h
		if (rtc.hour >= 22 && rtc.hour < 5) State = 0;
		if (rtc.hour ==4 & rtc.minute == 0) State =1;
		//
		if (auto_brightness < 1) auto_brightness = 1;
		if (auto_adjust_brightness && auto_brightness != matrix.Get_brightness())
		{
			debugSerial.print("Set_brightness=");
			debugSerial.println(matrix.Set_brightness(auto_brightness));
		}
	}
	if (ml - last_update_infor_ts > update_info_timeout)
	{		
		if (Bus_count>0)
		{
			Bus_count = 0;
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
	uint32_t ml = millis();
	if (ml - businfo_ts >= businfo_swap_ts)
	{
		businfo_ts = ml;
		if (Bus_count > 3)
		{
			if (Bus_count - businfo_index > 3) businfo_index += 3;
			else businfo_index = 0;	
			int index = frame_infor_index;
			
			for (int i=0;i<3;i++)
			{
				Frame[index].text = &Bus[businfo_index + i].route_no[0]; matrix.print_Rect(&Frame[index++]);
				if (Frame[index].display_code == display_car_no) Frame[index].text = &Bus[businfo_index + i].car_no[0];
				else if (Frame[index].display_code == display_bus_name) Frame[index].text = &Bus[businfo_index + i].bus_name[0];
				else Frame[index].text = &Bus[businfo_index + i].station_from[0]; 
				matrix.print_Rect(&Frame[index++]);
				Frame[index].text = &Bus[businfo_index + i].time_arrival[0]; matrix.print_Rect(&Frame[index++]);
			}
			Frame[3].text = &bendi[0];
			matrix.print_Rect(&Frame[3]);
			station_ts = ml;
			swap_station=false;
		}
		if (Frame_Stype>30)
		{
			int stype_max = Bus_count-businfo_index + 30;
			int new_stype = Frame_Stype+1;
			if (new_stype > stype_max) new_stype = 31;
			if (new_stype != Frame_Stype)
			{
				Frame_Stype = new_stype;
				Frame_Config(false);
				debugSerial.print("Frame_Stype="); debugSerial.println(Frame_Stype);
			}
		}
	}

	if (ml - station_ts >= 5000)
	{
		station_ts = ml;
		if (Frame_Stype==4){
		swap_station = !swap_station;
		if (swap_station) 
		{
			Frame[3].text = &benden[0];
			if (Bus_count-businfo_index >0) Frame[6].text = &Bus[businfo_index].station_to[0];
			if (Bus_count-businfo_index >1) Frame[9].text = &Bus[businfo_index+1].station_to[0];
			if (Bus_count-businfo_index >2) Frame[12].text = &Bus[businfo_index+2].station_to[0];
		}
		else 
		{	
			Frame[3].text = &bendi[0];
			if (Bus_count-businfo_index >0) Frame[6].text = &Bus[businfo_index].station_from[0];
			if (Bus_count-businfo_index >1) Frame[9].text = &Bus[businfo_index+1].station_from[0];
			if (Bus_count-businfo_index >2) Frame[12].text = &Bus[businfo_index+2].station_from[0];
		}
		matrix.print_Rect(&Frame[6]);
		matrix.print_Rect(&Frame[9]);
		matrix.print_Rect(&Frame[12]);
		matrix.print_Rect(&Frame[3]);
		}
	}
	if (ml - flash_ts >= 500)
	{
		b = true;
		flash_ts= ml;
		if (flash) Now();
		if (flash) Header[1].text[2] = ' ';
		else Header[1].text[2] = ':';
		flash = !flash;
		//debugSerial.println(minute*60+sec);
		// if (flash) 
		// {
			// if (++sec>=60) {
			// sec=0;
			// if (++minute>=60) {hour++; minute=0;}
			// }
		// }
		Header[1].text[0] = rtc.hour/10 + 0x30;
		Header[1].text[1] = rtc.hour%10 + 0x30;
		Header[1].text[3] = rtc.minute/10 + 0x30;
		Header[1].text[4] = rtc.minute%10 + 0x30;

		matrix.print_Rect(&Frame[1]);
		//matrix.swapBuffers(true);
		//debugSerial.println(analogRead(A7));
	}
	if (ml - last_scroll_ts >= scroll_ts)
	{
		for (int i=0;i<Frame_Max;i++)
		{
			if (Frame[i].w == 0) break;			
			if (Frame[i].isScroll) matrix.print_Rect_scroll(&Frame[i]);
			/* if (i==14)
			{
				debugSerial.print("index="); debugSerial.print(Frame[i].scroll_index);
				debugSerial.print(",pixel="); debugSerial.print(Frame[i].scroll_pixel);
				debugSerial.print(",scroll_x="); debugSerial.print(Frame[i].scroll_x);
				debugSerial.println();
			} */
		}
		for (int i=0;i<Line_Max;i++)
		{
			if (Line[i].x1 == 0 && Line[i].y1 == 0) break;
			matrix.drawLine(Line[i].x0,Line[i].y0,Line[i].x1,Line[i].y1,Line[i].color);
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
	if(CommAvailable()) {
		uint32_t t = millis();
		do {
			if(CommAvailable()){
				char c=CommSerial.read();
				t = millis();
				if (comm_count >= sizeof(comm_buffer) - 1) {
					// buffer full, discard first half
					comm_count = sizeof(comm_buffer) / 2 - 1;
					memcpy(comm_buffer, comm_buffer + sizeof(comm_buffer) / 2, comm_count);
				  }
				comm_buffer[comm_count++] = c;
				//if (c == '\n') break;
				comm_buffer[comm_count] = 0;
				
				if (strstr(comm_buffer, CheckRunning)) {
					Comm_Infor = isCheckRunning;
					break;
				}
				else if (strstr(comm_buffer, BusInfo)) {
					debugSerial.print("<-"); debugSerial.println(BusInfo);
					Comm_Infor = isBusInfo;
					CommSerial.println(BusInfo);
					break;
				}
				else if (strstr(comm_buffer, BusConfig)) {
					debugSerial.print("<-"); debugSerial.println(BusConfig);
					Comm_Infor = isBusConfig;
					break;
				}
				else if (strstr(comm_buffer, Set_Time)) {
					debugSerial.print("<-"); debugSerial.println(Set_Time);
					Comm_Infor = isSet_Time;
					break;
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
	else if (Comm_Infor == isBusInfo) Get_BusInfor();
	else if (Comm_Infor == isBusConfig) Get_BusConfig();
	else if (Comm_Infor == isSet_Time) Get_DateTime();
	else if (Comm_Infor == isSet_Brightness) Get_Brightness();
	else
	{
		memcpy(serial_buffer,comm_buffer,comm_count);
		serial_buffer[comm_count] = 0;
		Process_Serialbuffer(CommSerial);
	}
}
void Get_BusInfor()
{
	if (Comm_Infor != isBusInfo) return;
	Comm_Infor = isBusIdle;
	comm_count = 0;
	char* p,*p1,*p_endline;
	uint32_t chartimeout = 5000;
	uint32_t t = millis();
	bool bok=false;
	CommSerial.setTimeout(2000);
	do
	{
		if(CommAvailable()){
			/* char c=CommSerial.read();
			t = millis();
			if (comm_count >= sizeof(comm_buffer) - 1) {
				// buffer full, discard first half
				debugSerial.println("buffer full");
				comm_count = sizeof(comm_buffer) / 2 - 1;
				memcpy(comm_buffer, comm_buffer + sizeof(comm_buffer) / 2, comm_count);
			  }
			comm_buffer[comm_count++] = c;
			comm_buffer[comm_count] = 0; */
			comm_count = CommSerial.readBytesUntil('\r', comm_buffer, sizeof(comm_buffer)-1);
			comm_buffer[comm_count] = 0;
			//if (comm_count%200 ==0) CommSerial.print(200);
			if (strstr(comm_buffer, End_BusInfo)) {
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
			  //get station
			  /* p = strstr(comm_buffer,Set_busStop);
			  if (p)
			  {
				  p += sizeof(Set_busStop) - 1;
				  p1 = strchr(p,',');
				  int id=3;
				  memcpy(&Header[0].text[0],"MT-",id);
				  while (*p != ',') 
				  {
					  Header[0].text[id++] = *p++;
					  if (id >= sizeof(Header[0].text)-1) break;
				  }
				  Header[0].text[id++] = 0;
			  } */
				bok = true;
				CommSerial.println(End_BusInfo);
				debugSerial.println(End_BusInfo);
				last_update_infor_ts = millis();
				//[]
				if (sizercv == 2) infor_index = 1; //nobus
				//
				break;
			}
		}
	}while (millis()-t < chartimeout);
	
	if (comm_count>0)
	{
		debugSerial.println(comm_buffer);
	}
	if (!bok) return;
	//
	Bus_count = 0;
	//test only
	if (TMF_demo==1){
		//Bus[Bus_count].infor_index = 0;
		memcpy(&Bus[Bus_count].route_no[0],"TMF\0",sizeof(Bus[0].route_no)-1);
		memcpy(&Bus[Bus_count].bus_name[0],"Nại Hiên Đông - Công Viên 29/3\0",sizeof(Bus[0].bus_name)-1);
		memcpy(&Bus[Bus_count].station_from[0],"Nại Hiên Đông\0",sizeof(Bus[0].station_from)-1);
		memcpy(&Bus[Bus_count].station_to[0],"Công Viên 29/3\0",sizeof(Bus[0].station_to)-1);
		memcpy(&Bus[Bus_count].car_no[0],"43H12345\0",sizeof(Bus[0].car_no)-1);
		//memcpy(&Bus[Bus_count].time_arrival[0],"10:20",sizeof(Bus[0].time_arrival)-1);
		Bus[Bus_count].time_arrival[0] = rtc.hour/10 + 0x30;
		Bus[Bus_count].time_arrival[1] = rtc.hour%10 + 0x30;
		Bus[Bus_count].time_arrival[2] = ':';
		Bus[Bus_count].time_arrival[3] = rtc.minute/10 + 0x30;
		Bus[Bus_count].time_arrival[4] = rtc.minute%10 + 0x30;
		Bus[Bus_count].time_arrival[5] = 0;
		Bus_count+=1;
	}
	//busStop_name
	//"Tentram":"Công viên Hùng Vương"
	/* p = strstr(comm_buffer,"Tentram");
	if (p)
	{
		p1 = strchr(p,':') + 2;
		p = strchr(p1,'"');
		*p = 0;
		memcpy(&busStop_name[0],p1,sizeof(busStop_name)-1);
		*p='"';
		debugSerial.println(busStop_name);
	} */
	
	p = strstr(comm_buffer,"[{");
	while(p){
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
	debugSerial.println(Bus_count);
	//if (*p1 == '2') continue;
	//Bus[Bus_count].infor_index = 0;
	memcpy(&Bus[Bus_count].route_no[0],p1,sizeof(Bus[0].route_no)-1);
	debugSerial.println(Bus[Bus_count].route_no);
	//
	p1 = p + 1;
	p = strstr(p1,"tentuyen");
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Bus[Bus_count].bus_name[0],p1,sizeof(Bus[0].bus_name)-1);
	debugSerial.println(Bus[Bus_count].bus_name);
	int name_len = p-p1;
	//
	p1 = p + 1;
	p = strstr(p1,"biensoxe");
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Bus[Bus_count].car_no[0],p1,sizeof(Bus[0].car_no)-1);
	debugSerial.println(Bus[Bus_count].car_no);
	//add bien so xe vao ten tuyen
	memcpy(&Bus[Bus_count].bus_name[name_len],"- BS Xe: ",9);
	memcpy(&Bus[Bus_count].bus_name[name_len+9],p1,p-p1+1);
	//
	p1 = p + 1;
	p = strstr(p1,"soluonghanhkhach");
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 1;
	p = strchr(p1,',');
	*p = 0;
	memcpy(&Bus[Bus_count].passenger[0],p1,sizeof(Bus[0].passenger)-1);
	debugSerial.println(Bus[Bus_count].passenger);
	//
	p1 = p + 1;
	p = strstr(p1,"thoigianden"); //"thoigianden":"07:48:00"
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Bus[Bus_count].time_arrival[0],p1,sizeof(Bus[0].time_arrival)-1);
	debugSerial.println(Bus[Bus_count].time_arrival);
	//
	p1 = p + 1;
	p = strstr(p1,"Benden"); //"Benden":"Bến chợ Hàn"
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Bus[Bus_count].station_to[0],p1,sizeof(Bus[0].station_to)-1);
	debugSerial.println(Bus[Bus_count].station_to);
	//
	p1 = p + 1;
	p = strstr(p1,"Bendi"); //"Bendi":"Bến Kim Liên"
	if (p==NULL) continue;
	if (p > p_endline) {p = p_endline; continue;};
	p1 = strchr(p,':') + 2;
	p = strchr(p1,'"');
	*p = 0;
	memcpy(&Bus[Bus_count].station_from[0],p1,sizeof(Bus[0].station_from)-1);
	debugSerial.println(Bus[Bus_count].station_from);
	//
	Bus_count+=1;
	p = p_endline;
	//
	if (Bus_count >= Bus_Max) break;
	};
	//hien thi thong tin ten tuyen
	/* int offset=0;
	for (int i=0;i<Bus_count;i++)
	{
		char tuyen[] = "Tuyến";
		String str = String(tuyen);
		int len = str.length();
		memcpy(&Infor[infor_index].text[offset],&tuyen[0],len); offset += len;
		Infor[infor_index].text[offset++] = ' ';
		//
		str = String(Bus[i].route_no);
		len = str.length();
		memcpy(&Infor[infor_index].text[offset],&Bus[i].route_no[0],len); offset += len;
		Infor[infor_index].text[offset++] = ':';
		Infor[infor_index].text[offset++] = ' ';
		//
		str = String(Bus[i].bus_name);
		len = str.length();
		memcpy(&Infor[infor_index].text[offset],&Bus[i].bus_name[0],len); offset += len;
		if (i<Bus_count-1)
		{
			Infor[infor_index].text[offset++] = ',';
			Infor[infor_index].text[offset++] = ' ';
		}
		else Infor[infor_index].text[offset++] = 0;
	} */
	/* for (int i=0;i<Bus_count;i++)
	{
		p = strchr(Bus[i].bus_name,'-');
		int len = p - &Bus[i].bus_name[0];
		if (len > sizeof(Bus[i].station_to)-1) len = sizeof(Bus[i].station_to)-1;
		memcpy(&Bus[i].station_from[0],&Bus[i].bus_name[0],len);
		Bus[i].station_from[len-1] =0;
		p +=1;
		memcpy(&Bus[i].station_to[0],p,sizeof(Bus[i].station_to)-2);
	} */
	//xoa cac hang neu so luong hien thi <3
	for (int i=Bus_count;i<Bus_Max;i++)
	{
		memcpy(&Bus[i].route_no[0]," ",sizeof(Bus[0].route_no)-1);
		memcpy(&Bus[i].car_no[0]," ",sizeof(Bus[0].car_no)-1);
		memcpy(&Bus[i].bus_name[0]," ",sizeof(Bus[0].bus_name)-1);
		memcpy(&Bus[i].time_arrival[0]," ",sizeof(Bus[0].time_arrival)-1);
		memcpy(&Bus[i].station_from[0]," ",sizeof(Bus[0].station_from)-1);
		memcpy(&Bus[i].station_to[0]," ",sizeof(Bus[0].station_to)-1);
	}
	//update station no
	matrix.print_Rect(&Frame[0]);
	//for (int i=5;i<=13;i++)
	businfo_ts = millis();
	businfo_index = 0;
	uint8_t index = frame_infor_index;
	for (int i=0;i<3;i++)
	{
		/* Frame[index].text = &Bus[businfo_index + i].route_no[0]; matrix.print_Rect(&Frame[index++]);
		if (Frame[index].isdisplaybus_name) Frame[index].text = &Bus[businfo_index + i].bus_name[0];
		else Frame[index].text = &Bus[businfo_index + i].car_no[0]; 
		matrix.print_Rect(&Frame[index++]);
		Frame[index].text = &Bus[businfo_index + i].time_arrival[0]; matrix.print_Rect(&Frame[index++]); */
	}
	for (int i=0;i<Line_Max;i++)
	{
		if (Line[i].color == BLACK) break;
		matrix.drawLine(Line[i].x0,Line[i].y0,Line[i].x1,Line[i].y1,Line[i].color);
	}
	int infor_i;
	for (infor_i=0;infor_i<Infor_Max;infor_i++)
	{
		if (Compare2array(&Bus[0].route_no[0],&Infor[infor_i].route_no[0]))
		{
			if (infor_i == infor_index)
			{
				Frame_Config(false);
			}
			else 
			{
				infor_index = infor_i;
				debugSerial.print("route_no="); debugSerial.println(Bus[0].route_no);
				debugSerial.print("infor_index="); debugSerial.println(infor_index);
				Frame_Config();
			}
			break;
		}
	}
	//
	//matrix.swapBuffers(true);
}
void Get_BusConfig()
{
	if (Comm_Infor != isBusConfig) return;
	Comm_Infor = isBusIdle;
}
void Get_DateTime()
{
if (Comm_Infor != isSet_Time) return;
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
				debugSerial.println("buffer full");
				comm_count = sizeof(comm_buffer) / 2 - 1;
				memcpy(comm_buffer, comm_buffer + sizeof(comm_buffer) / 2, comm_count);
			  }
			comm_buffer[comm_count++] = c;
			comm_buffer[comm_count] = 0;
			
			if (strstr(comm_buffer, End_Time)) {
				bok = true;
				//CommSerial.println(End_Time);
				debugSerial.println(End_Time);
				// if (!(p = strchr(comm_buffer, ':'))) break;
				// hour = atoi(comm_buffer);				
				// minute = atoi(++p);
				// if (!(p = strchr(p, ':'))) break;
				// sec = atoi(++p);
				uint32_t u32 = atol(comm_buffer);
				if (u32 == 0) break;
				Unixtime_GMT7 = u32 - millis()/1000;
				Now();
				break;
			}
		}
	}while (millis()-t < chartimeout);	
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
				debugSerial.println("buffer full");
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
					debugSerial.print("Set_brightness=");
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
	infor_index =0;
	for (int i=Bus_count;i<Bus_Max;i++)
	{
		memcpy(&Bus[i].route_no[0]," ",sizeof(Bus[0].route_no)-1);
		memcpy(&Bus[i].car_no[0]," ",sizeof(Bus[0].car_no)-1);
		memcpy(&Bus[i].bus_name[0]," ",sizeof(Bus[0].bus_name)-1);
		memcpy(&Bus[i].time_arrival[0]," ",sizeof(Bus[0].time_arrival)-1);
		memcpy(&Bus[i].station_from[0]," ",sizeof(Bus[0].station_from)-1);
		memcpy(&Bus[i].station_to[0]," ",sizeof(Bus[0].station_to)-1);
	}
	debugSerial.println("Timeout");
	Frame_Config();
}