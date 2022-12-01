#ifndef EOC_H
#define EOC_H

#define EOC_TEX_MAX_RES (1024)
#define EOC_TEX_MIN_RES (8)
#define EOC_STATE_MAX_COUNT 15
typedef struct eoc_consts eoc_consts;
struct16 eoc_consts
{
  v2u UWinRes;
  v2u UTexRes;
  u32 UTime;
  u32 UStepCount;
  s32 UStateCount;
};
typedef struct eoc_ui eoc_ui;
struct eoc_ui
{
  s32 TexRes;
  u32 StepsPerFrame;
  u32 StepMod;
  b32 AutoStep;
  b32 DoStep;
  b32 DoReset;
  f32 Lambda;
  s32 StateCount;
};
typedef struct eoc eoc;
struct eoc
{
  ID3D11Buffer *VBuffer;
  ID3D11Buffer *Consts;
  
  v2u TexRes;
  ID3D11Texture2D *TexRender;
  ID3D11UnorderedAccessView *UAViewTexRender;
  ID3D11ShaderResourceView  *SRViewTexRender;
  ID3D11SamplerState        *SamTexRender;
  
  ID3D11Texture2D *TexState;
  ID3D11UnorderedAccessView *UAViewTexState;
  ID3D11ShaderResourceView  *SRViewTexState;
  ID3D11SamplerState        *SamTexState;
  ID3D11Texture2D *TexStateCopy;
  ID3D11UnorderedAccessView *UAViewTexStateCopy;
  ID3D11ShaderResourceView  *SRViewTexStateCopy;
  ID3D11SamplerState        *SamTexStateCopy;
  ID3D11Texture2D *TexStateStage;
  
  ID3D11Buffer *LUT;
  ID3D11UnorderedAccessView *UAViewLUT;
  s32 *StateTransitionLUT;
  u32 StateTransitionLUTLength;
  
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  d3d11_shader Reset;
  d3d11_shader Render;
  d3d11_shader Step;
  d3d11_shader CopyDown;
  eoc_ui UIState;
  arena Arena;
};

