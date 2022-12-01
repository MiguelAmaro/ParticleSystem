#ifndef PCG64_H
#define PCG64_H

#include <stdint.h>

// NOTE: you should use this on 64-bit architectures only. While it will work fine on	
// 32-bit too, it will be slower there. You should use this only if you need to have	
// 128-bit state. If all you need is 64-bit random, use pcg32_next64 from pcg32.h

#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 pcg64_uint128;
#define PCG64_INIT(a, b)    (((pcg64_uint128)(a)) << 64) + (b)
#define PCG64_IS_ZERO(r)    ((r) == (pcg64_uint128)0)
#define PCG64_ZERO(r)       (r) = (pcg64_uint128)0
#define PCG64_COPY(r, x)    (r) = (x)
#define PCG64_LOW(r)        (uint64_t)(r)
#define PCG64_HIGH(r)       (uint64_t)((r) >> 64)
#define PCG64_SHR(r, c)     (r) >>= (c)
#define PCG64_ADD(r, x)     (r) += (x)
#define PCG64_MUL(r, x)     (r) *= (x)
#define PCG64_MUL2(r, a, b) (r) = (pcg64_uint128)(a) * (b)
#else
typedef uint64_t pcg64_uint128[2];
#define PCG64_INIT(a, b) { (b), (a) }
#define PCG64_IS_ZERO(r) ((r)[0] == 0 && (r)[1] == 0)
#define PCG64_ZERO(r)    (r)[0] = (r)[1] = 0
#define PCG64_COPY(r, x) (r)[0] = (x)[0], (r)[1] = (x)[1]
#define PCG64_LOW(r)     (r)[0]
#define PCG64_HIGH(r)    (r)[1]
#if defined(_MSC_VER) && defined(_M_AMD64)
#include <intrin.h>
#define PCG64_SHR(r, c) (r)[0] = __shiftright128((r)[0], (r)[1], (c)), (r)[1] >>= (c)
#define PCG64_ADD(r, x) _addcarry_u64(_addcarry_u64(0, (r)[0], (x)[0], &(r)[0]), (r)[1], (x)[1], &(r)[1])
#define PCG64_MUL(r, x)                                 \
{                                                       \
uint64_t _temp = (r)[0] * (x)[1] + (r)[1] * (x)[0]; \
(r)[0] = _umul128((r)[0], (x)[0], &(r)[1]);         \
(r)[1] += _temp;                                    \
}    
#define PCG64_MUL2(r, a, b) (r)[0] = _umul128((a), (b), &(r)[1])
#elif defined(_MSC_VER) && defined(_M_ARM64)
#include <intrin.h>
#define PCG64_SHR(r, c) (r)[0] = (((r)[0] >> (c)) | ((r)[1] << (64 - (c)))), (r)[1] >>= (c)
#define PCG64_ADD(r, x) (r)[0] += (x)[0], (r)[1] += (x)[1] + ((r)[0] < (x)[0])
#define PCG64_MUL(r, x)                                                   \
{                                                                         \
(r)[1] = __umulh((r)[0], (x)[0]) + (r)[0] * (x)[1] + (r)[1] * (x)[0]; \
(r)[0] *= (x)[0];                                                     \
}
#define PCG64_MUL2(r, a, b)     \
{                               \
(r)[0] = (a) * (b);         \
(r)[1] = __umulh((a), (b)); \
}
#else
#define PCG64_SHR(r, c) (r)[0] = (((r)[0] >> (c)) | ((r)[1] << (64 - (c)))), (r)[1] >>= (c)
#define PCG64_ADD(r, x) (r)[0] += (x)[0], (r)[1] += (x)[1] + ((r)[0] < (x)[0])
#define PCG64_MUL(r, x)                                     \
{                                                           \
uint64_t _a = (r)[0];                                   \
uint64_t _b = (x)[0];                                   \
uint64_t _ll = (_a & 0xFFFFFFFF) * (_b & 0xFFFFFFFF);   \
uint64_t _hl = (_a >> 32)        * (_b & 0xFFFFFFFF);   \
uint64_t _lh = (_a & 0xFFFFFFFF) * (_b >> 32);          \
uint64_t _hh = (_a >> 32)        * (_b >> 32);          \
uint64_t _mid = (_ll >> 32) + (_hl & 0xFFFFFFFF) + _lh; \
uint64_t _top = (_hl >> 32) + (_mid >> 32)       + _hh; \
(r)[1] = _a * (x)[1] + (r)[1] * _b + _top;              \
(r)[0] = (_mid << 32) | (_ll & 0xFFFFFFFF);             \
}
#define PCG64_MUL2(r, a, b)                                 \
{                                                           \
uint64_t _a = (a);                                      \
uint64_t _b = (b);                                      \
uint64_t _ll = (_a & 0xFFFFFFFF) * (_b & 0xFFFFFFFF);   \
uint64_t _hl = (_a >> 32)        * (_b & 0xFFFFFFFF);   \
uint64_t _lh = (_a & 0xFFFFFFFF) * (_b >> 32);          \
uint64_t _hh = (_a >> 32)        * (_b >> 32);          \
uint64_t _mid = (_ll >> 32) + (_hl & 0xFFFFFFFF) + _lh; \
uint64_t _top = (_hl >> 32) + (_mid >> 32)       + _hh; \
(r)[0] = (_mid << 32) | (_ll & 0xFFFFFFFF);             \
(r)[1] = _top;                                          \
}
#endif
#endif

