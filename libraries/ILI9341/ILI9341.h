#ifndef ILI9341_h
#define ILI9341_h

#include <inttypes.h>

#include "Fonts.h"
#include "XUtils.h"

/* 1: Initial orientation landscape */
#define DISP_LANDSCAPE  0

/* RGB pixel data format (Create RGB565 from RGB888) */
#define RGB16(r,g,b)    (uint16_t)(((r) & 0xF8) << 8 | ((g) & 0xFC) << 3 | (b) >> 3)

/* A few handy color values */
#define C_BLACK     RGB16(0,0,0)
#define C_BLUE      RGB16(0,0,255)
#define C_RED       RGB16(255,0,0)
#define C_MAGENTA   RGB16(255,0,255)
#define C_GREEN     RGB16(0,255,0)
#define C_CYAN      RGB16(0,255,255)
#define C_YELLOW    RGB16(255,255,0)
#define C_WHITE     RGB16(255,255,255)
#define C_LGRAY     RGB16(160,160,160)
#define C_GRAY      RGB16(128,128,128)

class ILI9341: public XUtils {
  public:
    /**
     * Constructor
     *
     * @param cs Slave select pin
     * @param reset Reset pin
     * @param dc Data/command pin
     */
    ILI9341 (byte cs, byte reset, byte dc): _cs(cs), _reset(reset), _dc(dc) { };

    /**
     * Initialize display module ILI9341
     */
    void init (void);

    /**
     * Set orientation
     *
     * Mask and cursor settings are reset after an orientation change
     *
     * @param o Orientation
     */
    void set_orientation (uint8_t o);

    /**
     * Get display width in pixels
     *
     * @return width in pixels
     */
    int get_width (void);

    /**
     * Get display height in pixels
     *
     * @return height in pixels
     */
    int get_height (void);

    /**
     * Get width of chars (for the fontset currently registered)
     *
     * @return width in chars
     */
    int get_font_width (void);

    /**
     * Get height of chars (for the fontset currently registered)
     *
     * @return height in chars
     */
    int get_font_height (void);

    /**
     * Set vertical scroll definition
     *
     * @param tfa Top Fixed Area
     * @param vsa Vertical Scrolling Area
     * @param bfa Bottom Fixed Area
     */
    void set_scroll_def (int tfa, int vsa, int bfa);

    /**
     * Set vertical scroll start address
     *
     * @param vsp Vertical Scroll Start Address
     */
    void set_scroll_start (int vsp);

    /**
     * Set active drawing area
     *
     * The mask feature affects only graphics functions
     *
     * @param left Left end (0..DISP_XS-1)
     * @param right Right end (0..DISP_XS-1, >= left)
     * @param top Top end (0..DISP_YS-1)
     * @param bottom Bottom end (0..DISP_YS-1, >= top)
     */
    void setmask (int left, int right, int top, int bottom);

    /**
     * Draw a solid rectangle
     *
     * @param left Left end (-32768..32767)
     * @param right Right end (-32768..32767, >=left)
     * @param top Top end (-32768..32767, >=left)
     * @param bottom Bottom end (-32768..32767, >=top)
     */
    void rectfill (int left, int right, int top, int bottom, uint16_t color);

    /**
     * Draw a hollow rectangle
     *
     * @param left Left end (-32768..32767)
     * @param right Right end (-32768..32767, >=left)
     * @param top Top end (-32768..32767, >=left)
     * @param bottom Bottom end (-32768..32767, >=top)
     */
    void rect (int left, int right, int top, int bottom, uint16_t color);

    /**
     * Set current dot position for lineto
     *
     * @param x X position (-32768..32767)
     * @param y Y position (-32768..32767)
     */
    void moveto (int x, int y);

    /**
     * Draw a line from current position
     *
     * @param x X position for the line to (-32768..32767)
     * @param y Y position for the line to (-32768..32767)
     */
    void lineto (int x, int y, uint16_t col);

    /**
     * Draw a line
     *
     * @param x1 Start position X for the line (-32768..32767)
     * @param y1 Start position Y for the line (-32768..32767)
     * @param x2 End position X for the line (-32768..32767)
     * @param y2 End position Y for the line (-32768..32767)
     * @param col Line color
     */
    void line (int x1, int y1, int x2, int y2, uint16_t col);

    /**
     * Copy image data to the display
     *
     * @param left Left end (-32768..32767)
     * @param right Right end (-32768..32767, >=left)
     * @param top Top end (-32768..32767, >=left)
     * @param bottom Bottom end (-32768..32767, >=top)
     * @param pat Pattern data
     */
    void blt (int left, int right, int top, int bottom, const uint16_t *pat);

    /**
     * Set current character position for putc
     *
     * @param col Column position
     * @param row Row position
     */
    void locate (int col, int row);

    /**
     * Register text font
     *
     * @param font Pointer to the font structure in FONTX2 format
     */
    void font_face (const uint8_t *font);

    /**
     * Set current text color
     *
     * @param color (bg << 16) + fg
     */
    void font_color (uint32_t color);

    /**
     * Put a text character
     *
     * @param chr Character to output
     */
    void _putc (uint8_t chr);
    void xputc (char chr) {
      _putc((uint8_t) chr);
    }

    /**
     * Read a character
     *
     * @return character read
     */
    char xgetc (void) {
      return (char) 0; /* End of stream */
    }

  private:
    int MaskT, MaskL, MaskR, MaskB;  /* Drawing mask */
    int LocX, LocY;         /* Current dot position */
    uint32_t ChrColor;      /* Current character color ((bg << 16) + fg) */
    const uint8_t *FontS;   /* Current font */
    uint8_t Orientation;    /* Current orientation */

    byte _cs;
    byte _reset;
    byte _dc;

    /**
     * Set rectangular area to be transferred
     *
     * @param left Left end (0..DISP_XS-1)
     * @param right Right end (0..DISP_XS-1, >= left)
     * @param top Top end (0..DISP_YS-1)
     * @param bottom Bottom end (0..DISP_YS-1, >= top)
     */
    void setrect (int left, int right, int top, int bottom);
};

#endif