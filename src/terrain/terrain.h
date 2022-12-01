#ifndef TERRAIN_H
#define TERRAIN_H

typedef struct terrain_consts terrain_consts;
struct terrain_consts
{
  v2s UWinRes;
  u32 UTime;
};
typedef struct terrain_ui terrain_ui;
struct terrain_ui
{
  f32 Scale;
};
typedef struct terrain terrain;
struct terrain
{
  ID3D11Buffer *VBuffer; //Mesh
  ID3D11Buffer *Consts;
  
  ID3D11Texture2D *Tex;
  ID3D11UnorderedAccessView *UAViewTex;
  ID3D11ShaderResourceView  *SRViewTex;
  ID3D11SamplerState        *TexSampler;
  
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  wfc_ui UIState;
  arena Arena;
};
fn terrain_ui TerrainInitUIState(void)
{
  terrain_ui Result; 
  return Result;
}
fn terrain TerrainInit(void)
{
  terrain Result = {0};
  return Result;
}
fn void TerrainDraw(terrain Terrain, d3d11_base *Base, u64 FrameCount, v2s WindowDim)
{
  return;
}

#endif //TERRAIN_H
