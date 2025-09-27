#include "debug.h"
#include <stdexcept>
#include <string.h>
#include <math.h>
using namespace std;

bool parse_int(const char *str, int n, void *dest);
bool parse_double(const char *str, int n, void *dest);
bool parse_string(const char *str, int n, void *dest);

Arg Arg::none;

Arg::Arg() { parser = 0; }
Arg::Arg(int i) { ival = i; parser = 0; }
Arg::Arg(unsigned int i) { ival = (int)i; parser = 0; }
Arg::Arg(long long int i) { lval = i; parser = 0; }
//Arg::Arg(size_t i) { ival = (int)i; parser = 0; }
Arg::Arg(char c) { ival = (int)c; parser = 0; }
Arg::Arg(double d) { dval = d; parser = 0; }
Arg::Arg(const char *s) { cval = s; parser = 0; }
Arg::Arg(const string &s) { cval = s.c_str(); parser = 0; }

Arg::Arg(int *i) { pval = i; parser = parse_int; }
Arg::Arg(double *d) { pval = d; parser = parse_double; }
Arg::Arg(string *s) { pval = s; parser = parse_string; }
Arg::Arg(void *p) { pval = p; parser = 0; }

const int Arg::toInt() const { return ival; }
const double Arg::toDouble() const { return dval; }
const char *Arg::toCharP() const { return cval; }
const int *Arg::toIntP() const { return (int *)pval; }
const double *Arg::toDoubleP() const { return (double *)pval; }
const void *Arg::toVoidP() const { return pval; }

const bool Arg::isInt() const { return parser == parse_int; };
const bool Arg::isDouble() const { return parser == parse_double; };
const bool Arg::isString() const { return parser == parse_string; };

bool Arg::parse(const char *str, int n) const {
  if (!parser) throw std::runtime_error("Arg::parse: no parser set. Did you pass a value instead a pointer? (if you see this, talk to a developer)"); //return false;
  return parser(str, n, pval);
}

///////////////////////////////////////////////////////////////////////////////

bool parse_int(const char *str, int n, void *dest) {
  int i = 0;
  for (int j = 0; j < n; j++) {
    char c = str[j];
    if ('0' <= c && c <= '9') {
      i = i * 10 + (c - '0');
    } else {
      return false;
    }
  }
  *((int *)dest) = i;
  return true;
}

bool parse_double(const char *str, int n, void *dest) {
  double d = 0.0, f = 1.0;
  bool postdecimal = false;
  for (int j = 0; j < n; j++) {
    char c = str[j];
    if ('0' <= c && c <= '9') {
      d = d * 10 + (c - '0');
      if (postdecimal) f *= 10.0;
    } else if (c == '-') {
      f *= -1;
    } else if (c == '.') {
      postdecimal = true;
    } else {
      return false;
    }
  }
  d /= f;
  *((double *)dest) = d;
  return true;
}

bool parse_string(const char *str, int n, void *dest) {
  string s(str, n);
  *(string *)dest = s;
  return true;
}


///////////////////////////////////////////////////////////////////////////////

typedef unsigned long long uint64;

/////////////////////////////////// format ////////////////////////////////////

class FormatParams_ {
public:
  int width;
  int prec;
  char pad;
  bool sign;
  char conv;
  unsigned int base;
  bool uc;
  FormatParams_() : width(0),prec(-1),pad(' '),sign(false),conv('?'),base(10),uc(false) {
  }
  void init() {
    width = 0;
    prec = -1;
    pad = ' ';
    sign = false;
    conv = '?';
    base = 10;
    uc = false;
  }
};

#define NOSIGN ' '
#define STRING_TINY 0.00001

#define STRING_FIELD 1
#define STRING_WIDTH 2
#define STRING_PRECISION 3
#define STRING_TYPE 4

#define FLOOR(X) ((double)(bigint)X)
#define IFLOOR(X) ((bigint)X)

void _uitoa(string &buf, FormatParams_ &p, uint64 value) {
  // http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
  uint64 d = 1;
  int n = 0;
  if (value == 0) {
    buf += '0';
    return;
  }
  while (value / d >= p.base)
    d *= p.base;
  while (d != 0) {
    uint64 dgt = value / d;
    value %= d;
    d /= p.base;
    if (n || dgt > 0 || d == 0) {
      buf += dgt + (dgt < 10 ? '0' : (p.uc ? 'A' : 'a') - 10);
      ++n;
    }
  }
}

void _padl(string &result, FormatParams_ &p, int len, char sign) {
  if (sign == NOSIGN) {
    for (int i = len; i < p.width; i++)
      result += p.pad;
  } else {
    if (p.pad == '0') {
      // 0-pad: +00001
      result += sign;
      for (int i = len; i < p.width - 1; i++)
        result += p.pad;
    } else {
      // blank:     +1
      for (int i = len; i < p.width - 1; i++)
        result += p.pad;
      result += sign;
    }
  }
}

