/*
 * Display control module for ILI9341
 *
 * (C) 2016 Luigi Di Fraia
 */

#include "Arduino.h"
#include "SPI.h"
#include "ILI9341.h"
#include "Fonts.h"

#define CS_LOW()      digitalWrite(_cs, LOW)
#define CS_HIGH()     digitalWrite(_cs, HIGH)
#define RESET_LOW()   digitalWrite(_reset, LOW)
#define RESET_HIGH()  digitalWrite(_reset, HIGH)
#define DC_LOW()      digitalWrite(_dc, LOW)
#define DC_HIGH()     digitalWrite(_dc, HIGH)

/* Level 1 Commands (from the display Datasheet) */
#define ILI9341_CMD_NOP                             0x00
#define ILI9341_CMD_SOFTWARE_RESET                  0x01
#define ILI9341_CMD_READ_DISP_ID                    0x04
#define ILI9341_CMD_READ_DISP_STATUS                0x09
#define ILI9341_CMD_READ_DISP_POWER_MODE            0x0A
#define ILI9341_CMD_READ_DISP_MADCTRL               0x0B
#define ILI9341_CMD_READ_DISP_PIXEL_FORMAT          0x0C
#define ILI9341_CMD_READ_DISP_IMAGE_FORMAT          0x0D
#define ILI9341_CMD_READ_DISP_SIGNAL_MODE           0x0E
#define ILI9341_CMD_READ_DISP_SELF_DIAGNOSTIC       0x0F
#define ILI9341_CMD_ENTER_SLEEP_MODE                0x10
#define ILI9341_CMD_SLEEP_OUT                       0x11
#define ILI9341_CMD_PARTIAL_MODE_ON                 0x12
#define ILI9341_CMD_NORMAL_DISP_MODE_ON             0x13
#define ILI9341_CMD_DISP_INVERSION_OFF              0x20
#define ILI9341_CMD_DISP_INVERSION_ON               0x21
#define ILI9341_CMD_GAMMA_SET                       0x26
#define ILI9341_CMD_DISPLAY_OFF                     0x28
#define ILI9341_CMD_DISPLAY_ON                      0x29
#define ILI9341_CMD_COLUMN_ADDRESS_SET              0x2A
#define ILI9341_CMD_PAGE_ADDRESS_SET                0x2B
#define ILI9341_CMD_MEMORY_WRITE                    0x2C
#define ILI9341_CMD_COLOR_SET                       0x2D
#define ILI9341_CMD_MEMORY_READ                     0x2E
#define ILI9341_CMD_PARTIAL_AREA                    0x30
#define ILI9341_CMD_VERT_SCROLL_DEFINITION          0x33
#define ILI9341_CMD_TEARING_EFFECT_LINE_OFF         0x34
#define ILI9341_CMD_TEARING_EFFECT_LINE_ON          0x35
#define ILI9341_CMD_MEMORY_ACCESS_CONTROL           0x36
#define ILI9341_CMD_VERT_SCROLL_START_ADDRESS       0x37
#define ILI9341_CMD_IDLE_MODE_OFF                   0x38
#define ILI9341_CMD_IDLE_MODE_ON                    0x39
#define ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET         0x3A
#define ILI9341_CMD_WRITE_MEMORY_CONTINUE           0x3C
#define ILI9341_CMD_READ_MEMORY_CONTINUE            0x3E
#define ILI9341_CMD_SET_TEAR_SCANLINE               0x44
#define ILI9341_CMD_GET_SCANLINE                    0x45
#define ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS        0x51
#define ILI9341_CMD_READ_DISPLAY_BRIGHTNESS         0x52
#define ILI9341_CMD_WRITE_CTRL_DISPLAY              0x53
#define ILI9341_CMD_READ_CTRL_DISPLAY               0x54
#define ILI9341_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS  0x55
#define ILI9341_CMD_READ_CONTENT_ADAPT_BRIGHTNESS   0x56
#define ILI9341_CMD_WRITE_MIN_CAB_LEVEL             0x5E
#define ILI9341_CMD_READ_MIN_CAB_LEVEL              0x5F
#define ILI9341_CMD_READ_ID1                        0xDA
#define ILI9341_CMD_READ_ID2                        0xDB
#define ILI9341_CMD_READ_ID3                        0xDC

