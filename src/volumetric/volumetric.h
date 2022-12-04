#ifndef VOLUMETRIC_H
#define VOLUMETRIC_H
#define VOLUMETRIC_RESOLUTION (1000)

typedef struct volumetric_consts volumetric_consts;
struct16 volumetric_consts
{
  v2u UWinRes;
  u32 UTime;
};
typedef struct volumetric_ui volumetric_ui;
struct volumetric_ui
{
  u32 Res;
};
typedef struct volumetric volumetric;
struct volumetric
{
  ID3D11Buffer *VBuffer;
  ID3D11Buffer *Consts;
  
  ID3D11Texture2D *Tex;
  ID3D11UnorderedAccessView *UAViewTex;
  ID3D11ShaderResourceView  *SRViewTex;
  ID3D11SamplerState        *TexSampler;
  
  //ID3D11Texture2D *TexVOLUMETRIC;
  ID3D11ShaderResourceView  *SRViewTexVOLUMETRIC;
  ID3D11SamplerState        *TexVOLUMETRICSampler;
  
  image InputImage;
  
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  volumetric_ui UIState;
  arena Arena;
};

fn volumetric_ui VolumetricUIInit(void)
{
  volumetric_ui Result = 
  {
    .Res = VOLUMETRIC_RESOLUTION,
  };
  return Result;
}
fn volumetric VolumetricInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  volumetric Result = {0};
  Result.UIState = VolumetricUIInit();
  u64 MemSize = Gigabytes(1);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  Result.InputImage = ImageLoad(Str8("F:\\Dev\\ParticleSystem\\res\\lake.png"), &Result.Arena);
  struct vert { v3f Pos; v3f TexCoord; }; // NOTE(MIGUEL): update changes in draw.
  struct vert QuadVerts[6] =
  {
    //Tri A
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  0.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  1.0f, 0.0f } },
    { {  1.0f, -1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    //Tri B
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  0.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  1.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f }, {  0.0f,  0.0f, 0.0f } },
  };
  D3D11ScopedBase(Base)
  {
    D3D11BufferVertex(&Result.VBuffer, QuadVerts, sizeof(struct vert), 6);
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(volumetric_consts), Usage_Dynamic, Access_Write);
    D3D11Tex2D(&Result.Tex, &Result.SRViewTex, &Result.UAViewTex, V2s(Result.UIState.Res, Result.UIState.Res), NULL, sizeof(v4f), Float_RGBA, Usage_Default, 0);
    D3D11Tex2D(NULL, &Result.SRViewTexVOLUMETRIC, NULL, Result.InputImage.Dim, Result.InputImage.Data, Result.InputImage.Stride, Unorm_RGBA, Usage_Default, 0);
  }
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexSampler);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexVOLUMETRICSampler);
  }
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\volumetric\\volumetric.hlsl");
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
fn void VolumetricDraw(volumetric *Volumetric, d3d11_base *Base, u64 FrameCount, v2u WindowDim)
{
  D3D11BaseDestructure(Base);
  
  volumetric_consts Consts = 
  {
    .UWinRes = WindowDim,
    .UTime = (u32)FrameCount,
  };
  D3D11GPUMemoryWrite(Context, Volumetric->Consts, &Consts, sizeof(volumetric_consts), 1);
  
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Volumetric->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Volumetric->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Volumetric->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Volumetric->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Volumetric->Consts);
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Volumetric->TexVOLUMETRICSampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Volumetric->SRViewTexVOLUMETRIC);
  ID3D11DeviceContext_PSSetShader(Context, Volumetric->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}



#endif //VOLUMETRIC_H