string _fmt(const char *fmt, const Arg *args, size_t num_args) {
  int argp = 0;
  int state = 0;
  FormatParams_ p;
  string str;
  // Reserve some memory, assuming width 10 per argument
  str.reserve(strlen(fmt) + 10 * num_args);
  while (*fmt) {
    if (state == 0) {
      if (*fmt == '%') {
        state = STRING_FIELD;
        p.init();
      } else {
        str += *fmt;
      }
      fmt++;
    } else if (state == STRING_FIELD) {
      switch (*fmt) {
      case '+':
        p.sign = true; fmt++; break;
      case '0':
        p.pad = '0'; fmt++; break;
      case ' ':
        p.pad = ' '; fmt++; break;
      default:
        state = STRING_WIDTH; // don't advance
      }
    } else if (state == STRING_WIDTH) {
      if (*fmt == '*') {
        p.width = args[argp++].toInt(); fmt++;
      } else if (isdigit(*fmt)) {
        p.width = 10 * p.width + (*fmt - '0'); fmt++;
      } else if (*fmt == '.') {
        state = STRING_PRECISION; fmt++;
      } else
        state = STRING_TYPE; // don't advance
    } else if (state == STRING_PRECISION) {
      if (isdigit(*fmt)) {
        if (p.prec == -1) p.prec = 0;
        p.prec = 10 * p.prec + (*fmt-'0'); fmt++;
      } else if (*fmt == '*') {
        p.prec = args[argp++].toInt(); fmt++;
      } else
        state = STRING_TYPE;
    } else if (state == STRING_TYPE) {
      p.conv = *fmt;
      switch (*fmt) {
      case 'c':
        str += (char)args[argp++].toInt();
        break;
      case 'd': case 'i':
        {
          int value = args[argp++].toInt();
          string buf;
          buf.reserve(16);
          char sign = NOSIGN;
          if (p.sign && value > 0) sign = '+';
          if (value < 0) { sign = '-'; value *= -1; }
          _uitoa(buf, p, (uint64)value);
          _padl(str, p, (int)buf.length(), sign);
          str += buf;
        }
        break;
      case 'g': case 'G':
      case 'f': case 'F':
        {
          double value = args[argp++].toDouble();
          if (isnan(value)) {
            str += "NaN";
            break;
          }
          string buf;
          buf.reserve(16);
          char dbuf[16];
          char sign = NOSIGN;
          if (p.sign && value > 0.0) sign = '+';
          if (value < 0.0) { sign = '-'; value *= -1; }
          // Integral part
          double intpart = floor(value);
          int intpart_i = (int)intpart;
          // Post-decimal part
          value -= intpart;
          if (p.prec == -1) p.prec = 6;
          if (p.prec > 12) p.prec = 12;
          if (p.prec != 0) {
            double f = 1;
            int decimals = p.prec;
            double d = value;
            for (; decimals; decimals--) f *= 10;
            d *= f;
            if (d - floor(d) >= 0.5) {
              d = ceil(d);
            } else {
              d = floor(d);
            }
#if 0
            d /= f;
            // end of __String_round
            uint64 frac = long(d);
            //unsigned long nines = frac;
            for (int j = 0; j < p.prec; j++) {
              d *= 10.0;
              printf("\td=%.12f, ifloor(d)=%lld, then ", d, IFLOOR(d));
              frac = IFLOOR(d);
              d -= floor(d); //FLOOR(d);
              /*
                If you do this with the value 3.1415, you get "3.1414".
                The last value of d=5.0000000 is cast to 4.
                Why????
               */
              printf("d=%f, frac=%d => '%c'\n", d, (int)frac, char('0' + char(frac % 10)));
              dbuf[j] = char('0' + char(frac % 10));
            }
#else
            uint64 frac = long(d);
            for (int j = 0; j < p.prec; j++) {
              dbuf[p.prec - j - 1] = char('0' + char(frac % 10));
              frac /= 10;
            }
            if (frac >= 1) intpart_i++; // Added 2024/05/22 CM
#endif
            dbuf[p.prec] = 0;
            // remove trailing 0
            if (p.conv == 'g') {
              int pos = p.prec - 1;
              while (pos >= 0 && dbuf[pos] == '0') {
                dbuf[pos] = 0;
                --pos;
              }
            }
          } else {
            if (value > 0.5) intpart_i++;
          }
          _uitoa(buf, p, (uint64)intpart_i);
          if (p.prec > 0 && *dbuf != 0) { // has post-decimal part
            buf += '.';
            buf += dbuf;
          }
          _padl(str, p, (int)buf.length(), sign);
          str += buf;
        }
        break;
      case 'x': case 'X': case 'p':
        {
          p.base = 16;
          if (p.conv == 'X') p.uc = true;
          uint64 value = (uint64)(args[argp++].toInt());
          string buf;
          buf.reserve(16);
          _uitoa(buf, p, value);
          _padl(str, p, (int)buf.length(), NOSIGN);
          str += buf;
        }
        break;
      case 's':
        {
          const char *s = args[argp++].toCharP();
          if (s) str += s;
        }
        break;
      case '%':
        str += '%';
        break;
      default:
        //throw Exception("Unsupported format code %%%c", *fmt);
        str += *fmt;
        break;
      }
      state = 0;
      fmt++;
    }
  }
  return str;
}

///////////////////////////////////////////////////////////////////////////////
