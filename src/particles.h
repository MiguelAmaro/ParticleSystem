/* date = June 18th 2022 3:47 pm */

#ifndef PARTICLES_H
#define PARTICLES_H

typedef struct particle_const particle_const;
struct particle_const
{
  struct {
    v4f EmitterLocation;
    v4f ConsumerLocation;
    v4f TimeFactors;
  } sim_params;
  struct {
    v4f EmitterLocation;
    v4f RandomVector;
  } particle_params;
  struct {
    v4f EmitterLocation;
    v4f ConsumerLocation;
  } particle_render_params;
  struct {
    v4u NumParticles;
  } particle_count;
  struct {
    m4f WorldViewMatrix;                                   
    m4f ProjMatrix;                                   
  } transforms;
};

typedef struct particle particle;
struct particle
{
  v3f Pos;
  v3f Vel;
  f32 Time;
};

typedef struct particlesystem particlesystem;
struct particlesystem
{
  ID3D11Buffer *IndirectDrawArgs;
  u32           ArgBuffer[4];
  ID3D11Buffer *ParticleDataB;
  ID3D11Buffer *ParticleDataA;
  ID3D11Buffer *ConstParticleCount;
  ID3D11Buffer *ConstSimParams;
  ID3D11Buffer *ConstParticleParams;
  ID3D11Buffer *ConstParticleRenderParams;
  ID3D11Buffer *ConstUpdatedFrameData;
  ID3D11ComputeShader  *CMShader;
  ID3D11ComputeShader  *CHShader;
  ID3D11VertexShader   *VShader;
  ID3D11GeometryShader *GShader;
  ID3D11PixelShader    *PShader;
  particle_const ConstData;
  ID3D11UnorderedAccessView *UOAccessViewA;
  ID3D11UnorderedAccessView *UOAccessViewB;
  ID3D11ShaderResourceView  *ShaderResViewB;
  ID3D11InputLayout         *ParticleLayout;
  particle *ParticlesA;
  particle *ParticlesB;
  v4u       ParticleMaxCount;
};

