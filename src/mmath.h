/* date = September 20th 2021 7:46 pm */

#ifndef MATH_H
#define MATH_H

#include "types.h"



//-/ MACROS

#define Min(a, b) ((a<b)?a:b)
#define Max(a, b) ((a>b)?a:b)
#define Clamp(x, Low, High) (Min(Max(Low, x), High))

#define KILOBYTES(size) (         (size) * 1024LL)

#define MEGABYTES(size) (KILOBYTES(size) * 1024LL)
#define GIGABYTES(size) (MEGABYTES(size) * 1024LL)
#define TERABYTES(size) (GIGABYTES(size) * 1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))
#define MAXIMUM(a, b) ((a < b) ? (a) : (b))

#define PI32 (3.141592653589793f)
#define GOLDEN_RATIO32 (1.618033988749894f)


//~ TRANSENDENTAL FUNCTIONS

f32 Absolute(f32 Value)
{
  union
  { 
    f32 f;
    u32 u;
  } Result;
  
  Result.f  = Value;
  Result.u &= 0x7fffffff;
  
  return Result.f;
}

f32 Square(f32 Value)
{
  f32 Result = Value * Value;
  
  return Result;
};

//- VECTORS 

typedef union v2f v2f;
union v2f
{
  struct
  {
    f32 x;
    f32 y;
  };
  f32 c[2];
};

typedef union v2s v2s;
union v2s
{
  struct
  {
    s32 x;
    s32 y;
  };
  s32 c[2];
};

typedef union v3f v3f;
union v3f
{
  struct
  {
    f32 x;
    f32 y;
    f32 z;
  };
  struct
  {
    f32 r;
    f32 g;
    f32 b;
  };
  struct
  {
    v2f xy;
    f32 _ignored00;
  };
  f32 c[3];
};

typedef union v4f v4f;
union v4f
{
  struct
  {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
  };
  struct
  {
    f32 r;
    f32 g;
    f32 b;
    f32 a;
  };
  f32 c[4];
};


typedef union v4u v4u;
union v4u
{
  struct
  {
    u32 x;
    u32 y;
    u32 z;
    u32 w;
  };
  struct
  {
    u32 r;
    u32 g;
    u32 b;
    u32 a;
  };
  u32 c[4];
};

//- RECTANGLES 

typedef union r2f r2f;
union r2f
{
  struct
  {
    v2f min;
    v2f max;
  };
  f32 e[2];
};

typedef union r2s r2s;
union r2s
{
  struct
  {
    v2s min;
    v2s max;
  };
  s32 e[4];
};


//- MATRICES 

typedef union m2f m2f;
union m2f
{
  v2f r[2];
  f32   e[4];
  f32   x[2][2];
};

typedef union m3f32 m3f32;
union m3f32
{
  v3f r[3];
  f32 e[9];
  f32 x[3][3];
};


typedef union m4f m4f;
union m4f
{
  v4f r[4];
  f32 e[16];
  f32 x[4][4];
};


v2f V2f(f32 x, f32 y)
{
  v2f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  
  return Result;
}

v3f V3f(f32 x, f32 y, f32 z)
{
  v3f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  
  return Result;
}

v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  Result.w = w;
  
  return Result;
}

v4u V4u(u32 x, u32 y, u32 z, u32 w)
{
  v4u Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  Result.w = w;
  
  return Result;
}

r2f R2f(f32 minx, f32 miny, f32 maxx, f32 maxy)
{
  Assert(minx<=maxx);
  Assert(miny<=maxy);
  r2f Result = {0};
  Result.min.x = minx;
  Result.min.y = miny;
  Result.max.x = maxx;
  Result.max.y = maxy;
  
  return Result;
}

static m4f M4f(v4f r0, v4f r1, v4f r2, v4f r3)
{
  m4f Result = {0};
  Result.r[0] = r0;
  Result.r[1] = r1;
  Result.r[2] = r2;
  Result.r[3] = r3;
  return Result;
}

static m4f
M4fIdentity(void)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

static m4f
M4fOrtho(f32 LeftPlane,
         f32 RightPlane,
         f32 BottomPlane,
         f32 TopPlane,
         f32 NearPlane,
         f32 FarPlane)
{
  m4f Result = { 0 };
#if 0
  // NOTE(MIGUEL): This path has the origin in the middle of the viewport
  // NORMALIZING X
  Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
  
  // NORMALIZING Y
  Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
  
  // NORMALIZING Z
  Result.r[2].c[2] = 1.0f / (FarPlane - NearPlane);
  Result.r[2].c[3] = -1.0f * ((NearPlane) / (FarPlane - NearPlane));
  
  // DISREGARDING W
  Result.r[3].c[3] = 1.0f;
#else
  // NOTE(MIGUEL): This path has the origin in the lowerleft corner of the viewport
  
  // NORMALIZING X
  Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
  Result.r[0].c[3] = -1.0f * ((RightPlane + LeftPlane) / (RightPlane - LeftPlane));
  
  // NORMALIZING Y
  Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
  Result.r[1].c[3] = -1.0f * ((TopPlane + BottomPlane) / (TopPlane - BottomPlane));
  
  // NORMALIZING Z
  Result.r[2].c[2] = 2.0f / (NearPlane - FarPlane);
  Result.r[2].c[3] = -1.0f * ((NearPlane + FarPlane) / (NearPlane - FarPlane));
  
  // DISREGARDING W
  Result.r[3].c[3] = 1.0f;
#endif
  
  return Result;
}

#endif // MATH_H
