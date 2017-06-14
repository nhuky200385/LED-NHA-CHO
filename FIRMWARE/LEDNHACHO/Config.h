enum
{
	display_car_no,
	display_bus_name,
	display_station
};
typedef struct {
	int16_t x,y,w,h;
	uint8_t color,align;
	const GFXfont *f,*fvn;
	char* text;
	bool isScroll;
	int16_t yOffsetPlus;
	uint8_t display_code;
	bool toUpper;
	bool syncScroll;
	int16_t scroll_x,scroll_index,scroll_pixel,scroll_position,scroll_length;
}frame_struct;
typedef struct {
	int16_t x0,y0,x1,y1,color;
}line_struct;
typedef struct {
	char route_no[12],car_no[12],time_arrival[6],passenger[10],bus_name[200],station_from[100],station_to[100];
	uint16_t passenger_present, passenger_down, passenger_up;
}bus_struct;
typedef struct{
	char route_no[12];
	char text[1003];
}infor_struct;
typedef struct{
	char text[30];
}header_struct;