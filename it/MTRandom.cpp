#include "MTRandom.h"
#include <math.h>
#ifdef WINDOWS
#include <windows.h>
#else
#include <time.h>
#endif

#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

Random::Random(int s /*=0*/) {
  N = 624;
  M = 397;
  mt = new unsigned int[N];
  mti = N+1;
  iset = 0;
  gset = 0;
  if (s > 0) _seed(s);
}

Random::~Random() {
  delete [] mt;
}

void Random::seed(int s) {
  _seed(s);
}

void Random::_seed(unsigned int seed) {
  /* setting initial seeds to mt[N] using         */
  /* the generator Line 25 of Table 1 in          */
  /* [KNUTH 1981, The Art of Computer Programming */
  /*    Vol. 2 (2nd Ed.), pp102]                  */
  mt[0] = seed & 0xffffffff;
  for (mti=1; mti<N; mti++)
    mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

unsigned int Random::_genrand() {
  unsigned int y;
  static unsigned int mag01[2]={0x0, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */
  if (mti >= N) { /* generate N words at one time */
    int kk;
    if (mti == N+1) { 
      // No seed called, use random seed
#ifdef WINDOWS
      SYSTEMTIME st;
      GetSystemTime(&st);
      unsigned int s = st.wHour*60*60*1000 + 
        st.wMinute*60*1000 + 
        st.wSecond * 1000 + 
        st.wMilliseconds;
      //printf("%s: seed=%d\n", id, s);
      _seed(s); /* a default initial seed is used   */
    }
#else
      _seed((unsigned int)time(0));
  }
#endif
    for (kk = 0; kk < N-M; kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for (; kk < N-1; kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
    mti = 0;
  }
  y = mt[mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);

  //return ( (double)y / (unsigned long)0xffffffff ); /* reals */
  return y; /* for integer generation */
}

unsigned int Random::next() {
  return _genrand();
}

int Random::next(int min, int max) {
  int result = min - 1;
  // shameful...
  while (result < min || result >= max) {
    long long r = _genrand();
    r *= (max - min);
    r /= (unsigned int)0xffffffff;
    result = (int)(min + r);
  }
  return result;
}

int Random::next(int max) {
  return next(0, max);
}

double Random::uniform(double min, double max) {
  double r = _genrand();
  r *= (max - min);
  r /= (unsigned int)0xffffffff;
  return min + r;
}

double Random::normal(double mean, double variance) {
  double fac, rsq, v1, v2;

  if (iset == 0) {
    /* we don't have an extra deviate handy, so pick two uniform
       numbers in the square extending from -1 to +1 in each direction, see if
       they are in the unit circle, and if they are not, try again '*/
    do {
      v1 = 2.0 * (double)_genrand()/(unsigned int)0xffffffff - 1.0;
      v2 = 2.0 * (double)_genrand()/(unsigned int)0xffffffff - 1.0;
      rsq = v1 * v1 + v2 * v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac = sqrt(-2.0 * log(rsq) / rsq);
    /* now make the Box-Muller transformation to get two normal deviates
       return one and save the other for next time */
    gset = v1 * fac;
    iset = 1; /* set flag */
    return mean + variance * v2 * fac;
  }
  else {
    /* we have an extra deviate handy, so unset the flag and return it */
    iset = 0;
    return mean + variance * gset;
  }
}

double Random::exponential(double mean) {
  double u;
  do u = (double)_genrand()/(unsigned int)0xffffffff; while (u == 0.0);
  return -mean * log(u);
}


//////////////////////////////////////////////////////////////////////////////

int QuasiRandom::dimension = 1;

QuasiRandom::QuasiRandom(int seed) {
  if (seed == 0) {
    dim = findPrime(dimension++);
    dim2 = findPrime(dimension++);
    if (dim == 1) dim++;
    if (dim2 == 1) dim2++;
  } else {
    dim = findPrime(seed++);
    dim2 = findPrime(seed++);
    if (dim == 1) dim++;
    if (dim2 == 1) dim2++;
  }
  counter = 1;
  iset = 0;
}
QuasiRandom::~QuasiRandom() {
}

double QuasiRandom::halton(int n, int b) {
  double remainder;
  double output = 0.0;
  double fraction = 1.0 / (double)b;
  //int N1 = 0;
  int index = n;
  while(index > 0) {
    //N1 = (index / b);
    remainder = index % b;
    output += fraction * remainder;
    index = (int)(index / b);
    fraction /= (double)b;
  } 
  return output;
}

void QuasiRandom::reset() {
  counter = 1;
  iset = 0;
}

double QuasiRandom::uniform() {
  return halton(counter++, dim);
}

double QuasiRandom::normal() {
  double fac, rsq, v1, v2;
  if (iset == 0) {
    do {
      v1 = 2.0 * halton(counter, dim) - 1.0;
      v2 = 2.0 * halton(counter, dim2) - 1.0;
      rsq = v1 * v1 + v2 * v2;
      counter++;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac = sqrt(-2.0 * log(rsq) / rsq);
    /* now make the Box-Muller transformation to get two normal deviates
    return one and save the other for next time */
    gset = v1 * fac;
    iset = 1; /* set flag */
    return v2 * fac;
  } else {
    /* we have an extra deviate handy, so unset the flag and return it */
    iset = 0;
    return gset;
  }
}

bool QuasiRandom::isPrime(int number) {
  bool isIt = true;
  for (int i = 2; i < number; i++) {
    if (number % i == 0) {
      isIt = false;
      break;
    }
  }
  if(number == 2) {
    isIt = false;
  }
  return isIt;
}

int QuasiRandom::findPrime(int n) {
  int prime = 1;
  int found = 1;
  while (found != n) {
    prime += 2;
    if (isPrime(prime) == true) {
      found++;
    }
  }
  return prime;
}

//////////////////////////////////////////////////////////////////////////////
