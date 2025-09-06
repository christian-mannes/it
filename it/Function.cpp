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
#include "Function.h"
#include "State.h"
//#include "Gif.h"

#define COMPRESS

/************************************************************************/

Function::Function(String name, String label, int _pspace) { 
  this->name = name;
  this->descr = name;
  other = 0;
  doannotate = 0;
  defxmin = 0.0; defxmax = 1.0;
  defymin = 0.0; defymax = 1.0;
  pspace = _pspace;
  iscopy = false;
  doDebug = false;
}

Function *Function::copy_() {
  Function *copied = copy();
  if (copied != nullptr) {
    copied->iscopy = true;
    return copied;
  }
  return this;
}

Function *Function::copyArgsFrom(Function *f) {
  name = f->name;
  descr = f->descr;
  other = f->other;
  doannotate = f->doannotate;
  pspace = f->pspace;
  state = f->state;
  doDebug = f->doDebug;
  // assert args.count() == f->args.count()
  for (int i = 0; i < f->args.count(); i++) {
    ItArg *arg = f->args.getArgAt(i);
    String str = arg->toString();
    args.getArgAt(i)->parse(str.c_str());
  }
  return this;
}

Function::~Function() { 
}

void Function::defaults() {
  int n = args.count();
  for (int i = 0; i < n; i++) {
    ItArg *arg = args.getArgAt(i);
    arg->parse(pspace == 1 ? arg->value_p.c_str() : arg->value_d.c_str());
  }
}

void Function::setArg(const char *key, const char *val) {
  ItArg *arg = args.getArg(key);
  if (arg != 0) {
    arg->parse(val);
  }
}

String Function::getArg(const char *key) {
  ItArg *arg = args.getArg(key);
  if (arg == 0) return "?";
  return arg->toString();
}

void Function::setDefaultRange(double x, double xm, double y, double ym) {
  defxmin = x;
  defxmax = xm;
  defymin = y;
  defymax = ym;
}

void Function::setDefaultRangeParameterSpace(double x, double xm, double y, double ym) {
  if (pspace == 1) {
    defxmin = x;
    defxmax = xm;
    defymin = y;
    defymax = ym;
  }
}

void Function::setDefaultRangeDynamicalSpace(double x, double xm, double y, double ym) {
  if (pspace == 0) {
    defxmin = x;
    defxmax = xm;
    defymin = y;
    defymax = ym;
  }
}

void Function::setMaxDebug(int limit) {
  state->setMaxDebug(limit);
}
void Function::start(bool debug_enabled) {
  doDebug = debug_enabled;
  state->start();
}

double Function::rgb(int r, int g, int b) {
  union { double d; uint64_t i; } u;
  u.i = (0x8000000000000000ULL | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF));
  return u.d;
}

byte Function::iterate(double x, double y) {
  return 0;
}

double Function::iterate_(double x, double y) {
  return (double)iterate(x, y) / 255.0;
}

void Function::preview(double px, double py, State *thumbnail) {
  if (thumbnail == 0) return;
  int w = thumbnail->getWidth();
  int h = thumbnail->getHeight();
  setParameter(px, py);
  State *origstate = state;
  state = thumbnail;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      thumbnail->setPixel(x, y, iterate(thumbnail->X(x), thumbnail->Y(y)));
    }
  }
  state = origstate;
}

int Function::i_random(int n) {
  return random.next(n);
}

/************************************************************************
 
 This algorithm to draw rays and equipotentials is due to
 Christian Henriksen and Xavier Buff (Nov 17, 1999).
 
 *************************************************************************/
#define TWO_PI 6.2831853071796
#define ABS(x) sqrt(norm(x))	/* abs is slow */

/************************************************************************/

// The following is to draw rays and equipotentials
void Itinerary(int p, int q, int depth, double* itinerary, int degree) {
  
  int i;
  
  itinerary[0] = double(p)/q;
  for (i = 1; i < depth; i++) {
    p = p * degree;
    while (p >= q) {
      p = p - q; 
    }
    itinerary[i] = double(p)/q;
  } 
}

//*****************************************************************//
//In the dynamical plane, we compute f_c^n(w) and [f_c^n]'(w)      // 
//In the parameter plane, we compute f_c^n(c) and [f_c^n]'(c)      //
//where n is sufficiently large, so that |f_c^n(.)| > escaperadius //
//*****************************************************************//


void FnDfn(complex c, int depth, int pspace,
           complex w, double escaperadius,
           complex& Fn, complex&  Dfn, int& i)  { 
  
  complex dfn=1, param=0, cc = c;
  
  i=0; 
  if (pspace) {
    param = 1;
    cc = w;
  }
  while (norm(w) < escaperadius * escaperadius && i < depth) { 
    dfn = param + dfn * 2 * w;                  // dfn * df/dw //
    w = w * w + cc;                                // f_b(w) //
    i++;
  }
  
  Fn = w; 
  Dfn = dfn;
}

