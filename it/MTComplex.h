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
    * based on /usr/include/g++/std/complext.h
    * Original Written by Jason Merrill based upon the 
    * specification in the 27 May 1994
    * C++ working paper, ANSI document X3J16/94-0098.
    **************************************************************

*************************************************************************/

#ifndef __ITCOMPLEX__
#define __ITCOMPLEX__

#ifdef WINDOWS
#pragma warning(disable: 4996)
#endif

#include <cmath>

#define __attribute__(foo) /* Ignore constification */

class complex {
public:
  double re, im;
  complex (double r = 0, double i = 0): re (r), im (i) { }
  complex& operator += (const complex&);
  complex& operator -= (const complex&);
  complex& operator *= (const complex&);
  complex& operator /= (const complex&);
  double real () const { return re; }
  double imag () const { return im; }
  void set(double r, double i) { re = r; im = i; }
private:
  // These functions are specified as friends for purposes of name injection;
  // they do not actually reference private members.
  friend double real (const complex&) __attribute__ ((const));
  friend double imag (const complex&) __attribute__ ((const));
  friend complex operator + (const complex&, const complex&) __attribute__ ((const));
  friend complex operator + (const complex&, double) __attribute__ ((const));
  friend complex operator + (double, const complex&) __attribute__ ((const));
  friend complex operator - (const complex&, const complex&) __attribute__ ((const));
  friend complex operator - (const complex&, double) __attribute__ ((const));
  friend complex operator - (double, const complex&) __attribute__ ((const));
  friend complex operator * (const complex&, const complex&) __attribute__ ((const));
  friend complex operator * (const complex&, double) __attribute__ ((const));
  friend complex operator * (double, const complex&) __attribute__ ((const));
  friend complex operator / (const complex&, const complex&) __attribute__ ((const));
  friend complex operator / (const complex&, double) __attribute__ ((const));
  friend complex operator / (double, const complex&) __attribute__ ((const));
  friend bool operator == (const complex&, const complex&) __attribute__ ((const));
  friend bool operator == (const complex&, double) __attribute__ ((const));
  friend bool operator == (double, const complex&) __attribute__ ((const));
  friend bool operator != (const complex&, const complex&) __attribute__ ((const));
  friend bool operator != (const complex&, double) __attribute__ ((const));
  friend bool operator != (double, const complex&) __attribute__ ((const));
  friend complex polar (double, double) __attribute__ ((const));
  friend complex pow (const complex&, const complex&) __attribute__ ((const));
  friend complex pow (const complex&, double) __attribute__ ((const));
  friend complex pow (const complex&, int) __attribute__ ((const));
  friend complex pow (double, const complex&) __attribute__ ((const));
  //friend istream& operator>> (istream&, complex&);
  //friend ostream& operator<< (ostream&, const complex&);
};

inline complex&
complex::operator += (const complex& r)
{
  re += r.re;
  im += r.im;
  return *this;
}


inline complex&
complex::operator -= (const complex& r)
{
  re -= r.re;
  im -= r.im;
  return *this;
}


inline complex&
complex::operator *= (const complex& r)
{
  double f = re * r.re - im * r.im;
  im = re * r.im + im * r.re;
  re = f;
  return *this;
}

 inline double
imag (const complex& x) __attribute__ ((const))
{
  return x.imag ();
}

 inline double
real (const complex& x) __attribute__ ((const))
{
  return x.real ();
}

 inline complex
operator + (const complex& x, const complex& y) __attribute__ ((const))
{
  return complex (real (x) + real (y), imag (x) + imag (y));
}

 inline complex
operator + (const complex& x, double y) __attribute__ ((const))
{
  return complex (real (x) + y, imag (x));
}

 inline complex
operator + (double x, const complex& y) __attribute__ ((const))
{
  return complex (x + real (y), imag (y));
}

 inline complex
operator - (const complex& x, const complex& y) __attribute__ ((const))
{
  return complex (real (x) - real (y), imag (x) - imag (y));
}

 inline complex
