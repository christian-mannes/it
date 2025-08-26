#pragma once
/**************************** Types *************************************/

enum ArgType { T_int, T_float, T_double, T_complex, T_String };

/***************************** Class ************************************/
#include <string>
#include <unordered_map>
#include <vector>
#define String std::string
class ItArg { 
private:
  String _name;		/* name, may be different from var name */
  ArgType type;		/* what basic type */
  void *addr;			/* address of variable */
  String legal;		/* list of legal values */
  int basic;			/* whether it's part of Function */
public:
  String value;	  /* temporary value hold */
  String value_p; /* default value in parameter space */
  String value_d; /* default value in dynamical space */
public:
  ItArg(const char *name, ArgType t, void *a, const char *l, int basic);
  ItArg(ItArg &arg);
  ItArg(ItArg *arg);
  int is_basic() { return basic; }
  String &name() { return _name; }
  String& toString();		/* value=*addr; return value */
  double toDouble();
  int toInt();
  void setValue();		/* set value from *addr */
  void parse(const char *s);	/* set addr from s (don't touch value) */
  void apply(); // set addr from value
  void preset(String &val) { value = val; }
  void preset(char *val) { value = val; }
  //void preset(int val) { value = String::Format("%d", val); }
  //void preset(float val) { value = String::Format("%f", val); }
  //void preset(double val) { value = String::Format("%f", val);  }
  void rememberValue() { toString(); }
  void restoreValue() { parse(value.c_str()); }
  void applyValue() { parse(value.c_str()); }
};

class ItArgs { 
public:
  ItArgs();
  ~ItArgs();
  std::unordered_map<String,ItArg*> hash;
  std::vector<ItArg*> list;
  void apply();
  int count() { return list.size(); }
  ItArg *getArgAt(int i) { return list[i]; }
  void addArg(const char *name, ItArg *arg);
  ItArg *getArg(const char *name);
  void rememberValues();
  void restoreValues();
  double getDouble(const char *name);
  int getInt(const char *name);
  void copy(const ItArgs &args);
  //void store(Hash &externalhash);
  //void restore(Hash &externalhash);
};

/******************************* EOF *************************************/
