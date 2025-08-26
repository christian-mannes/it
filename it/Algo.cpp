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

*************************************************************************/
#include "Function.h"
#include "State.h"
#include "Algo.h"
#include "MTRandom.h"
#include <stdio.h>

void requestRedraw() {} // global function that requests a redraw

/*
 * IMPORTANT: if you add a new Algorithm, add it to registerAlgorithms at 
 * the end of this file.
 */
void Algorithm::start(Function *f, State *state) {
  this->state = state;
  f->state = state;
  xres = state->getWidth();
  yres = state->getHeight();
  f->setColors();
}

void Algorithm::piece(int part, int parts, Function *f) {
  if (parts <= 1) {
    init(f);
    running = true;
    run(f);
    running = false;
  } else {
    int chunk = yres / parts;
    int start = chunk * part;
    int end = part == parts - 1 ? yres : chunk * (part + 1);
    for (int y = start; y < end; y++) {
      for (int x = 0; x < xres; x++) {
        state->setPixel(x, y, f->iterate(state->X(x), state->Y(y)));
      }
    }  
  }
}

void Algorithm::stop() {
  running = false;
}

class Sandbox : public Algorithm {
public:
  Sandbox() : Algorithm("Sandbox") {}
  void init(Function *f) {  }
  void run(Function *f) {
    f->sandbox();
  }
};

class Linear : public Algorithm { 
  int x, y;
public:
  Linear() : Algorithm("Linear") { }
  void init(Function *f) {
    x = 0; y = 0;
  }
  void run(Function *f) {
    int count = 0;
    for (y = 0; y < yres; y++) {
      for (x = 0; x < xres; x++) {
        state->setPixel(x, y, f->iterate(state->X(x), state->Y(y)));
        if (!running) return;
        count++;
        if (count % 1000 == 0) 
          requestRedraw();
      }
    }
  }
};

class Refine : public Algorithm { 
  int n, rw, rh;
public:
  Refine() : Algorithm("Refine") {
  }
  void init(Function *f) {
    n = 1;			// current size box
    rw = xres - 1;
    rh = yres - 1;
  }
  void run(Function *f) {
    int i, j, x, y;
    bool goOn = true;
    while (running && goOn) {
      goOn = false;
      for (i = 0; i < n; i++) {	// for each rectangle of current size
        for (j = 0; j < n; j++) {
          if (!running) return;
          int l = (i*rw)/n;
          int r = ((i+1)*rw)/n;
          int t = (j*rh)/n;
          int b = ((j+1)*rh)/n;
          if ((r-l)*(b-t) < 4) { // small, fill up completely
            for (x = l; x <= r; x++) {
              for (y = t; y <= b; y++) {
                if (!state->isSet(x, y))
                  state->setPixel(x, y, f->iterate(state->X(x), state->Y(y)));
              }
            }
          } else {		// approximate
            int xm = (r+l)/2;
            int ym = (t+b)/2;
            //int aw = r-l+1;
            //int ah = b-t+1;
            byte col = f->iterate(state->X(xm), state->Y(ym));
            //if (!state->isSet(xm, ym)) 
              state->setPixel(xm, ym, col);
            //state->setRegionTemp(l, r, t, b, col);
            goOn = true;
          }
        }
      }
      n *= 2;
      if (n > 4) {
        requestRedraw();
      }
    }
  }
};

/*
class Quad_IIM : public Algorithm {
  int depth;
  double escape;
  int pixcount;
public:
  Quad_IIM() : Algorithm("Inverse Iteration") {
  }
  void init(Function *f) {}
  void run(Function *f) {
    pixcount = 0;
    depth = f->args.getInt("depth");
    escape = f->args.getDouble("escape");
    printf("%d %g\n", depth, escape);
    complex p[2];
    if (f->fixedPoints(p) != 2)
      return;
    inverse_r(p[0], ABS(2.0 * p[0]), 0);
    inverse_r(p[1], ABS(2.0 * p[1]), 0);
  }
  void inverse_r(complex z, double d, int level) {
    complex pre[2];
    if (level >= depth || d > escape)
      return;
    int x = state->invX(z.re);
    int y = state->invY(z.im);
    if (x >= 0 && x < xres && y >= 0 && y < yres) {
      state->setPixel(x, y, 255);
      pixcount++;
    }
    if (pixcount > xres * yres)
      return;
    f->preImages(z, pre);
    double dd = 2 * d * ABS(pre[0]);

    if (pixcount % 1000 == 0) {
      printf("%d pix\n", pixcount);
      requestRedraw();
    }
    inverse_r(pre[0], dd, level+1);
    inverse_r(pre[1], dd, level+1);
  }
};

class IFS_IIM : public Algorithm { 
  complex z[2];
  int effective;
  int pixcount;
  Random rand;
public:
  IFS_IIM() : Algorithm("IFS_IIM") {}
  void init() {}
  void run() {
    pixcount = 0;
    f->fixedPoints(z);
    effective = 0;
    complex pre[2];
    for (int i = 0; i < 1000; i++) {
      f->preImages(z[0], pre);
      pixcount++;
      if (pixcount > 10) {
        complex x = pre[rand.Next(2)];
        int ix = state->invX(x.re);
        int iy = state->invY(x.im);
        if (ix >= 0 && ix < xres && iy >= 0 && iy < yres) {
          state->setPixel(ix, iy, 150);
          effective++;
        }
      }
    }
    requestRedraw();
    //return (effective >= xres * yres ? 0 : 1);
  }
};
*/

#if 0
if (PARAMETER_SPACE)
return 0;
complex r(cos(alpha), sin(alpha));
z = complex((1/beta)*(-1 - sqrt(1-beta*beta)),0);
for (int i = 0; i < depth; i++) {
  z =  z * r * exp((beta/2)*(z - 1/z));
  setPixel(invX(z.re), invY(z.im), 255);
  pixcount++;
  if (pixcount % 1000 == 0)
    update();
}
return 1;
  }
  }};
#endif

Algorithm *makeAlgorithm(const char *name) {
  String n(name);
  for(auto& c : n) {
    c = tolower(c);
  }
  if (n == "linear") return new Linear();
  else if (n == "refine") return new Refine();
  else return new Linear();
}
/*
void registerAlgorithms(It *it) {
  it->addAlgo(new Refine());
  it->addAlgo(new Linear());
  it->addAlgo(new Quad_IIM());
  //it->addAlgo(new Sandbox());
}*/

/******************************* EOF ************************************/