//***************************************************************//
// We draw the ray with angle p/q, starting at startingpotential //
//***************************************************************//

void Function::rational_rays(complex cc, int depth,
                             int p, int q, double startingpot,
                             double escaperad, int npoints) {
  double dx=(state->xmax - state->xmin)/(2*state->xres);
  double dy=(state->ymax - state->ymin)/(2*state->yres);
  int degree = 2; // This is the degree of the polynomial //
  
  complex b=escaperad*polar(1,TWO_PI*p/q);
  complex fn=b, dfn=1, goal, c=cc;
  
  double *itinerary = new double[depth];
  int noit = 0, nit;
  
  Itinerary(p, q, depth, itinerary, degree);
  FnDfn(c, depth, pspace, b, escaperad, fn, dfn, nit);
  goal=abs(fn)*polar(1,TWO_PI*itinerary[nit]);
  
  while( norm(fn) >= escaperad*escaperad-1 && (noit < npoints) ) {
    if ( abs(fn)/2 > abs(dfn)*(dx+dy)/4 
        && state->xmin <= real(b) && real(b) <= state->xmax
        && state->ymin <= imag(b) && imag(b) <= state->ymax)
      goal = (abs(fn)-abs(dfn)*(dx+dy)/4)*polar(1,TWO_PI*itinerary[nit]);
    else
      goal = (abs(fn)/2)*polar(1,TWO_PI*itinerary[nit]);
    
    // We now compute an approximation of the new value of b //
    b = b + (goal-fn)/dfn;
    FnDfn(c, depth, pspace, b, escaperad, fn, dfn, nit);
    if ( (1/pow(1.0*degree,(double)nit)*log(abs(fn)) <= startingpot)  
        && state->xmin <= real(b) && real(b) <= state->xmax
        && state->ymin <= imag(b) && imag(b) <= state->ymax) {
      state->setPixel(state->invX(b.re), state->invY(b.im), 255);
    }
    noit++;
  }
  delete [] itinerary;
}

//********************************************************************//
// The purpose of this function is to find the point with coordinates //
// potential and angle, following itinerary to choose the roots       //
//********************************************************************//

void goto_potential(complex c, complex& b, complex& fn, complex& dfn, 
                    double potential, 
                    double* itinerary, int depth,
                    int pspace, double escaperadius, int& nit) {
  
  int degree = 2; // This is the degree of the polynomial //
  complex goal; 
  
  int noit=0;
  
  b=escaperadius*polar(1,TWO_PI*itinerary[0]); 
  
  FnDfn(c, depth, pspace, b, escaperadius, fn, dfn, nit);
  goal=abs(fn)*polar(1,TWO_PI*itinerary[nit]);
  
  while( 1/pow(1.0*degree,(double)nit)*log(abs(fn)) > potential && noit < 300000) {
    
    goal = (abs(fn)/2)*polar(1,TWO_PI*itinerary[nit]);
    if ( 1/pow(1.0*degree,(double)nit)*log(abs(goal)) < potential )
      goal = exp(complex(pow(1.0*degree,(double)nit)*potential,TWO_PI*itinerary[nit]));
    
    // We now compute an approximation of the new value of b //
    b = b + (goal-fn)/dfn;
    FnDfn(c, depth, pspace, b, escaperadius, fn, dfn, nit);
    noit++;
  }    
}

//********************************************************************//
// We draw the piece of equipotential at level potential, starting at //
// angle p_initial / q_initial, which must be in [0, 1),              //
// and ending at angle theta_terminal > p_initial / q_initial         //
//********************************************************************//

void Function::draw_equi(complex cc, double  potential, int p_initial, 
                         int q_initial, double theta_terminal, double escaperadius){
  
  double dx = (state->xmax - state->xmin)/(2*state->xres);
  double dy = (state->ymax - state->ymin)/(2*state->yres);
  int degree = 2; // this is the degree of the polynomial ; 
  
  
  int depth = 100 + 
  int(floor((log(log(escaperadius)) - log(potential)) / log(degree)));   
  double *itinerary = new double[depth];
  
  int noit=0, nit;
  complex b, fn, dfn, goal;
  
  Itinerary(p_initial, q_initial, depth, itinerary, degree);
  
  goto_potential(cc, b, fn, dfn, potential,
                 itinerary, depth, pspace, escaperadius, nit);
  
  
  double angle = itinerary[nit];
  double finalangle = itinerary[nit]+
  pow(1.0*degree,(double)nit)*(theta_terminal-double(p_initial)/ q_initial);
  
  while (angle < finalangle && noit < 300000 ) {
    
    if ( abs(fn)/2 > abs(dfn)*(dx+dy)/4 
        && state->xmin <= real(b) && real(b) <= state->xmax
        && state->ymin <= imag(b) && imag(b) <= state->ymax)
      angle += abs(dfn)*(dx+dy)/4/abs(fn)/TWO_PI;
    else 
      angle += 1/2.0/TWO_PI;
    
    goal = polar(exp(pow(1.0*degree,(double)nit)*potential),TWO_PI*angle);
    
    // We now compute an approximation of the new value of b //
    b = b + (goal-fn)/dfn;
    FnDfn(cc, depth, pspace, b, escaperadius, fn, dfn, nit);
    
    
    if( state->xmin <= real(b) && real(b) <= state->xmax
       && state->ymin <= imag(b) && imag(b) <= state->ymax)
      state->setPixel(state->invX(b.re), state->invY(b.im), 255);
    
    
    noit++; 
  }
  delete [] itinerary;
}