#if defined(__clang__)
#define PCG64_ROR(r, c) __builtin_rotateright64((r), (c))
#elif defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_ARM64))
#define PCG64_ROR(r, c) _rotr64((r), (c))
#else
#define PCG64_ROR(r, c) ((c) ? (((r) >> (c)) | (r) << (64 - (c))) : (r))
#endif

#define PCG_DEFAULT_MULTIPLIER_128_H 2549297995355413924ULL
#define PCG_DEFAULT_MULTIPLIER_128_L 4865540595714422341ULL
#define PCG_DEFAULT_INCREMENT_128_H  6364136223846793005ULL
#define PCG_DEFAULT_INCREMENT_128_L  1442695040888963407ULL

typedef struct {
  pcg64_uint128 state;
} pcg64;

static inline uint64_t pcg64_next(pcg64* rng)
{
  pcg64_uint128 state_mul = PCG64_INIT( PCG_DEFAULT_MULTIPLIER_128_H, PCG_DEFAULT_MULTIPLIER_128_L );
  pcg64_uint128 state_add = PCG64_INIT( PCG_DEFAULT_INCREMENT_128_H, PCG_DEFAULT_INCREMENT_128_L );
  
  pcg64_uint128 state;
  PCG64_COPY(state, rng->state);
  PCG64_MUL(state, state_mul);
  PCG64_ADD(state, state_add);
  PCG64_COPY(rng->state, state);
  
  // XSL-RR
  uint64_t value = PCG64_HIGH(state) ^ PCG64_LOW(state);
  int rot = PCG64_HIGH(state) >> 58;
  return PCG64_ROR(value, rot);
}

// returns value in [low; high) interval

static inline uint64_t pcg64_range(pcg64* rng, uint64_t low, uint64_t high)
{
  uint64_t bound = high - low;
  
#if 1
  
  // An optimal algorithm for bounded random integers: https://github.com/apple/swift/pull/39143
  
  pcg64_uint128 r1, r2;
  PCG64_MUL2(r1, pcg64_next(rng), bound);
  PCG64_MUL2(r2, pcg64_next(rng), bound);
  
  pcg64_uint128 t2 = PCG64_INIT( 0, PCG64_LOW(r1) );
  pcg64_uint128 r2h = PCG64_INIT( 0, PCG64_HIGH(r2) );
  PCG64_ADD(t2, r2h);
  
  return low + PCG64_HIGH(r1) + PCG64_HIGH(t2);
  
#else
  // Fast Random Integer Generation in an Interval: https://arxiv.org/abs/1805.10941
  pcg64_uint128 m;
  PCG64_MUL2(m, pcg64_next(rng), bound);
  uint64_t l = PCG64_LOW(m);
  if (l < bound)
  {
    uint64_t t = -(int64_t)bound % bound;
    while (l < t)
    {
      PCG64_MUL2(m, pcg64_next(rng), bound);
      l = PCG64_LOW(m);
    }
  }
  return low + PCG64_HIGH(m);
#endif
}

// returns float & double in [0; 1) interval

static inline float pcg64_nextf(pcg64* rng)
{
  uint64_t x = pcg64_next(rng);
  return (float)(int32_t)(x >> 40) * 0x1.0p-24f;
}

static inline double pcg64_nextd(pcg64* rng)
{
  uint64_t x = pcg64_next(rng);
  return (double)(int64_t)(x >> 11) * 0x1.0p-53;
}

static inline void pcg64_seed(pcg64* rng, uint64_t seed_high, uint64_t seed_low)
{
  pcg64_uint128 seed = PCG64_INIT( seed_high, seed_low );
  
  PCG64_ZERO(rng->state);
  pcg64_next(rng);
  PCG64_ADD(rng->state, seed);
  pcg64_next(rng);
}

static inline void pcg64_advance(pcg64* rng, uint64_t delta_high, uint64_t delta_low)
{
  pcg64_uint128 cur_mult = PCG64_INIT( PCG_DEFAULT_MULTIPLIER_128_H, PCG_DEFAULT_MULTIPLIER_128_L );
  pcg64_uint128 cur_plus = PCG64_INIT( PCG_DEFAULT_INCREMENT_128_H, PCG_DEFAULT_INCREMENT_128_L );
  
  pcg64_uint128 acc_mult = PCG64_INIT( 0ULL, 1ULL );
  pcg64_uint128 acc_plus = PCG64_INIT( 0ULL, 0ULL );
  
  pcg64_uint128 delta = PCG64_INIT( delta_high, delta_low );
  
  while (!PCG64_IS_ZERO(delta))
  {
    if (PCG64_LOW(delta) & 1)
    {
      PCG64_MUL(acc_mult, cur_mult);
      PCG64_MUL(acc_plus, cur_mult);
      PCG64_ADD(acc_plus, cur_plus);
    }
    pcg64_uint128 temp;
    PCG64_COPY(temp, cur_plus);
    PCG64_MUL(cur_plus, cur_mult);
    PCG64_ADD(cur_plus, temp);
    PCG64_MUL(cur_mult, cur_mult);
    PCG64_SHR(delta, 1);
  }
  
  PCG64_MUL(rng->state, acc_mult);
  PCG64_ADD(rng->state, acc_plus);
}

#undef PCG64_INIT
#undef PCG64_IS_ZERO
#undef PCG64_ZERO
#undef PCG64_COPY
#undef PCG64_LOW
#undef PCG64_HIGH
#undef PCG64_SHR
#undef PCG64_ADD
#undef PCG64_MUL
#undef PCG64_MUL2
#undef PCG64_ROR

#endif //PCG64_H
