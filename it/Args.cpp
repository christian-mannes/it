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
#include <cstring> // avoids pulling in C and C++ headers
#include <cwchar> // must do this despite warning
#include <cmath>
#include <string>
#include "MTComplex.h"
#include "Args.h"

/***************************** Arg class ********************************/

ItArg::ItArg(const char *name, ArgType t, void *a, const char *l, int b) 
  : _name(name), type(t), addr(a), legal(l), basic(b) { }

ItArg::ItArg(ItArg *a)		// make a shallow copy
  : _name(a->_name), type(a->type), addr(a->addr), legal(a->legal), 
    basic(a->basic), value(a->value) { }

/*
 * Convert whatever is in addr to a String
 */
void ItArg::setValue() {		// value = *addr
  char buf[1024];
  switch(type) {
  case T_int: snprintf(buf, 1024, "%d", *(int *)addr); break;
  case T_float: snprintf(buf, 1024, "%f", *(float *)addr); break;
  case T_double: snprintf(buf, 1024, "%g", *(double *)addr); break;
  case T_complex: {
      complex *c = (complex *)addr;
      snprintf(buf, 1024, "%g,%g", c->re, c->im);
      break;
    }
  case T_String: strcpy(buf, ((String *)addr)->c_str()); break;
  default: break;
  }
  value = buf;
}

/*
 * Set addr directly from a string. This does NOT set value
 */
void ItArg::parse(const char *s) {
  if (!s) return;
  switch(type) {
  case T_int: sscanf(s, "%d", (int *)addr); break;
  case T_float: sscanf(s, "%f", (float *)addr); break;
  case T_double: {
      float f; 
      sscanf(s, "%f", &f);
      *(double *)addr = (double)f; 
      break;
    }
  case T_complex: {
      float re, im;
      sscanf(s, "%g,%g", &re, &im); 
      *(complex *)addr = complex(re, im);
      break;
    }
  case T_String: *((String *)addr) = s; break;
  default: break;
  }
}

String& ItArg::toString() {
  setValue();
  return value;
}

double ItArg::toDouble() {
  if (type == T_double)
    return *(double *)addr;
  else return 0;
}

int ItArg::toInt() {
  if (type == T_int)
    return *(int *)addr;
  else return 0;
}

/***************************** Args class *******************************/

ItArgs::ItArgs() {
}

ItArgs::~ItArgs() {
  for (ItArg *arg: list) {
    delete arg;
  }
}

void ItArgs::copy(const ItArgs &args) {
  for (ItArg *arg: args.list) {
    ItArg *a = new ItArg(arg);
    hash[arg->name()] = a;
    list.push_back(a);
  }
}

void ItArgs::apply() {
  for (ItArg *arg: list) {
    arg->restoreValue();
  }
}

void ItArgs::addArg(const char *name, ItArg *arg) {
  hash[name] = arg;
  if (!arg->is_basic())
    list.push_back(arg);
}

ItArg *ItArgs::getArg(const char *name) {
  return hash[name];
}

/*
void ItArgs::store(Hash &externalhash) {
  for (hash.first(); hash.next(); ) {
    Arg *arg = (Arg *)hash.val();
    String *str = new String();
    *str = arg->toString();
    externalhash.put((char *)arg->name(), str);
  }
}

void ItArgs::restore(Hash &externalhash) {
  for (externalhash.first(); externalhash.next(); ) {
    String *str = (String *)externalhash.val();
    Arg *arg = getArg(externalhash.key());
    if (arg) {
      arg->parse(str->cstr());
      arg->setValue();
    }
    //delete str;
  }
}
*/
void ItArgs::rememberValues() {
  //for (hash.first(); hash.next(); ) {
  //  ((Arg *)(hash.val()))->rememberValue();
  //}
}

void ItArgs::restoreValues() {
  //for (hash.first(); hash.next(); ) {
  //  ((Arg *)(hash.val()))->restoreValue();
  //}
}

double ItArgs::getDouble(const char *name) {
  ItArg *arg = getArg(name);
  if (arg) return arg->toDouble();
  return 0.0;
}

int ItArgs::getInt(const char *name) {
  ItArg *arg = getArg(name);
  if (arg) return arg->toInt();
  return 0;
}
  
/******************************* EOF ************************************/
