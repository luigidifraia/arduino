/*
 * AVR string handler
 *
 * (C) 2016 Luigi Di Fraia
 */

#include <stdarg.h>

#include "Arduino.h"
#include "XUtils.h"

/*----------------------------------------------*/
/* Get a line from the input stream (excluding  */
/* the trailing LF character)                   */
/*----------------------------------------------*/

byte XUtils::xgets (  /* 0:End of stream, 1:A line arrived */
  char* buff, /* Pointer to the buffer */
  int len     /* Buffer length */
)
{
  char c;
  int i;

  i = 0;
  for (;;) {
    c = xgetc();          /* Get a char from the incoming stream */
    if (!c) return 0;     /* End of stream? */
    if (c == '\r') break; /* End of line? */
    if ((c == '\b' || c == 0x7F) && i) { /* Back space or Delete? */
      i--;
#if XGETS_CHAR_ECHO
      xputc('\b');
#endif
      continue;
    }
    if (c >= ' ' && i < len - 1) {  /* Visible chars */
      buff[i++] = c;
#if XGETS_CHAR_ECHO
      xputc(c);
#endif
    }
  }
  buff[i] = 0;  /* Terminate with a \0 */
#if XGETS_CHAR_ECHO
#if !XPUTC_LF_CRLF
  xputc('\r');
#endif
  xputc('\n');
#endif
  return 1;
}

/*----------------------------------------------*/
/* Get value off the string and step beyond it  */
/*----------------------------------------------*/

byte XUtils::xatoi (  /* 0:Failed, 1:Successful */
  const char **str, /* Pointer to pointer to the string */
  long *res   /* Pointer to the valiable to store the value */
)
{
  unsigned long val;
  byte c, r, s = 0;

  *res = 0;

  while ((c = **str) == ' ') (*str)++;  /* Skip leading spaces */

  if (c == '-') {   /* negative? */
    s = 1;
    c = *(++(*str));
  }

  if (c == '0') {
    c = *(++(*str));
    switch (c) {
    case 'x':   /* hexdecimal */
      r = 16; c = *(++(*str));
      break;
    case 'b':   /* binary */
      r = 2; c = *(++(*str));
      break;
    default:
      if (c <= ' ') return 1; /* single zero */
      if (c < '0' || c > '9') return 0; /* invalid char */
      r = 8;    /* octal */
    }
  } else {
    if (c < '0' || c > '9') return 0; /* EOL or invalid char */
    r = 10;     /* decimal */
  }

  val = 0;
  while (c > ' ') {
    if (c >= 'a') c -= 0x20;
    c -= '0';
    if (c >= 17) {
      c -= 7;
      if (c <= 9) return 0; /* invalid char */
    }
    if (c >= r) return 0;   /* invalid char for current radix */
    val = val * r + c;
    c = *(++(*str));
  }
  if (s) val = 0 - val;     /* apply sign if needed */

  *res = val;
  return 1;
}

/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void XUtils::xputs (          /* Put a string to the default device */
  const __FlashStringHelper* str     /* Pointer to the string */
)
{
  PGM_P pstr = reinterpret_cast<PGM_P>(str);
  char c;


  while (1) {
    c = pgm_read_byte(pstr++);
    if (!c) break;
    xputc(c);
  }
}

/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/

void XUtils::xputs (          /* Put a string to the default device */
  const char* str       /* Pointer to the string */
)
{
  char c;


  while (1) {
    c = pgm_read_byte(str++);
    if (!c) break;
    xputc(c);
  }
}

void XUtils::xvprintf (
  const __FlashStringHelper* fmt,  /* Pointer to the format string */
  va_list arp     /* Pointer to arguments */
)
{
  PGM_P pfmt = reinterpret_cast<PGM_P>(fmt);
  unsigned int r, i, j, w, f;
  unsigned long v;
  char s[16], c, d, *p;


  for (;;) {
    c = pgm_read_byte(pfmt++); /* Get a char */
    if (!c) break;        /* End of format? */
    if (c != '%') {       /* Pass through it if not a % sequense */
      xputc(c); continue;
    }
    f = 0;
    c = pgm_read_byte(pfmt++); /* Get first char of the sequense */
    if (c == '0') {       /* Flag: '0' padded */
      f = 1; c = pgm_read_byte(pfmt++);
    } else {
      if (c == '-') {     /* Flag: left justified */
        f = 2; c = pgm_read_byte(pfmt++);
      }
    }
    for (w = 0; c >= '0' && c <= '9'; c = pgm_read_byte(pfmt++)) /* Minimum width */
      w = w * 10 + c - '0';
    if (c == 'l' || c == 'L') { /* Prefix: Size is long int */
      f |= 4; c = pgm_read_byte(pfmt++);
    }
    if (!c) break;        /* End of format? */
    d = c;
    switch (d) {        /* Type is... */
    case 'S' :          /* Program memory string */
      p = va_arg(arp, char*);
      for (j = 0; p[j]; j++) ;
      while (!(f & 2) && j++ < w) xputc(' ');
      xputs(p);
      while (j++ < w) xputc(' ');
      continue;
    case 's' :          /* String */
      p = va_arg(arp, char*);
      for (j = 0; p[j]; j++) ;
      while (!(f & 2) && j++ < w) xputc(' ');
      while (*p) xputc(*p++);
      while (j++ < w) xputc(' ');
      continue;
    case 'c' :          /* Character */
      xputc((char)va_arg(arp, int)); continue;
    case 'b' :          /* Binary */
      r = 2; break;
    case 'o' :          /* Octal */
      r = 8; break;
    case 'd' :          /* Signed decimal */
    case 'u' :          /* Unsigned decimal */
      r = 10; break;
    case 'x' :          /* Hexdecimal */
      r = 16; break;
    default:          /* Unknown type (passthrough) */
      xputc(c); continue;
    }

    /* Get an argument and put it in numeral */
    v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
    if (d == 'D' && (v & 0x80000000)) {
      v = 0 - v;
      f |= 8;
    }
    i = 0;
    do {
      d = (char)(v % r); v /= r;
      if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
      s[i++] = d + '0';
    } while (v && i < sizeof(s));
    if (f & 8) s[i++] = '-';
    j = i; d = (f & 1) ? '0' : ' ';
    while (!(f & 2) && j++ < w) xputc(d);
    do xputc(s[--i]); while(i);
    while (j++ < w) xputc(' ');
  }
}

void XUtils::xprintf (      /* Put a formatted string to the default device */
  const __FlashStringHelper* fmt,  /* Pointer to the format string */
  ...         /* Optional arguments */
)
{
  va_list arp;


  va_start(arp, fmt);
  xvprintf(fmt, arp);
  va_end(arp);
}