fn eoc_ui EocUIInit(void)
{
  eoc_ui Result = 
  {
    .TexRes = EOC_TEX_MAX_RES,
    .StepsPerFrame = 1,
    .StepMod = 1,
    .AutoStep = 1,
    .DoStep = 0,
    .DoReset = 0,
    .Lambda = 0.6563,
    .StateCount = 5,
  };
  return Result;
}
fn eoc EocInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  eoc Result = {0};
  Result.UIState = EocUIInit();
  u64 MemSize = Gigabytes(1);
  Result.TexRes = V2u(Result.UIState.TexRes, Result.UIState.TexRes);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
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
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(eoc_consts), Usage_Dynamic, Access_Write);
    D3D11BufferStructUA(&Result.LUT, Result.StateTransitionLUT, sizeof(s32), Result.StateTransitionLUTLength = 1);
    D3D11BufferViewUA(Device, &Result.UAViewLUT, Result.LUT, Result.StateTransitionLUTLength = 1);
    D3D11Tex2D(&Result.TexRender, &Result.SRViewTexRender, &Result.UAViewTexRender,
               V2s(Result.UIState.TexRes, Result.UIState.TexRes), NULL, sizeof(v4f), Float_RGBA);
    D3D11Tex2D(&Result.TexState, &Result.SRViewTexState, &Result.UAViewTexState,
               V2s(Result.UIState.TexRes, Result.UIState.TexRes), NULL, sizeof(s32), Sint_R);
    D3D11Tex2D(&Result.TexStateCopy, &Result.SRViewTexStateCopy, &Result.UAViewTexStateCopy,
               V2s(Result.UIState.TexRes, Result.UIState.TexRes), NULL, sizeof(s32), Sint_R);
    D3D11Tex2DStage(&Result.TexStateStage, V2s(Result.UIState.TexRes, Result.UIState.TexRes),
                    NULL, sizeof(s32), Sint_R);
  }
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexState);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexRender);
  }
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\eoc\\eoc.hlsl");
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  Result.Render  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelRender"), NULL, 0, Base);
  Result.Reset  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelReset"), NULL, 0, Base);
  Result.Step  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelStep"), NULL, 0, Base);
  Result.CopyDown  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelCopyDown"), NULL, 0, Base);
  return Result;
}
fn void EocTransitionLUTInit(eoc *Eoc, s32 Seed)
{
  if(Eoc->StateTransitionLUT == NULL)
  {
    Eoc->StateTransitionLUTLength = EOC_STATE_MAX_COUNT*EOC_STATE_MAX_COUNT*EOC_STATE_MAX_COUNT; //left self right: 3;
    Eoc->StateTransitionLUT = ArenaPushArray(&Eoc->Arena, Eoc->StateTransitionLUTLength, s32);
  }
  s32 *LUT = Eoc->StateTransitionLUT;
  u32 LUTLength = Eoc->StateTransitionLUTLength;
  foreach(LUTId, LUTLength, s64) LUT[LUTId] = -1; 
  arena_temp Scratch = MemoryGetScratch(NULL, 0);
  s32 StateCount = Eoc->UIState.StateCount;
  f32 Lambda     = Eoc->UIState.Lambda;
  ConsoleLog(*Scratch.Arena, "Initializing Tabel: %d\n", StateCount);
  foreach(l, StateCount, s32) {
    foreach(s, StateCount, s32) {
      foreach(r, StateCount, s32)
      {
        int i  = l * EOC_STATE_MAX_COUNT*EOC_STATE_MAX_COUNT + s*EOC_STATE_MAX_COUNT + r;
        s32 State;
        if(RandUniLat() < Lambda) State = 0;
        else State = (s32)RandRange(1, StateCount);
        LUT[i] = State;
        ConsoleLog(*Scratch.Arena, "State (%d,%d,%d): %d\n", l,s,r,State);
      }
    }
  }
  MemoryReleaseScratch(Scratch);
  return;
}
fn void EocStep(eoc *Eoc, d3d11_base *Base, eoc_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Eoc->Step.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Eoc->Consts, &Consts, sizeof(eoc_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Eoc->UAViewTexState, NULL);  // Float
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Eoc->UAViewTexRender, NULL); // Float4
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Eoc->UAViewLUT, NULL); // Float4
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void EocCopyDown(eoc *Eoc, d3d11_base *Base, eoc_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Eoc->CopyDown.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Eoc->Consts, &Consts, sizeof(eoc_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Eoc->UAViewTexState, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Eoc->UAViewTexStateCopy, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Eoc->TexState, &Eoc->TexStateCopy, Eoc->TexStateStage);
  return;
}
fn void EocReset(eoc *Eoc, d3d11_base *Base, eoc_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  EocTransitionLUTInit(Eoc, 0); //no seed
  D3D11ScopedBase(Base)
  {
    if(Eoc->LUT) ID3D11Buffer_Release(Eoc->LUT);
    if(Eoc->UAViewLUT) ID3D11UnorderedAccessView_Release(Eoc->UAViewLUT);
    D3D11BufferStructUA(&Eoc->LUT, Eoc->StateTransitionLUT, sizeof(s32), Eoc->StateTransitionLUTLength);
    D3D11BufferViewUA(Device, &Eoc->UAViewLUT, Eoc->LUT, Eoc->StateTransitionLUTLength);
  }
  ConsoleLog("reseting\n");
  D3D11GPUMemoryWrite(Context, Eoc->Consts, &Consts, sizeof(eoc_consts), 1);
  ID3D11DeviceContext_CSSetShader(Context, Eoc->Reset.ComputeHandle, NULL, 0);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Eoc->UAViewTexState, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void EocRender(eoc *Eoc, d3d11_base *Base, eoc_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Eoc->Render.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Eoc->Consts, &Consts, sizeof(eoc_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Eoc->UAViewTexState, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Eoc->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void EocDraw(eoc *Eoc, d3d11_base *Base, u64 FrameCount, v2u WindowDim)
{
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  eoc_ui UIReq = Eoc->UIState;
  eoc_consts Consts = 
  {
    .UWinRes = WindowDim,
    .UTexRes = Eoc->TexRes,
    .UTime = (u32)FrameCount,
    .UStepCount = StepCount,
    .UStateCount = UIReq.StateCount,
  };
  D3D11GPUMemoryWrite(Context, Eoc->Consts, &Consts, sizeof(eoc_consts), 1);
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    foreach(Step, UIReq.StepsPerFrame, u32)
    {
      EocCopyDown(Eoc, Base, Consts);
      EocStep(Eoc, Base, Consts);
      StepCount++;
    }
    EocRender(Eoc, Base, Consts);
  }
  if(UIReq.DoReset)
  {
    EocReset(Eoc, Base, Consts);
    EocRender(Eoc, Base, Consts);
    StepCount = 0;
  }
  
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Eoc->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Eoc->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Eoc->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Eoc->Consts);
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Eoc->SamTexRender);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Eoc->SRViewTexRender);
  ID3D11DeviceContext_PSSetShader(Context, Eoc->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}



#endif //EOC_H
