#ifndef INSTANCING_H
#define INSTANCING_H

#define INSTANCING_MIN_TEX_RES (256)
#define INSTANCING_MAX_TEX_RES (2048)
#define INSTANCING_AGENTS_PER_THREADGROUP 64
#define INSTANCING_PIXELS_PER_THREADGROUP 32
typedef struct instancing_consts instancing_consts;
struct16 instancing_consts
{
  v2u UWinRes;
  v2u UTexRes;
  u32 UStepCount;
  u32 UFrameCount;
  f32 UBufferInit;
};
typedef struct instancing_ui instancing_ui;
struct instancing_ui
{
  s32 TexRes;
  s32 StepsPerFrame;
  s32 StepMod;
  b32 DoStep;
  b32 AutoStep;
  b32 DoReset;
};
typedef struct instancing instancing;
struct instancing
{
  ID3D11InputLayout        *Layout;
  ID3D11ShaderResourceView **SelectedTex;
  v2s TexRes;
  ID3D11Texture2D           *TexRead;
  ID3D11ShaderResourceView  *SRViewTexRead;
  ID3D11UnorderedAccessView *UAViewTexRead;
  ID3D11SamplerState        *SamTexRead;
  
  ID3D11Texture2D           *TexWrite;
  ID3D11ShaderResourceView  *SRViewTexWrite;
  ID3D11UnorderedAccessView *UAViewTexWrite;
  ID3D11SamplerState        *SamTexWrite;
  
  ID3D11Texture2D           *TexRender;
  ID3D11ShaderResourceView  *SRViewTexRender;
  ID3D11UnorderedAccessView *UAViewTexRender;
  ID3D11SamplerState        *SamTexRender;
  
  ID3D11Texture2D           *TexSwapStage;
  
  ID3D11Buffer             *VBuffer;
  ID3D11Buffer             *Consts;
  //Shader
  d3d11_shader Reset;
  d3d11_shader Render;
  d3d11_shader Instancing;
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  arena Arena; //only textures
  instancing_ui UIState;
};
instancing_ui InstancingUIStateInit(void)
{
  instancing_ui Result = 
  {
    .TexRes = INSTANCING_MAX_TEX_RES,
    .StepsPerFrame = 1,
    .StepMod = 1,
    .AutoStep = true,
    .DoStep = false,
    .DoReset = false,
  };
  return Result;
}
instancing InstancingInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  instancing Result = {0};
  u64 MemSize = Gigabytes(1);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  // NOTE(MIGUEL): In the following lines the tex resolution is determinded by UIState Initizaiton
  Result.UIState = InstancingUIStateInit();
  Result.TexRes = V2s((u32)Result.UIState.TexRes, (u32)Result.UIState.TexRes);
  struct vert { v3f Pos; v3f TexCoord; }; // NOTE(MIGUEL): update changes in draw.
  struct vert Data[6] =
  {
    //Tri A
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  0.0f, 0.0f } },
    { {  1.0f, -1.0f, 0.0f }, {  1.0f,  0.0f, 0.0f } },
    //Tri B
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  0.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f }, {  0.0f,  1.0f, 0.0f } },
  };
  arena_temp Temp = ArenaTempBegin(&Result.Arena);
  v2f*StateInitial = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, v2f);
  v4f *TexelInitial = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, v4f);
  foreach(Elm, Result.TexRes.x*Result.TexRes.y, s32)
  {
    StateInitial[Elm] = V2f(0.0, 0.0);
    TexelInitial[Elm] = V4f(0.0, 0.0, 0.0, 0.0);
  }
  D3D11ScopedBase(Base)
  {
    D3D11BufferVertex(&Result.VBuffer, Data, sizeof(struct vert), 6);
    D3D11Tex2D(&Result.TexRead,
               &Result.SRViewTexRead, &Result.UAViewTexRead,
               Result.TexRes, StateInitial, sizeof(v2f), Float_RG);
    D3D11Tex2D(&Result.TexWrite,
               &Result.SRViewTexWrite, &Result.UAViewTexWrite,
               Result.TexRes, StateInitial, sizeof(v2f), Float_RG);
    D3D11Tex2D(&Result.TexRender,
               &Result.SRViewTexRender, &Result.UAViewTexRender,
               Result.TexRes, TexelInitial, sizeof(v4f), Float_RGBA);
    D3D11Tex2DStage(&Result.TexSwapStage, Result.TexRes, StateInitial, sizeof(v2f), Float_RG); // Swap Stage
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(instancing_consts), Usage_Dynamic, Access_Write);
  }
  ArenaTempEnd(Temp);
  
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexRead);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexWrite);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexRender);
  }
  
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\instancing\\instancing.hlsl");
  Result.Reset      = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelReset"), NULL, 0, Base);
  return Result;
}
void InstancingRender(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Render.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Instancing->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Instancing->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
void InstancingStep(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Instancing.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Instancing->SRViewTexRead);             // Float
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Instancing->UAViewTexWrite, NULL);      // Float
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Instancing->UAViewTexRender, NULL); // Float4
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Instancing->TexRead, &Instancing->TexWrite, Instancing->TexSwapStage);
  return;
}
void InstancingReset(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  
  ConsoleLog("reseting");
  Consts.UBufferInit = 1.0f;
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Reset.ComputeHandle, NULL, 0);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Instancing->UAViewTexRead, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  Consts.UBufferInit = 0.0f;
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Reset.ComputeHandle, NULL, 0);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Instancing->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  
  //D3D11Tex2DSwap(Context, &Instancing->TexRead, &Instancing->TexWrite, Instancing->TexSwapStage);
  return;
}
void InstancingDraw(instancing *Instancing, d3d11_base *Base, instancing_ui UIReq, u64 FrameCount, v2u WinRes)
{
  OSProfileStart();
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  // INSTANCING PASS
  instancing_consts Consts = {
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Instancing->TexRes.x, (u32)Instancing->TexRes.y),
    .UStepCount = (u32)StepCount,
    .UFrameCount = (u32)FrameCount,
  };
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    foreach(Step, UIReq.StepsPerFrame, s32)
    {
      InstancingStep(Instancing, Base, Consts);
    }
    InstancingRender(Instancing, Base, Consts);
    StepCount++;
  }
  if(UIReq.DoReset)
  {
    InstancingReset(Instancing, Base, Consts);
    StepCount = 0;
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Instancing->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Instancing->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Instancing->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Instancing->SamTexRender);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Instancing->SRViewTexRender);
  ID3D11DeviceContext_PSSetSamplers       (Context, 1, 1, &Instancing->SamTexWrite);
  ID3D11DeviceContext_PSSetShaderResources(Context, 1, 1, &Instancing->SRViewTexWrite);
  ID3D11DeviceContext_PSSetShader(Context, Instancing->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  OSProfileEnd();
  return;
}


#endif //INSTANCING_H
