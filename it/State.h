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

#pragma once
#include <string>
#include <unordered_map>

// SETPIXEL(x, y, color) -- set a pixel corresponding to real coordinates x, y
// SETPIXEL_(x, y, color) -- set a pixel in image coordinates (0, 0) is lower left
// GETPIXEL(x, y) -- returns the pixel value at real coordinates x,y
// GETPIXEL_(x, y) -- returns the pixel value at image coordinates x, y
// HLINE(x, y, dx, col) -- draw a line from point x, y to x+dx, y
// VLINE(x, y, dy, col) -- draw a line from x, y to x, y+dy
// HLINE_(x, y, dx, col) -- draw a line from point x, y to x+dx, y (image coordinates)
// VLINE_(x, y, dy, col) -- draw a line from x, y to x, y+dy (image coordinates)
#define SETPIXEL(X, Y, C) state->setPixel(state->invX(X), state->invY(Y), (C))
#define GETPIXEL(X, Y) state->getPixel(state->invX(X), state->invY(Y))
#define HLINE(X, Y, DX, C) state->drawLine(state->invX(X), state->invY(Y), state->invX((X)+(DX)), state->invY(Y), (C))
#define VLINE(X, Y, DY, C) state->drawLine(state->invX(X), state->invY(Y), state->invX(X), state->invY((Y)+(DY)), (C))

#define SETPIXEL_(X, Y, C) state->setPixel(X, Y, (C))
#define GETPIXEL_(X, Y) state->getPixel(X, Y)
#define HLINE_(X, Y, DX, C) state->drawLine(X, Y, X+DX, Y, (C))
#define VLINE_(X, Y, DY, C) state->drawLine(X, Y, X, Y+DY, (C))

class Function;
class Colormap;

typedef unsigned char byte;

class State {
public:
  State(Function *function, Colormap *colormap, int w, int h);
  ~State();
  void resize(int w, int h);
  void setRange(double x, double X, double y, double Y);
  void setSelection(double x, double X, double y, double Y);
  void getRangeFromFunction();
  void clear();
  double X(int x);
  double Y(int y);
  double getXTranslate();
  double getXScale();
  double getYTranslate();
  double getYScale();
  int invX(double x);
  int invY(double y);
  int invW(double w);
  int invH(double h);

  bool isSet(int x, int y);
  bool isSetAt(int index);
  double getPixel(int x, int y);
  int getPixelIndex(int x, int y);
  double getPixelAt(int index);
  void setPixel(int x, int y, double col, bool set=true);
  void setPixelAt(int index, double col, bool set=true);
  void setPixelRegion(int x, int y, double col, int w, int h);

  void drawLine(int x, int y, int tx, int ty, byte col);
  void setColormap(Colormap *map);
  void setColor(int index, byte red, byte green, byte blue);
  void unsetColors();
  int getWidth() { return width; }
  int getHeight() { return height; }
  void storeArgs(Function *function);
  void restoreArgs(Function *function);
  Function *function;     /* Pointer to function */
  Colormap *colormap;     /* Pointer to colormap */
  //void writeImage(FILE *fp);
  //void readImage(FILE *fp);
  //bool saveGIF(const char *filename);
  //bool saveEPS(const char *filename, Colormap *colormap);
public:
  double xmin, xmax, ymin, ymax; /* image region in real coordinates */
  double selx, sely, selX, selY; /* selection in real coordinates */
  double xres, yres;             /* image resolution */
  bool annnotate;                /* allow annotations (TODO: here?) */
  bool sandbox;                  /* run sandbox code */
  int pspace;                    // 1 for parameter space (why?)
private:
  std::unordered_map<std::string,std::string> hash;/* hash to store all arguments */
  int width;              /* image width */
  int height;             /* image height */
  double *pix;            /* cached pixels: raw values as returned from iterate */
  bool *pixset;           /* remember if a pixel is set */
  int *mapped;            // tmp
};

/******************************** EOF ***********************************/
