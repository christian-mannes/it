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
#include "State.h"
#include "Function.h"
#include "Colormap.h"
//#include "Util.h"
//#include "Gif.h"

//void setPixel(void *ptr, int x, int y, unsigned char val) {
//  State *state = (State *)ptr;
//  state->setPixel(x, y, val);
//}

/***************************** State class ***************************/
#define CHANNELS 3
#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))

State::State(Function *function, Colormap *colormap, int w, int h) {
  this->function = function;
  this->colormap = colormap;
  pix = nullptr;
  width = height = 0;
  selx = sely = selX = selY = 0;
  xres = yres = 0;
  resize(w, h);
}

State::~State() {
  if (pix) delete [] pix;
  if (pixset) delete [] pixset;
}

void State::clear() {
  int n = width * height;
  selx = sely = selX = selY = 0;
  for (int i = 0; i < n; i++) {
    pix[i] = 0;
    pixset[i] = false;
  }
}

void State::resize(int w, int h) {
  if (pix) {
    if (width != w || height != h) {
      delete [] pix;
      delete [] pixset;
      pix = nullptr;
      pixset = nullptr;
    } else {
      clear();
      return;
    }
  }
  width = w;
  height = h;
  xres = w;
  yres = h;
  int n = width * height;
  pix = new double[n];
  pixset = new bool[n];
  clear();
}

void State::setRange(double x, double X, double y, double Y) {
  xmin = x;
  xmax = X;
  ymin = y;
  ymax = Y;
}

void State::setSelection(double x, double X, double y, double Y) {
  selx = x;
  sely = y;
  selX = X;
  selY = Y;
}

void State::getRangeFromFunction() {
  xmin = function->defxmin;
  xmax = function->defxmax;
  ymin = function->defymin;
  ymax = function->defymax;
}

//#define X(P) ((xmax - xmin) * (double)(P) / xres + xmin)
//#define invX(X) (int)(((X-xmin)*xres)/(xmax-xmin))
//#define Y(P) ((ymax - ymin) * (yres - (double)(P) - 1) / yres + ymin)
//#define invY(Y) (int)(-(((Y-ymin)*yres)/(ymax-ymin)-yres+1))

double State::getXTranslate() { return xmin; }
double State::getXScale() { return (xmax - xmin) / width; }
double State::getYTranslate() { return ymin; }
double State::getYScale() { return (ymax - ymin) / height; }

// X: screen x to coords s.t. 0 -> xmin and (w-1) -> xmax
double State::X(int x) {
  return xmin + (xmax - xmin) * ((double)x / (width - 1));
}
// invX: coords to screen s.t. xmin->0 and xmax->w-1
int State::invX(double x) {
  return (int)((width - 1) * ((x - xmin) / (xmax - xmin)));
}
int State::invW(double w) {
  return (int)((w*width)/(xmax-xmin)); // TODO
}
// Y: screen y to coords s.t. 0 (top) -> ymax and (h-1) (bottom) -> ymin
double State::Y(int y) {
  return ymin + (1.0 - (double)y / (height - 1)) * (ymax - ymin);
}
// invY: coords to screen y s.t. ymin -> h-1 and ymax -> 0
int State::invY(double y) {
  return (int)((height - 1) * (1.0 - (y - ymin) / (ymax - ymin)));
}
int State::invH(double h) {
  return (int)((h*height)/(ymax-ymin)); // TODO
}

bool State::isSet(int x, int y) {
  int index = y * width + x;
  return pixset[index];
}
bool State::isSetAt(int index) {
  return pixset[index];
}
double State::getPixel(int x, int y) {
  int index = y * width + x;
  return pix[index];
}
int State::getPixelIndex(int x, int y) {
  return y * width + x;
}
double State::getPixelAt(int index) {
  return pix[index];
}
void State::setPixel(int x, int y, double col, bool set) {
  if (x < 0 || x >= width || y < 0 || y >= height) return; // keep!
  int index = y * width + x;
  if (set) pixset[index] = true;
  if (col > 1.0) col /= 255.0;
  pix[index] = col;
}
void State::setPixelAt(int index, double col, bool set) {
  if (set) pixset[index] = true;
  pix[index] = col;
}
void State::setPixelRegion(int x, int y, double col, int w, int h) {
  int index = y * width + x;
  pixset[index] = true; // only set top left pixel
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      pix[j * width + i] = col;
    }
  }
}

void State::drawLine(int x, int y, int tx, int ty, byte col) {
  if (x != tx) {
    int x0 = MIN(x, tx);
    int x1 = MAX(x, tx);
    for (int i = x0; i <= x1; i++)
      setPixel(i, y, col);
  } else {
    int y0 = MIN(y, ty);
    int y1 = MAX(y, ty);
    for (int i = y0; i <= y1; i++)
      setPixel(x, i, col);    
  }
}

