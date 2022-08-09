
//~ C STRING
u64 CStrGetSize(const char *String, b32 IncludeNull)
{
  u64 Result = 0;
  while(String[Result] && Result<U32MAX) { Result++; }
  Result += IncludeNull?1:0;
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
//~ CONNICAL STRINGS
str8 Str8Base(u8 *String, u64 Size)
{
  str8 Result = {0};
  Assert(Size<=U32MAX);
  Result.Data = String;
  Result.Size = (u32)Size;
  return Result;
}
str8 Str8FromCStr(char *String)
{
  str8 Result = {0};
  Result.Data = (u8 *)String;
  Result.Size = CStrGetSize(String, 0);
  return Result;
}
str8 Str8FromArena(arena *Arena, u64 Size)
{
  str8 Result = {0};
  Result.Size = Size;
  Result.Data = ArenaPushArray(Arena, Size, u8);
  return Result;
}
b32 Str8IsEqual(str8 a, str8 b)
{
  b32 Result = 1;
  if(a.Size != b.Size) return 0;
  u64 Index = a.Size;
  while((Index>0) && (a.Data[Index-1]==b.Data[Index-1])) { Index--; }
  Result = (Index==0);
  return Result;
}
str8 Str8Concat(str8 a, str8 b, arena *Arena)
{
  str8 Result;
  u64 Size = a.Size + b.Size;
  u8 *Data = ArenaPushArray(Arena, Size, u8);
  MemoryCopy(a.Data,a.Size,Data,a.Size);
  MemoryCopy(b.Data,b.Size,Data+a.Size,b.Size);
  Result = Str8(Data, Size);
  return Result;
}
//
//str8 Str8InsertAt(char Char, const char *StringA, const char *StringB, arena Arena)
//{
//str8 Result = {0};
//Result.Data = (u8 *)String;
//Result.Size = CStrGetSize(String, 0);
//return Result;
//}
//

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
