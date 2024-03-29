#ifndef BOIDS_H
#define BOIDS_H
#define BOIDS_MAX_AGENTCOUNT (1000000)
#define BOIDS_MIN_TEX_RES (256)
#define BOIDS_MAX_TEX_RES (2048)

typedef struct boids_agent boids_agent;
struct boids_agent
{
  v2f Pos;
  v2f Vel;
  float  MaxSpeed;
  float  MaxForce;
};

/*
A modificatio to the shaders uniform/constants means:
for each changed var
1.changing it in the shader
2.changing the init UI Init code 
3.changing the Dearim gui interface
4.changint the IMgui interface struct 
5.changint the d3d11 const buffer
6.changing the asignment for uistate to const buffer vars
*/
typedef struct boids_consts boids_consts;
struct16 boids_consts
{
  v2u UWinRes;
  v2u UTexRes;
  u32 UAgentCount;
  u32 UStepCount;
  f32 USearchRange;
  f32 UFieldOfView;
  f32 UAlignmentFactor;
  f32 UCohesionFactor;
  f32 USeperationFactor;
  f32 UMaxForce;
  f32 UMaxSpeed;
  u32 UFrameCount;
};
typedef struct boids_ui boids_ui;
struct boids_ui
{
  s32 Res;
  s32 StepsPerFrame;
  s32 StepMod;
  b32 DoStep;
  b32 AutoStep;
  b32 DoReset;
  //Shader Consts
  u32 AgentCount;
  s32 SearchRange;
  f32 FieldOfView;
  f32 AlignmentFactor;
  f32 CohesionFactor;
  f32 SeperationFactor;
  f32 MaxForce;
  f32 MaxSpeed;
};
#define BOIDS_AGENTS_PER_THREADGROUP 64
#define BOIDS_PIXELS_PER_THREADGROUP 32
typedef struct boids boids;
struct boids
{
  v2s TexRes;
  //TEXTURES/BUFFERS
  ID3D11Texture2D           *TexRead;
  ID3D11Texture2D           *TexWrite;
  ID3D11Texture2D           *TexDebug;
  ID3D11Texture2D           *TexRender;
  ID3D11Buffer              *Agents;
  ID3D11Buffer              *AgentsStage;
  u32                        AgentCount;
  //VIEWS
  ID3D11ShaderResourceView  *SRViewTexRead;
  ID3D11ShaderResourceView  *SRViewTexRender; //pixel shader
  ID3D11UnorderedAccessView *UAViewTexRead;
  ID3D11UnorderedAccessView *UAViewTexWrite;
  ID3D11UnorderedAccessView *UAViewTexDebug;
  ID3D11UnorderedAccessView *UAViewTexRender; // compute shader
  ID3D11UnorderedAccessView *UAViewAgents; //AgentsBuffer
  ID3D11SamplerState        *TexReadSampler;
  ID3D11SamplerState        *TexRenderSampler;
  ID3D11Texture2D           *TexSwapStage;
  
  ID3D11Buffer             *VBuffer;
  ID3D11Buffer             *Consts;
  //Shader
  d3d11_shader AgentsReset;
  d3d11_shader AgentsMove;
  d3d11_shader AgentsTrails;
  d3d11_shader AgentsDebug;
  d3d11_shader TexReset;
  d3d11_shader TexDiffuse;
  d3d11_shader Render;
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  arena Arena; //only textures
  boids_ui UIState;
};

