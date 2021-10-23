#ifndef TMP006_h
#define TMP006_h

#include <inttypes.h>

/* The TMP006's sampling rate is 250 ms per sample, so it takes 4 seconds to average 16 samples (TMP006_CFG_16SAMPLE) */

#define TMP006_CFG_1SAMPLE  0x0000
#define TMP006_CFG_2SAMPLE  0x0200
#define TMP006_CFG_4SAMPLE  0x0400
#define TMP006_CFG_8SAMPLE  0x0600
#define TMP006_CFG_16SAMPLE 0x0800

#define TMP006_CFG_RESET   0x8000
#define TMP006_CFG_MODEON  0x7000
#define TMP006_CFG_DRDYEN  0x0100
#define TMP006_CFG_DRDY    0x0080

typedef struct {
	double	tobj;
	double	tdie;
} TMP006_t;

class TMP006 {
  public:
    byte init (uint16_t samples);
    byte setconfig (uint16_t mode);
    byte gettemp (TMP006_t *tmp006);

  private:
    byte tmp006_test (void);
};

#endif
