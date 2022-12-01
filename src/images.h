#ifndef IMAGES_H
#define IMAGES_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct image image;
struct image
{
  v2s Dim;
  s32 Stride;
  u8 *Data;
};
image ImageLoad(str8 Path, arena *Arena)
{
  image Result = {0};
  const char *FilePath = (const char *)Path.Data;
  u32 Status = stbi_info(FilePath, &Result.Dim.x, &Result.Dim.y, &Result.Stride);
  Assert(Status && "File not supported!!!");
  u8 *LoadedData = stbi_load(FilePath, &Result.Dim.x, &Result.Dim.y, &Result.Stride, 0);
  u64 ImageSize = Result.Dim.x*Result.Dim.y*Result.Stride;
  Result.Data = ArenaPushBlock(Arena, ImageSize);
  MemoryCopy(LoadedData, ImageSize, Result.Data, ImageSize);
  stbi_image_free(LoadedData);
  return Result;
}

#endif //IMAGES_H