void State::setColormap(Colormap *map) {
  colormap = map;
}

void State::setColor(int index, byte red, byte green, byte blue) {
  colormap->setColor((byte)index, red, green, blue);
}

void State::unsetColors() {
  colormap->restore();
}

void State::storeArgs(Function *function) {
  int n = function->args.count();
  for (int i = 0; i < n; i++) {
    ItArg *arg = function->args.getArgAt(i);
    hash[arg->name()] = arg->toString();
  }
}

void State::restoreArgs(Function *function) {
  int n = function->args.count();
  for (int i = 0; i < n; i++) {
    ItArg *arg = function->args.getArgAt(i);
    arg->parse(hash[arg->name()].c_str());
  }
}

#define FORM_FEED 12
#if 0
void State::writeImage(FILE *fp) {
  uint index = 0, length = width * height;
  byte count, val;

  // write a marker byte
  count = FORM_FEED; fwrite(&count, 1, 1, fp);
  // start picture
  while (index < length) {
    count = 0; val = pix[index];
    while(pix[index+count] == val && index+count < length && count < 255)
      count++;
    if (count == 0)
      return;
    fwrite(&count, 1, 1, fp);
    fwrite(&val, 1, 1, fp);
    index += count;
  }
}

void State::readImage(FILE *fp) {
  uint index = 0, length = width * height;
  byte count = 0, val;

  // skip forward to marker byte
  do { fread(&count, 1, 1, fp); } while(count != FORM_FEED);
  // start image
  while(index < length) {
    fread(&count, 1, 1, fp);
    fread(&val, 1, 1, fp);
    while(count--) {
      pix[index] = val;
      //mapped[index] = colormap[val];
      pix_set[index] = 1;
      index++;
    }
  }
}

bool State::saveGIF(const char *filename) {
  if (pix) {
    GIF_save((char *)filename, pix, width, height);
    return true;
  }
  return false;
}

void phex(FILE *fp, int n) {
  int upper = (0xF0 & n) >> 4;
  int lower = 0xF & n;
  fprintf(fp, "%X%X", upper, lower);
}

int getColor(Colormap *colormap, int i, int index) {
  switch (index) {
    case 0: return (int)colormap->r[i];
    case 1: return (int)colormap->g[i];
    case 2: return (int)colormap->b[i];
  }
  return 0;
}

#define COMPRESSEPS

