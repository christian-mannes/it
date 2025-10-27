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
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <stdio.h>
#include "Colormap.h"

#ifndef PI
#define PI 3.1415279
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
/***************************** Functions ********************************/

static double warp(double x, double xp) {
  if (x > xp) {
    return (xp * (x - 2) + 1) / (1 - xp);
  } else {
    return x * (1 - xp) / xp;
  }
}

inline int clamp(int value) {
    return std::max(0, std::min(255, value));
}

inline double clamp(double value) {
    return std::max(0.0, std::min(1.0, value));
}

// Hot Colormap - Classic heat map (black → red → yellow → white)
int hotColormap(double t) {
    t = clamp(t);
    int r, g, b;

    if (t < 0.4) {
        r = clamp(static_cast<int>(255 * t / 0.4));
        g = 0;
        b = 0;
    } else if (t < 0.8) {
        r = 255;
        g = clamp(static_cast<int>(255 * (t - 0.4) / 0.4));
        b = 0;
    } else {
        r = 255;
        g = 255;
        b = clamp(static_cast<int>(255 * (t - 0.8) / 0.2));
    }

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Cool Colormap - Blue to magenta gradient
int coolColormap(double t) {
    t = clamp(t);
    int r = clamp(static_cast<int>(255 * t));
    int g = clamp(static_cast<int>(255 * (1 - t)));
    int b = 255;

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Rainbow Colormap - Full spectrum HSV rainbow
int rainbowColormap(double t) {
    t = clamp(t);
    double h = t * 360.0;
    double c = 1.0;
    double x = c * (1.0 - std::abs(fmod(h / 60.0, 2.0) - 1.0));
    double r, g, b;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    int ri = clamp(static_cast<int>(r * 255));
    int gi = clamp(static_cast<int>(g * 255));
    int bi = clamp(static_cast<int>(b * 255));

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Plasma Colormap - Approximation of matplotlib's plasma
int plasmaColormap(double t) {
    t = clamp(t);
    double r = clamp(0.05 + 0.9 * std::pow(t, 0.8));
    double g = clamp(0.3 * std::sin(M_PI * t) + 0.7 * std::pow(t, 1.5));
    double b = clamp(0.9 - 0.8 * t);

    int ri = clamp(static_cast<int>(r * 255));
    int gi = clamp(static_cast<int>(g * 255));
    int bi = clamp(static_cast<int>(b * 255));

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Viridis Colormap - Approximation of matplotlib's viridis
int viridisColormap(double t) {
    t = clamp(t);
    double r = clamp(0.267 + 0.005 * t + 4.55 * t * t);
    double g = clamp(0.004 + 1.24 * t - 0.43 * t * t);
    double b = clamp(0.329 - 0.134 * t + 2.94 * t * t - 2.87 * t * t * t);

    int ri = clamp(static_cast<int>(r * 255));
    int gi = clamp(static_cast<int>(g * 255));
    int bi = clamp(static_cast<int>(b * 255));

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Fire Colormap - Deep red to bright orange/yellow
int fireColormap(double t) {
    t = clamp(t);
    double r = (t < 0.5) ? 255 * (2 * t) : 255;
    double g = (t < 0.25) ? 0 : (t < 0.75) ? 255 * (4 * (t - 0.25)) : 255;
    double b = (t < 0.75) ? 0 : 255 * (4 * (t - 0.75));

    int ri = clamp(static_cast<int>(r));
    int gi = clamp(static_cast<int>(g));
    int bi = clamp(static_cast<int>(b));

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Ocean Colormap - Deep blue to light cyan
int oceanColormap(double t) {
    t = clamp(t);
    int r = clamp(static_cast<int>(255 * (0.1 + 0.4 * t)));
    int g = clamp(static_cast<int>(255 * (0.2 + 0.6 * t)));
    int b = clamp(static_cast<int>(255 * (0.8 + 0.2 * t)));

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Magma Colormap - Approximation of matplotlib's magma
int magmaColormap(double t) {
    t = clamp(t);
    double r = clamp(-0.002 + 0.999 * std::pow(t, 0.7));
    double g = clamp(std::pow(t, 3));
    double b = clamp(0.5 * std::sin(M_PI * t) + 0.5 * t * t);

    int ri = clamp(static_cast<int>(r * 255));
    int gi = clamp(static_cast<int>(g * 255));
    int bi = clamp(static_cast<int>(b * 255));

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Sunset Colormap - Warm orange and pink tones
int sunsetColormap(double t) {
    t = clamp(t);
    int r = clamp(static_cast<int>(255 * (1 - 0.3 * std::sin(M_PI * t))));
    int g = clamp(static_cast<int>(255 * (0.5 + 0.5 * std::sin(M_PI * t - 1.57))));
    int b = clamp(static_cast<int>(255 * (0.2 + 0.3 * t)));

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Electric Colormap - Oscillating RGB for psychedelic effects
int electricColormap(double t) {
    t = clamp(t);
    double freq = 3.0;
    int r = clamp(static_cast<int>(255 * (0.5 + 0.5 * std::sin(freq * M_PI * t))));
    int g = clamp(static_cast<int>(255 * (0.5 + 0.5 * std::sin(freq * M_PI * t + 2.09))));
    int b = clamp(static_cast<int>(255 * (0.5 + 0.5 * std::sin(freq * M_PI * t + 4.19))));

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

int mandelbrotColorEnhanced(double t) {
    t = std::max(0.0, std::min(1.0, t)); // Clamp to [0,1]

    // Apply power function to enhance contrast in 0..1 region
    // Values closer to 0 get more space, values near 1 are compressed
    double enhanced = std::pow(t, 0.3);  // Adjust exponent (0.2-0.5 works well)

    // Apply a vibrant colormap with the enhanced values
    double h = enhanced * 360.0;
    double c = 1.0;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double r, g, b;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    int ri = (int)(r * 255);
    int gi = (int)(g * 255);
    int bi = (int)(b * 255);

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

int mandelbrotColorLog(double t) {
    t = std::max(0.0, std::min(1.0, t));

    // Logarithmic mapping - extreme contrast at low values
    double enhanced = std::log(1.0 + t * 9.0) / std::log(10.0); // Maps [0,1] -> [0,1] with log curve

    // Hot colormap for Mandelbrot
    int r, g, b;
    if (enhanced < 0.4) {
        r = (int)(255 * enhanced / 0.4);
        g = 0;
        b = 0;
    } else if (enhanced < 0.8) {
        r = 255;
        g = (int)(255 * (enhanced - 0.4) / 0.4);
        b = 0;
    } else {
        r = 255;
        g = 255;
        b = (int)(255 * (enhanced - 0.8) / 0.2);
    }

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

int mandelbrotColorBanded(double t) {
    t = std::max(0.0, std::min(1.0, t));

    // Split into bands with more bands at lower values
    int r, g, b;

    if (t < 0.1) {
        // First 10% gets deep blue to cyan
        double local = t / 0.1;
        r = 0;
        g = (int)(128 * local);
        b = (int)(128 + 127 * local);
    } else if (t < 0.3) {
        // Next 20% gets cyan to green
        double local = (t - 0.1) / 0.2;
        r = 0;
        g = (int)(128 + 127 * local);
        b = (int)(255 - 128 * local);
    } else if (t < 0.6) {
        // Next 30% gets green to yellow
        double local = (t - 0.3) / 0.3;
        r = (int)(255 * local);
        g = 255;
        b = (int)(127 * (1.0 - local));
    } else if (t < 0.85) {
        // Next 25% gets yellow to red
        double local = (t - 0.6) / 0.25;
        r = 255;
        g = (int)(255 * (1.0 - local));
        b = 0;
    } else {
        // Last 15% gets red to dark red
        double local = (t - 0.85) / 0.15;
        r = (int)(255 - 128 * local);
        g = 0;
        b = 0;
    }

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Method 4: Smooth continuous with sqrt for mild enhancement
int mandelbrotColorSmooth(double t) {
    t = std::max(0.0, std::min(1.0, t));

    // Square root provides nice balance - more contrast at low, still smooth
    double enhanced = std::sqrt(t);

    // Smooth plasma-like colormap
    double r = std::max(0.0, std::min(1.0, 0.05 + 0.9 * std::pow(enhanced, 0.8)));
    double g = std::max(0.0, std::min(1.0, 0.3 * std::sin(M_PI * enhanced) + 0.7 * std::pow(enhanced, 1.5)));
    double b = std::max(0.0, std::min(1.0, 0.9 - 0.8 * enhanced));

    int ri = (int)(r * 255);
    int gi = (int)(g * 255);
    int bi = (int)(b * 255);

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

// Method 5: Ultra-detail version with exponential mapping
int mandelbrotColorUltraDetail(double t) {
    t = std::max(0.0, std::min(1.0, t));

    // Very aggressive enhancement for maximum detail in early escape iterations
    double enhanced = 1.0 - std::exp(-5.0 * t); // Maps [0,1] with exponential curve

    // Multi-color gradient
    double cycles = enhanced * 3.0;  // 3 color cycles
    double phase = std::fmod(cycles, 1.0);
    int segment = (int)cycles;

    int r, g, b;

    switch (segment % 3) {
    case 0: // Blue to Cyan
        r = 0;
        g = (int)(255 * phase);
        b = 255;
        break;
    case 1: // Cyan to Yellow
        r = (int)(255 * phase);
        g = 255;
        b = (int)(255 * (1.0 - phase));
        break;
    case 2: // Yellow to Red
        r = 255;
        g = (int)(255 * (1.0 - phase));
        b = 0;
        break;
    }

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Method 6: Custom parametric with adjustable contrast
int mandelbrotColorParametric(double t) {
    double contrastPower = 0.35;
    t = std::max(0.0, std::min(1.0, t));

    // Adjustable power for fine-tuning contrast
    // contrastPower: 0.2 = extreme contrast, 0.5 = moderate, 1.0 = linear
    double enhanced = std::pow(t, contrastPower);

    // Classic Mandelbrot color scheme
    double freq = 2.0 * M_PI;
    int r = (int)(127.5 * (1.0 + std::sin(freq * enhanced)));
    int g = (int)(127.5 * (1.0 + std::sin(freq * enhanced + 2.094)));  // 120 degrees
    int b = (int)(127.5 * (1.0 + std::sin(freq * enhanced + 4.189)));  // 240 degrees

    return 0xff000000 | (r << 16) | (g << 8) | b;
}

// Method 7: Histogram equalization approach (requires data about iteration distribution)
// This is a simplified version that assumes more iterations at low values
int mandelbrotColorHistogram(double t) {
    t = std::max(0.0, std::min(1.0, t));

    // Simulated histogram equalization
    // More weight on lower values (where most interesting detail is)
    double enhanced;
    if (t < 0.5) {
        enhanced = 0.7 * std::pow(t / 0.5, 0.4);
    } else {
        enhanced = 0.7 + 0.3 * ((t - 0.5) / 0.5);
    }

    // Viridis-like colormap (perceptually uniform)
    double r = std::max(0.0, std::min(1.0, 0.267 + 0.005 * enhanced + 4.55 * enhanced * enhanced));
    double g = std::max(0.0, std::min(1.0, 0.004 + 1.24 * enhanced - 0.43 * enhanced * enhanced));
    double b = std::max(0.0, std::min(1.0, 0.329 - 0.134 * enhanced + 2.94 * enhanced * enhanced - 2.87 * std::pow(enhanced, 3)));

    int ri = (int)(r * 255);
    int gi = (int)(g * 255);
    int bi = (int)(b * 255);

    return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}


double sharpen(int iterations, int maxIterations, double zMagnitude) {
  if (iterations == maxIterations) {
    return 0xff000000;  // Black for points in the set
  }
  // Smooth coloring to avoid banding
  double smoothed = iterations + 1 - std::log(std::log(zMagnitude)) / std::log(2.0);
  double t = smoothed / maxIterations;
  return t;
}

/************************************************************************/

Colormap::Colormap() {
  int i;
  for (i = 0; i < 256; i++) {
    r_[i] = g_[i] = b_[i] = (unsigned char)i;
    origr[i] = origg[i] = origb[i] = (unsigned char)i;
  }
  colorfun = nullptr;
}

Colormap::Colormap(int (*f)(double)) {
  int i;
  for (i = 0; i < 256; i++) {
    r_[i] = g_[i] = b_[i] = (unsigned char)i;
    origr[i] = origg[i] = origb[i] = (unsigned char)i;
  }
  colorfun = f;
}

Colormap::~Colormap() {
}

/*static*/ void Colormap::getList(std::vector<std::string> &list) {
  list.push_back("hot");
  list.push_back("cool");
  list.push_back("rainbow");
  list.push_back("plasma");
  list.push_back("viridis");
  list.push_back("fire");
  list.push_back("ocean");
  list.push_back("magma");
  list.push_back("sunset");
  list.push_back("electric");

  //
  list.push_back("mandelbrotColorEnhanced");
  list.push_back("mandelbrotColorLog");
  list.push_back("mandelbrotColorBanded");
  list.push_back("mandelbrotColorSmooth");
  list.push_back("mandelbrotColorUltraDetail");
  list.push_back("mandelbrotColorParametric");
  list.push_back("mandelbrotColorHistogram");
}

/*static*/ Colormap *Colormap::getColormapByName(const std::string &name) {
  if (name == "hot") return new Colormap(hotColormap);
  else if (name == "cool") return new Colormap(coolColormap);
  else if (name == "rainbow") return new Colormap(rainbowColormap);
  else if (name == "plasma") return new Colormap(plasmaColormap);
  else if (name == "viridis") return new Colormap(viridisColormap);
  else if (name == "fire") return new Colormap(fireColormap);
  else if (name == "ocean") return new Colormap(oceanColormap);
  else if (name == "magma") return new Colormap(magmaColormap);
  else if (name == "sunset") return new Colormap(sunsetColormap);
  else if (name == "electric") return new Colormap(electricColormap);

  else if (name == "mandelbrotColorEnhanced") return new Colormap(mandelbrotColorEnhanced);
  else if (name == "mandelbrotColorLog") return new Colormap(mandelbrotColorLog);
  else if (name == "mandelbrotColorBanded") return new Colormap(mandelbrotColorBanded);
  else if (name == "mandelbrotColorSmooth") return new Colormap(mandelbrotColorSmooth);
  else if (name == "mandelbrotColorUltraDetail") return new Colormap(mandelbrotColorUltraDetail);
  else if (name == "mandelbrotColorParametric") return new Colormap(mandelbrotColorParametric);
  else if (name == "mandelbrotColorHistogram") return new Colormap(mandelbrotColorHistogram);
  else return nullptr;
}

void Colormap::load(const std::string &file) {
  FILE *fp = fopen(file.c_str(), "r");
  int i = 0;
  if (fp != NULL) {
    int red, green, blue;
    while(i < 256 && fscanf(fp, "%d%d%d", &red, &green, &blue) == 3) {
      r_[i] = (unsigned char)red;
      g_[i] = (unsigned char)green;
      b_[i] = (unsigned char)blue;
      origr[i] = r_[i];
      origg[i] = g_[i];
      origb[i] = b_[i];
      i++;
    }
    fclose(fp);
  }
}

void Colormap::setColor(unsigned char index, unsigned char red, unsigned char green, unsigned char blue) {
  r_[index] = red;
  g_[index] = green;
  b_[index] = blue;
  colorfun = nullptr; // want to use only particular colors
}

void Colormap::restore() {
  for (int i = 0; i < 256; i++) {
    r_[i] = origr[i];
    g_[i] = origg[i];
    b_[i] = origb[i];
  }
}

void Colormap::save(const char *file) {
  FILE *fp = fopen(file, "w");
  if (fp == NULL) return;
  for (int i = 0; i < 256; i++)
    fprintf(fp, "%d %d %d\n", r_[i], g_[i], b_[i]);
  fclose(fp);
}

uint Colormap::getColor(double x) {
  union { double d; uint64_t i; } u;
  u.d = x;
  if ((u.i & 0x8000000000000000ULL) == 0) { // regular values in [0, 1]
    if (colorfun) {
      return colorfun(x);
    } else {
      int index = clamp((int)(x * 255));
      int r = (int)r_[index];
      int g = (int)g_[index];
      int b = (int)b_[index];
      return (0xFFu << 24) |           // Alpha channel (bits 24-31): 0xFF
             ((r & 0xFFu) << 16) |     // Red channel (bits 16-23)
             ((g & 0xFFu) << 8) |      // Green channel (bits 8-15)
             (b & 0xFFu);              // Blue channel (bits 0-7)
    }
  } else { // RGB values
    return 0xFFFFFFFFu & u.i;
  }
}

/******************************** EOF ****************************************/

