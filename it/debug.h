
#pragma once
#include <string>

///////////////////////////////////////////////////////////////////////////////

class Arg {
  typedef bool (*Parser)(const char* str, int n, void* dest);
public:
  Arg();
  Arg(int i);
  Arg(unsigned int i);
  Arg(long long int i); // =int64_t
  //Arg(size_t i);
  Arg(char c);
  Arg(double d);
  Arg(const char *c);
  Arg(const std::string &s);
  Arg(int *i);
  Arg(double *d);
  Arg(std::string *s);
  Arg(void *p);
public:
  const int toInt() const;
  const double toDouble() const;
  const char *toCharP() const;
  const int *toIntP() const;
  const double *toDoubleP() const;
  const std::string toString() const;
  const std::string *toStringP() const;
  const void *toVoidP() const;
  const bool isInt() const;
  const bool isDouble() const;
  const bool isString() const;
public:
  bool parse(const char* str, int n) const;
public:
  static Arg none;
private:
  union {
    int ival;	// 4
    void *pval; //8
    const char *cval; //8
    double dval; // 8
    long long int lval; // 8
  };
  Parser parser; // 8
};

///////////////////////////////////////////////////////////////////////////////

#define VARARGDEF8 \
  const Arg &ptr1 = Arg::none,\
  const Arg &ptr2 = Arg::none,\
  const Arg &ptr3 = Arg::none,\
  const Arg &ptr4 = Arg::none,\
  const Arg &ptr5 = Arg::none,\
  const Arg &ptr6 = Arg::none,\
  const Arg &ptr7 = Arg::none,\
  const Arg &ptr8 = Arg::none

#define VARARGLIST8 \
  const Arg& ptr1,\
  const Arg& ptr2,\
  const Arg& ptr3,\
  const Arg& ptr4,\
  const Arg& ptr5,\
  const Arg& ptr6,\
  const Arg& ptr7,\
  const Arg& ptr8

#define VARARGPREAMBLE8(ARGS, NARGS) \
  const Arg* ARGS[8];\
  int NARGS = 0;\
  if (&ptr1  == &Arg::none) goto done; args[NARGS++] = &ptr1;\
  if (&ptr2  == &Arg::none) goto done; args[NARGS++] = &ptr2;\
  if (&ptr3  == &Arg::none) goto done; args[NARGS++] = &ptr3;\
  if (&ptr4  == &Arg::none) goto done; args[NARGS++] = &ptr4;\
  if (&ptr5  == &Arg::none) goto done; args[NARGS++] = &ptr5;\
  if (&ptr6  == &Arg::none) goto done; args[NARGS++] = &ptr6;\
  if (&ptr7  == &Arg::none) goto done; args[NARGS++] = &ptr7;\
  if (&ptr8  == &Arg::none) goto done; args[NARGS++] = &ptr8;\
done:\
  (void)0

#define VARARGDEF16 \
  const Arg &ptr1 = Arg::none,\
  const Arg &ptr2 = Arg::none,\
  const Arg &ptr3 = Arg::none,\
  const Arg &ptr4 = Arg::none,\
  const Arg &ptr5 = Arg::none,\
  const Arg &ptr6 = Arg::none,\
  const Arg &ptr7 = Arg::none,\
  const Arg &ptr8 = Arg::none,\
  const Arg &ptr9 = Arg::none,\
  const Arg &ptr10 = Arg::none,\
  const Arg &ptr11 = Arg::none,\
  const Arg &ptr12 = Arg::none,\
  const Arg &ptr13 = Arg::none,\
  const Arg &ptr14 = Arg::none,\
  const Arg &ptr15 = Arg::none,\
  const Arg &ptr16 = Arg::none

#define VARARGLIST16 \
  const Arg& ptr1,\
  const Arg& ptr2,\
  const Arg& ptr3,\
  const Arg& ptr4,\
  const Arg& ptr5,\
  const Arg& ptr6,\
  const Arg& ptr7,\
  const Arg& ptr8,\
  const Arg& ptr9,\
  const Arg& ptr10,\
  const Arg& ptr11,\
  const Arg& ptr12,\
  const Arg& ptr13,\
  const Arg& ptr14,\
  const Arg& ptr15,\
  const Arg& ptr16

#define VARARGPREAMBLE16(ARGS, NARGS) \
  const Arg* ARGS[16];\
  int NARGS = 0;\
  if (&ptr1  == &Arg::none) goto done; args[NARGS++] = &ptr1;\
  if (&ptr2  == &Arg::none) goto done; args[NARGS++] = &ptr2;\
  if (&ptr3  == &Arg::none) goto done; args[NARGS++] = &ptr3;\
  if (&ptr4  == &Arg::none) goto done; args[NARGS++] = &ptr4;\
  if (&ptr5  == &Arg::none) goto done; args[NARGS++] = &ptr5;\
  if (&ptr6  == &Arg::none) goto done; args[NARGS++] = &ptr6;\
  if (&ptr7  == &Arg::none) goto done; args[NARGS++] = &ptr7;\
  if (&ptr8  == &Arg::none) goto done; args[NARGS++] = &ptr8;\
  if (&ptr9  == &Arg::none) goto done; args[NARGS++] = &ptr9;\
  if (&ptr10  == &Arg::none) goto done; args[NARGS++] = &ptr10;\
  if (&ptr11  == &Arg::none) goto done; args[NARGS++] = &ptr11;\
  if (&ptr12  == &Arg::none) goto done; args[NARGS++] = &ptr12;\
  if (&ptr13  == &Arg::none) goto done; args[NARGS++] = &ptr13;\
  if (&ptr14  == &Arg::none) goto done; args[NARGS++] = &ptr14;\
  if (&ptr15  == &Arg::none) goto done; args[NARGS++] = &ptr15;\
  if (&ptr16  == &Arg::none) goto done; args[NARGS++] = &ptr16;\
done:\
  (void)0

#define PASSARGLIST8 \
ptr1,ptr2,ptr3,ptr4,ptr5,ptr6,ptr7,ptr8

#define PASSARGLIST16 \
ptr1,ptr2,ptr3,ptr4,ptr5,ptr6,ptr7,ptr8,ptr9,ptr10,ptr11,ptr12,\
ptr13,ptr14,ptr15,ptr16

#define NOARGLIST16 \
Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,\
Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none,Arg::none

///////////////////////////////////////////////////////////////////////////////

std::string _fmt(const char *format, const Arg *args, size_t num_args);

template <typename... Args>
std::string fmt(const char *format, const Args&... args) {
  Arg arg_array[] = {args...};
  return _fmt(format, arg_array, sizeof...(Args));
}

template <typename... Args>
void print(const char *format, const Args&... args) {
  Arg arg_array[] = {args...};
  std::string str = _fmt(format, arg_array, sizeof...(Args));
  printf("%s\n", str.c_str());
}
template <typename... Args>
void printn(const char *format, const Args&... args) {
  Arg arg_array[] = {args...};
  std::string str = _fmt(format, arg_array, sizeof...(Args));
  printf("%s", str.c_str());
}
template <typename... Args>
void noprint(const char *format, const Args&... args) {
}

void startDebug(int maxlines);


///////////////////////////////////////////////////////////////////////////////
