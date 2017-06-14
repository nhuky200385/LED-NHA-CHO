
#include "RGBmatrixPanel_7.h"
//#include "gamma.h"
#include "MyFont/VNFonts.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

#define DATAPORT PIOC->PIO_ODSR
#define DATADIR  PIOC->PIO_OER

#define oeSet() {PIOA->PIO_SODR=PIO_PA16;} //PA16 //0x10000
#define oeCLear() {PIOA->PIO_CODR=PIO_PA16;} //PA16
#define latSet() {PIOA->PIO_SODR=PIO_PA24;} //PA24
#define latCLear() {PIOA->PIO_CODR=PIO_PA24;} //PA24
#define clkSet() {PIOA->PIO_SODR=PIO_PA23;} //PA23
#define clkCLear() {PIOA->PIO_CODR=PIO_PA23;}
#define aSet() {PIOA->PIO_SODR=PIO_PA22;} //PA22
#define aCLear() {PIOA->PIO_CODR=PIO_PA22;}
#define bSet() {PIOA->PIO_SODR=PIO_PA6;} //PA6
#define bCLear() {PIOA->PIO_CODR=PIO_PA6;} //
#define cSet() {PIOA->PIO_SODR=PIO_PA4;} //PA4
#define cCLear() {PIOA->PIO_CODR=PIO_PA4;} //
#define dSet() {PIOA->PIO_SODR=PIO_PA3;} //PA3
#define dCLear() {PIOA->PIO_CODR=PIO_PA3;} //

#define brightness_Max 16
#define nPlanes 1
#define WidthScan 64
#define WidthPanel 2
#define HighPanel 3

static RGBmatrixPanel *activePanel = NULL;

// Code common to both the 16x32 and 32x32 constructors:
void RGBmatrixPanel::init(uint8_t rows, uint8_t a, uint8_t b, uint8_t c,
  uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf, uint8_t width) {

  nRows = rows; // Number of multiplexed rows; actual height is 2X this
  //nPlanes = width/16;
  // Allocate and initialize matrix buffer:
  int buffsize  = width * nRows * nPlanes;
  int allocsize = buffsize*buff_count;//(dbuf == true) ? (buffsize * 2) : buffsize;
  if(NULL == (matrixbuff[0] = (uint32_t *)malloc(allocsize))) return;
  memset(matrixbuff[0], 0, allocsize);
  // If not double-buffered, both buffers then point to the same address:
  //matrixbuff[1] = (dbuf == true) ? &matrixbuff[0][buffsize] : matrixbuff[0];
  //matrixbuff[1] = matrixbuff[0];
  if (buff_count>1)  matrixbuff[1] = &matrixbuff[0][buffsize]; else matrixbuff[1] = matrixbuff[0];
  if (buff_count>2)  matrixbuff[2] = &matrixbuff[1][buffsize];
  if (buff_count>3)  matrixbuff[3] = &matrixbuff[2][buffsize];
  
  scroll_buff_max = 64 * 6 * (buff_count-2);
  // Save pin numbers for use by begin() method later.
  _a     = a;
  _b     = b;
  _c     = c;
  _sclk  = sclk;
  _latch = latch;
  _oe    = oe;

  // Look up port registers and pin masks ahead of time,
  // avoids many slow digitalWrite() calls later.
  sclkpin   = digitalPinToBitMask(sclk);
  latport   = portOutputRegister(digitalPinToPort(latch));
  latpin    = digitalPinToBitMask(latch);
  oeport    = portOutputRegister(digitalPinToPort(oe));
  oepin     = digitalPinToBitMask(oe);
  addraport = portOutputRegister(digitalPinToPort(a));
  addrapin  = digitalPinToBitMask(a);
  addrbport = portOutputRegister(digitalPinToPort(b));
  addrbpin  = digitalPinToBitMask(b);
  addrcport = portOutputRegister(digitalPinToPort(c));
  addrcpin  = digitalPinToBitMask(c);
  plane     = nPlanes - 1;
  row       = nRows   - 1;
  swapflag  = false;
  backindex = 0;     // Array index of back buffer
}

// Constructor for 16x32 panel:
RGBmatrixPanel::RGBmatrixPanel(
  uint8_t a, uint8_t b, uint8_t c,
  uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf) :
  Adafruit_GFX(32, 16) {

  init(8, a, b, c, sclk, latch, oe, dbuf, 32);
}

