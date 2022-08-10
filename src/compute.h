#ifndef COMPUTE_H
#define COMPUTE_H


#define COMPUTE_MIN_TEX_RES (8)
#define COMPUTE_MAX_TEX_RES (2048)

typedef struct compute compute;
struct compute
{
  ID3D11InputLayout        *Layout;
  ID3D11ShaderResourceView **SelectedTex;
  v2s TexRes;
  ID3D11Texture2D           *StateTexA;
  ID3D11Texture2D           *StateTexB;
  ID3D11ShaderResourceView  *StateViewR; //State Read float
  ID3D11UnorderedAccessView *StateViewRW; //State Write flaot 
  ID3D11Texture2D           *StateStage;  //Staging for Tex swap
  ID3D11UnorderedAccessView *StateViewRenderC; //State Display float4 for rendering
  ID3D11ShaderResourceView  *StateViewRenderP; //State Read float
  ID3D11SamplerState       *Sampler;
  //
  ID3D11ComputeShader      *CShaderReset;
  ID3D11ComputeShader      *CShaderStep;
  ID3D11VertexShader       *VShader;
  ID3D11PixelShader        *PShader;
  ID3D11Buffer             *VBuffer;
  ID3D11Buffer             *Consts;
  arena Arena; //only textures
};

void TexWriteCheckered(void *Data, v2s TexDim)
{
  Assert(TexDim.x == TexDim.y);
  s32 Width  = TexDim.x;
  s32 Height = TexDim.y;
  u32 BlockSize = 2;
  foreach(x, (s32)(Width/BlockSize))
  {
    foreach(y, (s32)(Height/BlockSize))
    {
      f32 *Block = Data;
      u32 Main = y*BlockSize*(Width) + x*BlockSize;
      u32 OffTL = Main;
      u32 OffTR = Main + 1;
      u32 OffBL = Main + Width;
      u32 OffBR = Main + Width + 1;
      Block[OffTL] = 1.0;
      Block[OffTR] = 0.0;
      Block[OffBL] = 1.0;
      Block[OffBR] = 0.0;
    }
  }
  return;
}

compute ComputeInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  compute Result = {0};
  u64 MemSize = Gigabytes(2);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  Result.TexRes = V2s(COMPUTE_MIN_TEX_RES, COMPUTE_MIN_TEX_RES);
  struct vert { v3f Pos; v3f TexCoord; }; // NOTE(MIGUEL): update changes in draw.
  struct vert Data[6] =
  {
    //Tri A
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f } },
    { {  1.0f, -1.0f, 0.0f }, {  1.0f, -1.0f, 0.0f } },
    //Tri B
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f }, { -1.0f,  1.0f, 0.0f } },
  };
  arena_temp Temp = ArenaTempBegin(&Result.Arena);
  f32 *RState = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, f32);
  f32 *WState = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, f32);
  f32 *SState = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, f32);
  v4f *DState = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, v4f);
  TexWriteCheckered(RState, Result.TexRes);
  TexWriteCheckered(WState, Result.TexRes);
  D3D11VertexBuffer(Device, &Result.VBuffer, Data, sizeof(struct vert), 6);
  //State Views
  D3D11Tex2DViewSR(Device, &Result.StateViewR , &Result.StateTexA, Result.TexRes, RState, sizeof(f32), Float_R);
  D3D11Tex2DViewUA(Device, &Result.StateViewRW, &Result.StateTexB, Result.TexRes, WState, sizeof(f32), Float_R);
  D3D11Tex2DStage(Device, &Result.StateStage, Result.TexRes, SState, sizeof(f32), Float_R); // Swap Stage
  foreach(Float, Result.TexRes.x*Result.TexRes.y)
  {
    DState[Float] = V4f(0.0, 0.0, 0.0, 0.0);
  }
  //Render Views
  D3D11Tex2DViewSRAndUA(Device, &Result.StateViewRenderP, &Result.StateViewRenderC, Result.TexRes, DState, sizeof(v4f), Float_RGBA);
  Result.SelectedTex = &Result.StateViewRenderP;
  ArenaTempEnd(Temp);
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.Sampler);
  }
  ID3DBlob* CShaderBlob0 = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\compute.hlsl", "ResetKernel", "cs_5_0", "Compute System");
  ID3DBlob* CShaderBlob1 = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\compute.hlsl", "StepKernel", "cs_5_0", "Compute System");
  ID3DBlob* VShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\compute.hlsl", "VSMain", "vs_5_0", "Compute System");
  ID3DBlob* PShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\compute.hlsl", "PSMain", "ps_5_0", "Compute System");
  {
    D3D11_INPUT_ELEMENT_DESC Desc[] =
    {
      { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ID3D11Device_CreateInputLayout(Device, Desc, ARRAYSIZE(Desc), ID3D10Blob_GetBufferPointer(VShaderBlob), ID3D10Blob_GetBufferSize(VShaderBlob), &Result.Layout);
  }
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(CShaderBlob0), ID3D10Blob_GetBufferSize(CShaderBlob0), NULL, &Result.CShaderReset);
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(CShaderBlob1), ID3D10Blob_GetBufferSize(CShaderBlob1), NULL, &Result.CShaderStep);
  ID3D11Device_CreateVertexShader(Device, ID3D10Blob_GetBufferPointer(VShaderBlob), ID3D10Blob_GetBufferSize(VShaderBlob), NULL, &Result.VShader);
  ID3D11Device_CreatePixelShader(Device, ID3D10Blob_GetBufferPointer(PShaderBlob), ID3D10Blob_GetBufferSize(PShaderBlob), NULL, &Result.PShader);
  ID3D10Blob_Release(CShaderBlob0);
  ID3D10Blob_Release(CShaderBlob1);
  ID3D10Blob_Release(VShaderBlob);
  ID3D10Blob_Release(PShaderBlob);
  return Result;
}
void ComputeDraw(compute *Compute, d3d11_base *Base, b32 DoStep, b32 DoReset)
{
  D3D11BaseDestructure(Base);
  // COMPUTE PASS
  if(DoStep)
  {
    ID3D11DeviceContext_CSSetShader(Context, Compute->CShaderStep, NULL, 0);
    ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Compute->StateViewR);             // Float
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Compute->StateViewRW, NULL);      // Float
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Compute->StateViewRenderC, NULL); // Float4
    ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
    D3D11ClearComputeStage(Context);
    D3D11SwapTexture(Context, Compute->StateOld, Compute->StateNew, Compute->StateStage);
  }
  if(DoReset)
  {
    ID3D11DeviceContext_CSSetShader(Context, Compute->CShaderStep, NULL, 0);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Compute->StateViewRW, NULL);      // Float
    ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
    D3D11ClearComputeStage(Context);
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Compute->Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Compute->VBuffer, &Stride, &Offset);
  // Vertex Shader
  //ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &MMRender->ubuffer);
  ID3D11DeviceContext_VSSetShader(Context, Compute->VShader, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &Compute->Sampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Compute->StateViewRenderP);
  ID3D11DeviceContext_PSSetShader(Context, Compute->PShader, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}


#endif //COMPUTE_H
