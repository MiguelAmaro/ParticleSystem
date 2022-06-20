/* date = November 25th 2021 1:33 pm */

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef struct arena arena;
struct arena
{
  size_t  Size;
  size_t  Used;
  void   *Base;
};

typedef struct subarena subarena;
struct subarena
{
  size_t  Size;
  void   *Base;
};

void
MemorySet(u32 Value, void *SrcBuffer, size_t SrcSize);
void
MemoryCopy(void *SrcBuffer, u32 SrcSize,
           void *DstBuffer, u32 DstSize);

b32
MemoryIsEqual(u8 *a, u8 *b, u64 MemorySize);

#define ArenaPushType(  Arena,        Type) (Type *)ArenaPushBlock(Arena, sizeof(Type))
#define ArenaPushArray( Arena, Count, Type) (Type *)ArenaPushBlock(Arena, (Count) * sizeof(Type))
#define ArenaZeroType(  Instance          )         ArenaZeroBlock(sizeof(Instance), &(Instance))

#define SUBARENA_PUSHTYPE(Arena, Type) (Type *)SubArenaPush(Arena, sizeof(Type))

arena
ArenaInit(arena *Arena, size_t Size, void *BasePtr);

void *
ArenaPushBlock(arena *Arena, size_t Size);

void
ArenaPopCount(arena *Arena, size_t Size);

void
ArenaZeroBlock(size_t size, void *address);

arena
SubArenaInit(arena *Arena, size_t Size, void *BasePtr);

arena
SubArenaPush(arena *Arena, size_t Size, void *BasePtr);

void
ArenaFreeUnused(arena *Arena);

arena
ArenaTempInit(arena *Arena, arena *HostArena);

#endif //MEMORY_H
