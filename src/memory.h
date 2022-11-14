#ifndef MEMORY_H
#define MEMORY_H

#ifndef MEMORY_DEFAULT_ALIGNMENT
#define MEMORY_DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

#define threadlocal __declspec(thread)
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
#define TCTX_SCRATCH_POOL_COUNT 2
typedef struct thread_ctx thread_ctx;
struct thread_ctx
{ 
  u8   *Memory;
  arena ScratchPool[TCTX_SCRATCH_POOL_COUNT];
};

#define GetScratchMemory(used_arena_list, used_arena_count) \
OSThreadCtxGetScratch(OSThreadCtxGet(gWin32ThreadContextId), \
used_arena_list, used_arena_count);


#define ArenaPushType(  Arena,        Type) (Type *)ArenaPushBlock(Arena, sizeof(Type))
#define ArenaPushArray( Arena, Count, Type) (Type *)ArenaPushBlock(Arena, (Count) * sizeof(Type))
#define ArenaZeroType(  Instance          )         ArenaZeroBlock(sizeof(Instance), &(Instance))
#define ArenaLocalInit(arena, size) \
{ \
u8 _buffer[size] = {0}; \
arena = ArenaInit(NULL, size, _buffer); \
} 
#define MemoryGetScratch(used_arena_list, used_arena_count) \
ThreadCtxGetScratch(ThreadCtxGet(), used_arena_list, used_arena_count);
#define MemoryReleaseScratch(scratch) ArenaTempEnd(scratch);
#define IsEqual(a, b, object_type) MemoryIsEqual(a, b, sizeof(object_type))

// MEMORY
void MemorySet(u32 Value, void *SrcBuffer, size_t SrcSize);
void MemoryCopy(void *SrcBuffer, u64 SrcSize, void *DstBuffer, u64 DstSize);
fn b32 MemoryIsEqual(void *a, void *b, u64 MemorySize);
// ARENA
arena_temp ArenaTempBegin(arena *Arena);
arena ArenaInit(arena *Arena, size_t Size, void *BasePtr);
void *ArenaPushBlock(arena *Arena, size_t Size);
void ArenaPopCount(arena *Arena, size_t Size);
void ArenaZeroBlock(size_t size, void *address);
void ArenaFreeUnused(arena *Arena);
void ArenaReset(arena *Arena);
// THREAD CTX
arena_temp ThreadCtxGetScratch(thread_ctx *Ctx, arena **Conflicts, u64 ConflictCount);
void ThreadCtxSet(void *Ptr);
thread_ctx *ThreadCtxGet(void);
thread_ctx *ThreadCtxGet(void);
void ThreadCtxInit(thread_ctx *Ctx, void *Memory, u64 MemorySize);
#endif //MEMORY_H
