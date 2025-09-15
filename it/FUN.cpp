#include "Function.h"
#include "State.h"

CLASS(Quadratic, "Quadratic function") { public:
  // Declare your parameters and working variables here
  complex C;
  int depth;
  double escape;

Quadratic(String name, String label, int pspace) : Function(name, label, pspace) {
  // List every parameter you want to appear in the "parameters" section
  // using
  // PARAM(variable, name, type, default-value-parameters-space, default-value-dynamical-space)
  PARAM(C, "C", complex, complex(0, 0), complex(-1,0));
  PARAM(depth, "depth", int, 150, 150);
  PARAM(escape, "escape", double, 1000, 1000);
  // Set the default range for both spaces
  setDefaultRangeParameterSpace(-2.2, 1.4, -1.8, 1.8);
  setDefaultRangeDynamicalSpace(-2, 2, -2, 2);
}

//Function *copy() {
//  Quadratic *f = new Quadratic(name, "", pspace);
//  return f->copyArgsFrom(this);
//}

double iterate_(double x, double y) {
  int i;
  complex z, c;
  if (PARAMETER_SPACE) {
    z = complex(0, 0);
    c.set(x, y);
  } else {
    z = complex(x, y);
    c = C;
  }
  for (i = 0; i < depth; i++) {
    z = z * z + c;
    if (norm(z) > escape * escape) break;
  }
  //debug("%d %d", i, depth);
  return (double)i / depth;
}

// Forward orbit: assign f(x) to x
void orbit(complex &x) {
  x =  x * x + C;
}

// Set the parameter in dynamical space
void setParameter(double x, double y) {
  C.set(x, y);
}

void sandbox() {
  // Draws the orbit of the critical point
  complex z = 0;
  int i;
  for (i = 0; i < depth; i++) {
    z = z * z + C;
    SETPIXEL(z.real(), z.imag(), 100);
  }
  SetStrokeColor(0, 0, 0);
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
