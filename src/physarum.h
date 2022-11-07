#ifndef PHYSARUM_H
#define PHYSARUM_H

#define PHYSARUM_MIN_TEX_RES (256)
#define PHYSARUM_MAX_TEX_RES (2048)

typedef struct physarum_consts physarum_consts;
struct16 physarum_consts
{
  v2u UWinRes;
  v2u UTexRes;
  u32 UStepCount;
  u32 UAgentCount;
  s32 UTrailDecay;
  f32 UTrailDiffuse;
  s32 USearchRange;
  f32 UCuriosity;
  f32 UDispersion;
  f32 UIndecision;
  f32 UFieldOfView;
  u32 UFrameCount;
};
typedef struct physarum_ui physarum_ui;
struct physarum_ui
{
  s32 Res;
  s32 StepsPerFrame;
  s32 StepMod;
  b32 DoStep;
  b32 AutoStep;
  b32 DoReset;
  //Shader Consts
  u32 AgentCount;
  s32 TrailDecay;
  f32 TrailDiffuse;
  s32 SearchRange;
  f32 Curiosity;
  f32 Dispersion;
  f32 Indecision;
  f32 FieldOfView;
};
#define Physarum_AGENTS_PER_THREADGROUP 64
#define Physarum_PIXELS_PER_THREADGROUP 32
typedef struct physarum physarum;
struct physarum
{
  v2s TexRes;
  //TEXTURES/BUFFERS
  ID3D11Texture2D           *TexRead;
  ID3D11Texture2D           *TexWrite;
  ID3D11Texture2D           *TexDebug;
  ID3D11Texture2D           *TexRender;
  ID3D11Buffer              *Agents;
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
  physarum_ui UIState;
};
physarum_ui PhysarumUIStateInit(void)
{
  physarum_ui Result =
  {
    .StepMod = 1,
    .Res = BOIDS_MAX_TEX_RES/2,
    .AutoStep = 1,
    .AgentCount = 512,
    .SearchRange = 4,
    .FieldOfView = 0.5,
  };
  return Result;
}
physarum PhysarumInit(d3d11_base *Base, physarum_ui UIReq)
{
  D3D11BaseDestructure(Base);
  physarum Result = {0};
  u64 MemSize = Gigabytes(2);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  Result.UIState = PhysarumUIStateInit();
  Result.TexRes = V2s(UIReq.Res, UIReq.Res);
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
  struct agent
  {
    v2f Pos;
    v2f Vel;
    float  MaxSpeed;
    float  MaxForce;
  };
  arena_temp Temp = ArenaTempBegin(&Result.Arena);
  struct agent *AgentsInitial = ArenaPushArray(Temp.Arena, UIReq.AgentCount, struct agent);
  v4f *TexInitial    = ArenaPushArray(Temp.Arena, Result.TexRes.x*Result.TexRes.y, v4f);
  foreach(Agent, UIReq.AgentCount, u32) AgentsInitial[Agent] = (struct agent){V2f(0.0f,0.0f), V2f(0.0f,0.0f), 0.0f, 0.0f};
  foreach(Float, Result.TexRes.x*Result.TexRes.y, s32) TexInitial[Float] = V4f(0.0, 0.0, 0.0, 0.0);
  D3D11VertexBuffer(Device, &Result.VBuffer, Data, sizeof(struct vert), 6);
  //State Views
  D3D11Tex2DViewSRAndUA(Device, &Result.TexRead,
                        &Result.SRViewTexRead, &Result.UAViewTexRead,
                        Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
  D3D11Tex2DViewSRAndUA(Device, &Result.TexRender,
                        &Result.SRViewTexRender, &Result.UAViewTexRender, 
                        Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
  D3D11Tex2DViewUA(Device, &Result.UAViewTexDebug, &Result.TexDebug, Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
  D3D11Tex2DViewUA(Device, &Result.UAViewTexWrite, &Result.TexWrite , Result.TexRes, TexInitial, sizeof(v4f), Float_RGBA);
  D3D11Tex2DStage(Device, &Result.TexSwapStage, Result.TexRes, TexInitial, sizeof(v2f), Float_RGBA); // Swap Stage
  D3D11StructuredBuffer(Device, &Result.Agents, AgentsInitial, sizeof(struct agent), UIReq.AgentCount);
  D3D11BufferViewUA(Device, &Result.UAViewAgents, Result.Agents, UIReq.AgentCount);
  D3D11ConstantBuffer(Device, &Result.Consts, NULL, sizeof(physarum_consts), Usage_Dynamic, Access_Write);
  ArenaTempEnd(Temp);
  
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
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\physarum.hlsl");
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
fn void PhysarumKernelAgentsMoveRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/Physarum_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->AgentsMove.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Physarum->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Physarum->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Physarum->UAViewTexDebug, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Physarum->UAViewAgents, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelTexDiffuseRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/Physarum_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->TexDiffuse.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Physarum->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Physarum->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Physarum->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelAgentTrailsRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UAgentCount/Physarum_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->AgentsTrails.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Physarum->UAViewAgents, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Physarum->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelRenderRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/Physarum_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->Render.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetSamplers(Context, 0, 1, &Physarum->TexReadSampler);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Physarum->SRViewTexRead);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Physarum->UAViewTexDebug, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Physarum->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelAgentsDebugRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/Physarum_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->AgentsDebug.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Physarum->UAViewAgents, NULL);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Physarum->UAViewTexRender, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelTexResetRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/Physarum_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->TexReset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Physarum->UAViewTexRead, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Physarum->UAViewTexWrite, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumKernelAgentsResetRun(physarum *Physarum, d3d11_base *Base, physarum_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/Physarum_AGENTS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Physarum->AgentsReset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryOp(Context, Physarum->Consts, &Consts, sizeof(physarum_consts), 1, GPU_MEM_WRITE);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, &Physarum->UAViewAgents, NULL);
  ID3D11DeviceContext_Dispatch(Context, GroupCount, 1, 1);
  D3D11ClearComputeStage(Context);
  return;
}
fn void PhysarumStep(physarum *Physarum, d3d11_base *Base, physarum_consts Consts, u32 StepCount)
{
  D3D11BaseDestructure(Base);
  PhysarumKernelAgentsMoveRun(Physarum, Base, Consts);
  if(StepCount%2==0)
  {
    PhysarumKernelTexDiffuseRun(Physarum, Base, Consts);
    PhysarumKernelAgentTrailsRun(Physarum, Base, Consts);
    D3D11Tex2DSwap(Context, &Physarum->TexRead, &Physarum->TexWrite, Physarum->TexSwapStage);
  }
  PhysarumKernelRenderRun(Physarum, Base, Consts);
  PhysarumKernelAgentsDebugRun(Physarum, Base, Consts);
  return;
}
fn void PhysarumReset(physarum *Physarum, d3d11_base *Base, physarum_consts Consts, u32 StepCount)
{
  PhysarumKernelAgentsResetRun(Physarum, Base, Consts);
  PhysarumKernelTexResetRun(Physarum, Base, Consts);
  return;
}
fn void PhysarumDraw(physarum *Physarum, d3d11_base *Base, physarum_ui UIReq, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  scoped_global u32 StepCount = 0;
  // COMPUTE PASS
  //UIReq.ApplyAlignment += 1;
  physarum_consts Consts = {
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Physarum->TexRes.x, (u32)Physarum->TexRes.y),
    .UAgentCount = UIReq.AgentCount,
    .UStepCount = StepCount,
    .USearchRange = (s32)UIReq.SearchRange,
    .UFieldOfView = UIReq.FieldOfView,
    .UFrameCount = (u32)FrameCount,
  };
  
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    PhysarumStep(Physarum, Base, Consts, StepCount);
    StepCount++;
  }
  if(UIReq.DoReset)
  {
    PhysarumReset(Physarum, Base, Consts, StepCount);
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Physarum->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Physarum->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Physarum->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Physarum->Consts);
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &Physarum->TexRenderSampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Physarum->SRViewTexRender);
  ID3D11DeviceContext_PSSetShader(Context, Physarum->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}
#endif //PHYSARM_H
