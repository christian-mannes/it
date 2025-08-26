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

class Function;
class State;
class Colormap;

class Algorithm {
protected:
  State *state;
  int xres, yres;
  bool running;   // while true, should continue. Stop if false
private:
  virtual void init(Function *f) = 0;  // initialize yourself
  virtual void run(Function *f) = 0;   // do your work
public:
  String name;
  Algorithm(const char *name) { this->name = name; }
  virtual ~Algorithm() { }
  void start(Function *f, State *state);
  void piece(int part, int total, Function *f);
  void stop(); // sets running=false, stop as soon as interruptible
};

Algorithm *makeAlgorithm(const char *name);

/******************************* EOF *************************************/
