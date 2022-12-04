#ifndef INSTANCING_H
#define INSTANCING_H

#define INSTANCING_TEX_MIN_RES (8)
#define INSTANCING_TEX_MAX_RES (1024)
#define INSTANCING_AGENTS_PER_THREADGROUP 64
#define INSTANCING_PIXELS_PER_THREADGROUP 32
typedef struct instancing_consts instancing_consts;
struct16 instancing_consts
{
  m4f UProj; 
  m4f UModel; 
  v2u UWinRes;
  v2u UTexRes;
  u32 UFrameCount;
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
  v2u TexRes;
  ID3D11InputLayout        *Layout;
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
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  d3d11_shader Reset;
  d3d11_shader Step;
  d3d11_shader Render;
  
  v3f Pos;
  v3f Dim;
  v3f Rot;
  f32 Scale;
  m4f Proj;
  m4f Model;
  
  arena Arena; //only textures
  instancing_ui UIState;
};
fn instancing_ui InstancingUIStateInit(void)
{
  instancing_ui Result = 
  {
    .TexRes = INSTANCING_TEX_MAX_RES/2,
    .StepsPerFrame = 1,
    .StepMod = 1,
    .AutoStep = true,
    .DoStep = false,
    .DoReset = false,
  };
  return Result;
}
fn instancing InstancingInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  instancing Result = {0};
  u64 MemSize = Gigabytes(1);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  // NOTE(MIGUEL): In the following lines the tex resolution is determinded by UIState Initizaiton
  Result.UIState = InstancingUIStateInit();
  Result.TexRes = V2u(Result.UIState.TexRes, Result.UIState.TexRes);
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
  D3D11ScopedBase(Base)
  {
    v2s TexRes = V2s(Result.UIState.TexRes, Result.UIState.TexRes);
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(instancing_consts), Usage_Dynamic, Access_Write);
    D3D11BufferVertex(&Result.VBuffer, Data, sizeof(struct vert), 6);
    D3D11Tex2D(&Result.TexRead, &Result.SRViewTexRead, &Result.UAViewTexRead,
               TexRes, NULL, sizeof(s32), Sint_R, Usage_Default, 0);
    D3D11Tex2D(&Result.TexWrite, &Result.SRViewTexWrite, &Result.UAViewTexWrite,
               TexRes, NULL, sizeof(s32), Sint_R, Usage_Default, 0);
    D3D11Tex2DStage(&Result.TexSwapStage, TexRes, NULL, sizeof(s32), Sint_R); // Swap Stage
    D3D11Tex2D(&Result.TexRender,
               &Result.SRViewTexRender, &Result.UAViewTexRender,
               TexRes, NULL, sizeof(v4f), Float_RGBA, Usage_Default, 0);
  }
  // SAMPLERS
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
  Result.Vertex      = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel      = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  Result.Reset      = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelReset"), NULL, 0, Base);
  Result.Step      = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelStep"), NULL, 0, Base);
  Result.Render      = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelRender"), NULL, 0, Base);
  return Result;
}
fn void Logmat(m4f m, char *str, arena *Arena)
{
  ConsoleLog(*Arena, "\n %s", str);
  foreach(i, 4, s32)
  {
    ConsoleLog("\n");
    foreach(j, 4, s32)
    {
      ConsoleLog(*Arena, "%f ,", m.x[i][j]);
    }
  }
}
fn void InstancingUpdate(instancing *Instancing, u64 FrameCount, v2u WinRes)
{
  ConsoleLog("\n\n/////////////////NEW//////////////////////");
  arena_temp Scratch = MemoryGetScratch(NULL,0);
  f32 t = (f32)FrameCount*0.05f;
  m4f S = Scalem4f(2.0f, 2.0f, 2.0f);
  m4f R = M4fRotate(0.0f, 0.0f, 0.0f);
  m4f T = M4fTranslate(V3f(0.0f, 0.0f, 0.0f));
  m4f Model = Mul(Mul(R, S), T);
  m4f Proj = M4fPerspective(0.0f, 0.0f, (f32)WinRes.y, (f32)WinRes.x, 1.0f, 200.0f);
  Instancing->Proj = Proj;
  Instancing->Model = Model;
  v4f p = V4f(1.0f, 0.0f, 0.0f, 1.0);
  {
    Logmat(S, "scale", Scratch.Arena);
    Logmat(R, "rot", Scratch.Arena);
    Logmat(T, "translate", Scratch.Arena);
    Logmat(Model, "world", Scratch.Arena); 
    Logmat(Proj, "proj", Scratch.Arena);
    //Logmat(Instancing->Transform, "transfrom", Scratch.Arena);
    //p = Mulm4f_v4f(Instancing->Transform, p);
    //p = Mulm4f_v4f(S, p); //test: scale works s
    //p = Mulm4f_v4f(T, p); //test: translation works
    ConsoleLog(*Scratch.Arena, "\np: (%.4f, %.4f, %.4f, %.4f)", p.x, p.y, p.z, p.w);
    p = Mulm4f_v4f(Mul(S, T), p); //test: translation works
    Logmat(Mul(S, T), "t*s", Scratch.Arena);
    ConsoleLog(*Scratch.Arena, "\np: (%.4f, %.4f, %.4f, %.4f)", p.x, p.y, p.z, p.w);
  }
  MemoryReleaseScratch(Scratch );
  
  return;
}
fn void InstancingReset(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/INSTANCING_PIXELS_PER_THREADGROUP);
  ConsoleLog("reseting\n");
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Reset.ComputeHandle, NULL, 0);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Instancing->UAViewTexRead, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void InstancingStep(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/INSTANCING_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Step.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Instancing->SRViewTexRead);       
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Instancing->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Instancing->TexRead, &Instancing->TexWrite, Instancing->TexSwapStage);
  return;
}
fn void InstancingRender(instancing *Instancing, d3d11_base *Base, instancing_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/INSTANCING_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Instancing->Render.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Instancing->Consts, &Consts, sizeof(instancing_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Instancing->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Instancing->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Instancing->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void InstancingDraw(instancing *Instancing, d3d11_base *Base, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  // INSTANCING PASS
  instancing_consts Consts = {
    .UProj = Instancing->Proj,
    .UModel = Instancing->Model,
    .UWinRes = WinRes,
    .UTexRes = Instancing->TexRes,
    .UFrameCount = (u32)FrameCount,
  };
  instancing_ui UIReq = Instancing->UIState;
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
  ID3D11DeviceContext_PSSetShader(Context, Instancing->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}


#endif //INSTANCING_H