//*************************************************************//
// We draw a "diagonal ray" (boundary of a sector)  with slope (slope) 
//along the ray p/q //
//*************************************************************//

void Function::draw_sect(complex c, int p, int q, double slope, 
                         double startingpotential, 
                         double escaperadius, int depth,
                         int npoints){
  double dx = (state->xmax - state->xmin)/(2*state->xres);
  double dy = (state->ymax - state->ymin)/(2*state->yres);
  
  int degree = 2; // this is the degree of the polynomial ; 
  
  double *itinerary = new double[depth];
  Itinerary(p, q, depth, itinerary, degree);
  
  complex b = polar(escaperadius,TWO_PI*p/q+slope*log(escaperadius)); 
  
  complex fn=b, dfn=1, goal;
  double goalpot, goalarg;
  int nit, noit=0;
  
  FnDfn(c, depth, pspace, b, escaperadius, fn, dfn, nit);
  
  
  while( norm(fn) > escaperadius*escaperadius-1 && noit < npoints ){
    if ( abs(fn)/4 > abs(dfn)*(dx+dy)/4 
        && state->xmin <= real(b) && real(b) <= state->xmax
        && state->ymin <= imag(b) && imag(b) <= state->ymax
        )
      goalpot = log(abs(fn))*(abs(log(fn))-abs(dfn)*(dx+dy)/4/abs(fn))
      /(abs(log(fn)));
    else
      goalpot = log(abs(fn))*(abs(log(fn))-1.0/4)/(abs(log(fn)));
    goalarg = TWO_PI*(itinerary[nit]+slope*goalpot/TWO_PI-
                      floor((double)(itinerary[nit]+slope*goalpot/TWO_PI+.5
                            -arg(fn)/TWO_PI)));
    
    goal = complex(goalpot,goalarg);
    
    // We now compute an approximation of the new value of b //
    b = b + (goal-log(fn))*fn/dfn;
    FnDfn(c, depth, pspace, b, escaperadius, fn, dfn, nit);
    
    if ( (1/pow(1.0*degree,(double)nit)*log(abs(fn)) <= startingpotential)  
        && state->xmin <= real(b) && real(b) <= state->xmax
        && state->ymin <= imag(b) && imag(b) <= state->ymax) {
      state->setPixel(state->invX(b.re),state-> invY(b.im), 255);
    }
    noit++;
  }
  delete [] itinerary;
}

/////////////////////////// Drawing functions ////////////////////////////

void Function::ClearAnnotations() {
  for (Annotation *a: annotations) delete a;
  annotations.clear();
}
void Function::AddAnnotation(const char *fun, double rc, double x0, double x1, double x2, double x3, const char *str) {
  annotations.push_back(new Annotation(fun, rc, x0, x1, x2, x3, str));
}

void Function::SetStrokeColor(double r, double g, double b, double opa) {
  AddAnnotation("SetStrokeColor", false, r, g, b, opa);
}
void Function::SetFillColor(double r, double g, double b, double opa) {
  AddAnnotation("SetFillColor", false, r, g, b, opa);
}
void Function::SetLineWidth(double w) {
  AddAnnotation("SetLineWidth", false, w, 0, 0, 0);
}
void Function::DrawLine(double x0, double y0, double x1, double y1, bool realcoords) {
  AddAnnotation("DrawLine", realcoords, x0, y0, x1, y1);
}
void Function::DrawRect(double x, double y, double w, double h, bool realcoords) {
  AddAnnotation("DrawRect", realcoords, x, y, w, h);
}
void Function::FillRect(double x, double y, double w, double h, bool realcoords) {
  AddAnnotation("FillRect", realcoords, x, y, w, h);
}
void Function::DrawEllipseInRect(double x, double y, double w, double h, bool realcoords) {
  AddAnnotation("DrawEllipseInRect", realcoords, x, y, w, h);
}
void Function::FillEllipseInRect(double x, double y, double w, double h, bool realcoords) {
  AddAnnotation("FillEllipseInRect", realcoords, x, y, w, h);
}
void Function::SetFont(const char *name, double size) {
  AddAnnotation("SetFont", false, size, 0, 0, 0, name);
}
void Function::DrawText(const char *txt, double x, double y, bool realcoords) {
  AddAnnotation("DrawText", realcoords, x, y, 0, 0, txt);
}
/******************************** EOF ***********************************/
