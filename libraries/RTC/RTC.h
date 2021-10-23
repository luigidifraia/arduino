#ifndef RTC_h
#define RTC_h

#include <inttypes.h>

#define DS1307_I2C_ADDRESS 0x68
#define DS3231_I2C_ADDRESS 0x68

typedef struct {
  word  year; /* 2000..2099 */
  byte  month;  /* 1..12 */
  byte  mday; /* 1..31 */
  byte  wday; /* 1..7  */
  byte  hour; /* 0..23 */
  byte  min;  /* 0..59 */
  byte  sec;  /* 0..59 */
} TIME_t;

class RTC {
  public:
    RTC (uint8_t address): _address(address) { };
    byte init (void);
    byte gettime (TIME_t *t);
    byte settime (TIME_t *t);

  private:
    byte decToBcd (byte val);
    byte bcdToDec (byte val);

    uint8_t _address;
};

#endif