#include "memory.c"
void ParticleSystemDraw(particlesystem *System,  ID3D11DeviceContext * Context, b32 IsFirstFrame,
                        D3D11_VIEWPORT *Viewport,
                        ID3D11RasterizerState* RastState,
                        ID3D11DepthStencilState* DepthState,
                        ID3D11BlendState* BlendState, 
                        ID3D11RenderTargetView* RTView,
                        ID3D11DepthStencilView* DSView)
{
  //~ Particle System
  /*TODOs
1. Supply a view and proj matrix (done)
2. Unordered access view for defining particle count (done)
3. Work on provideing nessaary unifrom data (done)
4. Do const buffer mapping (done)
5. Understand draw inderect
***Providing Initial Data
-    1. Init data. What is the expected space(screen, clip, ect) of the data?
-    2. Tell UAV num expected(max) particles in buffer. (done)
-        - [first frame]Struft BufferA gets paraticle count
-        - [first frame]Struft BufferB count is 0
-        - [nth   frame]Struft BufferA & B count is -1 (dynamical updeted in compute shader)

-   // NOTE(MIGUEL): I going to start resolving errors insolation by impleniting some minilmal
-                    task that requires each part of the pipeline.
-
-
-
*/
  ID3D11UnorderedAccessView *NullUAV[1] = {NULL};
  ID3D11ShaderResourceView  *NullSRV[1] = {NULL};
  ID3D11InputLayout         *NullLayout[1] = {NULL};
  
  if(IsFirstFrame == 1)
  {
    
    // Compute Shader Helper
    ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &System->ConstParticleParams);
    if(IsFirstFrame == 1)
    { 
      ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &System->UOAccessViewA,
                                                    &System->ParticleMaxCount.x);
    }
    else
    {
      /*u32 Counts[1] = {-1};*/
      ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &System->UOAccessViewA,
                                                    (u32[]){(u32)-1});
    }
    
    ID3D11DeviceContext_CSSetShader(Context, System->CHShader, NULL, 0);
    ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
    // Compute Shader Main
    ID3D11DeviceContext_CSSetConstantBuffers(Context, 1, 1, &System->ConstSimParams);
    ID3D11DeviceContext_CSSetConstantBuffers(Context, 2, 1, &System->ConstParticleCount);
    // NOTE(MIGUEL): UOAccessViewA is already bound
    //ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewA,
    //&System->ParticleMaxCount.x);
  }
  ID3D11DeviceContext_CopyStructureCount(Context, System->ConstParticleCount, 0, System->UOAccessViewA);
  
  if(IsFirstFrame == 1)
  {
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewB, 0);
  }
  else
  {
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewB, (u32[]){(u32)-1});
  }
  ID3D11DeviceContext_CopyStructureCount(Context, System->IndirectDrawArgs, 0, System->UOAccessViewB);
  
  ID3D11DeviceContext_CSSetShader(Context, System->CMShader, NULL, 0);
  ID3D11DeviceContext_Dispatch(Context, System->ParticleMaxCount.x/512, 1, 1); //
  
  //UBIND for render
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, NullUAV, &System->ParticleMaxCount.x);
  //~RENDERICNG
  // Input Assembler
  ID3D11DeviceContext_IASetInputLayout(Context, NullLayout[0]); //
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &System->ConstUpdatedFrameData);
  ID3D11DeviceContext_VSSetShaderResources(Context, 1, 1, &System->ShaderResViewB);
  ID3D11DeviceContext_VSSetShader(Context, System->VShader, NULL, 0);
  // Geometry Shader
  ID3D11DeviceContext_GSSetConstantBuffers(Context, 0, 1, &System->ConstUpdatedFrameData);
  ID3D11DeviceContext_GSSetConstantBuffers(Context, 1, 1, &System->ConstParticleRenderParams);
  ID3D11DeviceContext_GSSetShader(Context, System->GShader, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  //ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &sampler);
  //ID3D11DeviceContext_PSSetShaderResources(Context, 1, 1, &textureView);
  ID3D11DeviceContext_PSSetShader(Context, System->PShader, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  // draw 3 vertices
  ID3D11DeviceContext_DrawInstancedIndirect(Context, System->IndirectDrawArgs, 0);
  
  //UNBIND for next frame
  ID3D11DeviceContext_VSSetShaderResources(Context, 1, 1, NullSRV);
  return;
}

void ParticleSystemLoadShaders(particlesystem *System, ID3D11Device* Device)
{
  FILE *ParticleShaderFile;
  FILE *ParticleHelperShaderFile;
  fopen_s(&ParticleShaderFile      , (const char *)"F:\\Dev\\ParticleSystem\\src\\particles.hlsl", "rb");
  fopen_s(&ParticleHelperShaderFile, (const char *)"F:\\Dev\\ParticleSystem\\src\\particleshelper.hlsl", "rb");
  
  str8 ParticleShaderSrc       = Str8FromFile(ParticleShaderFile);
  str8 ParticleHelperShaderSrc = Str8FromFile(ParticleHelperShaderFile);
  HRESULT hr;
  ID3DBlob* error;
  
  //Particle
  ID3DBlob* PSysCMBlob;
  ID3DBlob* PSysCHBlob;
  ID3DBlob* PSysVBlob;
  ID3DBlob* PSysGblob;
  ID3DBlob* PSysPBlob;
  
  UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  
  hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "CSMAIN", "cs_5_0", flags, 0, &PSysCMBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main compute shader!");
  }
  hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "VSMAIN", "vs_5_0", flags, 0, &PSysVBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main vertex shader!");
  }
  hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "GSMAIN", "gs_5_0", flags, 0, &PSysGblob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main geometry shader!");
  }
  hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "PSMAIN", "ps_5_0", flags, 0, &PSysPBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main pixels shader!");
  }
  //ParticleHelper
  hr = D3DCompile(ParticleHelperShaderSrc.Data, ParticleHelperShaderSrc.Size, NULL, NULL, NULL, "CSHELPER", "cs_5_0", flags, 0, &PSysCHBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile helper compute shader!");
  }
  
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(PSysCMBlob), ID3D10Blob_GetBufferSize(PSysCMBlob), NULL, &System->CMShader);
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(PSysCHBlob), ID3D10Blob_GetBufferSize(PSysCHBlob), NULL, &System->CHShader);
  ID3D11Device_CreateVertexShader  (Device, ID3D10Blob_GetBufferPointer(PSysVBlob), ID3D10Blob_GetBufferSize(PSysVBlob), NULL, &System->VShader);
  ID3D11Device_CreateGeometryShader(Device, ID3D10Blob_GetBufferPointer(PSysGblob), ID3D10Blob_GetBufferSize(PSysGblob), NULL, &System->GShader);
  ID3D11Device_CreatePixelShader(Device, ID3D10Blob_GetBufferPointer(PSysPBlob), ID3D10Blob_GetBufferSize(PSysPBlob), NULL, &System->PShader);
  
  
  ID3D11InputLayout *PLayout;
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(v3f), D3D11_INPUT_PER_INSTANCE_DATA, 0 },
  };
  ID3D11Device_CreateInputLayout(Device, Desc, ARRAYSIZE(Desc), ID3D10Blob_GetBufferPointer(PSysVBlob), ID3D10Blob_GetBufferSize(PSysVBlob), &PLayout);
  // NOTE(MIGUEL): Acceptign no checking that layout was created succescfully at the momoent. d3d11 will report
  System->ParticleLayout = PLayout;
  ID3D10Blob_Release(PSysCMBlob);
  ID3D10Blob_Release(PSysCHBlob);
  ID3D10Blob_Release(PSysVBlob);
  ID3D10Blob_Release(PSysGblob);
  ID3D10Blob_Release(PSysPBlob);
  return;
}

