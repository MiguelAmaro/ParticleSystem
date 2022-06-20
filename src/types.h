/* date = September 20th 2021 7:46 pm */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define  SIM_NULL  (0)
#define  SIM_TRUE  (1)
#define  SIM_FALSE (0)

//#define  fn
#define global

#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
//Compile Time Assert
#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define StaticAssert(cond,msg) \
typedef struct { int CTASTR(static_assertion_failed_,msg) : !!(cond); } \
CTASTR(static_assertion_failed_,__COUNTER__)

typedef size_t memory_index;

typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef  int8_t  s8 ;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef  uint8_t  b8 ;
typedef  uint16_t b16;
typedef  uint32_t b32;
typedef  uint64_t b64;

typedef float f32;
typedef double f64;

//#define F32MAX UINT32_MAX
#define U32MAX UINT32_MAX
#define U64MAX UINT64_MAX

#endif //TYPES_H