// Constructor for 32x32 or 32x64 panel:
RGBmatrixPanel::RGBmatrixPanel(
  uint8_t a, uint8_t b, uint8_t c, uint8_t d,
  uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf, uint8_t width,uint8_t height) :
  Adafruit_GFX(width*WidthPanel, height*HighPanel) {

  init(16, a, b, c, sclk, latch, oe, dbuf, width);

  // Init a few extra 32x32-specific elements:
  _d        = d;
  // addrdport = portOutputRegister(digitalPinToPort(d));
  // addrdpin  = digitalPinToBitMask(d);
}
void startTimer(Tc *tc, uint32_t channel, IRQn_Type irq) {
	uint32_t frequency = 50000; //25k=100000 125k=500000
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)irq);
        TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
        //VARIANT_MCK=84000000
		uint32_t rc = VARIANT_MCK/frequency;
        TC_SetRA(tc, channel, rc/2); //50% high, 50% low
        TC_SetRC(tc, channel, rc);
        TC_Start(tc, channel);
        tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;
        tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;
        NVIC_EnableIRQ(irq);
}
void RGBmatrixPanel::begin(void) {

// Serial.print("_width=");Serial.println(_width);
// Serial.print("_height=");Serial.println(_height);
// DATAPORT = 0x21Fe;

 // Serial.println(DATAPORT,HEX);
 //Serial.println(pgm_read_byte(&gamma1[100]));
  backindex   = 0;                         // Back buffer
  buffptr     = matrixbuff[1 - backindex]; // -> front buffer
  activePanel = this;                      // For interrupt hander

  // Enable all comm & address pins as outputs, set default states:
  pinMode(_sclk , OUTPUT); digitalWrite(_sclk,LOW);  // Low
  pinMode(_latch, OUTPUT); digitalWrite(_latch,LOW);   // Low
  pinMode(_oe   , OUTPUT); digitalWrite(_oe,HIGH);     // High (disable output)
  pinMode(_a    , OUTPUT); digitalWrite(_a,LOW); // Low
  pinMode(_b    , OUTPUT); digitalWrite(_b,LOW); // Low
  pinMode(_c    , OUTPUT); digitalWrite(_c,LOW); // Low
  if(nRows > 8) {
    pinMode(_d  , OUTPUT); digitalWrite(_d,LOW); // Low
  }
  pinMode(LED_BUILTIN  , OUTPUT);
  Set_brightness(1);//brightness_Max
  // pinMode(_sclk , OUTPUT); SCLKPORT   &= ~sclkpin;  // Low
  // pinMode(_latch, OUTPUT); *latport   &= ~latpin;   // Low
  // pinMode(_oe   , OUTPUT); *oeport    |= oepin;     // High (disable output)
  // pinMode(_a    , OUTPUT); *addraport &= ~addrapin; // Low
  // pinMode(_b    , OUTPUT); *addrbport &= ~addrbpin; // Low
  // pinMode(_c    , OUTPUT); *addrcport &= ~addrcpin; // Low
  // if(nRows > 8) {
    // pinMode(_d  , OUTPUT); *addrdport &= ~addrdpin; // Low
  // }

  // The high six bits of the data port are set as outputs;
  // Might make this configurable in the future, but not yet.
  DATADIR  = 0xFFFFFFFF;//PC29 ~ IO_10 at Input  & ~(PIO_PC29); 
  DATAPORT = 0;

  // Set up Timer1 for interrupt:
  // TCCR1A  = _BV(WGM11); // Mode 14 (fast PWM), OC1A off
  // TCCR1B  = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Mode 14, no prescale
  // ICR1    = 100;
  // TIMSK1 |= _BV(TOIE1); // Enable Timer1 interrupt
  // sei();                // Enable global interrupts
  //startTimer1(TC2, 0, TC6_IRQn);
  startTimer(TC1, 0, TC3_IRQn);
  // PIOC->PIO_PER = 0x01FF;  // Configure PORTC to PIO controller, pins 33-40
  // PIOC->PIO_OER = 0x01FF;  // Enable PORTC to output
}
// Original RGBmatrixPanel library used 3/3/3 color.  Later version used
// 4/4/4.  Then Adafruit_GFX (core library used across all Adafruit
// display devices now) standardized on 5/6/5.  The matrix still operates
// internally on 4/4/4 color, but all the graphics functions are written
// to expect 5/6/5...the matrix lib will truncate the color components as
// needed when drawing.  These next functions are mostly here for the
// benefit of older code using one of the original color formats.

