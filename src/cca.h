#ifndef CCA_H
#define CCA_H

#define CCA_MIN_TEX_RES (256)
#define CCA_MAX_TEX_RES (2048)

typedef struct cca_consts cca_consts;
struct16 cca_consts
{
  v2u UWinRes;
  v2u UTexRes;
  f32 UMaxStates;
  f32 UThreashold;
  s32 URange;
  s32 UOverCount;
  u32 UFrameCount;
};
typedef struct cca_ui cca_ui;
struct cca_ui
{
  s32 Res;
  s32 StepsPerFrame;
  s32 StepMod;
  b32 DoStep;
  b32 AutoStep;
  b32 DoReset;
  s32 MaxStates;
  s32 Threashold;
  s32 Range;
  s32 OverCount;
};
typedef struct cca cca;
struct cca
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
  
  ID3D11Buffer             *VBuffer;
  ID3D11Buffer             *Consts;
  //Shader
  d3d11_shader Reset;
  d3d11_shader Step;
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  arena Arena; //only textures
};

void TexWriteCheckered(void *Data, v2s TexDim)
{
  Assert(TexDim.x == TexDim.y);
  s32 Width  = TexDim.x;
  s32 Height = TexDim.y;
  u32 BlockSize = 2;
  foreach(x, (s32)(Width/BlockSize), u32)
  {
    foreach(y, (s32)(Height/BlockSize), u32)
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
cca CcaInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  cca Result = {0};
  u64 MemSize = Gigabytes(2);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  Result.TexRes = V2s(CCA_MIN_TEX_RES, CCA_MIN_TEX_RES);
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
  foreach(Float, Result.TexRes.x*Result.TexRes.y, s32) DState[Float] = V4f(0.0, 0.0, 0.0, 0.0);
  //TexWriteCheckered(RState, Result.TexRes);
  //TexWriteCheckered(WState, Result.TexRes);
  D3D11VertexBuffer(Device, &Result.VBuffer, Data, sizeof(struct vert), 6);
  //State Views
  D3D11Tex2DViewSR(Device, &Result.StateViewR , &Result.StateTexA, Result.TexRes, RState, sizeof(f32), Float_R);
  D3D11Tex2DViewUA(Device, &Result.StateViewRW, &Result.StateTexB, Result.TexRes, WState, sizeof(f32), Float_R);
  D3D11Tex2DStage(Device, &Result.StateStage, Result.TexRes, SState, sizeof(f32), Float_R); // Swap Stage
  D3D11ConstantBuffer(Device, &Result.Consts, NULL, sizeof(cca_consts), Usage_Dynamic, Access_Write);
  //Render Views
  D3D11Tex2DViewSRAndUA(Device, NULL, &Result.StateViewRenderP, &Result.StateViewRenderC, Result.TexRes, DState, sizeof(v4f), Float_RGBA);
  Result.SelectedTex = &Result.StateViewRenderP;
  ArenaTempEnd(Temp);
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.Sampler);
  }
  
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\cca.hlsl");
  Result.Reset = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("ResetKernel"), NULL, 0, Base);
  Result.Step  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("StepKernel"), NULL, 0, Base);
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
void CcaDraw(cca *Cca, d3d11_base *Base, cca_ui UIReq, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  // CCA PASS
  cca_consts CcaConsts = {
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Cca->TexRes.x, (u32)Cca->TexRes.y),
    .UThreashold = (f32)UIReq.Threashold,
    .UMaxStates = (f32)UIReq.MaxStates,
    .URange = UIReq.Range,
    .UOverCount = UIReq.OverCount,
    .UFrameCount = (u32)FrameCount,
  };
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    ID3D11DeviceContext_CSSetShader(Context, Cca->Step.ComputeHandle, NULL, 0);
    D3D11GPUMemoryOp(Context, Cca->Consts, &CcaConsts, sizeof(cca_consts), 1, GPU_MEM_WRITE);
    ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
    ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Cca->StateViewR);             // Float
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->StateViewRW, NULL);      // Float
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Cca->StateViewRenderC, NULL); // Float4
    ID3D11DeviceContext_Dispatch(Context, Cca->TexRes.x, Cca->TexRes.y, 1);
    D3D11ClearComputeStage(Context);
    D3D11Tex2DSwap(Context, &Cca->StateTexA, &Cca->StateTexB, Cca->StateStage);
  }
  if(UIReq.DoReset)
  {
    ID3D11DeviceContext_CSSetShader(Context, Cca->Reset.ComputeHandle, NULL, 0);
    D3D11GPUMemoryOp(Context, Cca->Consts, &CcaConsts, sizeof(cca_consts), 1, GPU_MEM_WRITE);
    ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->StateViewRW, NULL);      // Float
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Cca->StateViewRenderC, NULL); // Float4
    ID3D11DeviceContext_Dispatch(Context, Cca->TexRes.x, Cca->TexRes.y, 1);
    D3D11ClearComputeStage(Context);
    D3D11Tex2DSwap(Context, &Cca->StateTexA, &Cca->StateTexB, Cca->StateStage);
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Cca->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Cca->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Cca->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &Cca->Sampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Cca->StateViewRenderP);
  ID3D11DeviceContext_PSSetShader(Context, Cca->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}


#endif //CCA_H
