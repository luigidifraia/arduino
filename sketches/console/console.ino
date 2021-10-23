/*
 * This sketch was developed on a Pro-Micro board and demonstrates
 * the classes as per below.
 *
 * XUtils:
 * - the use of xatoi() instead of the Serial.parseInt() function
 *   as the latter doesn't report parsing errors;
 *   
 * XConsole:
 * - the use of xprintf() mixed with Serial.println(), the
 *   former inherited from XUtils for formatted output;
 * - the use of xputs(), also inherited from XUtils, for
 *   outputting paragraphs with embedded LF characters,
 *   optionally converted to CRLF;
 *
 * RTC (Pin 2: SDA, Pin 3: SCL):
 * - how to set/get the current time with a Maxim DS3231 RTC IC;
 *
 * TMP006 (Pin 2: SDA, Pin 3: SCL):
 * - how to get a temperature reading from a TMP006 contactless
 *   temperature sensor;
 *
 * ILI9341 (Pin 16: SPI MOSI, Pin 14: SPI MISO, Pin 15: SPI SCK,
 * Pin 7: SPI SS, Pin 8: ILI RESET, Pin 9: ILI D/C):
 * - the use of various graphic operations, including lines,
 *   rectangles, text, etc.;
 * - the use of xprintf() and xputs() inherited from XConsole,
 *   whom ILI9341 is a child class of;
 *
 * (C) 2016 Luigi Di Fraia
 */

#include <Wire.h>
#include <SPI.h>
#include <XUtils.h>
#include <XConsole.h>
#include <RTC.h>
#include <ILI9341.h>
#include <TMP006.h>

/* 1: Use ILI9341 display */
#define USE_ILI9341 1

/* 1: Use RTC */
#define USE_DS3231  1

/* 1: Use TMP006 sensor */
#define USE_TMP006  1

XConsole console(Serial);

#if USE_ILI9341
ILI9341 disp(0x07, 0x08, 0x09);  /* SS, RESET, D/C */
#endif

#if USE_DS3231
RTC rtc(DS3231_I2C_ADDRESS);
byte RtcOk = 0;    /* RTC is available */
#endif

#if USE_TMP006
TMP006 tmp006;
byte Tmp006Ok = 0; /* TMP006 is available */
#endif

/*----------------------------------------------*/
/* Parse command line and execute commands      */
/*----------------------------------------------*/

void parse_and_execute_command (
  const char *ptr   /* Pointer to the command string */
)
{
#if USE_DS3231
  static const PROGMEM char months[] = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec\0";
#endif
  long p1;
#if USE_ILI9341
  long p2, p3, p4, p5;
#endif
#if USE_DS3231
  TIME_t t;
#endif
#if USE_TMP006
  TMP006_t temp;
#endif

  switch (*ptr++) {
#if USE_ILI9341
  case 'g' :  /* Graphic controls */
    switch (*ptr++) {
    case 'i' :  /* gi - Initialize display */
      disp.init();
      disp.font_color(C_WHITE);
      disp.xputs(F("Hello world!\n"));
#if USE_DS3231
      if (RtcOk && rtc.gettime(&t)) {
        disp.xprintf(F("It's %S %u %u, %02u:%02u:%02u\n"), &months[(t.month - 1) * 4], t.mday, t.year, t.hour, t.min, t.sec);
      }
#endif
#if USE_TMP006
      if (Tmp006Ok && tmp006.gettemp(&temp)) {
        disp.xprintf(F("Object temperature is: %d.%02d C\n"), (int) temp.tobj, ((int) (temp.tobj * 100.0)) % 100);
      }
#endif
      break;

    case 'k' :  /* gk <l> <r> <t> <b> - Set mask */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2) || !XUtils::xatoi(&ptr, &p3) || !XUtils::xatoi(&ptr, &p4)) break;
      disp.setmask(p1, p2, p3, p4);
      //console.xprintf(F("%ld, %ld, %ld, %ld\n"), p1, p2, p3, p4);
      break;

    case 'f' :  /* gf <l> <r> <t> <b> <col> - Rectangular fill */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2) || !XUtils::xatoi(&ptr, &p3) || !XUtils::xatoi(&ptr, &p4) || !XUtils::xatoi(&ptr, &p5)) break;
      disp.rectfill(p1, p2, p3, p4, p5);
      //console.xprintf(F("%ld, %ld, %ld, %ld, %ld\n"), p1, p2, p3, p4, p5);
      break;

    case 'm' :  /* gm <x> <y> - Set current position */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2)) break;
      disp.moveto(p1, p2);
      //console.xprintf(F("%ld, %ld\n"), p1, p2);
      break;

    case 'l' :  /* gl <x> <y> <col> - Draw line */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2) || !XUtils::xatoi(&ptr, &p3)) break;
      disp.lineto(p1, p2, p3);
      //console.xprintf(F("%ld, %ld, %ld\n"), p1, p2, p3);
      break;

    case 'o' :  /* go <value> - Change display orientation */
      if (!XUtils::xatoi(&ptr, &p1)) break;
      disp.set_orientation(p1);
      //console.xprintf(F("%ld\n"), p1);
      break;

    case 'c' :  /* gc <col> - Set current text color */
      if (!XUtils::xatoi(&ptr, &p1)) break;
      disp.font_color(p1);
      //console.xprintf(F("%ld\n"), p1);
      break;

    case 's' :  /* gs <x> <y> - Set current character position */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2)) break;
      disp.locate(p1, p2);
      //console.xprintf(F("%ld, %ld\n"), p1, p2);
      break;

    case 'v' :  /* gv <top fixed> <scroll area> <bottom fixed> - Vertical scroll definition */
      if (!XUtils::xatoi(&ptr, &p1) || !XUtils::xatoi(&ptr, &p2) || !XUtils::xatoi(&ptr, &p3)) break;
      disp.set_scroll_def(p1, p2, p3);
      //console.xprintf(F("%ld, %ld, %ld\n"), p1, p2, p3);
      break;

    case 'a' :  /* ga <start address> - Set vertical scroll start address */
      if (!XUtils::xatoi(&ptr, &p1)) break;
      disp.set_scroll_start(p1);
      //console.xprintf(F("%ld\n"), p1);
      break;

    case 'w' :  /* gw <text> - Write text */
      while (*ptr == ' ') ptr++;
      if (!*ptr) break;
      disp.xprintf(F("%s"), ptr);
      //console.xprintf(F("%s\n"), ptr);
      break;
    }
    break;
