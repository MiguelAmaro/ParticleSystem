
#include "md.h"
#include "md.c"


static MD_Arena *Arena;
int main(void)
{
  Arena = MD_ArenaAlloc();
  // parse a string
  MD_String8 name = MD_S8Lit("<name>");
  MD_String8 hello_world = MD_S8Lit("hello world");
  MD_ParseResult parse = MD_ParseWholeString(Arena, name, hello_world);
  
  // print the results
  MD_PrintDebugDumpFromNode(stdout, parse.node, MD_GenerateFlags_All);
  return;
}