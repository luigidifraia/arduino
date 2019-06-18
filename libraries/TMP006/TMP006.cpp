/*
 * Wrapper for TMP006 access through the Wire interface
 *
 * Note: Class methods, other than the constructor,
 * can be used just after a call to Wire.begin() is
 * made - usually in setup().
 *
 * (C) 2016 Luigi Di Fraia
 */

#include "Arduino.h"
#include "Wire.h"
#include "TMP006.h"

#define WIRE_READ_WORD_MSB()  (((uint16_t) Wire.read() << 8) | Wire.read())

/* TMP006 address */

const PROGMEM uint8_t tmp006_i2c_address = 0x40;

/* TMP006 registers */

#define TMP006_REG_VOBJ    0x00
#define TMP006_REG_TAMB    0x01
#define TMP006_REG_CONFIG  0x02
#define TMP006_REG_MANID   0xFE
#define TMP006_REG_DEVID   0xFF

/* TMP006 conversion constants */

const PROGMEM double s0   =  6e-14;
const PROGMEM double a1   =  1.75e-3;
const PROGMEM double a2   = -1.678e-5;
const PROGMEM double tref =  298.15;
const PROGMEM double b0   = -2.94e-5;
const PROGMEM double b1   = -5.7e-7;
const PROGMEM double b2   =  4.63e-9;
const PROGMEM double c2   =  13.4;

byte TMP006::tmp006_test (void)
{
  uint16_t conf;

  Wire.beginTransmission(tmp006_i2c_address);
  Wire.write(TMP006_REG_CONFIG);
  if (Wire.endTransmission()) return 0;

  Wire.requestFrom(tmp006_i2c_address, (uint8_t) 2);
  if (Wire.available() < 2) return 0;

  conf = WIRE_READ_WORD_MSB();

  return (conf & TMP006_CFG_DRDY); // test if results are ready to read
}

byte TMP006::gettemp (TMP006_t *tmp006)
{
  uint8_t cnt;
  uint16_t regsensorvolt, regdietemp;
  double vobj, tdie, tdie_tref;
  double s, vobj_vos, fvobj, tobj;

  for (cnt = 20; cnt > 0; cnt--) { // 5 seconds timeout
    if (tmp006_test())
      break;
    delay(250);
  }
  if (!cnt) return 0;

  Wire.beginTransmission(tmp006_i2c_address);
  Wire.write(TMP006_REG_VOBJ);
  if (Wire.endTransmission()) return 0;

  Wire.requestFrom(tmp006_i2c_address, (uint8_t) 2);
  if (Wire.available() < 2) return 0;

  /* From datasheet: If the MSB is '1', the integer is negative and the absolute 
     value can be obtained by inverting all bits and adding '1'. An alternative 
     method of calculating the absolute value of negative integers is abs(i) = i 
     xor FFFFh + 1. */
  regsensorvolt = WIRE_READ_WORD_MSB();
  if (regsensorvolt & 0x8000)
    vobj = -1.0 * ((regsensorvolt ^0xFFFF) + 1);
  else
    vobj = regsensorvolt;

  vobj *= 156.25; // 156.25 nV per LSB
  vobj /= 1000; // nV -> uV
  vobj /= 1000; // uV -> mV
  vobj /= 1000; // mV -> V

  Wire.beginTransmission(tmp006_i2c_address);
  Wire.write(TMP006_REG_TAMB);
  if (Wire.endTransmission()) return 0;

  Wire.requestFrom(tmp006_i2c_address, (uint8_t) 2);
  if (Wire.available() < 2) return 0;

  regdietemp = WIRE_READ_WORD_MSB();
  if (regdietemp & 0x8000)
    tdie = -1.0 * (((regdietemp ^ 0xFFFF) >> 2) + 1);
  else
    tdie = regdietemp >> 2;

  tdie *= 0.03125; // 0.03125 Celcius per LSB
  tdie += 273.15; // convert to Kelvin

  tdie_tref = tdie - tref;
  s = s0 * (1 + a1 * tdie_tref + a2 * tdie_tref * tdie_tref);
  vobj_vos = vobj - (b0 + b1 * tdie_tref + b2 * tdie_tref * tdie_tref);
  fvobj = vobj_vos + c2 * vobj_vos * vobj_vos;
  tobj = sqrt(sqrt((tdie * tdie * tdie * tdie) + (fvobj / s)));

  tmp006->tdie = tdie - 273.15; // convert to Celsius
  tmp006->tobj = tobj - 273.15; // convert to Celsius

  return 1;
}

byte TMP006::setconfig (uint16_t mode)
{
  Wire.beginTransmission(tmp006_i2c_address);
  Wire.write(TMP006_REG_CONFIG);
  Wire.write(mode >> 8);
  Wire.write(mode);
  return Wire.endTransmission();
}

byte TMP006::init (uint16_t samples)
{
  byte ret;

  Wire.beginTransmission(tmp006_i2c_address);
  ret = Wire.endTransmission();
  if (ret) return ret;

  /* Enable continuous conversion */
  return setconfig(TMP006_CFG_MODEON | samples);
}
