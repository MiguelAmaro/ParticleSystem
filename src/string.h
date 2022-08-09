#ifndef STRING_H
#define STRING_H

typedef struct str8 str8;
struct str8
{
  u8  *Data;
  u64  Size;
};

typedef struct str16 str16;
struct str16
{
  u16 *Data;
  u64 Size;
};

typedef struct str32 str32;
struct str32
{
  u32 *Data;
  u64  Size;
};

typedef struct ucodepoint ucodepoint;
struct ucodepoint
{
  u32 CodePoint;
  u64 Size;
};

#define Str8(...) _Generic(ARG1(__VA_ARGS__),  \
u8 *   : Str8Base,    \
char * : Str8FromCStr, \
arena *: Str8FromArena)(__VA_ARGS__)

//~ C STRINGS
u64 CStrGetSize(const char *String, b32 IncludeNull);
static b32 CStrIsEqual(const char *a, const char *b);
//~ 8 BIT STRINGS
str8 Str8Base(u8 *String, u64 Size);
str8 Str8FromCStr(char *String);
str8 Str8FromArena(arena *Arena, u64 Size);
b32  Str8IsEqual(str8 a, str8 b);
str8 Str8Concat(str8 a, str8 b, arena *Arena);
str8 Str8InsertAt(char Char, const char *StringA, const char *StringB, arena Arena);
//~ 32 BIT STRINGS
str32 Str32(u32 *Data, u32 Size);
str32 Str32FromArena(arena Arena);

#endif //STRING_H
