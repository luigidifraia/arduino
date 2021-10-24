/*
 * AVR string handler for user console interface
 *
 * (C) 2016 Luigi Di Fraia
 */

#include "Arduino.h"
#include "XConsole.h"

/*----------------------------------------------*/
/* Get a char from the input stream             */
/*----------------------------------------------*/

char XConsole::xgetc (void) {
  for (;;) {
    if (_serial.available()) {
      return (char) _serial.read();
    } else {
#if CAN_DETECT_SERIAL_DISCONNECT
      /* Makes sense if disconnection can be detected */
      if (!_serial.dtr())
        return 0;
#endif
    }
  }
}

/*----------------------------------------------*/
/* Put a char into the output stream            */
/*----------------------------------------------*/

void XConsole::xputc (
  char c    /* Character to be sent */
)
{
  if (XPUTC_LF_CRLF && c == '\n')
    xputc('\r');    /* LF -> CRLF */

  for (;;) {
    if (_serial.availableForWrite()) {
      _serial.write(c);
      return;
    } else {
#if CAN_DETECT_SERIAL_DISCONNECT
      /* Makes sense if disconnection can be detected */
      if (!_serial.dtr())
        return;
#endif
    }
  }
}
