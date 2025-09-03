/************************************************************************

    Copyright (C) 1998-2008  Mannes Technology (http://www.mannes-tech.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/
#pragma once
#include "Args.h"
#include "State.h"
#include "MTRandom.h"
#include "MTComplex.h"
#include "debug.h"
#include <vector>
#define String std::string
/**************************** Macros ************************************/

#define ARG(VAR, NAME, TYPE, VALUE, LEGAL) \
  do { ItArg *arg = new ItArg(NAME, T_##TYPE, (void *)&VAR, LEGAL, 0); \
    VAR = VALUE; args.addArg(#VAR, arg); } while(0)

#define BASEARG(VAR, NAME, TYPE, VALUE, LEGAL) \
  do { ItArg *arg = new ItArg(NAME, T_##TYPE, (void *)&VAR, LEGAL, 1); \
    VAR = VALUE; args.addArg(#VAR, arg); } while(0)

#define PARAM(VAR, NAME, TYPE, VALUEP, VALUED) \
  do { \
    ItArg *arg = new ItArg(NAME, T_##TYPE, (void *)&VAR, "", 0); \
    VAR = VALUED; \
    arg->setValue(); \
    arg->value_d = arg->value; \
    VAR = VALUEP; \
    arg->value_p = arg->value; \
    args.addArg(#VAR, arg); \
  } while(0)

#define ANY ""
#define FLAG "0,1"

#define PARAMETER_SPACE (pspace==1)
#define DYNAMICAL_SPACE (pspace==0)
#define XMIN (state->xmin)
#define XMAX (state->xmax)
#define YMIN (state->ymin)
#define YMAX (state->ymax)
#define XRES (state->xres)
#define YRES (state->yres)
#define SETCOLOR(i,r,g,b) state->setColor(i,r,g,b)

#define CLASS(CN, LBL) class CN : public Function
#ifdef ABS
#undef ABS
#endif
#define ABS(x) sqrt(norm(x))	/* abs is slow */

/************************************************************************/

class Annotation {
public:
  String f;
  bool realcoords;
  double p0, p1, p2, p3;
  String s;
  Annotation(const char *fun, double rc, double x0, double x1, double x2, double x3, const char *str) {
    f = fun;
    realcoords = rc;
    p0 = x0;
    p1 = x1;
    p2 = x2;
    p3 = x3;
    if (str) s = str;
  }
};

typedef unsigned char byte;

class Function {
public:
  Function(String name, String label, int pspace);
  virtual ~Function();
  Function *copy_(); // wrapped copy
  Function *copyArgsFrom(Function *f);
  void preview(double px, double py, State *thumbnail);
  void setArg(const char *key, const char *val);
  String getArg(const char *key);  
  double rgb(int r, int g, int b);
  void setDefaultRange(double x, double xm, double y, double ym);
  void setDefaultRangeParameterSpace(double x, double xm, double y, double ym);
  void setDefaultRangeDynamicalSpace(double x, double xm, double y, double ym);
  void start(bool debug); // called before rendering, init debugging
  /* 
   * These can/must be overridden by subclasses 
   */
  virtual Function *copy() { return 0; } // can only use parallelism if copy works
  virtual void defaults();
  virtual void setColors() {}
  virtual void setParameter(double x, double y) {}
  // Classif iterate: return 0..255
  virtual byte iterate(double x, double y);
  // Return double in [0,1] or a NAN (all 1s first 11 bits) with 0xffRRGGBB
  // Override one or the other
  virtual double iterate_(double x, double y);
  virtual void orbit(complex &z) {} // compute next point in orbit
  virtual int fixedPoints(complex p[]) { return 0; }
  virtual int preImages(complex z, complex p[]) { return 0; }
  virtual void sandbox() {}
  virtual void annotate() {}
  virtual void mouseDown(double x, double y) {}
public:
  void rational_rays(complex cc, int depth,
                int p, int q, double startingpot,
                double escaperad, int npoints);
  void draw_equi(complex cc, double  potential, int p_initial, 
            int q_initial, double theta_terminal, double escaperadius);
  void draw_sect(complex c, int p, int q, double slope, 
            double startingpotential, 
            double escaperadius, int depth,
            int npoints);
public:
  String getName() { return name; }
  String getDescr() { return descr; }
  int i_random(int n);
public:
  ItArgs args;        // arguments
  String name;			  // my name
  String descr;			  // short textual description
  int pspace;			    // pspace==1: parameter space
  int doannotate;     // whether to run annotation on top
  Function *other;    // the other space 
  double defxmin, defxmax, defymin, defymax;
  Random random;      // random generator
  bool iscopy;        // Set true for copies
public:
  void ClearAnnotations();
  void SetStrokeColor(double r, double g, double b, double opa=255);
  void SetFillColor(double r, double g, double b, double opa=255);
  void SetLineWidth(double w);
  void DrawLine(double x0, double y0, double x1, double y1, bool realcoords = true);
  void DrawRect(double x, double y, double w, double h, bool realcoords = true);
  void FillRect(double x, double y, double w, double h, bool realcoords = true);
  void DrawEllipseInRect(double x, double y, double w, double h, bool realcoords = true);
  void FillEllipseInRect(double x, double y, double w, double h, bool realcoords = true);
  void SetFont(const char *name, double size);
  void DrawText(const char *txt, double x, double y, bool realcoords = true);
public:
  State *state;       // current state (do NOT touch this)
  std::vector<Annotation *> annotations;
  void AddAnnotation(const char *fun, double rc, double x0, double x1, double x2, double x3, const char *str = 0);
protected:
  bool doDebug;
  void setMaxDebug(int limit);

  template <typename... Args>
  std::string fmt(const char *format, const Args&... args) {
    Arg arg_array[] = {args...};
    return _fmt(format, arg_array, sizeof...(Args));
  }

  template <typename... Args>
  void debug(const char *format, const Args&... args) {
    if (!doDebug) return;
    Arg arg_array[] = {args...};
    std::string str = _fmt(format, arg_array, sizeof...(Args));
    state->_debug(str, true);
  }

  template <typename... Args>
  void debugn(const char *format, const Args&... args) {
    if (!doDebug) return;
    Arg arg_array[] = {args...};
    std::string str = _fmt(format, arg_array, sizeof...(Args));
    state->_debug(str, false);
  }
  template <typename... Args>
  void noprint(const char *format, const Args&... args) {
  }
};

Function *createBuiltinFunction(const std::string &name);

/******************************** EOF ***********************************/