/* Level 2 Commands (from the display Datasheet) */
#define ILI9341_CMD_RGB_SIGNAL_CONTROL              0xB0
#define ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL       0xB1
#define ILI9341_CMD_FRAME_RATE_CONTROL_IDLE_8COLOR  0xB2
#define ILI9341_CMD_FRAME_RATE_CONTROL_PARTIAL      0xB3
#define ILI9341_CMD_DISPLAY_INVERSION_CONTROL       0xB4
#define ILI9341_CMD_BLANKING_PORCH_CONTROL          0xB5
#define ILI9341_CMD_DISPLAY_FUNCTION_CONTROL        0xB6
#define ILI9341_CMD_ENTRY_MODE_SET                  0xB7
#define ILI9341_CMD_BACKLIGHT_CONTROL_1             0xB8
#define ILI9341_CMD_BACKLIGHT_CONTROL_2             0xB9
#define ILI9341_CMD_BACKLIGHT_CONTROL_3             0xBA
#define ILI9341_CMD_BACKLIGHT_CONTROL_4             0xBB
#define ILI9341_CMD_BACKLIGHT_CONTROL_5             0xBC
#define ILI9341_CMD_BACKLIGHT_CONTROL_6             0xBD
#define ILI9341_CMD_BACKLIGHT_CONTROL_7             0xBE
#define ILI9341_CMD_BACKLIGHT_CONTROL_8             0xBF
#define ILI9341_CMD_POWER_CONTROL_1                 0xC0
#define ILI9341_CMD_POWER_CONTROL_2                 0xC1
#define ILI9341_CMD_VCOM_CONTROL_1                  0xC5
#define ILI9341_CMD_VCOM_CONTROL_2                  0xC7
#define ILI9341_CMD_NVMEM_WRITE                     0xD0
#define ILI9341_CMD_NVMEM_PROTECTION_KEY            0xD1
#define ILI9341_CMD_NVMEM_STATUS_READ               0xD2
#define ILI9341_CMD_READ_ID4                        0xD3
#define ILI9341_CMD_POSITIVE_GAMMA_CORRECTION       0xE0
#define ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION       0xE1
#define ILI9341_CMD_DIGITAL_GAMMA_CONTROL_1         0xE2
#define ILI9341_CMD_DIGITAL_GAMMA_CONTROL_2         0xE3
#define ILI9341_CMD_INTERFACE_CONTROL               0xF6

#define CMD_WRB(c)  { DC_LOW(); SPI.transfer(c); DC_HIGH(); } /* Write a command to the OLED */
#define DATA_WRB(d) { SPI.transfer(d); }  /* Write a byte to the OLED */
#define DATA_WRW(d) { SPI.transfer(d>>8); SPI.transfer(d); }  /* Write a word to the OLED */
#define DATA_WPX(d) { SPI.transfer(d>>8); SPI.transfer(d); }  /* Write a pixel to the OLED */

void ILI9341::setrect (
  int left,       /* Left end (0..DISP_XS-1) */
  int right,      /* Right end (0..DISP_XS-1, >= left) */
  int top,        /* Top end (0..DISP_YS-1) */
  int bottom      /* Bottom end (0..DISP_YS-1, >= top) */
)
{
  CS_LOW();          /* Select display */

  CMD_WRB(ILI9341_CMD_COLUMN_ADDRESS_SET);    /* Set H range */
  DATA_WRW(left); DATA_WRW(right);

  CMD_WRB(ILI9341_CMD_PAGE_ADDRESS_SET);  /* Set V range */
  DATA_WRW(top); DATA_WRW(bottom);

  CMD_WRB(ILI9341_CMD_MEMORY_WRITE);  /* Ready to receive pixel data */
}

