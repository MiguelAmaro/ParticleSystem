#ifndef PCG32_H
#define PCG32_H
#pragma once

#include <stdint.h>

#define PCG_DEFAULT_MULTIPLIER_64 6364136223846793005ULL
#define PCG_DEFAULT_INCREMENT_64  1442695040888963407ULL

typedef struct {
  uint64_t state;
} pcg32;

static inline uint32_t pcg32_next(pcg32* rng)
{
  uint64_t state = rng->state;
  rng->state = state * PCG_DEFAULT_MULTIPLIER_64 + PCG_DEFAULT_INCREMENT_64;
  
  // XSH-RR
  uint32_t value = (uint32_t)((state ^ (state >> 18)) >> 27);
  int rot = state >> 59;
  return rot ? (value >> rot) | (value << (32 - rot)) : value;
}

static inline uint64_t pcg32_next64(pcg32* rng)
{
  uint64_t value = pcg32_next(rng);
  value <<= 32;
  value |= pcg32_next(rng);
  return value;
}

// returns value in [low; high) interval

static inline uint32_t pcg32_range(pcg32* rng, uint32_t low, uint32_t high)
{
  uint32_t bound = high - low;
  uint32_t threshold = -(int32_t)bound % bound;
  
  for (;;)
  {
    uint32_t r = pcg32_next(rng);
    if (r >= threshold)
    {
      return low + (r % bound);
    }
  }
}

// returns float & double in [0; 1) interval

static inline float pcg32_nextf(pcg32* rng)
{
  uint32_t x = pcg32_next(rng);
  return (float)(int32_t)(x >> 8) * 0x1.0p-24f;
}

static inline double pcg32_nextd(pcg32* rng)
{
  uint64_t x = pcg32_next64(rng);
  return (double)(int64_t)(x >> 11) * 0x1.0p-53;
}

static inline void pcg32_seed(pcg32* rng, uint64_t seed)
{
  rng->state = 0ULL;
  pcg32_next(rng);
  rng->state += seed;
  pcg32_next(rng);
}

static inline void pcg32_advance(pcg32* rng, uint64_t delta)
{
  uint64_t cur_mult = PCG_DEFAULT_MULTIPLIER_64;
  uint64_t cur_plus = PCG_DEFAULT_INCREMENT_64;
  
  uint64_t acc_mult = 1;
  uint64_t acc_plus = 0;
  
  while (delta != 0)
  {
    if (delta & 1)
    {
      acc_mult *= cur_mult;
      acc_plus = acc_plus * cur_mult + cur_plus;
    }
    cur_plus = (cur_mult + 1) * cur_plus;
    cur_mult *= cur_mult;
    delta >>= 1;
  }
  
  rng->state = acc_mult * rng->state + acc_plus;
}
#endif //PCG32_H
