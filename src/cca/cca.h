#ifndef CCA_H
#define CCA_H

#define CCA_MIN_TEX_RES (256)
#define CCA_MAX_TEX_RES (2048)
#define CCA_AGENTS_PER_THREADGROUP 64
#define CCA_PIXELS_PER_THREADGROUP 32
typedef struct cca_consts cca_consts;
struct16 cca_consts
{
  v2u UWinRes;
  v2u UTexRes;
  f32 UMaxStates;
  f32 UThreashold;
  s32 URange;
  s32 UOverCount;
  u32 UStepCount;
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
  ID3D11Texture2D           *TexRead;
  ID3D11ShaderResourceView  *SRViewTexRead;
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
  d3d11_shader Step;
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  arena Arena; //only textures
  cca_ui UIState;
};
cca_ui CCaUIStateInit(void)
{
  cca_ui Result =
  {
    .StepMod = 1,
    .Res = CCA_MAX_TEX_RES,
    .AutoStep = 1,
    .Threashold = 4,
    .MaxStates = 10,
    .Range = 1,
    .OverCount = 4,
  };
  return Result;
}
void CcaUI(cca_ui *Req)
{
  // SIM PARAMS
  {
    igSliderInt("MaxState", (s32 *)&Req->MaxStates, 1, 20, NULL, 0);
    igSliderInt("Threashold", (s32 *)&Req->Threashold, 1, Req->MaxStates, NULL, 0);
    igSliderInt("Search Range", (s32 *)&Req->Range, 1, 5, NULL, 0);
    igSliderInt("OverCount", (s32 *)&Req->OverCount, 1, (2*Req->Range+1)*(2*Req->Range+1), NULL, 0);
    igSpacing();
  }
  // SYS CONTROLS
  {
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->Res, CCA_MIN_TEX_RES, CCA_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
void CcaDisplayTextures(cca *Cca)
{
  ImVec2 Dim;
  igGetWindowSize(&Dim);
  igColumns(3, NULL, 0);
  igText("read");
  igImage(Cca->SRViewTexRead, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  igNextColumn();
  igText("write");
  igImage(Cca->SRViewTexWrite, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  igNextColumn();
  igText("render");
  igImage(Cca->SRViewTexRender, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  return;
}
cca CcaInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  cca Result = {0};
  Result.UIState = CCaUIStateInit();
  u64 MemSize = Gigabytes(2);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  s32 Res = CCA_MAX_TEX_RES/4;
  Result.TexRes = V2s(Res, Res);
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
  f32 *StateInitial = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, f32);
  v4f *TexelInitial = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, v4f);
  foreach(Elm, Result.TexRes.x*Result.TexRes.y, s32)
  {
    StateInitial[Elm] = 0.0f;
    TexelInitial[Elm] = V4f(0.0, 0.0, 0.0, 0.0);
  }
  D3D11ScopedBase(Base)
  {
    
    D3D11BufferVertex(&Result.VBuffer, Data, sizeof(struct vert), 6);
    D3D11Tex2DViewSR(Device, &Result.SRViewTexRead ,
                     &Result.TexRead, Result.TexRes, StateInitial, sizeof(f32), Float_R);
    D3D11Tex2D(&Result.TexWrite,
               &Result.SRViewTexWrite, &Result.UAViewTexWrite,
               Result.TexRes, StateInitial, sizeof(f32), Float_R, Usage_Default, 0);
    D3D11Tex2D(&Result.TexRender,
               &Result.SRViewTexRender, &Result.UAViewTexRender,
               Result.TexRes, TexelInitial, sizeof(v4f), Float_RGBA, Usage_Default, 0);
    D3D11Tex2DStage(&Result.TexSwapStage, Result.TexRes, StateInitial, sizeof(f32), Float_R); // Swap Stage
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(cca_consts), Usage_Dynamic, Access_Write);
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
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\cca\\cca.hlsl");
  Result.Reset = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("ResetKernel"), NULL, 0, Base);
  Result.Step  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("StepKernel"), NULL, 0, Base);
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
void CcaStep(cca *Cca, d3d11_base *Base, cca_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Cca->Step.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Cca->Consts, &Consts, sizeof(cca_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Cca->SRViewTexRead);             // Float
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->UAViewTexWrite, NULL);      // Float
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Cca->UAViewTexRender, NULL); // Float4
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Cca->TexRead, &Cca->TexWrite, Cca->TexSwapStage);
  return;
}
void CcaReset(cca *Cca, d3d11_base *Base, cca_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Cca->Reset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Cca->Consts, &Consts, sizeof(cca_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->UAViewTexWrite, NULL);      // Float
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Cca->TexRead, &Cca->TexWrite, Cca->TexSwapStage);
  return;
}
void CcaDraw(cca *Cca, d3d11_base *Base, cca_ui UIReq, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  
  // CCA PASS
  cca_consts Consts = {
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Cca->TexRes.x, (u32)Cca->TexRes.y),
    .UThreashold = (f32)UIReq.Threashold,
    .UMaxStates = (f32)UIReq.MaxStates,
    .URange = UIReq.Range,
    .UOverCount = UIReq.OverCount,
    .UStepCount = (u32)StepCount,
    .UFrameCount = (u32)FrameCount,
  };
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    CcaStep(Cca, Base, Consts);
    StepCount++;
  }
  if(UIReq.DoReset)
  {
    CcaReset(Cca, Base, Consts);
    StepCount = 0;
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
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Cca->SamTexRender);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Cca->SRViewTexRender);
  ID3D11DeviceContext_PSSetSamplers       (Context, 1, 1, &Cca->SamTexWrite);
  ID3D11DeviceContext_PSSetShaderResources(Context, 1, 1, &Cca->SRViewTexWrite);
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
