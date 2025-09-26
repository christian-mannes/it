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

int Colormap::getColor(double x) {
  if (x >= 0) { // regular values in [0, 1]
    if (colorfun) {
      return colorfun(x);
    } else {
      int index = clamp((int)(x * 255));
      int r = (int)r_[index];
      int g = (int)g_[index];
      int b = (int)b_[index];
      return (0xFF << 24) |           // Alpha channel (bits 24-31): 0xFF
             ((r & 0xFF) << 16) |     // Red channel (bits 16-23)
             ((g & 0xFF) << 8) |      // Green channel (bits 8-15)
             (b & 0xFF);              // Blue channel (bits 0-7)
    }
  } else { // RGB values
    union { double d; uint64_t i; } u;
    u.d = x;
    return 0xFFFFFFFF & u.i;
  }
}

/******************************** EOF ****************************************/