// Promote 3/3/3 RGB to Adafruit_GFX 5/6/5
uint16_t RGBmatrixPanel::Color333(uint8_t r, uint8_t g, uint8_t b) {
  // RRRrrGGGgggBBBbb
  // uint16_t c=0;
  // if (r>0) c &= 1;
  // if (g>0) c &= 2;
  // return c;
  return ((r & 0x7) << 13) | ((r & 0x6) << 10) |
         ((g & 0x7) <<  8) | ((g & 0x7) <<  5) |
         ((b & 0x7) <<  2) | ((b & 0x6) >>  1);
}
void RGBmatrixPanel::print_Rect(const GFXfont *f,const GFXfont *fvn,char *s,uint8_t color,uint8_t xoffset,uint8_t yoffset,
								uint8_t width,uint8_t height,uint8_t align, bool isScroll,int yOffsetPlus,bool toUpper)
{
	int16_t x_new,x_draw;
	int16_t x=0,y=0,x_upper;
	char c=0,c1,c2,c3;
	uint16_t w=96,h=8;
	
    char *p;//return;
	//String  str="";
	
	if (s == NULL)
	{
		fillRect(xoffset,yoffset,width,height,BLACK);
		return;
	}
	setFont(f);
	setTextColor(color);
	_rwidth = width + xoffset;
	_rheight = height + yoffset;
	_rcolor=color;
	_rxoffset=xoffset;
	_ryoffset=yoffset;
	x_draw=xoffset;
	rect_using=true;
	_ralign=align;
	p=s;
	draw_scrollbuffer = isScroll;
	setyOffsetPlus(yOffsetPlus);
	
	if (align>0)
	{
		offset_calc = true;
		setCursor(xoffset,_ryoffset);
		while (*p != NULL)
		{
			if (*p <= 0x7F)
			{
				char ch = *p;
				setFont(f);
				if (toUpper) if (( ch >= 'a' ) && ( ch <= 'z' ))  ch &= 0xDF;
				write(ch);				
			}
			else
			{
				c1 = *p;
				c2 = *(p+1);
				c3 = *(p+2);
				x = -1;
					if (c1 == 0xC3)
					{
						for (int i=0;i<VNFonts_C3_Length;i++)
						{
							if (VNFonts_C3[i].value == c2)
							{
								x_upper = VNFonts_C3[i].toUpper;
								x = VNFonts_C3[i].offset; y=1; break;
							}
						}
					}
					else if (c1 == 0xC4 || c1 == 0xC6)
					{
						for (int i=0;i<VNFonts_C46_Length;i++)
						{
							if (VNFonts_C46[i].value == c2)
							{
								x_upper = VNFonts_C46[i].toUpper;
								x = VNFonts_C46[i].offset; y=1; break;
							}
						}
					}
					else if (c1 == 0xC5)
					{
						for (int i=0;i<VNFonts_C5_Length;i++)
						{
							if (VNFonts_C5[i].value == c2)
							{
								x_upper = VNFonts_C5[i].toUpper;
								x = VNFonts_C5[i].offset; y=1; break;
							}
						}
					}
					else if (c1 == 0xE1)
					{
						y=2;
						if (c2 == 0xBA)
						{
							for (int i=0;i<VNFonts_E1BA_Length;i++)
							{
								if (VNFonts_E1BA[i].value == c3)
								{
									x_upper = VNFonts_E1BA[i].toUpper;
									x = VNFonts_E1BA[i].offset; break;
								}
							}
						}
						else if (c2 == 0xBB && c3 <0xA0)
						{
							for (int i=0;i<VNFonts_E1BB_Length;i++)
							{
								if (VNFonts_E1BB[i].value == c3)
								{
									x_upper = VNFonts_E1BB[i].toUpper;
									x = VNFonts_E1BB[i].offset; break;
								}
							}
						}
						else if (c2 == 0xBB && c3 >=0xA0)
						{
							for (int i=0;i<VNFonts_E1BBA0_Length;i++)
							{
								if (VNFonts_E1BBA0[i].value == c3)
								{
									x_upper = VNFonts_E1BBA0[i].toUpper;
									x = VNFonts_E1BBA0[i].offset; break;
								}
							}
						}
					}
					x_upper -= 1;
					x -=1; //vi VNFonts2 Offset tu 1 -> 0
					if (toUpper) x = x_upper;
					if (x>=0)
					{
						if (fvn != NULL)
						{
							setFont(fvn);
							write(x + VNFonts_Offset);
						}
						else
						{
							write(VN2EN[x]);
						}
					}
					p +=y;
			}
			p++;
		}
		w = getCursorX() - xoffset;// - 1;
		if (align == CENTER)
		{
			x_draw= (width-w)/2+ xoffset;
		}
		else if (align == RIGHT)
		{
			x_draw = _rwidth-w;
		}
		if (x_draw < xoffset) x_draw = xoffset;
		offset_calc = false;
	}
	if (isScroll)
	{
		for (int i=2;i<buff_count;i++) memset(matrixbuff[i], BLACK, WidthScan * nRows* 4);
		setCursor(x_draw,0);
	}
	else
	{
		fillRect(xoffset,yoffset,width,height,BLACK);
		setCursor(x_draw,_ryoffset);
	}
	//print(s);
	p=s;
	while (*p != NULL)
	{
		if (*p <= 0x7F)
		{
			setFont(f);
			write(*p);
		}
		else
		{
			c1 = *p;
			c2 = *(p+1);
			c3 = *(p+2);
			x = -1; y = 0;
				if (c1 == 0xC3)
				{
					for (int i=0;i<VNFonts_C3_Length;i++)
					{
						if (VNFonts_C3[i].value == c2)
						{
							x = VNFonts_C3[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xC4 || c1 == 0xC6)
				{
					for (int i=0;i<VNFonts_C46_Length;i++)
					{
						if (VNFonts_C46[i].value == c2)
						{
							x = VNFonts_C46[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xC5)
				{
					for (int i=0;i<VNFonts_C5_Length;i++)
					{
						if (VNFonts_C5[i].value == c2)
						{
							x = VNFonts_C5[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xE1)
				{
					y=2;
					if (c2 == 0xBA)
					{
						for (int i=0;i<VNFonts_E1BA_Length;i++)
						{
							if (VNFonts_E1BA[i].value == c3)
							{
								x = VNFonts_E1BA[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 <0xA0)
					{
						for (int i=0;i<VNFonts_E1BB_Length;i++)
						{
							if (VNFonts_E1BB[i].value == c3)
							{
								x = VNFonts_E1BB[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 >=0xA0)
					{
						for (int i=0;i<VNFonts_E1BBA0_Length;i++)
						{
							if (VNFonts_E1BBA0[i].value == c3)
							{
								x = VNFonts_E1BBA0[i].offset; break;
							}
						}
					}
				}
				x -=1; //vi VNFonts2 Offset tu 1 -> 0
				if (x>=0)
				{
					if (fvn != NULL)
					{
						setFont(fvn);
						write(x + VNFonts_Offset);
					}
					else
					{
						write(VN2EN[x]);
					}
				}
			p +=y;
		}
		p++;
	}
	rect_using=false;
	if (isScroll)
	{
		scroll_overbuffer = false;
		scroll_length = getCursorX() + 32;
		draw_scrollbuffer = false;
		if (scroll_length>scroll_buff_max) {scroll_length = scroll_buff_max;scroll_overbuffer=true;}
	}
	setyOffsetPlus(0);
}
void RGBmatrixPanel::print_Rect_scroll(frame_struct *fr)
{
	
	//if (fr->isScroll == false) return print_Rect(&fr);
	char *s = fr->text;
	uint8_t color=fr->color;
	uint8_t xoffset=fr->x;
	uint8_t yoffset=fr->y;
	uint8_t width=fr->w;
	uint8_t height=fr->h;
	uint8_t align=fr->align;
	int yOffsetPlus=fr->yOffsetPlus;
	int16_t x_new,x_draw;
	int16_t x=0,y=0,x_upper;
	char c=0,c1,c2,c3;
	uint8_t firstcharwidth=0;
	uint8_t firstcharbytes=0;
	//uint16_t w=96,h=8;
	int16_t fr_cursor_x;
    char *p;//return;
	//String  str="";
	if (fr->scroll_x > xoffset)
	{
		fr->scroll_index=0;
		fr->scroll_pixel=0;
	}
	else if (fr->scroll_x < xoffset) fr->scroll_x = xoffset;
	if (s == NULL)
	{
		fillRect(xoffset,yoffset,width,height,BLACK);
		return;
	}
	setFont(fr->f);
	setTextColor(color);
	//xoffset += fr->scroll_pixel;
	_rwidth = width + xoffset;// - 2*(fr->scroll_pixel); // scroll_pixel<0
	_rheight = height + yoffset;
	_rcolor=color;
	_rxoffset=xoffset;
	_ryoffset=yoffset;
	x_draw=fr->scroll_x + fr->scroll_pixel;
	rect_using=true;
	_ralign=align;
	p=s;
	setyOffsetPlus(yOffsetPlus);
	
	//if (x_draw>xoffset) fillRect(xoffset,yoffset,x_draw-xoffset,height,BLACK);
	fillRect(xoffset,yoffset,width,height,BLACK);	
	setCursor(x_draw,_ryoffset);
	//print(s);
	p=s + fr->scroll_index;
	
	bool firstchar = true;
	fr_cursor_x = xoffset; //x_draw > xoffset ? x_draw : xoffset;
	//fr_cursor_x = 0;
	while (*p != NULL)
	{
		y = 0;
		if (*p <= 0x7F)
		{
			char ch = *p;
			setFont(fr->f);
			if (fr->toUpper) if (( ch >= 'a' ) && ( ch <= 'z' ))  ch &= 0xDF;
			write(ch);
		}
		else
		{
			c1 = *p;
			c2 = *(p+1);
			c3 = *(p+2);
			x = -1;
				if (c1 == 0xC3)
				{
					for (int i=0;i<VNFonts_C3_Length;i++)
					{
						if (VNFonts_C3[i].value == c2)
						{
							x_upper = VNFonts_C3[i].toUpper;
							x = VNFonts_C3[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xC4 || c1 == 0xC6)
				{
					for (int i=0;i<VNFonts_C46_Length;i++)
					{
						if (VNFonts_C46[i].value == c2)
						{
							x_upper = VNFonts_C46[i].toUpper;
							x = VNFonts_C46[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xC5)
				{
					for (int i=0;i<VNFonts_C5_Length;i++)
					{
						if (VNFonts_C5[i].value == c2)
						{
							x_upper = VNFonts_C5[i].toUpper;
							x = VNFonts_C5[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xE1)
				{
					y=2;
					if (c2 == 0xBA)
					{
						for (int i=0;i<VNFonts_E1BA_Length;i++)
						{
							if (VNFonts_E1BA[i].value == c3)
							{
								x_upper = VNFonts_E1BA[i].toUpper;
								x = VNFonts_E1BA[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 <0xA0)
					{
						for (int i=0;i<VNFonts_E1BB_Length;i++)
						{
							if (VNFonts_E1BB[i].value == c3)
							{
								x_upper = VNFonts_E1BB[i].toUpper;
								x = VNFonts_E1BB[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 >=0xA0)
					{
						for (int i=0;i<VNFonts_E1BBA0_Length;i++)
						{
							if (VNFonts_E1BBA0[i].value == c3)
							{
								x_upper = VNFonts_E1BBA0[i].toUpper;
								x = VNFonts_E1BBA0[i].offset; break;
							}
						}
					}
				}
				x_upper -= 1;
				x -=1; //vi VNFonts2 Offset tu 1 -> 0
				if (fr->toUpper) x = x_upper;
				if (x>=0)
				{
					if (fr->fvn != NULL)
					{
						setFont(fr->fvn);
						write(x + VNFonts_Offset);
					}
					else
					{
						write(VN2EN[x]);
					}
				}
		}
		p += y;
		if (firstchar)
		{
			firstchar = false;
			firstcharwidth = fr_cursor_x - cursor_x + 1;
			firstcharbytes = y + 1;
		}
		p++;
		if (cursor_x > _rwidth)
		{
			if (fr->syncScroll && fr->scroll_length ==0) offset_calc = true;
			else break;
		}
	}
	if (fr->syncScroll && fr->scroll_length ==0)
	{
		fr->scroll_length = getCursorX();
		rect_using=false;
		offset_calc = false;
		setyOffsetPlus(0);
		return;
	}
	//Tinh scroll_pixel cho lan tiep theo
	if (fr->scroll_x > xoffset) fr->scroll_x -=1;
	else
	{
		if (firstcharwidth < abs(fr->scroll_pixel))
		{
			fr->scroll_index +=firstcharbytes;
			fr->scroll_pixel = 0;
		}
		else
		{
			fr->scroll_pixel -=1;
		}
	}
	//kiem tra ket thuc chuoi
	if(fr->syncScroll)		
	{
		fr->scroll_position +=1;
		if (fr->scroll_position>=fr->scroll_length)
		{
			fr->scroll_position = 0;
			fr->scroll_x = width + xoffset - 1;
			fr->scroll_index = 0;
			fr->scroll_pixel = 0;
		}
	}
	else if (cursor_x <= xoffset)
	{
		fr->scroll_position = 0;
		fr->scroll_x = width + xoffset - 1;
		fr->scroll_index = 0;
		fr->scroll_pixel = 0;
	}
	rect_using=false;
	setyOffsetPlus(0);
}
void RGBmatrixPanel::print_Rect(frame_struct *fr)
{
	if (fr->isScroll == true) return print_Rect_scroll(fr);
	else return print_Rect(fr->f,fr->fvn,fr->text,fr->color,fr->x,fr->y,fr->w,fr->h,fr->align,fr->isScroll,fr->yOffsetPlus,fr->toUpper);
	
/* 	uint8_t align = fr.align;
	int16_t x_new,x_draw;
	int16_t x=0,y=0;
	char c=0,c1,c2,c3;
	uint16_t w=96,h=8;
    char *p;
	String  str="";
	setFont(fr.f);
	_rcolor=fr.color;
	_rxoffset=fr.x;
	_ryoffset=fr.y;
	_rwidth = fr.w + _rxoffset;
	_rheight = fr.h + _ryoffset;	
	x_draw=_rxoffset;
	rect_using=true;	
	setTextColor(fr.color);
	p=fr.text;
	draw_scrollbuffer = fr.isScroll;
	setyOffsetPlus(fr.yOffsetPlus);
	if (align>0)
	{
		offset_calc = true;
		setCursor(_rxoffset,_ryoffset);
		while (*p != NULL)
		{			
			if (*p <= 0x7F)
			{
				setFont(fr.f);
				write(*p);
			}
			else
			{
				c1 = *p;
				c2 = *(p+1);
				c3 = *(p+2);
				x = -1; y = 0;
				if (c1 == 0xC3)
				{
					for (int i=0;i<VNFonts_C3_Length;i++)
					{
						if (VNFonts_C3[i].value == c2)
						{
							x = VNFonts_C3[i].offset; y=1; break;
						}
					}
				}
				else if (c1 >= 0xC4 && c1 <= 0xC6)
				{
					for (int i=0;i<VNFonts_C456_Length;i++)
					{
						if (VNFonts_C456[i].value == c2)
						{
							x = VNFonts_C456[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xE1)
				{
					y=2;
					if (c2 == 0xBA)
					{
						for (int i=0;i<VNFonts_E1BA_Length;i++)
						{
							if (VNFonts_E1BA[i].value == c3)
							{
								x = VNFonts_E1BA[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 <0xA0)
					{
						for (int i=0;i<VNFonts_E1BB_Length;i++)
						{
							if (VNFonts_E1BB[i].value == c3)
							{
								x = VNFonts_E1BB[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 >=0xA0)
					{
						for (int i=0;i<VNFonts_E1BBA0_Length;i++)
						{
							if (VNFonts_E1BBA0[i].value == c3)
							{
								x = VNFonts_E1BBA0[i].offset; break;
							}
						}
					}
				}
				x -=1;
				if (x>=0)
				{
					if (fr.fvn != NULL)
					{
						setFont(fr.fvn);
						write(x + VNFonts_Offset);
					}
					else
					{
						write(VN2EN[x]);
					}
				}
				p +=y;
			}
			p++;
		}
		w = getCursorX() - _rxoffset;
		if (align == CENTER)
		{
			x_draw = (fr.w-w)/2 + _rxoffset;
		}
		else if (align == RIGHT)
		{
			x_draw = _rwidth-w;
		}
		if (x_draw < _rxoffset) x_draw = _rxoffset;
		offset_calc = false;
	}
	if (fr.isScroll)
	{
		for (int i=2;i<buff_count;i++) memset(matrixbuff[i], BLACK, WidthScan * nRows* 4);
		setCursor(x_draw,0);
	}
	else
	{
		fillRect(_rxoffset,_ryoffset,fr.w,fr.h,BLACK);
		setCursor(x_draw,_ryoffset);
	}
	//print(s);
	p=fr.text;
	while (*p != NULL)
	{
		if (*p <= 0x7F)
		{
			setFont(fr.f);
			write(*p);
		}
		else
		{
			c1 = *p;
			c2 = *(p+1);
			c3 = *(p+2);
			x = -1; y = 0;
				if (c1 == 0xC3)
				{
					for (int i=0;i<VNFonts_C3_Length;i++)
					{
						if (VNFonts_C3[i].value == c2)
						{
							x = VNFonts_C3[i].offset; y=1; break;
						}
					}
				}
				else if (c1 >= 0xC4 && c1 <= 0xC6)
				{
					for (int i=0;i<VNFonts_C456_Length;i++)
					{
						if (VNFonts_C456[i].value == c2)
						{
							x = VNFonts_C456[i].offset; y=1; break;
						}
					}
				}
				else if (c1 == 0xE1)
				{
					y=2;
					if (c2 == 0xBA)
					{
						for (int i=0;i<VNFonts_E1BA_Length;i++)
						{
							if (VNFonts_E1BA[i].value == c3)
							{
								x = VNFonts_E1BA[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 <0xA0)
					{
						for (int i=0;i<VNFonts_E1BB_Length;i++)
						{
							if (VNFonts_E1BB[i].value == c3)
							{
								x = VNFonts_E1BB[i].offset; break;
							}
						}
					}
					else if (c2 == 0xBB && c3 >=0xA0)
					{
						for (int i=0;i<VNFonts_E1BBA0_Length;i++)
						{
							if (VNFonts_E1BBA0[i].value == c3)
							{
								x = VNFonts_E1BBA0[i].offset; break;
							}
						}
					}
				}
				x -=1; //vi VNFonts2 Offset tu 1 -> 0
				if (x>=0)
				{
					if (fr.fvn != NULL)
					{
						setFont(fr.fvn);
						write(x + VNFonts_Offset);
					}
					else
					{
						write(VN2EN[x]);
					}
				}
			p +=y;
		}
		p++;
	}
	rect_using=false;
	if (fr.isScroll)
	{
		scroll_overbuffer = false;
		scroll_length = getCursorX() + 32;
		draw_scrollbuffer = false;
		if (scroll_length<_width) scroll_length = _width;
		if (scroll_length>scroll_buff_max) {scroll_length = scroll_buff_max;scroll_overbuffer=true;}
	}
	setyOffsetPlus(0); */
}
uint8_t RGBmatrixPanel::getPixel_scrollbuffer(int16_t x, int16_t y)
{
  bool r, g;
  uint8_t c;
  uint32_t  *ptr;
  uint32_t Mask01=0x01;

  if((x < 0) || (x >= scroll_buff_max) || (y < 0) || (y >= 32)) return 0;

	//r
	ptr = &matrixbuff[2][x*2]; // Base addr
	r =  *ptr & (Mask01<<y);
	//g
	ptr++;// = &matrixbuff[2][x*2 + 1]; // Base addr
	g =  *ptr & (Mask01<<y);
  c=0;
  if (r) c+=1;
  if (g) c+=2;
  return c;
}
uint8_t RGBmatrixPanel::getPixel(int16_t x, int16_t y)
{
  uint8_t panel;
  uint8_t c=0;
  uint32_t  *ptr;
  bool r,g;
  uint32_t Mask01=0x01;
  
  if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return c;
  
  panel=1;
  if (y>=64)
  {
	  panel=5;
	  y-=64;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //6
		  x -=WidthScan;
	  }
  }
  else if (y>=32)
  {
	  panel=3;
	  y-=32;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //4
		  x -=WidthScan;
	  }
  }
  else if (x>=WidthScan) 
  {
	  panel+=1; //2
	  x -=WidthScan;
  }
  
  if(y < nRows)
  {
	  ptr = &matrixbuff[backindex][y * WidthScan * nPlanes + x]; // Base addr
	  if (panel==3) {		  
		// r = (*ptr & (Mask01<<1) == 0) ? false : true;
		// g = (*ptr & (Mask01<<3) == 0) ? false : true;
		r = *ptr & (Mask01<<6);
		g = *ptr & (Mask01<<7);
	  }
	  else if (panel==4) {
		r = *ptr & (Mask01<<6);
		g = *ptr & (Mask01<<7);
	  }
	  else if (panel==1) {
		r = *ptr & (Mask01<<13);
		g = *ptr & (Mask01<<12);
	  }
	  else if (panel==2) {
		r = *ptr & (Mask01<<17);
		g = *ptr & (Mask01<<16);
	  }
	  else if (panel==5) {
		r = *ptr & (Mask01<<21);
		g = *ptr & (Mask01<<22);
	  }
	  else if (panel==6) {
		r = *ptr & (Mask01<<25);
		g = *ptr & (Mask01<<28);
	  }
  }
  else {
	  ptr = &matrixbuff[backindex][(y - nRows) * WidthScan * nPlanes + x];
	  if (panel==3) {
		r = *ptr & (Mask01<<2);
		g = *ptr & (Mask01<<5);
	  }
	  else if (panel==4) {
		r = *ptr & (Mask01<<6);
		g = *ptr & (Mask01<<9);
	  }
	  else if (panel==1) {
		r = *ptr & (Mask01<<14);
		g = *ptr & (Mask01<<15);
	  }
	  else if (panel==2) {
		r = *ptr & (Mask01<<18);
		g = *ptr & (Mask01<<19);
	  }
	  else if (panel==5) {
		r = *ptr & (Mask01<<23);
		g = *ptr & (Mask01<<24);
	  }
	  else if (panel==6) {
		r = *ptr & (Mask01<<26);
		g = *ptr & (Mask01<<29);
	  }
  }
  c=0;
  if (r) c+=1;
  if (g) c+=2;
  return c;
}
void RGBmatrixPanel::drawPixel(int16_t x, int16_t y, uint16_t c) {
  uint8_t r, g, b, panel;
  uint32_t  *ptr;
  uint32_t Mask11=0x03;
  uint32_t Mask01=0x01;
//offset_calc = true // dung de tinh offset khong ve len bang led
if (offset_calc) return;
// if (draw_scrollbuffer)
// {
	// drawPixel_Scrollbuffer(x,y,c);
	// return;
// }
if (rect_using)
{
	if((x < _rxoffset) || (x >= _rwidth) || (y < _ryoffset) || (y >= _rheight)) return;
}

 if (Test_board)
 {
	 drawPixel_Test_board(x,y,c);
	 return;
 }
  if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  // switch(rotation) {
   // case 1:
    // _swap_int16_t(x, y);
    // x = WIDTH  - 1 - x;
    // break;
   // case 2:
    // x = WIDTH  - 1 - x;
    // y = HEIGHT - 1 - y;
    // break;
   // case 3:
    // _swap_int16_t(x, y);
    // y = HEIGHT - 1 - y;
    // break;
  // }

  // Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
  // 4/4/4.  Pluck out relevant bits while separating into R,G,B:
  r =  (c & RED);        // RRRRrggggggbbbbb
  g = (c & GREEN); // rrrrrGGGGggbbbbb
  b = 0;//(c >>  1) & 0xF; // rrrrrggggggBBBBb

  // Loop counter stuff
  // bit   = 1;
  // limit = 1 << nPlanes;
  panel=1;
  if (y>=64)
  {
	  panel=5;
	  y-=64;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //6
		  x -=WidthScan;
	  }
  }
  else if (y>=32)
  {
	  panel=3;
	  y-=32;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //4
		  x -=WidthScan;
	  }
  }
  else if (x>=WidthScan) 
  {
	  panel+=1; //2
	  x -=WidthScan;
  }
  
  if(y < nRows)
  {
	  ptr = &matrixbuff[backindex][y * WidthScan * nPlanes + x]; // Base addr
	  if (panel==3) {
		       *ptr &= ~(Mask11<<1);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<1); // Plane N R: bit 1
		  if(g) *ptr |= (Mask01<<2); // Plane N G: bit 2
	  }
	  else if (panel==4) {
			   *ptr &= ~(Mask11<<5);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<5); // Plane N R: bit 5
		  if(g) *ptr |= (Mask01<<6); // Plane N G: bit 6
	  }
	  else if (panel==1) {
			   *ptr &= ~(Mask11<<12);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<12); // Plane N R: bit 12
		  if(g) *ptr |= (Mask01<<13); // Plane N G: bit 13
	  }
	  else if (panel==2) {
			   *ptr &= ~(Mask11<<16);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<16); // Plane N R: bit 16
		  if(g) *ptr |= (Mask01<<17); // Plane N G: bit 17
	  }
	  else if (panel==5) {
			   *ptr &= ~(Mask11<<21);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<21); // Plane N R: bit 21
		  if(g) *ptr |= (Mask01<<22); // Plane N G: bit 22
	  }
	  else if (panel==6) {
			   *ptr &= ~(Mask11<<25);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<25); // Plane N R: bit 25
		  if(g) *ptr |= (Mask01<<26); // Plane N G: bit 26
	  }
  }
  else {
	  ptr = &matrixbuff[backindex][(y - nRows) * WidthScan * nPlanes + x];
	  if (panel==3) {
		       *ptr &= ~(Mask11<<3);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<3); // Plane N R: bit 3
		  if(g) *ptr |= (Mask01<<4); // Plane N G: bit 4
	  }
	  else if (panel==4) {
			   *ptr &= ~(Mask11<<7);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<7); // Plane N R: bit 7
		  if(g) *ptr |= (Mask01<<8); // Plane N G: bit 8
	  }
	  else if (panel==1) {
			   *ptr &= ~(Mask11<<14);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<14); // Plane N R: bit 14
		  if(g) *ptr |= (Mask01<<15); // Plane N G: bit 15
	  }
	  else if (panel==2) {
			   *ptr &= ~(Mask11<<18);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<18); // Plane N R: bit 18
		  if(g) *ptr |= (Mask01<<19); // Plane N G: bit 19
	  }
	  else if (panel==5) {
			   *ptr &= ~(Mask11<<23);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<23); // Plane N R: bit 23
		  if(g) *ptr |= (Mask01<<24); // Plane N G: bit 24
	  }
	  else if (panel==6) {
			   *ptr &= ~(Mask11<<28);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<28); // Plane N R: bit 28
		  if(g) *ptr |= (Mask01<<29); // Plane N G: bit 29
	  }
  }
}
void RGBmatrixPanel::drawPixel_Test_board(int16_t x, int16_t y, uint16_t c) {
  uint8_t r, g, b, bit, limit,panel;
  uint32_t  *ptr;
  uint32_t Mask11=0x03;
  uint32_t Mask01=0x01;

  if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  switch(rotation) {
   case 1:
    _swap_int16_t(x, y);
    x = WIDTH  - 1 - x;
    break;
   case 2:
    x = WIDTH  - 1 - x;
    y = HEIGHT - 1 - y;
    break;
   case 3:
    _swap_int16_t(x, y);
    y = HEIGHT - 1 - y;
    break;
  }

  // Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
  // 4/4/4.  Pluck out relevant bits while separating into R,G,B:
  r =  (c & RED);        // RRRRrggggggbbbbb
  g = (c & GREEN); // rrrrrGGGGggbbbbb
  b = 0;//(c >>  1) & 0xF; // rrrrrggggggBBBBb

  // Loop counter stuff
  bit   = 1;
  limit = 1 << nPlanes;
  panel=1;
  if (y>=64)
  {
	  panel=5;
	  y-=64;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //6
		  x -=WidthScan;
	  }
  }
  else if (y>=32)
  {
	  panel=3;
	  y-=32;
	  if (x>=WidthScan) 
	  {
		  panel+=1; //4
		  x -=WidthScan;
	  }
  }
  else if (x>=WidthScan) 
  {
	  panel+=1; //2
	  x -=WidthScan;
  }
  
  if(y < nRows)
  {
	  ptr = &matrixbuff[backindex][y * WidthScan * nPlanes + x]; // Base addr
	  if (panel==3) {
		       //*ptr &= ~(Mask11<<1);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<1); // Plane N R: bit 1
		  else  *ptr &= ~(Mask01<<1);
		  if(g) *ptr |= (Mask01<<3); // Plane N G: bit 2
		  else  *ptr &= ~(Mask01<<3);
	  }
	  else if (panel==4) {
			  // *ptr &= ~(Mask11<<5);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<4); // Plane N R: bit 5
		  else  *ptr &= ~(Mask01<<4);
		  if(g) *ptr |= (Mask01<<7); // Plane N G: bit 6
		  else  *ptr &= ~(Mask01<<7);
	  }
	  else if (panel==1) {
			   *ptr &= ~(Mask11<<12);// Mask out R,G,B in one op
		  if(g) *ptr |= (Mask01<<12); // Plane N R: bit 12
		  if(r) *ptr |= (Mask01<<13); // Plane N G: bit 13
	  }
	  else if (panel==2) {
			   *ptr &= ~(Mask11<<16);// Mask out R,G,B in one op
		  if(g) *ptr |= (Mask01<<16); // Plane N R: bit 16
		  if(r) *ptr |= (Mask01<<17); // Plane N G: bit 17
	  }
	  else if (panel==5) {
			   *ptr &= ~(Mask11<<21);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<21); // Plane N R: bit 21
		  if(g) *ptr |= (Mask01<<22); // Plane N G: bit 22
	  }
	  else if (panel==6) {
			   //*ptr &= ~(Mask11<<25);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<25); // Plane N R: bit 25
		  else  *ptr &= ~(Mask01<<25);
		  if(g) *ptr |= (Mask01<<28); // Plane N G: bit 26
		  else  *ptr &= ~(Mask01<<28);
	  }
  }
  else {
	  ptr = &matrixbuff[backindex][(y - nRows) * WidthScan * nPlanes + x];
	  if (panel==3) {
		      // *ptr &= ~(Mask11<<3);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<2); // Plane N R: bit 3
		  else  *ptr &= ~(Mask01<<2);
		  if(g) *ptr |= (Mask01<<5); // Plane N G: bit 4
		  else  *ptr &= ~(Mask01<<5);
	  }
	  else if (panel==4) {
			  // *ptr &= ~(Mask11<<7);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<6); // Plane N R: bit 7
		  else  *ptr &= ~(Mask01<<6);
		  if(g) *ptr |= (Mask01<<9); // Plane N G: bit 8
		  else  *ptr &= ~(Mask01<<9);
	  }
	  else if (panel==1) {
			   *ptr &= ~(Mask11<<14);// Mask out R,G,B in one op
		  if(g) *ptr |= (Mask01<<14); // Plane N R: bit 14
		  if(r) *ptr |= (Mask01<<15); // Plane N G: bit 15
	  }
	  else if (panel==2) {
			   *ptr &= ~(Mask11<<18);// Mask out R,G,B in one op
		  if(g) *ptr |= (Mask01<<18); // Plane N R: bit 18
		  if(r) *ptr |= (Mask01<<19); // Plane N G: bit 19
	  }
	  else if (panel==5) {
			   *ptr &= ~(Mask11<<23);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<23); // Plane N R: bit 23
		  if(g) *ptr |= (Mask01<<24); // Plane N G: bit 24
	  }
	  else if (panel==6) {
		//	   *ptr &= ~(Mask11<<26);// Mask out R,G,B in one op
		  if(r) *ptr |= (Mask01<<26); // Plane N R: bit 28
		  else  *ptr &= ~(Mask01<<26);
		  if(g) *ptr |= (Mask01<<29); // Plane N G: bit 29
		  else  *ptr &= ~(Mask01<<29);
	  }
  }
}
void RGBmatrixPanel::drawPixel_Scrollbuffer(int16_t x, int16_t y, uint16_t c) {
  uint8_t r, g, b,panel;
  uint32_t  *ptr;
  uint32_t Mask11=0x03;
  uint32_t Mask01=0x01;
  int16_t x_max = 64 * 6 * (buff_count-2);

  if((x < 0) || (x >= x_max) || (y < 0) || (y >= 32) || !draw_scrollbuffer) return;

  r =  (c & RED);   // r luu vao bit 0-15
  g = (c & GREEN); //  g luu vao bit 16-31
	//r
	ptr = &matrixbuff[2][x*2]; // Base addr
	if(r) *ptr |= (Mask01<<y);
	else  *ptr &= ~(Mask01<<y);
	//g
	ptr = &matrixbuff[2][x*2 + 1]; // Base addr
	if(g) *ptr |= (Mask01<<y);
	else  *ptr &= ~(Mask01<<y);
}
void RGBmatrixPanel::fillScreen(uint16_t c) {
  if((c == 0x0000) || (c == 0xffff)) {
    // For black or white, all bits in frame buffer will be identically
    // set or unset (regardless of weird bit packing), so it's OK to just
    // quickly memset the whole thing:
    memset(matrixbuff[backindex], c, WidthScan * nRows* 4 * buff_count); // 4=4bytes -> 1dword
  } else {
    // Otherwise, need to handle it the long way:
    Adafruit_GFX::fillScreen(c);
  }
}

// Return address of back buffer -- can then load/store data directly
uint32_t *RGBmatrixPanel::backBuffer() {
  return matrixbuff[backindex];
}

// For smooth animation -- drawing always takes place in the "back" buffer;
// this method pushes it to the "front" for display.  Passing "true", the
// updated display contents are then copied to the new back buffer and can
// be incrementally modified.  If "false", the back buffer then contains
// the old front buffer contents -- your code can either clear this or
// draw over every pixel.  (No effect if double-buffering is not enabled.)
void RGBmatrixPanel::swapBuffers(bool copy) {
  if(matrixbuff[0] != matrixbuff[1]) {
    // To avoid 'tearing' display, actual swap takes place in the interrupt
    // handler, at the end of a complete screen refresh cycle.
    swapflag = true;                  // Set flag here, then...
    while(swapflag == true) delay(1); // wait for interrupt to clear it
    if(copy == true)
      memcpy(matrixbuff[backindex], matrixbuff[1-backindex], WidthScan * nRows * 4);
  }
}

// Dump display contents to the Serial Monitor, adding some formatting to
// simplify copy-and-paste of data as a PROGMEM-embedded image for another
// sketch.  If using multiple dumps this way, you'll need to edit the
// output to change the 'img' name for each.  Data can then be loaded
// back into the display using a pgm_read_byte() loop.
void RGBmatrixPanel::dumpMatrix(void) {

  int i, buffsize = WidthScan * nRows * nPlanes;
  Serial.print(matrixbuff[0][0],HEX); Serial.print(" ");
  Serial.println(matrixbuff[1][0],HEX);
  return;
//buffsize=64;
  Serial.print(F("\n\n"
    "matrixbuff[] = {\n  "));

  for(i=0; i<buffsize; i++) {
    Serial.print(F("0x"));
    if(matrixbuff[backindex][i] < 0x10) Serial.write('0');
    Serial.print(matrixbuff[backindex][i],HEX);
    if(i < (buffsize - 1)) {
      if((i & 7) == 7) Serial.print(F(",\n  "));
      else             Serial.write(',');
    }
  }
  Serial.println(F("\n};"));
}

// -------------------- Interrupt handler stuff --------------------

// ISR(TIMER1_OVF_vect, ISR_BLOCK) { // ISR_BLOCK important -- see notes later
  // activePanel->updateDisplay();   // Call refresh func for active display
  // TIFR1 |= TOV1;                  // Clear Timer1 interrupt flag
// }
void TC3_Handler()
{
	activePanel->updateDisplay(); 
	TC_GetStatus(TC1, 0);
}
uint8_t RGBmatrixPanel::Set_brightness(uint8_t percent)
{
	if (percent>brightness_Max) percent=brightness_Max;
	brightness = percent;
	return brightness;
}
uint8_t RGBmatrixPanel::Get_brightness()
{
	return brightness;
}
void RGBmatrixPanel::updateDisplay(void) {
  uint8_t  i;
 
 if (oe_count<brightness) oe_enable=true;
	else oe_enable=false;
	if (oe_count--<=0) oe_count=brightness_Max;
	
	 oeSet();
	 latSet();

 
    if(++row >= nRows) {        // advance row counter.  Maxed out?
      row     = 0;              // Yes, reset row counter, then...
      if(swapflag == true) {    // Swap front/back buffers if requested
        backindex = 1 - backindex;
        swapflag  = false;
      }
      buffptr = matrixbuff[1-backindex]; // Reset into front buffer
    }
    // Plane 0 was loaded on prior interrupt invocation and is about to
    // latch now, so update the row address lines before we do that:
    if(row & 0x1) {aSet();}
	else {aCLear();}  //*addraport &= ~addrapin;
	if(row & 0x2) {bSet();}
	else {bCLear();}
	if(row & 0x4) {cSet();}
	else {cCLear();}
	if(row & 0x8) {dSet();}
	else {dCLear();}
  
//latSet();
  // buffptr, being 'volatile' type, doesn't take well to optimization.
  // A local register copy can speed some things up:
  //ptr = (uint8_t *)buffptr;	
	
    for(i=0; i<WidthScan; i++) {
		DATAPORT = *buffptr++;
	  clkSet();
	  clkCLear();
    }
	latCLear();
	if (oe_enable) oeCLear();
}

