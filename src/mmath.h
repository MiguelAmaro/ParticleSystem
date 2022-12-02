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
fn m4f M4fRotate(f32 x, f32 y, f32 z )
{
  m4f Result = { 0 };
  f32 sx = Sin(x);
  f32 cx = Cos(x);
  f32 sy = Sin(y);
  f32 cy = Cos(y);
  f32 sz = Sin(z);
  f32 cz = Cos(z);
  Result.r[0] = V4f(cz*cy, cz*sy*sx-sz*sx, cz*sy*cx+sz*sx, 0.0f);
  Result.r[1] = V4f(sz*cy, sz*sy*sx+cz*cx, sz*sy*sx-cz*cx, 0.0f);
  Result.r[2] = V4f(-sy, cy*sx, cy*sx, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
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
m4f Scalem4f(f32 x, f32 y, f32 z)
{
  m4f Result = {0};
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

v4f Mulm4f_v4f(m4f m, v4f a)
{
  v4f Result = {0};
  
#define AB(arc,brc) m.arc*a.brc
  Result.x = AB(_00, x) + AB(_01, y) + AB(_03, z) + AB(_03, w);
  Result.y = AB(_10, x) + AB(_11, y) + AB(_13, z) + AB(_13, w);
  Result.z = AB(_20, x) + AB(_21, y) + AB(_23, z) + AB(_23, w);
  Result.w = AB(_30, x) + AB(_31, y) + AB(_33, z) + AB(_33, w);
#undef AB
  return Result;
}

/* NOTE(MIGUEL): Issues that im having with matrices.
*                - How does changing the order of the operands change the output of the operation?
*                  mul(S,T) vs mul(T, S). How do i know which is valid?
*                
*                - What is the clipping volume of d3d11 and what is the clipping volume of opengl?
*                  How does that change the projection matrix?
*                
*                
*                - The projection matrix is packing apect ratio correction, normalization of points
*                  inside definded planes, perspective distortion. Where/how are those being applied in
*                  the matrix?
*                
*                - What is matrix ordering?
*                
*                - Can the projection matrix be composed with the model/world matrix?
*                
*                - What is the significance of the cos,sin terms in the generic 3d rotation matrix? How can i check for correctness
*                
*                - Is the first [col][row] of a matrix always the frist elem? if so how does the layout of the data
*                  change between col-major and row-major matrices? Try drawing the change with row col indices.
*                
*                - What in the projection matrix will change the haned-ness of the coordinate system?
*                
*                - What tests can i write for all of this?
*                
*                Resources:
*                WARANGING: (v) this is d3d9. is it applicable to d3d11?
*                https://learn.microsoft.com/en-us/windows/win32/direct3d9/projection-transform
*              https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math?redirectedfrom=MSDN#Matrix_Ordering
*             https://social.msdn.microsoft.com/Forums/sqlserver/en-US/73696d3c-debe-4840-a062-925449f0a366/perspective-projection-matrix-in-d3d11?forum=wingameswithdirectx   
*/
fn m4f Mulm4f(m4f a, m4f b)
{
  m4f Result = { 0 };
  //Row Compenent
#define AB(arc,brc) a.arc*b.brc
  Result._00 = AB(_00, _00) + AB(_10, _01) + AB(_20, _02) + AB(_30, _03);
  Result._01 = AB(_01, _00) + AB(_11, _01) + AB(_21, _02) + AB(_31, _03);
  Result._02 = AB(_02, _00) + AB(_12, _01) + AB(_22, _02) + AB(_32, _03);
  Result._03 = AB(_03, _00) + AB(_13, _01) + AB(_23, _02) + AB(_33, _03);
  
  Result._10 = AB(_00, _10) + AB(_10, _11) + AB(_20, _12) + AB(_30, _13);
  Result._11 = AB(_01, _10) + AB(_11, _11) + AB(_21, _12) + AB(_31, _13);
  Result._12 = AB(_02, _10) + AB(_12, _11) + AB(_22, _12) + AB(_32, _13);
  Result._13 = AB(_03, _10) + AB(_13, _11) + AB(_23, _12) + AB(_33, _13);
  
  Result._20 = AB(_00, _20) + AB(_10, _21) + AB(_20, _22) + AB(_30, _23);
  Result._21 = AB(_01, _20) + AB(_11, _21) + AB(_21, _22) + AB(_31, _23);
  Result._22 = AB(_02, _20) + AB(_12, _21) + AB(_22, _22) + AB(_32, _23);
  Result._23 = AB(_03, _20) + AB(_13, _21) + AB(_23, _22) + AB(_33, _23);
  
  Result._30 = AB(_00, _30) + AB(_10, _31) + AB(_20, _32) + AB(_30, _33);
  Result._31 = AB(_01, _30) + AB(_11, _31) + AB(_21, _32) + AB(_31, _33);
  Result._32 = AB(_02, _30) + AB(_12, _31) + AB(_22, _32) + AB(_32, _33);
  Result._33 = AB(_03, _30) + AB(_13, _31) + AB(_23, _32) + AB(_33, _33);
#undef AB
  return Result;
}
#if 0
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
#endif
static m4f
M4fTranslate(v3f PosDelta)
{
  m4f Result = { 0 };
  //row major
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, PosDelta.x);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, PosDelta.y);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, PosDelta.z);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
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
m4f M4fPerspective(f32 b, f32 l, f32 t, f32 r, f32 n, f32 f)
{
  m4f Result = {0};
  Result.r[0] = V4f(2*n/(r-l), 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 2*n/(t-b), 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, -(f+n)/(f-n), 1.0f);
  Result.r[3] = V4f(0.0f, 0.0f, -2*f*n/(f-n), 0.0f);
#if 0
  Result.r[0] = V4f(2*n/(r-l), 0.0f, (r+l)/(r-l), 0.0f);
  Result.r[1] = V4f(0.0f, 2*n/(t-b), (t+b)/(t-b), 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, -(f+n)/(f-n), -2*f*n/(f-n));
  Result.r[3] = V4f(0.0f, 0.0f, -1.0f, 0.0f);
#endif
  return Result;
}

///EXTRA
#include "pcg64.h"
pcg64 _RandGen = {0};
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
u64 RandRange(u64 Low, u64 High)
{
  u64 Result = pcg64_range(&_RandGen, Low, High);
  return Result;
}
f32 RandUniLat(void)
{
  f32 Result = pcg64_nextf(&_RandGen);
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
s32 Pow(s32 b, s32 e)
{
  s32 Result = 1;
  foreach(mult, e, s32)
  {
    Result *= b;
  }
  return Result;
}
#endif // MATH_H