bool State::saveEPS(const char *filename, Colormap *colormap) {
  FILE *fp = fopen(filename, "w");
  if (pix == NULL) // || mapped == NULL)
    return false;
  if (!fp) {
    fprintf(stderr, "Could not open %s\n", filename);
    return false;
  }
  int w = width;
  int h = height;
  // A4 is 210 x 297 mm
  double pagewidth = 8.2 * 72;	// 8.2 inches expressed in points
  double pageheight = 11.6 * 72;	// 11.6 inches in points
  double xscale = pagewidth / w;
  double yscale = pageheight / h;
  double scale = xscale < yscale ? xscale : yscale;
  double bbx = (pagewidth - w * scale) / 2.0;
  double bby = (pageheight - h * scale) / 2.0;
  double bbw = pagewidth - bbx;
  double bbh = pageheight - bby;
  int i;
#define P fprintf
#define F fp,
  P(F "%%!PS-Adobe-2.0 EPSF-2.0\n%%%%Creator: It\n");
  P(F "%%%%Title: %s\n", filename);
  P(F "%%%%Pages: 1\n");
  P(F "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n", bbx, bby, bbw, bbh);
  P(F "%%%%EndComments\n");
#ifdef COMPRESSEPS
  P(fp, "/rmap [");
  for (i = 0; i < 256; i++) P(fp, "%d ", getColor(colormap, i, 0));
  P(fp, "] def\n");
  P(fp, "/gmap [");
  for (i = 0; i < 256; i++) P(fp, "%d ", getColor(colormap, i, 1));
  P(fp, "] def\n");
  P(fp, "/bmap [");
  for (i = 0; i < 256; i++) P(fp, "%d ", getColor(colormap, i, 2));
  P(fp, "] def\n");
  P(fp, "/red %d string def\n", w);
  P(fp, "/green %d string def\n", w);
  P(fp, "/blue %d string def\n", w);
  P(fp, "/raw %d string def\n/x 1 string def\n", w);
  P(fp, "/redcmd { \n"
    " 0 {	                \n" // 0==idx
    "   currentfile x readhexstring pop 0 get\n" // idx len (len is length-1)
    "	dup			\n" // idx len len
    "	2 index			\n" // idx len len idx
    "	add exch pop		\n" // idx last
    "	dup 1 add		\n" // idx last next
    "	currentfile x readhexstring pop 0 get\n" // idx last next val
    "	4 2 roll		\n" // next val idx last
    "	1 exch			\n" // next val idx 1 last
    "	{                       \n" // 
    "	    raw exch		\n" // ... raw i
    "	    2 index		\n" // ... raw i val
    "	    put			\n" // ...
    "	} for			\n" // next val
    "   pop dup			\n" // next next
    "	%d ge {exit} if	        \n" // next
    "   } loop                  \n" // 
    "   0 1 %d {		\n" // now copy raw to red
    "	  dup			\n" // i
    "	  raw exch get		\n" // i r (raw pixel value)
    "	  rmap exch get		\n" // i red[r]
    "	  red			\n" // i red[r] red
    "	  3 1 roll		\n" // red i red[r]
    "	  put			\n" // red[i] = red[r]
    "   } for                   \n" // 
    "   red                     \n" // put it on the stack
    "} bind def                 \n", w, w-1);
  P(fp, "/greencmd {              \n" // 
    "  0 1 %d {		        \n" // now copy raw to red
    "	 dup			\n" // i
    "	 raw exch get		\n" // i r (raw pixel value)
    "	 gmap exch get		\n" // i red[r]
    "	 green			\n" // i red[r] red
    "	 3 1 roll		\n" // red i red[r]
    "	 put			\n" // -  (and red[i] = red[r])
    "  } for                    \n" // 
    "  green                    \n" //
    "} bind def                 \n", w-1);
  P(fp, "/bluecmd {               \n" // 
    "  0 1 %d {		        \n" // now copy raw to red
    "	 dup			\n" // i
    "	 raw exch get		\n" // i r (raw pixel value)
    "	 bmap exch get		\n" // i red[r]
    "	 blue			\n" // i red[r] red
    "	 3 1 roll		\n" // red i red[r]
    "	 put			\n" // -  (and red[i] = red[r])
    "  } for                    \n" // 
    "  blue                     \n" //
    "} bind def                 \n", w-1);
  P(fp, "%%%%EndProlog\n%%%%Page: 1 1\ngsave\n");
  P(fp, "%.1f %.1f translate\n", bbx, bby);
  P(fp, "%.1f %.1f scale\n", scale * w, scale * h);
  P(fp, "%d %d 8\n", w, h);
  P(fp, "[%d 0 0 %d 0 %d]\n", w, -h, h);
  P(fp, "{ redcmd } { greencmd } { bluecmd }\n");
  P(fp, "true 3\ncolorimage\n");
  //
  // Now the data - runlength encoded
  //
  int val, length, y, count = 0;
  for (y = 0; y < h; y++) {
    int index = y * w, end = (y + 1) * w;
    while (index < end) {
      val = (int)pix[index];
      length = 0; 
      while(pix[index+length] == val && index+length < end && length < 255)
        length++;
      if (length == 0)
        break;
      phex(fp, length-1);
      phex(fp, val);
      index += length;
      count += 4;
      if ((count % 60) == 0)
        P(fp, "\n");
    }
  }
#else // NON-COMPRESSING VERSION
  P(F "/readstring { currentfile exch readhexstring pop } bind def\n");
  P(F "/r %d string def\n/g %d string def\n/b %d string def\n", w, w, w);
  P(F "%%%%EndProlog\n%%%%Page: 1 1\ngsave\n");
  P(F "%.1f %.1f translate\n", bbx, bby);
  P(F "%.1f %.1f scale\n", scale * w, scale * h);
  P(F "%d %d 8\n", w, h);
  P(F "[%d 0 0 %d 0 %d]\n", w, -h, h);
  P(F "{ r readstring }\n{ g readstring }\n{ b readstring }\n");
  P(F "true 3\ncolorimage\n");
  // Now the pixels
  int val, x, y, col, count = 0;
  for (y = 0; y < h; y++) {
    for (col = 0; col < 3; col++) {
      for (x = 0; x < w; x++) {
        val = getColor(colormap, (int)pix[y * w + x], col);
        phex(fp, val);
        count++;
        if ((count % 30) == 0)
          P(F "\n");
      }
    }
  }
#endif // COMPRESSEPS
  P(F "\n");
  P(F "grestore\nshowpage\n%%%%Trailer\n");
#undef P
#undef F
  fclose(fp);
  return true;
}
#endif
/******************************** EOF ***********************************/
