/*
 * Wrapper for RTC access through the Wire interface
 *
 * Note: Class methods, other than the constructor,
 * can be used just after a call to Wire.begin() is
 * made - usually in setup().
 *
 * (C) 2016 Luigi Di Fraia
 */

#include "Arduino.h"
#include "Wire.h"
#include "RTC.h"

byte RTC::decToBcd (byte val)
{
  return((val % 10) + ((val / 10) << 4));
}

byte RTC::bcdToDec (byte val)
{
  return((val & 0x0F) + ((val >> 4) * 10));
}

byte RTC::init (void)
{
  Wire.beginTransmission(_address);
  return Wire.endTransmission();
}

byte RTC::gettime (TIME_t *t)
{
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  if (Wire.endTransmission()) return 0;

  Wire.requestFrom(_address, (uint8_t) 7);
  if (Wire.available() < 7) return 0;

  t->sec = bcdToDec(Wire.read());
  t->min = bcdToDec(Wire.read());
  t->hour = bcdToDec(Wire.read() & 0xBF);   /* Mask out the 12/24 mode bit */
  t->wday = Wire.read();
  t->mday = bcdToDec(Wire.read());
  t->month = bcdToDec(Wire.read() & 0x7F);  /* Mask out the Century bit */
  t->year = 2000 + bcdToDec(Wire.read());

  return 1;
}

byte RTC::settime (TIME_t *t)
{
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  Wire.write(decToBcd(t->sec));
  Wire.write(decToBcd(t->min));
  Wire.write(decToBcd(t->hour));
  Wire.write(t->wday & 0x07);
  Wire.write(decToBcd(t->mday));
  Wire.write(decToBcd(t->month));
  Wire.write(decToBcd(t->year - 2000));
  if (Wire.endTransmission()) return 0;

  return 1;
}