#endif

  case 'c' :  /* c <value> - Convert numeric input to decimal */
    if (!XUtils::xatoi(&ptr, &p1)) break;
    Serial.println(p1);
    break;

#if USE_DS3231
  case 't' :  /* t [<year> <month> <mday> <hour> <min> <sec>] - Set/Show current time */
    if (!RtcOk) break;
    if (XUtils::xatoi(&ptr, &p1)) {
      t.year = (word)p1;
      XUtils::xatoi(&ptr, &p1); t.month = (byte)p1;
      XUtils::xatoi(&ptr, &p1); t.mday = (byte)p1;
      XUtils::xatoi(&ptr, &p1); t.hour = (byte)p1;
      XUtils::xatoi(&ptr, &p1); t.min = (byte)p1;
      if (!XUtils::xatoi(&ptr, &p1)) break;
      t.sec = (byte)p1;
      rtc.settime(&t);
    }
    if (rtc.gettime(&t)) {
      console.xprintf(F("%u/%u/%u %02u:%02u:%02u\n"), t.year, t.month, t.mday, t.hour, t.min, t.sec);
    }
    break;
#endif

#if USE_TMP006
  case 'm' :  /* Show object temperature */
    if (!Tmp006Ok) break;
    if (tmp006.gettemp(&temp)) {
      Serial.println(temp.tobj);
    }
    break;
#endif

  case 'v' :  /* v - Show sketch version */
    Serial.println(F("1.4"));
    break;

  case '?' :  /* ? - Show command list */
    console.xputs(F(
#if USE_ILI9341
      "[Graphic commands]\n"
      " gi - Initialize display module\n"
      " gk <l> <r> <t> <b> - Set active area\n"
      " gf <l> <r> <t> <b> <col> - Draw solid rectangular\n"
      " gm <x> <y> - Move current position\n"
      " gl <x> <y> <col> - Draw line to\n"
      " go <value> - Change display orientation\n"
      " gc <col> - Set current text color\n"
      " gs <x> <y> - Set current character position\n"
      " gw <text> - Write text\n"
      " gv <top fixed> <scroll area> <bottom fixed> - Vertical scroll definition\n"
      " ga <start address> - Set vertical scroll start address\n"
#endif
      "[Misc Commands]\n"
      " c <value> - Convert numeric input to decimal\n"
#if USE_DS3231
      " t [<year> <month> <mday> <hour> <min> <sec>] - Set/Show current time\n"
#endif
#if USE_TMP006
      " m - Show object temperature\n"
#endif
      " v - Show sketch version\n"
      "\n"));
    break;
  }
}

/*----------------------------------------------*/
/* Sketch core                                  */
/*----------------------------------------------*/

void setup (void)
{
  /* Put your setup code here, to run once */

  Serial.begin(9600); /* Initialize USB Serial (always 12 Mbit/sec) */
#if USE_DS3231
  Wire.begin(); /* Initialize the TWI bus used by the RTC and TMP006 modules */
#endif
#if USE_ILI9341
  SPI.begin();  /* Initialize the SPI bus used by the TFT display module */
#endif

#if USE_DS3231
  if (rtc.init() == 0) RtcOk = 1;
#endif
#if USE_TMP006
  if (tmp006.init(TMP006_CFG_1SAMPLE) == 0) Tmp006Ok = 1;  /* New sample available every 250 ms */
#endif
}

void loop (void)
{
  /* Put your main code here, to run repeatedly */

  char Line[64];  /* Console input buffer */
#if USE_DS3231
  TIME_t t;
#endif
#if USE_TMP006
  TMP006_t temp;
#endif

  /* Wait until the USB CDC serial connection is opened/reopened */
  while (!Serial || !Serial.dtr()) ;

  /* Discard anything that was received */
  while (Serial.available() > 0)
    Serial.read();

  /* Show banner */
  Serial.println(F("\e[H\e[2JTest Console for Arduino by Luigi Di Fraia"));

  /* Remind user that a CR character suffices to terminate lines */
  Serial.println(F("Note: Ensure your terminal ends lines just with a CR"));

#if USE_DS3231
  /* Show current time */
  if (RtcOk && rtc.gettime(&t)) {
    console.xprintf(F("Current time: %u/%u/%u %02u:%02u:%02u\n"), t.year, t.month, t.mday, t.hour, t.min, t.sec);
  } else {
    Serial.println(F("RTC is not available"));
  }
#endif
#if USE_TMP006
  /* Show object temperature */
  if (Tmp006Ok && tmp006.gettemp(&temp)) {
    Serial.print(F("Object temperature is: "));
    Serial.println(temp.tobj);
  } else {
    Serial.println(F("TMP006 is not available"));
  }
#endif

  /* Listen for commands and process them */
  for (;;) {
    Serial.print(F(">"));
    if (!console.xgets(Line, sizeof(Line)))
      break;  /* User disconnected */
    parse_and_execute_command(Line);
  }
}
