#ifndef SORT_H
#define SORT_H

typedef b32 sort_comparator(void *a, void *b);

// TODO(MIGUEL): This needs to be tested
void MergeSortItrnl(u8 *a, u8 *b, u64 Count, u64 TypeSize, sort_comparator Comparator)	
{
  if(Count <= 1) return;
  u64 CountL = Count/2;
  u64 CountR = Count-CountL;
  u64 RightOffset = CountL*TypeSize;
  MergeSortItrnl(a, b, CountL, TypeSize, Comparator);
  MergeSortItrnl(a + RightOffset, b + RightOffset, CountR, TypeSize, Comparator);
  // NOTE(MIGUEL): Pointer Arithamatic: Increment by (typesize * count). No de/increment ops(++ --)
  //               Assignment: Memory copy of typesize
  u8 *Middle = (u8 *)b + CountL*TypeSize;
  u8 *End    = (u8 *)b + Count*TypeSize;
  u8 *l = b;
  u8 *r = b + CountL*TypeSize;
  u8 *Out = a;
  foreach(Elm, Count, u64)
  {
    b32 _a = (l < Middle);
    b32 _b = (r >= End);
    b32 _c = Comparator(r, l);
    if(_a && (_b || _c))
    {
      MemoryCopy(l, TypeSize, Out, TypeSize);
      l += TypeSize; Out += TypeSize;
    }
    else
    {
      MemoryCopy(r, TypeSize, Out, TypeSize);
      r += TypeSize; Out += TypeSize;
    }
  }
  MemoryCopy(a, TypeSize*Count, b, TypeSize*Count);
  return;
}
void MergeSortInitItrnl(void *a, u64 Count, u64 TypeSize, sort_comparator Comparator, 
                        arena **ArenaToIgnore, u32 IgnoreCount)
{
  //OSProfileStart();
  u64 BlockSize = Count*TypeSize;
  arena_temp Scratch = MemoryGetScratch(ArenaToIgnore, IgnoreCount);
  void *b = ArenaPushBlock(Scratch.Arena, Count*TypeSize);
  Assert(b);
  MemoryCopy(a, BlockSize, b, BlockSize);
  ga = a;
  gb = b;// TODO(MIGUEL): get rid of this
  //OSProfileLinesStart("MergeSortItrnl");
  MergeSortItrnl((u8 *)a, (u8 *)b, Count, TypeSize, Comparator);
  //OSProfileEnd();
  MemoryReleaseScratch(Scratch);
  //OSProfileEnd();
  return;
}

#define SortParamAsRef(type, var, param); type *var = (type *)param
#define MergeSort(array, count, type, comparator, arena_to_ignore, ignore_count) MergeSortInitItrnl(array, count, sizeof(type), comparator, arena_to_ignore, ignore_count)

#endif //SORT_H
