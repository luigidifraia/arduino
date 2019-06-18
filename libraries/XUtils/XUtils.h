#ifndef XUtils_h
#define XUtils_h

/* 1: Convert \n ==> \r\n in xputc function */
#define XPUTC_LF_CRLF 1

/* 1: Echo back input chars in xgets function */
#define XGETS_CHAR_ECHO 1

/*
 * Supported formats:
 *
 *  xprintf(F("%d"), 1234);            "1234"
 *  xprintf(F("%6d,%3d%%"), -200, 5);  "  -200,  5%"
 *  xprintf(F("%-6u"), 100);           "100   "
 *  xprintf(F("%ld"), 12345678L);      "12345678"
 *  xprintf(F("%04x"), 0xA3);          "00a3"
 *  xprintf(F("%08lx"), 0x123ABC);     "00123ABC"
 *  xprintf(F("%016b"), 0x550F);       "0101010100001111"
 *  xprintf(F("%s"), "String");        "String"
 *  xprintf(F("%-4s"), "abc");         "abc "
 *  xprintf(F("%4s"), "abc");          " abc"
 *  xprintf(F("%S"), PSTR("String"));  "String" (from program memory)
 *  xprintf(F("%c"), 'a');             "a"
 *  xprintf(F("%f"), 10.0);            <xprintf lacks floating point support>
 */

class XUtils {
  public:
    virtual void xputc (char c);
    virtual char xgetc (void);
    void xputs (const __FlashStringHelper* str);
    void xprintf (const __FlashStringHelper* fmt, ...);
    static byte xatoi (const char **str, long *res);
    byte xgets (char* buff, int len);

  private:
    void xputs (const char* str);
    void xvprintf (const __FlashStringHelper* fmt, va_list arp);
};

#endif