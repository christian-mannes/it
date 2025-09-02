#include "Function.h"
#include "State.h"

CLASS(Quadratic, "Quadratic function") { public:
  // Declare your parameters and working variables here
  complex c;
  int depth;
  double escape;

Quadratic(String name, String label, int pspace) : Function(name, label, pspace) {
  // List every parameter you want to appear in the "parameters" section
  // using
  // PARAM(variable, name, type, default-value-parameters-space, default-value-dynamical-space)
  PARAM(c, "c", complex, complex(0, 0), complex(-1,0));
  PARAM(depth, "depth", int, 150, 150);
  PARAM(escape, "escape", double, 1000, 1000);
  // Set the default range for both spaces
  setDefaultRangeParameterSpace(-2.2, 1.4, -1.8, 1.8);
  setDefaultRangeDynamicalSpace(-2, 2, -2, 2);
}

Function *copy() {
  Quadratic *f = new Quadratic(name, "", pspace);
  return f->copyArgsFrom(this);
}

byte iterate(double x, double y) {
  int i;
  complex z, cc;
//  double a;
  if (PARAMETER_SPACE) {
    z = complex(0, 0);
    cc.set(x, y);
  } else {
    z = complex(x, y);
    cc = c;
  }
  for (i = 0; i < depth; i++) {
    z = z * z + cc;
    if (norm(z) > escape * escape) break;
  }
  //int x = (int)(depth * (x + 2.2) / 3.6);
  //return x;
  //int xx = (int)(255 * (x + 2.2) / 3.6);
  //return RGB(xx, 0, 0);
  //int xx = (int)(255 * (y + 1.8) / 3.6);
  //return RGB(xx, 0, 0);
  //int xx = (int)(255 * i / depth);
  //return RGB(xx, xx, xx);

  debug("%d %d", i, depth);

  return (byte)(i * 255 / depth);
}

// Forward orbit: assign f(x) to x
void orbit(complex &x) {
  x =  x * x + c;
}

// Set the parameter in dynamical space
void setParameter(double x, double y) {
  c.set(x, y);
}

void sandbox() {
  // Draws the orbit of the critical point
  complex z = 0;
  int i;
  for (i = 0; i < depth; i++) {
    z = z * z + c;
    SETPIXEL(z.real(), z.imag(), 100);
  }
  SetStrokeColor(0, 0, 0);
  DrawText("hello", 0, 0);
}
};

///////////////////////////////////////////////////////////////////////////////

Function *createBuiltinFunction(const std::string &name) {
  Function *p = nullptr, *d = nullptr;
  if (name == "Mandi") {
    p = new Quadratic("a", "b", 1);
    d = new Quadratic("a", "a", 0);
  }
  if (p && d) { p->other = d; d->other = p; }
  return p;
}

///////////////////////////////////////////////////////////////////////////////