void ILI9341::init (void)
{
  static const PROGMEM uint8_t ili9341[] = {
    2, ILI9341_CMD_POWER_CONTROL_1, 0x25,       /* Set the GVDD level to 4.70 V */
    2, ILI9341_CMD_POWER_CONTROL_2, 0x11,       /* Set factor used in the step-up circuits */
    3, ILI9341_CMD_VCOM_CONTROL_1, 0x5C, 0x4C,  /* Set the VCOMH/VXOML voltage */
    2, ILI9341_CMD_VCOM_CONTROL_2, 0x94,        /* Set the VCOM offset voltage */
    2, ILI9341_CMD_MEMORY_ACCESS_CONTROL, 0x08 | (1 << 6) |  /* Define read/write scanning direction of frame memory */
        ((DISP_LANDSCAPE << 5) | (DISP_LANDSCAPE << 7)),
    2, ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET, 0x05,   /* Set the pixel format for the RGB image data to 16 bits / pixel */
    3, ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL, 0x00, 0x18,   /* Set the division ratio to 1 and 24 clocks per line */
    1, ILI9341_CMD_SLEEP_OUT,                   /* Turn off sleep mode */
    0,
    ILI9341_CMD_DISPLAY_ON                      /* Display on */
  };
  const uint8_t *p;
  uint8_t cmd;
  int n, i;


  /* Initialize display module control port */
  pinMode(_cs, OUTPUT);
  pinMode(_reset, OUTPUT);
  pinMode(_dc, OUTPUT);

  CS_HIGH();
  RESET_HIGH();

  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE3);

  CS_LOW();          /* Select display */

  /* Reset display module */
  delay(10);
  RESET_LOW();
  delay(10);
  RESET_HIGH();
  delay(150);

  /* Send initialization data */
  p = ili9341;
  while ((n = pgm_read_byte(p++)) != 0) {
    cmd = pgm_read_byte(p++); n--;
    CMD_WRB(cmd);
    for (i = 0; i < n; i++) DATA_WRB(pgm_read_byte(p++));
  }

  delay(150);

  /* Set initial orientation for get_width()/get_height() */
  Orientation = DISP_LANDSCAPE ? 3 : 0;

  /* Clear screen */
  setmask(0, get_width() - 1, 0, get_height() - 1);
  rectfill(0, get_width() - 1, 0, get_height() - 1, C_BLACK);

  CS_LOW();          /* Select display */

  /* Display ON */
  CMD_WRB(pgm_read_byte(p));

  CS_HIGH();          /* Release display */

  /* Register text fonts */
  font_face(FontH8);

  /* Reset current position */
  moveto(0, 0);
}

void ILI9341::set_orientation (uint8_t o)
{
  if (o > 3) return;

  Orientation = o;

  CS_LOW();          /* Select display */

  CMD_WRB(ILI9341_CMD_MEMORY_ACCESS_CONTROL);    /* Scanning direction of frame memory */

  switch (Orientation) {
  case 0:
    DATA_WRB(0x08 | (1 << 6));
    break;
  case 1:
    DATA_WRB(0x08 | (1 << 5));
    break;
  case 2:
    DATA_WRB(0x08 | (1 << 7));
    break;
  case 3:
    DATA_WRB(0x08 | (1 << 7) | (1 << 6) | (1 << 5));
    break;
  }

  CS_HIGH();          /* Release display */

  /* Reset mask */
  setmask(0, get_width() - 1, 0, get_height() - 1);

  /* Reset current position */
  moveto(0, 0);
}

int ILI9341::get_width (void)
{
  return ((Orientation & 1) ? 320 : 240);
}

int ILI9341::get_height (void)
{
  return ((Orientation & 1) ? 240 : 320);
}

int ILI9341::get_font_width (void)
{
  if (FontS) return pgm_read_byte(&FontS[14]);
  else return 0;
}

int ILI9341::get_font_height (void)
{
  if (FontS) return pgm_read_byte(&FontS[15]);
  else return 0;
}

void ILI9341::set_scroll_def (
  int tfa,        /* Top Fixed Area */
  int vsa,        /* Vertical Scrolling Area */
  int bfa         /* Bottom Fixed Area */
)
{
  CS_LOW();          /* Select display */

  CMD_WRB(ILI9341_CMD_VERT_SCROLL_DEFINITION);    /* Set Vertical Scroll Definition */
  DATA_WRW(tfa);
  DATA_WRW(vsa);
  DATA_WRW(bfa);

  CS_HIGH();          /* Release display */
}

void ILI9341::set_scroll_start (
  int vsp         /* Vertical Scroll Start Address */
)
{
  CS_LOW();          /* Select display */

  CMD_WRB(ILI9341_CMD_VERT_SCROLL_START_ADDRESS); /* Set Vertical Scroll Start Address */
  DATA_WRW(vsp);

  CS_HIGH();          /* Release display */
}

void ILI9341::setmask (
  int left,       /* Left end of active window (0..DISP_XS-1) */
  int right,      /* Right end of active window (0..DISP_XS-1, >=left) */
  int top,        /* Top end of active window (0..DISP_YS-1) */
  int bottom      /* Bottom end of active window (0..DISP_YS-1, >=top) */
)
{
  if (left >= 0 && right < get_width() && left <= right && top >= 0 && bottom < get_height() && top <= bottom) {
    MaskL = left;
    MaskR = right;
    MaskT = top;
    MaskB = bottom;
  }
}

