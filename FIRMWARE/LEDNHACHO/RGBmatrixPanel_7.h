#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
 #include "pins_arduino.h"
#endif
#include "Adafruit_GFX.h"
#include "Config.h"

#define Test_board 0

#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLACK 0

#define LEFT 0
#define CENTER 1
#define RIGHT 2

#define  buff_count 2

class RGBmatrixPanel : public Adafruit_GFX {

 public:
	int16_t scroll_length,scroll_buff_max;
	bool scroll_overbuffer;
  // Constructor for 16x32 panel:
  RGBmatrixPanel(uint8_t a, uint8_t b, uint8_t c,
    uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf);

  // Constructor for 32x32 panel (adds 'd' pin):
  RGBmatrixPanel(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
    uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf, uint8_t width=64,uint8_t height=32);
  
  uint8_t getPixel(int16_t x, int16_t y),
			getPixel_scrollbuffer(int16_t x, int16_t y);
  void
    begin(void),	
    drawPixel(int16_t x, int16_t y, uint16_t c),
	drawPixel_Test_board(int16_t x, int16_t y, uint16_t c),
	drawPixel_Scrollbuffer(int16_t x, int16_t y, uint16_t c),
	print_Rect_scroll(frame_struct *fr),
	print_Rect(frame_struct *fr),
    fillScreen(uint16_t c),
    updateDisplay(void),
    swapBuffers(bool),
    dumpMatrix(void);
  uint8_t Set_brightness(uint8_t percent);
  uint8_t Get_brightness();
  uint32_t
    *backBuffer(void);
  uint16_t
    Color333(uint8_t r, uint8_t g, uint8_t b),
    Color444(uint8_t r, uint8_t g, uint8_t b),
    Color888(uint8_t r, uint8_t g, uint8_t b),
    Color888(uint8_t r, uint8_t g, uint8_t b, bool gflag),
    ColorHSV(long hue, uint8_t sat, uint8_t val, bool gflag);

  void print_Rect(const GFXfont *f,const GFXfont *fvn,char *s,uint8_t color,uint8_t xoffset,uint8_t yoffset,
					uint8_t width,uint8_t height,uint8_t align=0, bool isScroll=false,int yOffsetPlus=0,bool toUpper=false);
 private:
  bool rect_using, offset_calc, draw_scrollbuffer;
  uint8_t _rcolor,_rxoffset,_ryoffset,_rwidth,_rheight,_ralign;
  
  uint32_t         *matrixbuff[2];
  uint8_t          nRows;
  volatile uint8_t backindex;
  volatile bool swapflag;

  // Init/alloc code common to both constructors:
  void init(uint8_t rows, uint8_t a, uint8_t b, uint8_t c,
	    uint8_t sclk, uint8_t latch, uint8_t oe, bool dbuf, 
	    uint8_t width);
// void startTimer(Tc *tc, uint32_t channel, IRQn_Type irq);
// void TC3_Handler();
  // PORT register pointers, pin bitmasks, pin numbers:
  volatile uint32_t
    *latport, *oeport, *addraport, *addrbport, *addrcport, *addrdport;
  uint32_t
    sclkpin, latpin, oepin, addrapin, addrbpin, addrcpin, addrdpin,
    _sclk, _latch, _oe, _a, _b, _c, _d;
  // Counters/pointers for interrupt handler:
  volatile uint8_t row, plane;  
   volatile bool oe_enable;
   volatile int oe_count;
   volatile uint8_t brightness; //16 level
  volatile uint32_t *buffptr;
};

