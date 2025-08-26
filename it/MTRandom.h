#pragma once

class Random {
public:
  Random(int seed = 0);
  ~Random();
public:
  // Return uniformly distributed integer [0,0xffffffff]
  unsigned int next();
  /// Return uniformly distributed integer from [min, max[
  int next(int min, int max);
  /// Return uniformly distributed integer in [0, max[ (not including max)
  int next(int max);
  /// Uniform double in [min, max]
  double uniform(double min, double max);
  double normal(double mean, double variance);
  double exponential(double mean);
  void seed(int s);
private:
  int N, M;
  unsigned int *mt; /* the array for the state vector  */
  int mti; /* mti==N+1 means mt[N] is not initialized */
  int iset;
  double gset;
  unsigned int _genrand(); // in range 0..0xffffffff
  void _seed(unsigned int s);
};

// Halton sequence
class QuasiRandom {
public:
  QuasiRandom(int seed = 0);
  ~QuasiRandom();
  double uniform();
  double normal();
  void reset();
private:
  double halton(int n, int b);
  bool isPrime(int number);
  int findPrime(int n);
  int dim;
  int dim2;
  int counter;
  int iset;
  double gset;
  static int dimension;
};
