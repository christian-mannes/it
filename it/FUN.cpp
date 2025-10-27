#include "Function.h"
#include "State.h"

CLASS(SampleQuadratic, "Quadratic function") { public:
  // Declare your parameters and working variables here
  complex C;
  int depth;
  double escape;

  SampleQuadratic(String name, String label, int pspace) : Function(name, label, pspace) {
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

CLASS(SampleMilnor, "Milnor function") { public:
  // Declare your parameters and working variables here
  complex a;
  int depth;
  double escape;

  SampleMilnor(String name, String label, int pspace) : Function(name, label, pspace) {
    // List every parameter you want to appear in the "parameters" section
    // using ARG(variable, parameter-name, default-value, allowed-values)
    PARAM(a, "a", complex, complex(1, 0), complex(-1,0));
    PARAM(depth, "depth", int, 75, 75);
    PARAM(escape, "bound", double, 1000, 1000);
    // Set the default range for both spaces
    setDefaultRangeParameterSpace(-9, 6, -7.5, 7.5);
    setDefaultRangeDynamicalSpace(-2, 2, -2, 2);
  }

  Function *copy() {
    SampleMilnor *f = new SampleMilnor(name, "", pspace);
    return f->copyArgsFrom(this);
  }

  double iterate_(double x, double y) {
    int i;
    complex z;
    if (PARAMETER_SPACE) {
      a.set(x, y);
      z = 2.0/3.0;
    } else {
      a.set(1, 0); /// Nuria?
      z = complex(x, y);
    }
    for (i = 0; i < depth; i++) {
      z = a * z * z * (z - 1);
      if (norm(z) < 1/(escape*escape)) return 0.9;
      if (norm(z) > escape * escape) break;
    }
    return (double)i/depth;
  }

  // Forward orbit: assign f(x) to x
  void orbit(complex &x) {
    x = a * x * x * (x - 1);
  }

  // Set the parameter in dynamical space
  void setParameter(double x, double y) {
    a.set(x, y);
  }
};

CLASS(SampleNewton, "Newton's Method") { public:
  complex a;
  int depth;
  double escape;

  SampleNewton(String name, String label, int pspace) : Function(name, label, pspace) {
    PARAM(a, "a", complex, complex(0.5, 0.866025), complex(0.5, 0.866025));
    PARAM(depth, "depth", int, 50, 50);
    PARAM(escape, "bound", double, 1000, 1000);
    setDefaultRangeParameterSpace(-1, 2, 0, 3);
    setDefaultRangeDynamicalSpace(-0.8, 1.8, -1.04, 1.56);
  }

  Function *copy() {
    SampleNewton *f = new SampleNewton(name, "", pspace);
    return f->copyArgsFrom(this);
  }

  double iterate_(double x, double y) {
    int i;
    complex z;
    if (PARAMETER_SPACE) {
      a.set(x, y);
      z = (1.0+a)/3.0;
    } else {
      a.set(0.5, 0.866025);
      z = complex(x, y);
    }
    for (i = 0; i < depth; i++) {
      z = z*z*(1+a-2*z)/(-a+2*z+2*a*z-3*z*z);
      if (norm(z-a)<(1/escape)||norm(z-1)<(1/escape)||norm(z)<(1/escape))
        return (double)i/depth;
    }
  //  if (norm(z-a)<(1/escape)) return (byte) (0);
  //  if (norm(z-1)<(1/escape)) return (byte) (1);
  //  if (norm(z)<(1/escape)) return (byte) (2);
    return 1.0;
  }

  void orbit(complex &z) {
    z = z*z*(1+a-2*z)/(-a+2*z+2*a*z-3*z*z);
  }

  void setParameter(double x, double y) {
    a.set(x, y);
  }
};
///////////////////////////////////////////////////////////////////////////////

Function *createBuiltinFunction(const std::string &name) {
  Function *p = nullptr, *d = nullptr;
  if (name == "SampleQuadratic") {
    p = new SampleQuadratic("a", "b", 1);
    d = new SampleQuadratic("a", "a", 0);
  } else if (name == "SampleMilnor") {
    p = new SampleMilnor("a", "b", 1);
    d = new SampleMilnor("a", "a", 0);
  } else if (name == "SampleNewton") {
    p = new SampleNewton("a", "b", 1);
    d = new SampleNewton("a", "a", 0);
  }
  if (p && d) { p->other = d; d->other = p; }
  return p;
}

///////////////////////////////////////////////////////////////////////////////