particlesystem CreateParticleSystem(ID3D11Device* Device, u32 ParticleCount, f32 WindowWidth,
                                    f32 WindowHeight)
{
  particlesystem Result = {0};
  Result.ParticleMaxCount = V4u(ParticleCount, 0, 0, 0);
  Assert(Result.ParticlesA==SIM_NULL);
  Assert(Result.ParticlesB==SIM_NULL);
  Result.ParticlesA = malloc(Result.ParticleMaxCount.x*sizeof(particle));
  Result.ParticlesB = malloc(Result.ParticleMaxCount.x*sizeof(particle));
  MemorySet(0, Result.ParticlesA, Result.ParticleMaxCount.x*sizeof(particle));
  MemorySet(0, Result.ParticlesB, Result.ParticleMaxCount.x*sizeof(particle));
  for(u32 i=0;i<Result.ParticleMaxCount.x;i++) Result.ParticlesA[i].Pos = V3f(100.0f, 100.0f, 0.0f);
  for(u32 i=0;i<Result.ParticleMaxCount.x;i++) Result.ParticlesB[i].Pos = V3f(100.0f, 100.0f, 0.0f);
  v4f EmitterLocation = V4f(50.f, 50.0f, 0.0f, 0.0f);
  v4f CounsumerLocation = V4f(250.0f, 250.0f, 0.0f, 0.0f);
  v4f RandomVector = V4f(cosf(PI32/5), sinf(PI32/5), 0.0f, 0.0f);
  
  Result.ConstData.particle_render_params.EmitterLocation = EmitterLocation;
  Result.ConstData.particle_render_params.ConsumerLocation = CounsumerLocation;
  
  Result.ConstData.sim_params.EmitterLocation = EmitterLocation;
  Result.ConstData.sim_params.ConsumerLocation = CounsumerLocation;
  Result.ConstData.sim_params.TimeFactors = V4f(6.33, 0.0f, 0.0f, 0.0f);
  
  Result.ConstData.particle_params.EmitterLocation = EmitterLocation;
  Result.ConstData.particle_params.RandomVector = RandomVector;
  
  Result.ConstData.particle_count.NumParticles = Result.ParticleMaxCount;
  
  Result.ConstData.transforms.WorldViewMatrix = M4fIdentity();
  Result.ConstData.transforms.ProjMatrix = M4fOrtho(0.0f, WindowWidth,
                                                    0.0f, WindowHeight,
                                                    0.0f, 100.0f);
  
  //ping pong stuct buffers(append/consume)
  {
    //Used by CSMain & CSHelper
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = Result.ParticleMaxCount.x*sizeof(particle);
    Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    Desc.StructureByteStride = sizeof(particle);
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = Result.ParticlesA;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ParticleDataA);
  }
  {
    //Used by CSMain & CSHelper
    D3D11_UNORDERED_ACCESS_VIEW_DESC Desc = {0};
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    Desc.Buffer.FirstElement = 0;
    Desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    Desc.Buffer.NumElements = Result.ParticleMaxCount.x;
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Result.ParticleDataA, &Desc, &Result.UOAccessViewA);
  }
  {
    //Used by CSMain
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = Result.ParticleMaxCount.x*sizeof(particle);
    Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    Desc.StructureByteStride = sizeof(particle);
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = Result.ParticlesB;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ParticleDataB);
  }
  {
    //Used by CSMain
    D3D11_UNORDERED_ACCESS_VIEW_DESC Desc = {0};
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    Desc.Buffer.FirstElement = 0;
    Desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    Desc.Buffer.NumElements = Result.ParticleMaxCount.x;
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Result.ParticleDataB, &Desc, &Result.UOAccessViewB);
  }
  {
    //Used by VS
    // NOTE(MIGUEL): I dont know if i neeed to spec the count here doesnt the count get modified 
    //               by the gpu any way and im not sure if technically im saying that there are 100
    //               particles. It make sense because the buffer is a fixed size, right?
#if 1
    D3D11_SHADER_RESOURCE_VIEW_DESC Desc = {0};
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    Desc.Buffer.FirstElement = 0;
    Desc.Buffer.NumElements = Result.ParticleMaxCount.x;
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Result.ParticleDataB, &Desc, &Result.ShaderResViewB);
#endif
  }
  {
    //Indirect Args
    Result.ArgBuffer[0] = 1; //VertexCountPerInstance: 4(expects quad)
    Result.ArgBuffer[1] = 0; //InstanceCount: definede by gpu proveide via ::CopyStructCount
    Result.ArgBuffer[2] = 0; //StartVertexLocation: not sure, no vertex buffer, quad data is generated
    Result.ArgBuffer[3] = 0; //StartInstanceLocation: start of sturcturedbuffer(append/consume)
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(Result.ArgBuffer);
    Desc.StructureByteStride = 0;
    Desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = 0;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Args;
    Args.pSysMem = Result.ArgBuffer;
    ID3D11Device_CreateBuffer(Device, &Desc, &Args, &Result.IndirectDrawArgs);
  }
  {
    //Used By CSMain
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(Result.ConstData.sim_params);
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = &Result.ConstData.sim_params;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ConstSimParams);
  }
  {
    //Used By CSMain
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(v4u);
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = &Result.ParticleMaxCount;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ConstParticleCount);
  }
  {
    //Used by CSHelper
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(Result.ConstData.particle_params);
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = &Result.ConstData.particle_params;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ConstParticleParams);
  }
  {
    //Used By GSMain
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(Result.ConstData.transforms);
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = &Result.ConstData.transforms;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ConstUpdatedFrameData);
  }
  {
    //Used By GSMain
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = sizeof(Result.ConstData.particle_render_params);
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = &Result.ConstData.particle_render_params;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Result.ConstParticleRenderParams);
  }
  return Result;
}


#endif //PARTICLES_H
