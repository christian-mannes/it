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


#ifndef COLORMAP_H
#define COLORMAP_H
#include <vector>
#include <string>

class Colormap {
public:
  // Classic: tabulating RGB per index 0..255
  unsigned char r_[256], origr[256];   // red values
  unsigned char g_[256], origg[256];   // green values
  unsigned char b_[256], origb[256];   // blue values
  int contrast;           // index of contrast color
public:
  Colormap();
  Colormap(int (*f)(double));
  virtual ~Colormap();
public:
  static void getList(std::vector<std::string> &list);
  static Colormap *getColormapByName(const std::string &name);
public:
  void setColor(unsigned char index, unsigned char red, unsigned char green, unsigned char blue);
  void restore();
  void load(const std::string &filename);
  void save(const char *file);
  uint getColor(double x); // x in [0, 1]

private:
  int (*colorfun)(double t);
  std::vector<uint> table;
};

#endif /* Colormap_H */

/******************************** EOF ***********************************/