operator - (const complex& x, double y) __attribute__ ((const))
{
  return complex (real (x) - y, imag (x));
}

 inline complex
operator - (double x, const complex& y) __attribute__ ((const))
{
  return complex (x - real (y), - imag (y));
}

 inline complex
operator * (const complex& x, const complex& y) __attribute__ ((const))
{
  return complex (real (x) * real (y) - imag (x) * imag (y),
			   real (x) * imag (y) + imag (x) * real (y));
}

 inline complex
operator * (const complex& x, double y) __attribute__ ((const))
{
  return complex (real (x) * y, imag (x) * y);
}

 inline complex
operator * (double x, const complex& y) __attribute__ ((const))
{
  return complex (x * real (y), x * imag (y));
}

 inline complex
operator / (const complex& x, double y) __attribute__ ((const))
{
  return complex (real (x) / y, imag (x) / y);
}

 inline complex
operator + (const complex& x) __attribute__ ((const))
{
  return x;
}

 inline complex
operator - (const complex& x) __attribute__ ((const))
{
  return complex (-real (x), -imag (x));
}

 inline bool
operator == (const complex& x, const complex& y) __attribute__ ((const))
{
  return real (x) == real (y) && imag (x) == imag (y);
}

 inline bool
operator == (const complex& x, double y) __attribute__ ((const))
{
  return real (x) == y && imag (x) == 0;
}

 inline bool
operator == (double x, const complex& y) __attribute__ ((const))
{
  return x == real (y) && imag (y) == 0;
}

 inline bool
operator != (const complex& x, const complex& y) __attribute__ ((const))
{
  return real (x) != real (y) || imag (x) != imag (y);
}

 inline bool
operator != (const complex& x, double y) __attribute__ ((const))
{
  return real (x) != y || imag (x) != 0;
}

 inline bool
operator != (double x, const complex& y) __attribute__ ((const))
{
  return x != real (y) || imag (y) != 0;
}

// Some targets don't provide a prototype for hypot when -ansi.
//extern "C" double hypot (double, double) __attribute__ ((const));

 inline double
abs (const complex& x) __attribute__ ((const))
{
  return hypot (real (x), imag (x));
}

 inline double
arg (const complex& x) __attribute__ ((const))
{
  return atan2 (imag (x), real (x));
}

 inline complex
polar (double r, double t) __attribute__ ((const))
{
  return complex (r * cos (t), r * sin (t));
}

 inline complex
conj (const complex& x)  __attribute__ ((const))
{
  return complex (real (x), -imag (x));
}

 inline double
norm (const complex& x) __attribute__ ((const))
{
  return real (x) * real (x) + imag (x) * imag (x);
}

// Declarations of templates in complext.ccI

 complex
  operator / (const complex&, const complex&) __attribute__ ((const));
 complex
  operator / (double, const complex&) __attribute__ ((const));
 complex
  cos (const complex&) __attribute__ ((const));
 complex
  cosh (const complex&) __attribute__ ((const));
 complex
  exp (const complex&) __attribute__ ((const));
 complex
  log (const complex&) __attribute__ ((const));
 complex
  pow (const complex&, const complex&) __attribute__ ((const));
 complex
  pow (const complex&, double) __attribute__ ((const));
 complex
  pow (const complex&, int) __attribute__ ((const));
 complex
  pow (double, const complex&) __attribute__ ((const));
 complex
  sin (const complex&) __attribute__ ((const));
 complex
  sinh (const complex&) __attribute__ ((const));
 complex
  sqrt (const complex&) __attribute__ ((const));

class istream;
class ostream;
 istream& operator >> (istream&, complex&);
 ostream& operator << (ostream&, const complex&);

/*************************************************************************/
/**************************** Extra functions ****************************/
/*************************************************************************/

complex	exponent(complex);
complex mod2pi(complex);
complex cacos(complex);

#ifndef asinh
double asinh(double x);
#endif
#ifndef acosh
double acosh(double x);
#endif

#endif /* __ITCOMPLEX__ */

/********************************* EOF ************************************/