void ILI9341::rectfill (
  int left,       /* Left end (-32768..32767) */
  int right,      /* Right end (-32768..32767, >=left) */
  int top,        /* Top end (-32768..32767) */
  int bottom,     /* Bottom end (-32768..32767, >=top) */
  uint16_t color  /* Box color */
)
{
  uint32_t n;


  if (left > right || top > bottom) return;   /* Check validity */
  if (left > MaskR || right < MaskL  || top > MaskB || bottom < MaskT) return;    /* Check if in active area */

  if (top < MaskT) top = MaskT;       /* Clip top of rectangular if it is out of active area */
  if (bottom > MaskB) bottom = MaskB; /* Clip bottom of rectangular if it is out of active area */
  if (left < MaskL) left = MaskL;     /* Clip left of rectangular if it is out of active area */
  if (right > MaskR) right = MaskR;   /* Clip right of rectangular if it is out of active area */

  setrect(left, right, top, bottom);

  n = (uint32_t)(right - left + 1) * (uint32_t)(bottom - top + 1);
  do { DATA_WPX(color); } while (--n);

  CS_HIGH();          /* Release display */
}

void ILI9341::rect (
  int left,       /* Left end (-32768..32767) */
  int right,      /* Right end (-32768..32767, >=left) */
  int top,        /* Top end (-32768..32767) */
  int bottom,     /* Bottom end (-32768..32767, >=top) */
  uint16_t color  /* Box color */
)
{
  rectfill(left, left, top, bottom, color);
  rectfill(right, right, top, bottom, color);
  rectfill(left, right, top, top, color);
  rectfill(left, right, bottom, bottom, color);
}

void ILI9341::moveto (
  int x,      /* X position (-32768..32767) */
  int y       /* Y position (-32768..32767) */
)
{
  LocX = x;
  LocY = y;
}

void ILI9341::lineto (
  int x,      /* X position for the line to (-32768..32767) */
  int y,      /* Y position for the line to (-32768..32767) */
  uint16_t col    /* Line color */
)
{
  int32_t xr, yr, xp, yp, xd, yd;
  int ctr;


  xd = x - LocX; xr = (int32_t) LocX << 16; LocX = x;
  yd = y - LocY; yr = (int32_t) LocY << 16; LocY = y;
  if ((xd < 0 ? 0 - xd : xd) >= (yd < 0 ? 0 - yd : yd)) {
    ctr = (xd < 0 ? 0 - xd : xd) + 1;
    yd = (yd << 16) / (xd < 0 ? 0 - xd : xd);
    xd = (int32_t) (xd < 0 ? -1 : 1) << 16;
  } else {
    ctr = (yd < 0 ? 0 - yd : yd) + 1;
    xd = (xd << 16) / (yd < 0 ? 0 - yd : yd);
    yd = (int32_t) (yd < 0 ? -1 : 1) << 16;
  }
  xr += (int32_t) 1 << 15;
  yr += (int32_t) 1 << 15;

  CS_LOW();          /* Select display */

  do {
    xp = xr >> 16; yp = yr >> 16;
    if (xp >= MaskL && xp <= MaskR && yp >= MaskT && yp <= MaskB) {
      CMD_WRB(ILI9341_CMD_COLUMN_ADDRESS_SET);    /* Set H position */
      DATA_WRW(xp); DATA_WRW(xp);
      CMD_WRB(ILI9341_CMD_PAGE_ADDRESS_SET);  /* Set V position */
      DATA_WRW(yp); DATA_WRW(yp);
      CMD_WRB(ILI9341_CMD_MEMORY_WRITE);  /* Write a pixel data */
      DATA_WPX(col);
    }
    xr += xd; yr += yd;
  } while (--ctr);

  CS_HIGH();          /* Release display */
}

void ILI9341::line (
  int x1,     /* Start position X for the line (-32768..32767) */
  int y1,     /* Start position Y for the line (-32768..32767) */
  int x2,     /* End position X for the line (-32768..32767) */
  int y2,     /* End position Y for the line (-32768..32767) */
  uint16_t col    /* Line color */
)
{
  moveto(x1, y1);
  lineto(x2, y2, col);
}

