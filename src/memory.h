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
#define ArenaLocalInit(arena, size) \
{ \
u8 _buffer[size] = {0}; \
arena = ArenaInit(NULL, size, _buffer); \
} 


#define IsEqual(a, b, object_type) MemoryIsEqual(a, b, sizeof(object_type))

void MemorySet(u32 Value, void *SrcBuffer, size_t SrcSize);
void MemoryCopy(void *SrcBuffer, u64 SrcSize, void *DstBuffer, u64 DstSize);
fn b32 MemoryIsEqual(void *a, void *b, u64 MemorySize);
arena ArenaInit(arena *Arena, size_t Size, void *BasePtr);
void *ArenaPushBlock(arena *Arena, size_t Size);
void ArenaPopCount(arena *Arena, size_t Size);
void ArenaZeroBlock(size_t size, void *address);
void ArenaFreeUnused(arena *Arena);
void ArenaReset(arena *Arena);

#endif //MEMORY_H
