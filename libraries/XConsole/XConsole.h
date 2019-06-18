#ifndef XConsole_h
#define XConsole_h

#include "XUtils.h"

/* 1: Make xprint and xprintln available as wrappers to Serial equivalents */
#define EXPOSE_PRINT_INTERFACE  0

/* 1: User disconnection can be detected via DTR (true for SerialUSB ports) */
#define CAN_DETECT_SERIAL_DISCONNECT 1

class XConsole: public XUtils {
  public:
#if defined(CORE_TEENSY)
    XConsole(usb_serial_class& serial = Serial): _serial(serial) { };
#else
    XConsole(Serial_& serial = Serial): _serial(serial) { };
#endif
    virtual char xgetc (void);
    virtual void xputc (char c);

#if EXPOSE_PRINT_INTERFACE
    inline size_t xprint(const __FlashStringHelper *ifsh) { _serial.print(ifsh); };
    inline size_t xprint(const String & s) { _serial.print(s); }
    inline size_t xprint(const char str[]) { _serial.print(str) ; };
    inline size_t xprint(char c) { _serial.print(c) ; };
    inline size_t xprint(unsigned char b, int base = DEC) { _serial.print(b, base) ; };
    inline size_t xprint(int n, int base = DEC) { _serial.print(n, base) ; };
    inline size_t xprint(unsigned int n, int base = DEC) { _serial.print(n, base) ; };
    inline size_t xprint(long n, int base = DEC) { _serial.print(n, base) ; };
    inline size_t xprint(unsigned long n, int base = DEC) { _serial.print(n, base) ; };
    inline size_t xprint(double n, int base = 2) { _serial.print(n, base) ; };
    inline size_t xprint(const Printable& x) { _serial.print(x) ; };

    inline size_t xprintln(const __FlashStringHelper *ifsh) { _serial.println(ifsh); };
    inline size_t xprintln(const String & s) { _serial.println(s); }
    inline size_t xprintln(const char str[]) { _serial.println(str) ; };
    inline size_t xprintln(char c) { _serial.println(c) ; };
    inline size_t xprintln(unsigned char b, int base = DEC) { _serial.println(b, base) ; };
    inline size_t xprintln(int n, int base = DEC) { _serial.println(n, base) ; };
    inline size_t xprintln(unsigned int n, int base = DEC) { _serial.println(n, base) ; };
    inline size_t xprintln(long n, int base = DEC) { _serial.println(n, base) ; };
    inline size_t xprintln(unsigned long n, int base = DEC) { _serial.println(n, base) ; };
    inline size_t xprintln(double n, int base = 2) { _serial.println(n, base) ; };
    inline size_t xprintln(const Printable& x) { _serial.println(x) ; };
#endif

  private:
#if defined(CORE_TEENSY)
    usb_serial_class& _serial;
#else
    Serial_& _serial;
#endif
};

#endif