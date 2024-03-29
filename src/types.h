/* date = September 20th 2021 7:46 pm */

#ifndef TYPES_H
#define TYPES_H

// CSTDLIB
#include <stdint.h>
#include <float.h>

#undef NULL
#define NULL 0
#define struct16 __declspec(align(16)) struct
#define local_persist static
#define global
#define fn 

#define ARG1(a, ...) (a)
#define iterate(type, current, list) for(type *current = list->Next; \
current != NULL; \
current = current->Next)
#define foreach(a, b, index_type) for(index_type a=0; a<b;a++)
#define loopblocks(curr, start, type, stride, count) for(type *curr=start; \
(u64)curr<(u64)start+(count*sizeof(type)); \
curr+=stride)
#define Assert(expression) if(!(expression)){ __debugbreak(); } while (0)
//Compile Time Assert
#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define StaticAssert(cond,msg) \
typedef struct { int CTASTR(static_assertion_failed_,msg) : !!(cond); } \
CTASTR(static_assertion_failed_,__COUNTER__)


#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

#define Min(a, b) ((a<b)?a:b)
#define Max(a, b) ((a>b)?a:b)
#define Clamp(x, Low, High) (Min(Max(Low, x), High))

#define Kilobytes(size) (         (size) * 1024LL)
#define Megabytes(size) (Kilobytes(size) * 1024LL)
#define Gigabytes(size) (Megabytes(size) * 1024LL)
#define Terabytes(size) (Gigabytes(size) * 1024LL)

// MATHEMATICAL CONSTANTS
#define Pi32     (3.141592653589793f)
#define Golden32 (1.618033988749894f)

typedef   uint8_t  u8;
typedef  uint16_t u16;
typedef  uint32_t u32;
typedef  uint64_t u64;
typedef    int8_t  s8;
typedef   int16_t s16;
typedef   int32_t s32;
typedef   int64_t s64;
typedef   uint8_t  b8;
typedef  uint16_t b16;
typedef  uint32_t b32;
typedef  uint64_t b64;
typedef     float f32;
typedef    double f64;

// VARIABLE ARGS
#include <stdarg.h>

// LIMITS
#define F32MAX FLT_MAX
#define U32MAX UINT32_MAX
#define U64MAX UINT64_MAX
#define S32MAX INT32_MAX
#define S64MAX INT64_MAX

typedef enum axis2 axis2;
enum axis2 { Axis2_X, Axis2_Y, Axis2_Count, };
typedef enum axis3 axis3;
enum axis3 { Axis3_X, Axis3_Y, Axis3_Z, Axis3_Count, };


//TIME
typedef struct datetime datetime;
struct datetime
{ s16 year; u8 mon; u8 day; u8 hour; u8 min; u8 sec; u16 ms; };


//VECTORS
typedef union v2f v2f;
union v2f
{
  struct { f32 x; f32 y; };
  f32 e[2];
};
typedef union v2s v2s;
union v2s
{
  struct { s32 x; s32 y; };
  s32 e[2];
};
typedef union v2u v2u;
union v2u
{
  struct { u32 x; u32 y; };
  u32 e[2];
};
typedef union v3f v3f;
union v3f
{
  struct { f32 x; f32 y; f32 z; };
  struct { f32 r; f32 g; f32 b; };
  struct { v2f xy; f32 _ignored00; };
  struct { f32 _ignored01; v2f yz; };
  f32 e[3];
};
typedef union v4f v4f;
union v4f
{
  struct { f32 x; f32 y; f32 z; f32 w; };
  struct { f32 r; f32 g; f32 b; f32 a; };
  f32 e[4];
};
typedef union v4u v4u;
union v4u
{
  struct { u32 x; u32 y; u32 z; u32 w; };
  struct { u32 r; u32 g; u32 b; u32 a; };
  u32 e[4];
};
// Inteivals
typedef union i2f i2f;
union i2f
{
  struct { f32 minx; f32 miny; f32 maxx; f32 maxy; };
  f32 e[2];
};
typedef union i2s64 i2s64;
union i2s64
{
  struct { s64 minx; s64 miny; s64 max; s64 maxy; };
  s32 e[4];
};
// MATRICES
typedef union m2f m2f;
union m2f
{
  v2f r[2]; f32 e[4]; f32 x[2][2];
};
typedef union m3f m3f;
union m3f
{
  v3f r[3]; f32 e[9]; f32 x[3][3];
};
typedef union m4f m4f;
union m4f
{
  v4f r[4]; f32 e[16]; f32 x[4][4];
  struct {
    f32 _00, _01, _02, _03;
    f32 _10, _11, _12, _13;
    f32 _20, _21, _22, _23;
    f32 _30, _31, _32, _33;
  };
};
//- INITIALIZERS
v2f V2f(f32 x, f32 y)
{
  v2f Result = { x, y };
  return Result;
}
v2s V2s(s32 x, s32 y)
{
  v2s Result = { x, y };
  return Result;
}
v2u V2u(u32 x, u32 y)
{
  v2u Result = { x, y };
  return Result;
}\
v3f V3f(f32 x, f32 y, f32 z)
{
  v3f Result = { x, y, z };
  return Result;
}
v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { x, y, z, w };
  return Result;
}
v4u V4u(u32 x, u32 y, u32 z, u32 w)
{
  v4u Result = { x, y, z, w };
  return Result;
}
i2f I2f(f32 minx, f32 miny, f32 maxx, f32 maxy)
{
  Assert(minx<=maxx); Assert(miny<=maxy);
  i2f Result = { minx, miny, maxx, maxy };
  return Result;
}
static m4f M4f(v4f r0, v4f r1, v4f r2, v4f r3)
{
  m4f Result = { r0, r1, r2, r3 };
  return Result;
}
b32 IsPowerOfTwo(u64 a)
{
  b32 Result = (a & (a-1)) == 0;
  return Result;
}
u64 MurmurHash64A ( const void * key, int len, u64 seed )
{
  const u64 m = 0xc6a4a7935bd1e995LLU;
  const int r = 47;
  u64 h = seed ^ (len * m);
  const u64 * data = (const u64 *)key;
  const u64 * end = data + (len/8);
  while(data != end)
  {
    u64 k = *data++;
    k *= m;  k ^= k >> r;  k *= m; 
    h ^= k; h *= m; 
  }
  const unsigned char * data2 = (const unsigned char*)data;
  switch(len & 7)
  {
    case 7: h ^= ((u64) data2[6]) << 48;
    case 6: h ^= ((u64) data2[5]) << 40;
    case 5: h ^= ((u64) data2[4]) << 32;
    case 4: h ^= ((u64) data2[3]) << 24;
    case 3: h ^= ((u64) data2[2]) << 16;
    case 2: h ^= ((u64) data2[1]) << 8;
    case 1: h ^= ((u64) data2[0]);
    h *= m;
  };
  h ^= h >> r;
  h *= m;
  h ^= h >> r;
  return h;
} 
#endif //TYPES_H