boids_ui BoidsUIStateInit(void)
{
  boids_ui Result =
  {
    .StepMod = 1,
    .Res = BOIDS_MAX_TEX_RES/2,
    .AutoStep = 1,
    .MaxSpeed = 0.5,
    .MaxForce = 0.5,
    //range[0.0, 5.0]
    .AlignmentFactor = 1.0f,
    .SeperationFactor = 1.0f,
    .CohesionFactor = 1.0f,
    .AgentCount = 20000,
    .SearchRange = 4,
    .FieldOfView = 0.5,
  };
  return Result;
}
void BoidsUI(boids_ui *Req)
{
  {  //SYS PARAMS
    igSliderInt("Agent Count", (s32 *)&Req->AgentCount, 1, BOIDS_MAX_AGENTCOUNT, NULL, 0);
    igSliderInt("Search Range", (s32 *)&Req->SearchRange, 1, 100, NULL, 0);
    igSliderFloat("FieldOfView", (f32 *)&Req->FieldOfView, 0.0, 1.0, NULL, 0);
    igSliderFloat("AlignmentFactor", (f32 *)&Req->AlignmentFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("CohesionFactor", (f32 *)&Req->CohesionFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("SeperationFactor", (f32 *)&Req->SeperationFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("Max Speed", (f32 *)&Req->MaxSpeed, 0.0, 5.0, NULL, 0);
    igSliderFloat("Max Force", (f32 *)&Req->MaxForce, 0.0, 5.0, NULL, 0);
    igSpacing();
  }
  {  //SYS PARAMS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->Res, BOIDS_MIN_TEX_RES, BOIDS_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    if(igButton("Step", *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
boids BoidsInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  boids Result = {0};
  u64 MemSize = Gigabytes(1); // NOTE(MIGUEL): Yo wtf lol. i allocate a gig on every system???
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  Result.UIState = BoidsUIStateInit();
  Result.AgentCount = Result.UIState.AgentCount;
  Result.TexRes = V2s(Result.UIState.Res, Result.UIState.Res);
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
  
  arena_temp Scratch = MemoryGetScratch(NULL, 0);
  boids_agent *AgentsInitial = ArenaPushArray(Scratch.Arena, BOIDS_MAX_AGENTCOUNT, boids_agent);
  v4f *TexInitial    = ArenaPushArray(Scratch.Arena, Result.TexRes.x*Result.TexRes.y, v4f);
  foreach(Agent, Result.UIState.AgentCount, u32) 
  {
    AgentsInitial[Agent] = (boids_agent){V2f(0.0f,0.0f), V2f(0.0f,0.0f), 0.0f, 0.0f};
  }
  foreach(Float, Result.TexRes.x*Result.TexRes.y, s32) TexInitial[Float] = V4f(0.0, 0.0, 0.0, 0.0);
  
  D3D11ScopedBase(Base)
  {
    D3D11BufferVertex(&Result.VBuffer, Data, sizeof(struct vert), 6);
    //State Views
    D3D11Tex2D(&Result.TexRead,
               &Result.SRViewTexRead, &Result.UAViewTexRead,
               Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA,
               Usage_Default, 0);
    D3D11Tex2D(&Result.TexRender,
               &Result.SRViewTexRender, &Result.UAViewTexRender, 
               Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA, Usage_Default, 0);
    D3D11Tex2DViewUA(Device, &Result.UAViewTexDebug, &Result.TexDebug, Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
    D3D11Tex2DViewUA(Device, &Result.UAViewTexWrite, &Result.TexWrite , Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
    D3D11Tex2DStage(&Result.TexSwapStage, Result.TexRes, TexInitial, sizeof(v2f), Float_RGBA); // Swap Stage
    D3D11BufferStructUA(&Result.Agents, AgentsInitial, sizeof(boids_agent), Result.UIState.AgentCount);
    D3D11BufferViewUA(Device, &Result.UAViewAgents, Result.Agents, Result.UIState.AgentCount);
    D3D11BufferStaging(&Result.AgentsStage, NULL, Result.UIState.AgentCount*sizeof(boids_agent));
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(boids_consts), Usage_Dynamic, Access_Write);
  }
  MemoryReleaseScratch(Scratch);
  
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT,
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexReadSampler);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.TexRenderSampler);
  }
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\boids\\boids.hlsl");
  Result.AgentsReset = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile,Str8("KernelAgentsReset"), NULL, 0, Base);
  Result.AgentsMove  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelAgentsMove"), NULL, 0, Base);
  Result.AgentsTrails  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelAgentsTrails"), NULL, 0, Base);
  Result.AgentsDebug = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelAgentsDebug"), NULL, 0, Base);
  Result.TexReset  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelTexReset"), NULL, 0, Base);
  Result.TexDiffuse = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelTexDiffuse"), NULL, 0, Base);
  Result.Render = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("KernelRender"), NULL, 0, Base);
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
fn void BoidsKernelAgentsMoveRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/BOIDS_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->AgentsMove.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Boids->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Boids->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Boids->UAViewTexDebug, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Boids->UAViewAgents, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelTexDiffuseRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->TexDiffuse.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Boids->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Boids->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Boids->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelAgentTrailsRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/BOIDS_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->AgentsTrails.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Boids->UAViewAgents, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Boids->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelRenderRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->Render.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Boids->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Boids->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Boids->UAViewTexDebug, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Boids->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelAgentsDebugRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/BOIDS_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->AgentsDebug.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Boids->UAViewAgents, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Boids->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelTexResetRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->TexReset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Boids->UAViewTexRead, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Boids->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsKernelAgentsResetRun(boids *Boids, d3d11_base *Base, boids_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/BOIDS_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Boids->AgentsReset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Boids->Consts, &Consts, sizeof(boids_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Boids->UAViewAgents, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void BoidsStep(boids *Boids, d3d11_base *Base, boids_consts Consts, u32 StepCount)
{
  D3D11BaseDestructure(Base);
  BoidsKernelAgentsMoveRun(Boids, Base, Consts);
  if(StepCount%2==0)
  {
    BoidsKernelTexDiffuseRun(Boids, Base, Consts);
    BoidsKernelAgentTrailsRun(Boids, Base, Consts);
    D3D11Tex2DSwap(Context, &Boids->TexRead, &Boids->TexWrite, Boids->TexSwapStage);
  }
  BoidsKernelRenderRun(Boids, Base, Consts);
  BoidsKernelAgentsDebugRun(Boids, Base, Consts);
  return;
}
fn void BoidsReset(boids *Boids, d3d11_base *Base, boids_consts Consts, u32 StepCount)
{
  BoidsKernelAgentsResetRun(Boids, Base, Consts);
  BoidsKernelTexResetRun(Boids, Base, Consts);
  return;
}
fn void BoidsDraw(boids *Boids, d3d11_base *Base, boids_ui UIReq, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  
  // NOTE(MIGUEL): Big problem here!!!! Since you are alowed to update the count via imgue but dont
  ///              update and enforce that the UI.Agent count is actually the agent count it cannot be
  //               relied on here. The agent count is detemined in UIStateInit and never changed. This 
  //               count may be wrong if it is changed and not updated here.
  // TODO(MIGUEL): track actual agent count seperatly from ui state. do not rely on UIState.AgentCount. update acual agent count
  //               in reset if succesfull. 
  
  // COMPUTE PASS
  boids_consts Consts = {
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Boids->TexRes.x, (u32)Boids->TexRes.y),
    .UAgentCount = UIReq.AgentCount,
    .UStepCount = StepCount,
    .USearchRange = (f32)UIReq.SearchRange,
    .UFieldOfView = -1.0f+2.0f*UIReq.FieldOfView,
    .UMaxSpeed = UIReq.MaxSpeed,
    .UMaxForce = UIReq.MaxForce,
    .UAlignmentFactor = UIReq.AlignmentFactor,
    .UCohesionFactor = UIReq.CohesionFactor,
    .USeperationFactor = UIReq.SeperationFactor,
    .UFrameCount = (u32)FrameCount,
  };
#if 0
  ConsoleLog(Arena,
             "al: %f co: %f%\n"
             "fov: %f \n",
             Consts.UApplyAlignment, Consts.UApplyCohesion,
             Consts.UFieldOfView);
#endif
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    BoidsStep(Boids, Base, Consts, StepCount);
    StepCount++;
  }
  if(UIReq.DoReset)
  {
    BoidsReset(Boids, Base, Consts, StepCount);
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Boids->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Boids->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Boids->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Boids->Consts);
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &Boids->TexRenderSampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Boids->SRViewTexRender);
  ID3D11DeviceContext_PSSetShader(Context, Boids->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}
#endif //BOIDS_H