void ILI9341::blt (
  int left,       /* Left end (-32768..32767) */
  int right,      /* Right end (-32768..32767, >=left) */
  int top,        /* Top end (-32768..32767) */
  int bottom,     /* Bottom end (-32768..32767, >=top) */
  const uint16_t *pat /* Pattern data */
)
{
  int yc, xc, xl, xs;
  uint16_t pd;


  if (left > right || top > bottom) return;   /* Check validity */
  if (left > MaskR || right < MaskL  || top > MaskB || bottom < MaskT) return;    /* Check if in active area */

  yc = bottom - top + 1;          /* Vertical size */
  xc = right - left + 1; xs = 0;  /* Horizontal size and skip */

  if (top < MaskT) {      /* Clip top of source image if it is out of active area */
    pat += xc * (MaskT - top);
    yc -= MaskT - top;
    top = MaskT;
  }
  if (bottom > MaskB) {   /* Clip bottom of source image if it is out of active area */
    yc -= bottom - MaskB;
    bottom = MaskB;
  }
  if (left < MaskL) {     /* Clip left of source image if it is out of active area */
    pat += MaskL - left;
    xc -= MaskL - left;
    xs += MaskL - left;
    left = MaskL;
  }
  if (right > MaskR) {    /* Clip right of source image if it is out of active area */
    xc -= right - MaskR;
    xs += right - MaskR;
    right = MaskR;
  }

  setrect(left, right, top, bottom); /* Set rectangular area to fill */

  do {    /* Send image data */
    xl = xc / 2;
    while (xl--) {
      pd = pgm_read_word(pat++); DATA_WPX(pd);
      pd = pgm_read_word(pat++); DATA_WPX(pd);
    }
    if (xc & 1) {
      pd = pgm_read_word(pat++); DATA_WPX(pd);
    }
    pat += xs;
  } while (--yc);

  CS_HIGH();          /* Release display */
}

void ILI9341::locate (
  int col,    /* Column position */
  int row     /* Row position */
)
{
  if (FontS) {
    LocX = col * pgm_read_byte(&FontS[14]);
    LocY = row * pgm_read_byte(&FontS[15]);
  }
}

void ILI9341::font_face (
    const uint8_t *font /* Pointer to the font structure in FONTX2 format */
)
{
  if (!memcmp_P("FONTX2", font, 6)) {
    FontS = font;
  }
}

void ILI9341::font_color (
  uint32_t color  /* (bg << 16) + fg */
)
{
  ChrColor = color;
}

void ILI9341::_putc (
  uint8_t chr     /* Character to be output */
)
{
  const uint8_t *fnt;;
  uint8_t b, d;
  uint16_t dchr;
  uint32_t col;
  int h, wc, w, wb, i, fofs;


  if ((fnt = FontS) == 0) return; /* Exit if no font registerd */

  if (chr < 0x20) {   /* Processes the control character */
    switch (chr) {
    case '\n':  /* LF */
      LocY += pgm_read_byte(&fnt[15]);
      /* follow next case */
    case '\r':  /* CR */
      LocX = 0;
      return;
    case '\b':  /* BS */
      LocX -= pgm_read_byte(&fnt[14]);
      if (LocX < 0) LocX = 0;
      return;
    case '\f':  /* FF */
      rectfill(0, get_width() - 1, 0, get_height() - 1, RGB16(0,0,0));
      LocX = LocY = 0;
      return;
    }
  }

  /* Exit if current position is out of screen */
  if (LocX >= get_width() || LocY >= get_height()) return;

  dchr = chr - FONT_START_CHAR;
  fofs = 17;      /* Font area start address */

  h = pgm_read_byte(&fnt[15]); w = pgm_read_byte(&fnt[14]); wb = (w + 7) / 8; /* Font size: height, dot width and byte width */
  fnt += fofs + dchr * wb * h;                /* Goto start of the bitmap */

  if (LocX + w > get_width()) w = get_width() - LocX; /* Clip right of font face at right edge */
  if (LocY + h > get_height()) h = get_height() - LocY; /* Clip bottom of font face at bottom edge */

  setrect(LocX, LocX + w - 1, LocY, LocY + h - 1);

  d = 0;
  do {
    wc = w; b = i = 0;
    do {
      if (!b) {       /* Get next 8 bits */
        b = 0x80;
        d = pgm_read_byte(&fnt[i++]);
      }
      col = ChrColor;
      if (!(b & d)) col >>= 16;   /* Select color, BG or FG */
      b >>= 1;        /* Next bit */
      DATA_WPX(col);  /* Put the color */
    } while (--wc);
    fnt += wb;      /* Next raster */
  } while (--h);

  LocX += w;  /* Update current position */

  CS_HIGH();          /* Release display */
}
