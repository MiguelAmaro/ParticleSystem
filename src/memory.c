#include "memory.h"

void
MemoryCopy(void *SrcBuffer, u32 SrcSize,
           void *DstBuffer, u32 DstSize)
{
  
  u8* Src = (u8 *)SrcBuffer;
  u8* Dst = (u8 *)DstBuffer;
  
  for(u32 Index = 0;
      Index < SrcSize && Index < DstSize;
      Index++, Src++, Dst++)
  {
    *Dst = *Src;
  }
  
  return;
}

void MemorySet(u32 Value, void *Memory, u64 Size)
{
  u8 *Dest = (u8 *)Memory;
  while(Size--) {*Dest++ = (u8)Value;}
  return;
}

b32 MemoryIsEqual(u8 *a, u8 *b, u64 MemorySize)
{
  b32 Result = 0;
  u64 Index = MemorySize;
  while((Index>0) && (a[Index-1]==b[Index-1])) { Index--; }
  Result = (Index==0);
  return Result;
}

//- ARENAS 

arena
ArenaInit(arena *Arena, size_t Size, void *Base)
{
  arena Result;
  Result.Base = Base;
  Result.Size = Size;
  Result.Used = 0;
  if(Arena) { *Arena = Result; }
  return Result;
}

void
ArenaPopCount(arena *Arena, size_t Size)
{
  Assert(((u8 *)Arena->Base+(Arena->Used-Size)) >= (u8 *)Arena->Base);
  Arena->Used -= Size;
  
  return;
}

void
ArenaDiscard(arena *Arena)
{
  // NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
  //               results in a noticable slow down.
  MemorySet(0, Arena->Base, Arena->Used);
  
  Arena->Base = 0;
  Arena->Size    = 0;
  Arena->Used    = 0;
  
  return;
}

void *
ArenaPushBlock(arena *Arena, size_t Size)
{
  Assert((Arena->Used + Size) <= Arena->Size);
  
  void *NewArenaPartitionAddress  = (u8 *)Arena->Base + Arena->Used;
  Arena->Used  += Size;
  
  return NewArenaPartitionAddress;
}

inline void
ArenaZeroBlock(size_t size, void *address)
{
  u8 *byte = (u8 *)address;
  
  while(size--)
  {
    *byte++ = 0;
  }
  
  return;
}

void ArenaFreeUnused(arena *Arena)
{
  Assert(Arena->Used<Arena->Size);
  Arena->Size = Arena->Used;
  return;
}

arena ArenaTempInit(arena *Arena, arena *HostArena)
{
  arena Result = {0};
  Result.Base = ((u8 *)HostArena->Base)+HostArena->Used;
  Result.Size = HostArena->Size-HostArena->Used;
  Result.Used = 0;
  if(Arena) { *Arena = Result; }
  return Result;
}