/* date = April 6th 2022 8:34 pm */

#ifndef STRING_H
#define STRING_H

#include "types.h"

typedef struct str8 str8;
struct str8
{
  u8  *Data;
  u32  Size;
};

typedef struct str16 str16;
struct str16
{
  u16 *Data;
  u32  Size;
};

typedef struct str32 str32;
struct str32
{
  u32 *Data;
  u32  Size;
};

typedef struct ucodepoint ucodepoint;
struct ucodepoint 
{
  u32 CodePoint;
  u32 Size;
};

//~ STRING FUNCTIONS

/*
*  16 | 8 | 4 | 2 | 1
*  1  | 0 | 0 | 0 | 0 = 16
*
*  16 | 8 | 4 | 2 | 1
*  1  | 1 | 0 | 0 | 0 = 24
*
*  16 | 8 | 4 | 2 | 1
*  1  | 1 | 1 | 0 | 0 = 28
*
*  16 | 8 | 4 | 2 | 1
*  1  | 1 | 1 | 1 | 0 = 28
                                            
*  2^5
*  1111 1111
        *     >>    3
*  1111 1(000)
*         5(2^5 = 32)
*  000 (1 1111)
*  cases:
            *  0xxxx xxx: 1 Byte utf8
            *  10xxx xxx: Trailing bytes
            *  110xx xxx: 2 Byte utf8
            *  1110x xxx: 3 Byte utf8
            *  11110 xxx: 4 Byte utf8
            *  11111 xxx: Error condition
            *  
            *  index | offset | output
    *  0  + 16: 1
*  16 +  8: 0
*  24 +  4: 2
*  28 +  2: 3
*  30 +  1: 4
    *  31 +  0: 0 (OutOfBoundsError)
    *
    *  Max Bitwidth for a code point is 21 but we only get this 
*  in the case of 4byte utf8s  and in that case the number of
*  codepoints in the lead byte is 3
*  This is why there is a leftshift of 18(21-3)
*/


str8 Str8(u8 *String, u64 Size)
{
  str8 Result = {0};
  Assert(Size<=U32MAX);
  Result.Data = String;
  Result.Size = (u32)Size;
  return Result;
}

str8 Str8FromFile(FILE *File)
{
  str8 Result = {0};
  u64 FileSize = GetFileSizeFromCSTL(File);
  Assert(FileSize<=U32MAX);
  Result.Data = malloc(FileSize);
  Result.Size = (u32)FileSize;
  size_t ReadSize = 0;
  while((ReadSize<FileSize) && !feof(File))
  {
    ReadSize += fread(Result.Data + ReadSize, sizeof(u8), Result.Size, File);
  }
  return Result;
}

u32 CStrGetSize(const char *String, b32 IncludeNull)
{
  u32 Result = 0;
  while(String[Result] && Result<U32MAX) { Result++; }
  Result += IncludeNull?1:0;
  return Result;
}

str8 Str8FromCStr(const char *String)
{
  str8 Result = {0};
  Result.Data = (u8 *)String;
  Result.Size = CStrGetSize(String, 0);
  return Result;
}

static b32 Str8IsEqual(str8 a, str8 b)
{
  b32 Result = 1;
  if(a.Size != b.Size) return 0;
  u32 Index = a.Size;
  while((Index>0) && (a.Data[Index-1]==b.Data[Index-1])) { Index--; }
  Result = (Index==0);
  return Result;
}

static b32 CStrIsEqual(const char *a, const char *b)
{
  b32 Result = 1;
  size_t Length = CStrGetSize(a, 1);
  Assert(Length<3000);
  while((*a++==*b++) && (0<Length))
  {Length--;}
  Result = Length==0;
  
  return Result;
}

//~ STRING 32

str32 Str32(u32 *Data, u32 Size)
{
  str32 Result = {0};
  Result.Data = Data;
  Result.Size = Size;
  return Result;
}
//
//str32 Str32FromArena(arena Arena)
//{
//str32 Result = {0};
//Result.Data = Arena.Base;
//Result.Size = (u32)(Arena.Size/sizeof(u32));
//return Result;
//}
//
#endif //STRING_H
