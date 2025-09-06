/************************************************************************

    Copyright (C) 1998-2006  Mannes Technology (http://www.mannes-tech.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    **************************************************************
    * This file is a modification of the standard gnu c++ library,
    * based on /usr/include/g++/std/complext.cc
    * Original Written by Jason Merrill based upon the 
    * specification in the 27 May 1994
    * C++ working paper, ANSI document X3J16/94-0098.
    **************************************************************

*************************************************************************/

#include "MTComplex.h"

#ifdef NEED_ASINH
double asinh(double x) {
  return log(x + sqrt(x * x + 1));
}
double acosh(double x) {
  return log(x + sqrt(x * x - 1));
}
#endif

complex
cos (const complex& x)
{
  return complex (cos (real (x)) * cosh (imag (x)),
			   - sin (real (x)) * sinh (imag (x)));
}

 complex
cosh (const complex& x)
{
  return complex (cosh (real (x)) * cos (imag (x)),
			   sinh (real (x)) * sin (imag (x)));
}

 complex
exp (const complex& x)
{
  return polar (double (exp (real (x))), imag (x));
}

 complex
log (const complex& x)
{
  return complex (log (abs (x)), arg (x));
}

 complex
pow (const complex& x, const complex& y)
{
  double logr = log (abs (x));
  double t = arg (x);

  return polar (double (exp (logr * real (y) - imag (y) * t)),
		double (imag (y) * logr + real (y) * t));
}

 complex
pow (const complex& x, double y)
{
  return exp (double (y) * log (x));
}

 complex
pow (double x, const complex& y)
{
  return exp (y * double (log (x)));
}

 complex
sin (const complex& x)
{
  return complex (sin (real (x)) * cosh (imag (x)),
			   cos (real (x)) * sinh (imag (x)));
}

 complex
sinh (const complex& x)
{
  return complex (sinh (real (x)) * cos (imag (x)),
			   cosh (real (x)) * sin (imag (x)));
}

#if 0 // -- CM
#include <iostream.h>

 istream&
operator >> (istream& is, complex& x)
{
  double re, im = 0;
  char ch = 0;

  if (is.ipfx0 ())
    {
      if (is.peek () == '(')
	is >> ch;
      is >> re;
      if (ch == '(')
	{
	  is >> ch;
	  if (ch == ',')
	    is >> im >> ch;
	}
    }
  is.isfx ();

  if (ch != 0 && ch != ')')
    is.setstate (ios::failbit);
  else if (is.good ())
    x = complex (re, im);

  return is;
}

 ostream&
operator << (ostream& os, const complex& x)
{
  return os << '(' << real (x) << ',' << imag (x) << ')';
}

#endif

// The code below is adapted from f2c's libF77, and is subject to this
// copyright:

/****************************************************************
Copyright 1990, 1991, 1992, 1993 by AT&T Bell Laboratories and Bellcore.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the names of AT&T Bell Laboratories or
Bellcore or any of their entities not be used in advertising or
publicity pertaining to distribution of the software without
specific, written prior permission.

AT&T and Bellcore disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall AT&T or Bellcore be liable for
any special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

 complex& complex::
operator /= (const complex& y)
{
  double ar = fabs (y.re);
  double ai = fabs (y.im);
  double nr, ni;
  double t, d;
  if (ar <= ai)
    {
      t = y.re / y.im;
      d = y.im * (1 + t*t);
      nr = (re * t + im) / d;
      ni = (im * t - re) / d;
    }
  else
    {
      t = y.im / y.re;
      d = y.re * (1 + t*t);
      nr = (re + im * t) / d;
      ni = (im - re * t) / d;
    }
  re = nr;
  im = ni;
  return *this;
}

 complex
operator / (const complex& x, const complex& y)
{
  double ar = fabs (real (y));
  double ai = fabs (imag (y));
  double nr, ni;
  double t, d;
  if (ar <= ai)
    {
      t = real (y) / imag (y);
      d = imag (y) * (1 + t*t);
      nr = (real (x) * t + imag (x)) / d;
      ni = (imag (x) * t - real (x)) / d;
    }
  else
    {
      t = imag (y) / real (y);
      d = real (y) * (1 + t*t);
      nr = (real (x) + imag (x) * t) / d;
      ni = (imag (x) - real (x) * t) / d;
    }
  return complex (nr, ni);
}

 complex
operator / (double x, const complex& y)
{
  double ar = fabs (real (y));
  double ai = fabs (imag (y));
  double nr, ni;
  double t, d;
  if (ar <= ai)
    {
      t = real (y) / imag (y);
      d = imag (y) * (1 + t*t);
      nr = x * t / d;
      ni = -x / d;
    }
  else
    {
      t = imag (y) / real (y);
      d = real (y) * (1 + t*t);
      nr = x / d;
      ni = -x * t / d;
    }
  return complex (nr, ni);
}

 complex
pow (const complex& xin, int y)
{
  if (y == 0)
    return complex (1.0);
  complex r (1.0);
  complex x (xin);
  if (y < 0)
    {
      y = -y;
      x = 1/x;
    }
  for (;;)
    {
      if (y & 1)
	r *= x;
      if (y >>= 1)
	x *= x;
      else
	return r;
    }
}

 complex
sqrt (const complex& x)
{
  double r = abs (x);
  double nr, ni;
  if (r == 0.0)
    nr = ni = r;
  else if (real (x) > 0)
    {
      nr = sqrt (0.5 * (r + real (x)));
      ni = imag (x) / nr / 2;
    }
  else
    {
      ni = sqrt (0.5 * (r - real (x)));
      if (imag (x) < 0)
	ni = - ni;
      nr = imag (x) / ni / 2;
    }
  return complex (nr, ni); 
}

/*************************************************************************/
/**************************** Extra functions ****************************/
/* 
 * 1995 - 1998, Christian Mannes, Nuria Fagella.
 */
/*************************************************************************/
#ifndef PI
#define PI 3.1415926535897932385
#endif

complex	exponent(complex z)
{
  double m = norm(z);
  return complex(z.real()-z.real()/m, z.imag()-z.imag()/m);
}

complex mod2pi(complex z)
{
  int ip = (int)(z.real() / (2*PI));
  return complex(z.real()-ip*2*PI, z.imag());
}

complex cacos(complex z)
{
  double m = norm(z);
  if(z.imag() == 0.0) {
    if (m <= 1.0)
      return complex(acos(z.real()), 0);
    else {
      if(z.real() > 0.0) 
	return complex(0, acosh(z.real()));
      else
	return complex(PI, acosh(-z.real()));
    }
  } else {
    double im = asinh(sqrt(0.5*(m-1+sqrt((m-1)*(m-1)+4*z.imag()*z.imag()))));
    double re = acos(z.real()/cosh(im));
    if(z.imag() > 0.0) {
      return complex(-re,im);
    } else {
      return complex(re,im);
    }
  }
}

/********************************* EOF ************************************/
