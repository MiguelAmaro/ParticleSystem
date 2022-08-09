/* date = September 20th 2021 7:46 pm */

#ifndef MATH_H
#define MATH_H

#include <immintrin.h>
// CSTDLIB
#include <math.h>
#include <stdlib.h>
#include "types.h"

//~ TRANSENDENTAL FUNCTIONS
f32 Abs(f32 a)
{
  union { f32 f; u32 u; } Result;
  Result.f  = a;
  Result.u &= 0x7fffffff;
  return Result.f;
}
f32 Sqr(f32 a)
{
  f32 Result = a * a;
  return Result;
}
f32 Cos(f32 a)
{
  f32 Result = cosf(a);
  return Result;
};
f32 Sin(f32 a)
{
  f32 Result = sinf(a);
  return Result;
};

// OVERLOADABLE OPERATIONS
#define Length(a) _Generic((a),           \
v2f: Lengthv2f, \
v3f: Lengthv3f,  \
v4f: Lengthv4f)((a))
#define Normalize(a) _Generic((a),                 \
v2f: Normalizev2f, \
v3f: Normalizev3f,  \
v4f: Normalizev4f)((a))
#define Lerp(a, b, t) _Generic((a),        \
f32: Lerpf,  \
v2f: Lerpv2f, \
v3f: Lerpv3f,  \
v4f: Lerpv4f)((a), (b), (t))
#if 0
#define Distance(a, b) _Generic((a),             \
v2f: Distancev2f, \
v3f: Distancev3f,  \
v4f: Distancev4f)((a), (b))
#endif
#define Dot(a, b) _Generic((a), \
v2f: Dotv2f, \
v3f: Dotv3f,  \
v4f: Dotv4f)((a), (b))
#define Hadamard(a, b) _Generic((a, b),          \
v2f: Hadamardv2f, \
v3f: Hadamardv3f,  \
v4f: Hadamardv4f)((a), (b))
#define Mul(...) _Generic(ARG1(__VA_ARGS__),  \
m4f: Mulm4f)(__VA_ARGS__)
#define Cross(a, b) _Generic((a, b),          \
v3f: Crossv3f)((a), (b))
#define Bias(a) _Generic((a),              \
v3f: Biasv3f)((a))
#define Scale(a, b) _Generic((a),          \
v2f: Scalev2f, \
v3f: Scalev3f,  \
v4f: Scalev4f)((a), (b))
#define Add(a, b) _Generic((a),        \
v2f: Addv2f, \
v3f: Addv3f,  \
v4f: Addv4f)((a), (b))
#define Sub(a, b) _Generic((a),        \
v2f: Subv2f, \
v3f: Subv3f,  \
v4f: Subv4f)((a), (b))
#define SqrRoot(a) _Generic((a),           \
f32: SqrRootf,  \
v2f: SqrRootv2f, \
v3f: SqrRootv3f,  \
v4f: SqrRootv4f)((a))
fn v2f Scalev2f(v2f a, f32 b)
{
  v2f Result = {a.x*b, a.y*b};
  return Result;
}
fn v3f Scalev3f(v3f a, f32 b)
{
  v3f Result = {a.x*b, a.y*b, a.z*b};
  return Result;
}
fn v4f Scalev4f(v4f a, f32 b)
{
  v4f Result = {a.x*b, a.y*b, a.z*b, a.w*b};
  return Result;
}
fn v2f Hadamardv2f(v2f a, v2f b)
{
  v2f Result = {a.x*b.x, a.y*b.y};
  return Result;
}
fn v3f Hadamardv3f(v3f a, v3f b)
{
  v3f Result = {a.x*b.x, a.y*b.y, a.z*b.z};
  return Result;
}
fn v4f Hadamardv4f(v4f a, v4f b)
{
  v4f Result = {a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w};
  return Result;
}
fn f32 SqrRootf(f32 a)
{
  f32 Result = sqrtf(a);
  return Result;
}
fn v2f SqrRootv2f(v2f a)
{
  v2f Result = {sqrtf(a.x), sqrtf(a.y)};
  return Result;
}
fn v3f SqrRootv3f(v3f a)
{
  v3f Result = {sqrtf(a.x), sqrtf(a.y), sqrtf(a.z)};
  return Result;
}
fn v4f SqrRootv4f(v4f a)
{
  v4f Result = {sqrtf(a.x), sqrtf(a.y), sqrtf(a.z), sqrtf(a.w)};
  return Result;
}
fn f32 Dotv2f(v2f a, v2f b)
{
  f32 Result = (a.x*b.x + a.y*b.y);
  return Result;
}
fn f32 Dotv3f(v3f a, v3f b)
{
  f32 Result = (a.x*b.x + a.y*b.y + a.z*b.z);
  return Result;
}
fn f32 Dotv4f(v4f a, v4f b)
{
  f32 Result = (a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w);
  return Result;
}
fn v2f Addv2f(v2f a, v2f b)
{
  v2f Result = {a.x + b.x, a.y + b.y};
  return Result;
}
fn v3f Addv3f(v3f a, v3f b)
{
  v3f Result = {a.x + b.x, a.y + b.y, a.z + b.z};
  return Result;
}
fn v4f Addv4f(v4f a, v4f b)
{
  v4f Result ={a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
  return Result;
}
fn v2f Subv2f(v2f a, v2f b)
{
  v2f Result = {a.x - b.x, a.y - b.y};
  return Result;
}
fn v3f Subv3f(v3f a, v3f b)
{
  v3f Result = {a.x - b.x, a.y - b.y, a.z - b.z};
  return Result;
}
fn v4f Subv4f(v4f a, v4f b)
{
  v4f Result ={a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
  return Result;
}
fn f32 Lengthv2f(v2f a)
{
  f32 Result = SqrRoot(Dot(a, a));
  return Result;
}
fn f32 Lengthv3f(v3f a)
{
  f32 Result = SqrRoot(Dot(a, a));
  return Result;
}
fn f32 Lengthv4f(v4f a)
{
  f32 Result = SqrRoot(Dot(a, a));
  return Result;
}
fn v2f Normalizev2f(v2f a)
{
  v2f Result = Scale(a, 1.0f/Length(a));
  return Result;
}
fn v3f Normalizev3f(v3f a)
{
  v3f Result = Scale(a, 1.0f/Length(a));
  return Result;
}
fn v4f Normalizev4f(v4f a)
{
  v4f Result = Scale(a, 1.0f/Length(a));
  return Result;
}
fn v3f Crossv3f(v3f a, v3f b)
{
  return V3f(a.e[1] * b.e[2] - a.e[2] * b.e[1],
             a.e[2] * b.e[0] - a.e[0] * b.e[2],
             a.e[0] * b.e[1] - a.e[1] * b.e[0]);
}
fn v3f Biasv3f(v3f a)
{
  v3f Result = Add(Scale(a, 0.5f), V3f(0.5f, 0.5f, 0.5f));
  return Result;
}
fn f32 Lerpf(f32 a, f32 b, f32 t)
{
  f32 Result = a*t + (1.0f-t)*b;
  return Result;
}
fn v2f Lerpv2f(v2f a, v2f b, f32 t)
{
  v2f Result = Add(Scale(a, t), Scale(b, (1.0f-t)));
  return Result;
}
fn v3f Lerpv3f(v3f a, v3f b, f32 t)
{
  v3f Result = Add(Scale(a, t), Scale(b, (1.0f-t)));
  return Result;
}
fn v4f Lerpv4f(v4f a, v4f b, f32 t)
{
  v4f Result = Add(Scale(a, t), Scale(b, (1.0f-t)));
  return Result;
}
fn m4f M4fIdentity(void)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}
fn m4f M4fScale(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(x   , 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, y   , 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, z   , 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}
fn m4f M4fRotate2D(f32 cos, f32 sin)
{
  m4f Result = M4fIdentity();
  
  Result.r[0] = V4f( cos, -sin, 0.0f, 0.0f);
  Result.r[1] = V4f( sin,  cos, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

fn m4f Mulm4f(m4f a, m4f b)
{
  m4f Result = { 0 };
#if 0
  for(u32 BColumn = 0; BColumn < 4; BColumn ++)
  {
    for(u32 ARow = 0; ARow < 4; ARow++)
    {
      v4f Entry = &Result.r[BColumn];
      Entry
        a
        for(u32 ScanElement = 0; ScanElement < 4; ScanElement++)
      { *Entry += a->r[ScanRow].c[ScanElement] * b->r[ScanElement].c[ScanCol]; }
    }
  }
#else
  __m128 res[4];
  __m128 sum;
  __m128 prod0, prod1, prod2, prod3;
  __m128 ar0 = _mm_loadu_ps(a.r[0].e);
  __m128 ar1 = _mm_loadu_ps(a.r[1].e);
  __m128 ar2 = _mm_loadu_ps(a.r[2].e);
  __m128 ar3 = _mm_loadu_ps(a.r[3].e);
  // NOTE(MIGUEL):(Expected to be column major)
  __m128 bc[4];
  bc[0] = _mm_loadu_ps(b.r[0].e);
  bc[1] = _mm_loadu_ps(b.r[1].e);
  bc[2] = _mm_loadu_ps(b.r[2].e);
  bc[3] = _mm_loadu_ps(b.r[3].e);
  
  for(u32 i=0;i<4;i++)
  {
    //DOTPRODUCT
    prod0 = _mm_mul_ps (bc[i], ar0);
    prod1 = _mm_mul_ps (bc[i], ar1);
    prod2 = _mm_mul_ps (bc[i], ar2);
    prod3 = _mm_mul_ps (bc[i], ar3);
    _MM_TRANSPOSE4_PS(prod0, prod1, prod2, prod3);
    sum = _mm_add_ps (prod0, prod1);
    sum = _mm_add_ps (sum , prod2);
    sum = _mm_add_ps (sum , prod3);
    res[i] = sum;
  }
  _MM_TRANSPOSE4_PS(res[0], res[1], res[2], res[3]);
  _mm_store_ps (Result.r[0].e, res[0]);
  _mm_store_ps (Result.r[1].e, res[1]);
  _mm_store_ps (Result.r[2].e, res[2]);
  _mm_store_ps (Result.r[3].e, res[3]);
#endif
  return Result;
}
fn m4f M4fRotate(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  m4f RotationX = M4fIdentity();
  if(x != 0.0f)
  {
    RotationX.r[0] = V4f(1.0f,      0.0f,      0.0f, 0.0f);
    RotationX.r[1] = V4f(0.0f, Cos(x),  -Sin(x), 0.0f);
    RotationX.r[2] = V4f(0.0f,   Sin(x), Cos(x), 0.0f);
    RotationX.r[3] = V4f(0.0f,      0.0f,      0.0f, 1.0f);
  }
  
  m4f RotationY = M4fIdentity();
  if(y != 0.0f)
  {
    RotationY.r[0] = V4f(Cos(y), 0.0f,   Sin(y), 0.0f);
    RotationY.r[1] = V4f(     0.0f, 1.0f,      0.0f, 0.0f);
    RotationY.r[2] = V4f( -Sin(y), 0.0f, Cos(y), 0.0f);
    RotationY.r[3] = V4f(     0.0f, 0.0f,      0.0f, 1.0f);
  }
  
  
  m4f RotationZ = M4fIdentity();
  if(z != 0.0f)
  {
    RotationZ.r[0] = V4f(Cos(z),  -Sin(z), 0.0f, 0.0f);
    RotationZ.r[1] = V4f(  Sin(z), Cos(z), 0.0f, 0.0f);
    RotationZ.r[2] = V4f(     0.0f,      0.0f, 1.0f, 0.0f);
    RotationZ.r[3] = V4f(     0.0f,      0.0f, 0.0f, 1.0f);
  }
  
  Result = Mul(RotationX, Mul(RotationY, RotationZ));
  
  return Result;
}
static m4f
M4fTranslate(v3f PosDelta)
{
  m4f Result = { 0 };
#if 1
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, PosDelta.x);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, PosDelta.y);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, PosDelta.z);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
#else
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(PosDelta.x, PosDelta.y, PosDelta.z, 1.0f);
#endif
  return Result;
}
static m4f
M4fViewport(v2f WindowDim)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(WindowDim.x / 2.0f, 0.0f, 0.0f, (WindowDim.x - 1.0f) / 2.0f);
  Result.r[1] = V4f(0.0f, WindowDim.y / 2.0f, 0.0f, (WindowDim.y - 1.0f) / 2.0f);
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
  Result.r[0].e[0] = 2.0f / (RightPlane - LeftPlane);
  Result.r[0].e[3] = -1.0f * ((RightPlane + LeftPlane) / (RightPlane - LeftPlane));
  
  // NORMALIZING Y
  Result.r[1].e[1] = 2.0f / (TopPlane - BottomPlane);
  Result.r[1].e[3] = -1.0f * ((TopPlane + BottomPlane) / (TopPlane - BottomPlane));
  
  // NORMALIZING Z
  Result.r[2].e[2] = 2.0f / (NearPlane - FarPlane);
  Result.r[2].e[3] = -1.0f * ((NearPlane + FarPlane) / (NearPlane - FarPlane));
  
  // DISREGARDING W
  Result.r[3].e[3] = 1.0f;
#endif
  
  return Result;
}

///EXTRA
inline s32 RoundF32toS32(f32 a)
{
  s32 Result = (s32)(a + 0.5f);
  return Result;
}
inline u32 RoundF32toU32(f32 a)
{
  u32 Result = (u32)(a + 0.5f);
  return Result;
}
f32 RandUniLat(void)
{
  f32 Result = (f32)rand()/(f32)RAND_MAX;
  return Result;
}
f32 RandBiLat(void)
{
  f32 Result = -1.0f + 2.0f*RandUniLat();
  return Result;
}
u32 BGRAPack4x8(v4f Data)
{
  u32 Result = ((RoundF32toU32(Data.a) << 24) |
                (RoundF32toU32(Data.r) << 16) |
                (RoundF32toU32(Data.g) <<  8) |
                (RoundF32toU32(Data.b) <<  0));
  return Result;
}

#endif // MATH_H
