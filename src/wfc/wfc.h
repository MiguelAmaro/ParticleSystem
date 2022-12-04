#ifndef WFC_H
#define WFC_H

#define WFC_RESOLUTION (1000)

typedef struct wfc_consts wfc_consts;
struct16 wfc_consts
{
  v2u UWinRes;
  u32 UTime;
};
typedef struct wfc_ui wfc_ui;
struct wfc_ui
{
  u32 Res;
};
typedef struct wfc wfc;
struct wfc
{
  ID3D11Buffer *VBuffer;
  ID3D11Buffer *Consts;
  
  ID3D11Texture2D *Tex;
  ID3D11UnorderedAccessView *UAViewTex;
  ID3D11ShaderResourceView  *SRViewTex;
  ID3D11SamplerState        *TexSampler;
  
  //ID3D11Texture2D *TexWFC;
  ID3D11ShaderResourceView  *SRViewTexWFC;
  ID3D11SamplerState        *TexWFCSampler;
  
  image InputImage;
  
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  wfc_ui UIState;
  arena Arena;
};

fn wfc_ui WfcUIInit(void)
{
  wfc_ui Result = 
  {
    .Res = WFC_RESOLUTION,
  };
  return Result;
}
fn wfc WfcInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  wfc Result = {0};
  Result.UIState = WfcUIInit();
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
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(wfc_consts), Usage_Dynamic, Access_Write);
    D3D11Tex2D(&Result.Tex, &Result.SRViewTex, &Result.UAViewTex, V2s(Result.UIState.Res, Result.UIState.Res), NULL, sizeof(v4f), Float_RGBA, Usage_Default, 0);
    D3D11Tex2D(NULL, &Result.SRViewTexWFC, NULL, Result.InputImage.Dim, Result.InputImage.Data, Result.InputImage.Stride, Unorm_RGBA, Usage_Default, 0);
  }
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexSampler);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexWFCSampler);
  }
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\wfc\\wfc.hlsl");
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
fn void WfcDraw(wfc *Wfc, d3d11_base *Base, u64 FrameCount, v2u WindowDim)
{
  D3D11BaseDestructure(Base);
  
  wfc_consts Consts = 
  {
    .UWinRes = WindowDim,
    .UTime = (u32)FrameCount,
  };
  D3D11GPUMemoryWrite(Context, Wfc->Consts, &Consts, sizeof(wfc_consts), 1);
  
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Wfc->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Wfc->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Wfc->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Wfc->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Wfc->Consts);
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Wfc->TexWFCSampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Wfc->SRViewTexWFC);
  ID3D11DeviceContext_PSSetShader(Context, Wfc->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}


#endif //WFC_H
