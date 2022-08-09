#ifndef MEMORY_H
#define MEMORY_H

#ifndef MEMORY_DEFAULT_ALIGNMENT
#define MEMORY_DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

typedef struct arena arena;
struct arena
{
  u8  *Base;
  u64  Size;
  u64  CurrOffset;
  u64  PrevOffset;
};

typedef struct arena_temp arena_temp;
struct arena_temp
{
	arena *Arena;
  u64 PrevOffset;
	u64 CurrOffset;
};

#define ArenaPushType(  Arena,        Type) (Type *)ArenaPushBlock(Arena, sizeof(Type))
#define ArenaPushArray( Arena, Count, Type) (Type *)ArenaPushBlock(Arena, (Count) * sizeof(Type))
#define ArenaZeroType(  Instance          )         ArenaZeroBlock(sizeof(Instance), &(Instance))
#define SUBARENA_PUSHTYPE(Arena, Type) (Type *)SubArenaPush(Arena, sizeof(Type))

#define IsEqual(a, b, object_type) MemoryIsEqual(a, b, sizeof(object_type));

void MemorySet(u32 Value, void *SrcBuffer, size_t SrcSize);
void MemoryCopy(void *SrcBuffer, u64 SrcSize, void *DstBuffer, u64 DstSize);
b32  MemoryIsEqual(u8 *a, u8 *b, u64 MemorySize);
arena ArenaInit(arena *Arena, size_t Size, void *BasePtr);
void *ArenaPushBlock(arena *Arena, size_t Size);
void ArenaPopCount(arena *Arena, size_t Size);
void ArenaZeroBlock(size_t size, void *address);
arena ubArenaInit(arena *Arena, size_t Size, void *BasePtr);
arena SubArenaPush(arena *Arena, size_t Size, void *BasePtr);
void ArenaFreeUnused(arena *Arena);
arena ArenaTempInit(arena *Arena, arena *HostArena);
void ArenaReset(arena *Arena);

#endif //MEMORY_H
