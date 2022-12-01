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
  ID3D11Buffer  *DbgStageBuffer;
  ID3D11ComputeShader  *CMShader;
  ID3D11ComputeShader  *CHShader;
  ID3D11VertexShader   *VShader;
  ID3D11GeometryShader *GShader;
  ID3D11PixelShader    *PShader;
  particle_const ConstData;
  ID3D11SamplerState* Sampler;
  ID3D11ShaderResourceView* TextureView;
  ID3D11UnorderedAccessView *UOAccessViewA;
  ID3D11UnorderedAccessView *UOAccessViewB;
  ID3D11ShaderResourceView  *ShaderResViewB;
  ID3D11InputLayout         *ParticleLayout;
  particle *ParticlesA;
  particle *ParticlesB;
  v4u       ParticleMaxCount;
};

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
  
  //- Compute Shader Helper
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &System->ConstParticleParams);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &System->UOAccessViewA,
                                                &System->ParticleMaxCount.x);
  ID3D11DeviceContext_CSSetShader(Context, System->CHShader, NULL, 0);
  ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
  //- Compute Shader Main
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 1, 1, &System->ConstSimParams);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 2, 1, &System->ConstParticleCount);
  // NOTE(MIGUEL): UOAccessViewA is already bound
  //ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewA,
  //&System->ParticleMaxCount.x);
  // NOTE(MIGUEL): Can I readback the vertex count an binde AC buffer to differend slots
  //               for the weird pingpong.
  
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewB, &System->ParticleMaxCount.x);
  //ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &System->UOAccessViewB, (u32[]){(u32)-1});
  
  ID3D11DeviceContext_CopyStructureCount(Context, System->IndirectDrawArgs, 0, System->UOAccessViewB);
#if 1
  //DEBUG ARGBUFFER
  //For staging/debug
  ID3D11DeviceContext_CopyResource(Context,
                                   (ID3D11Resource*)System->DbgStageBuffer,
                                   (ID3D11Resource*)System->ParticleDataB);
  // NOTE(MIGUEL): This is not tested.
  // TODO(MIGUEL): Verify this works.
  D3D11GPUMemoryRead(Context,
                     System->DbgStageBuffer,
                     &System->ParticlesA,
                     sizeof(u32), //particle 
                     64
                     //System->ParticleMaxCount.x,
                     );
  
  ID3D11DeviceContext_CopyStructureCount(Context, System->ConstParticleCount, 0, System->UOAccessViewA);
#endif
  ID3D11DeviceContext_CSSetShader(Context, System->CMShader, NULL, 0);
  ID3D11DeviceContext_Dispatch(Context, System->ParticleMaxCount.x/512, 1, 1); //
  
  //Cleanup for render
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, NullUAV, 0);
  //~RENDERICNG
  // Input Assembler
  ID3D11DeviceContext_IASetInputLayout(Context, NullLayout[0]); //
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  
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
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &System->Sampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &System->TextureView);
  ID3D11DeviceContext_PSSetShader(Context, System->PShader, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  // draw 3 vertices
  ID3D11DeviceContext_DrawInstancedIndirect(Context, System->IndirectDrawArgs, 0);
  
  //UNBIND for next frame
  ID3D11DeviceContext_VSSetShaderResources(Context, 1, 1, NullSRV);
  D3D11ClearPipeline(Context);
  return;
}

void ParticleSystemLoadShaders(particlesystem *System, ID3D11Device* Device)
{
  ID3DBlob* PSysCMBlob = D3D11ShaderLoadAndCompile(Str8("F:\\Dev\\ParticleSystem\\src\\particles.hlsl"),  Str8("CSMAIN"), "cs_5_0", "Particle System");
  ID3DBlob* PSysCHBlob = D3D11ShaderLoadAndCompile(Str8("F:\\Dev\\ParticleSystem\\src\\particleshelper.hlsl"), Str8("CSHELPER"), "cs_5_0", "Particle System");
  ID3DBlob* PSysVBlob  = D3D11ShaderLoadAndCompile(Str8("F:\\Dev\\ParticleSystem\\src\\particles.hlsl"),  Str8("VSMAIN"), "vs_5_0", "Particle System");
  ID3DBlob* PSysGblob  = D3D11ShaderLoadAndCompile(Str8("F:\\Dev\\ParticleSystem\\src\\particles.hlsl"),  Str8("GSMAIN"), "gs_5_0", "Particle System");
  ID3DBlob* PSysPBlob  = D3D11ShaderLoadAndCompile(Str8("F:\\Dev\\ParticleSystem\\src\\particles.hlsl"),  Str8("PSMAIN"), "ps_5_0", "Particle System");
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(PSysCMBlob), ID3D10Blob_GetBufferSize(PSysCMBlob), NULL, &System->CMShader);
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(PSysCHBlob), ID3D10Blob_GetBufferSize(PSysCHBlob), NULL, &System->CHShader);
  ID3D11Device_CreateVertexShader  (Device, ID3D10Blob_GetBufferPointer(PSysVBlob), ID3D10Blob_GetBufferSize(PSysVBlob), NULL, &System->VShader);
  ID3D11Device_CreateGeometryShader(Device, ID3D10Blob_GetBufferPointer(PSysGblob), ID3D10Blob_GetBufferSize(PSysGblob), NULL, &System->GShader);
  ID3D11Device_CreatePixelShader(Device, ID3D10Blob_GetBufferPointer(PSysPBlob), ID3D10Blob_GetBufferSize(PSysPBlob), NULL, &System->PShader);
  
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(particle, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IAVEL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(particle, Vel), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  ID3D11Device_CreateInputLayout(Device, Desc, ARRAYSIZE(Desc), ID3D10Blob_GetBufferPointer(PSysVBlob), ID3D10Blob_GetBufferSize(PSysVBlob), &System->ParticleLayout);
  // NOTE(MIGUEL): Acceptign no checking that layout was created succescfully at the momoent. d3d11 will report
  ID3D10Blob_Release(PSysCMBlob);
  ID3D10Blob_Release(PSysCHBlob);
  ID3D10Blob_Release(PSysVBlob);
  ID3D10Blob_Release(PSysGblob);
  ID3D10Blob_Release(PSysPBlob);
  return;
}

particlesystem CreateParticleSystem(d3d11_base* Base, u32 ParticleCount, f32 WindowWidth,
                                    f32 WindowHeight)
{
  D3D11BaseDestructure(Base);
  particlesystem Result = {0};
  Result.ParticleMaxCount = V4u(ParticleCount, 0, 0, 0);
  Assert(Result.ParticlesA==NULL);
  Assert(Result.ParticlesB==NULL);
  Result.ParticlesA = malloc(Result.ParticleMaxCount.x*sizeof(particle));
  Result.ParticlesB = malloc(Result.ParticleMaxCount.x*sizeof(particle));
  MemorySet(0, Result.ParticlesA, Result.ParticleMaxCount.x*sizeof(particle));
  MemorySet(0, Result.ParticlesB, Result.ParticleMaxCount.x*sizeof(particle));
  for(u32 i=0;i<Result.ParticleMaxCount.x;i++) Result.ParticlesA[i].Pos = V3f(100.0f, 100.0f, 0.0f);
  for(u32 i=0;i<Result.ParticleMaxCount.x;i++) Result.ParticlesB[i].Pos = V3f(100.0f, 100.0f, 0.0f);
  v4f EmitterLocation = V4f(50.f, 50.0f, 0.0f, 0.0f);
  v4f CounsumerLocation = V4f(250.0f, 250.0f, 0.0f, 0.0f);
  v4f RandomVector = V4f(cosf(Pi32/5), sinf(Pi32/5), 0.0f, 0.0f);
  
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
  D3D11ScopedBase(Base)
  {
    //ping pong stuct buffers(append/consume)
    D3D11BufferStructUA(&Result.ParticleDataA, Result.ParticlesA, sizeof(particle), Result.ParticleMaxCount.x);
    //Used by CSMain & CSHelper
    D3D11BufferViewUA(Device, &Result.UOAccessViewA, Result.ParticleDataA, Result.ParticleMaxCount.x);
    //Used by CSMain
    D3D11BufferStructUA(&Result.ParticleDataB, Result.ParticlesB, sizeof(particle), Result.ParticleMaxCount.x);
    //Used by CSMain
    D3D11BufferViewUA(Device, &Result.UOAccessViewB, Result.ParticleDataB, Result.ParticleMaxCount.x);
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
    
    //Indirect Args
    Result.ArgBuffer[0] = 1; //VertexCountPerInstance: 4(expects quad)
    Result.ArgBuffer[1] = 0; //InstanceCount: definede by gpu proveide via ::CopyStructCount
    Result.ArgBuffer[2] = 0; //StartVertexLocation: not sure, no vertex buffer, quad data is generated
    Result.ArgBuffer[3] = 0; //StartInstanceLocation: start of sturcturedbuffer(append/consume)
    D3D11BufferArgs(&Result.IndirectDrawArgs, Result.ArgBuffer, sizeof(Result.ArgBuffer));
    // This is a staging buffer so its data can be read from the cpu. The contents of the ArgBuffer will be 
    // copied to this buffer. 
    u32 Dummy[64] = {0};
    D3D11BufferStaging(&Result.DbgStageBuffer, Dummy, sizeof(Dummy));
    //Used By CSMain
    D3D11BufferConstant(&Result.ConstSimParams,
                        &Result.ParticleMaxCount, 
                        sizeof(Result.ConstData.sim_params), Usage_Default, Access_None);
    //Used By CSMain
    D3D11BufferConstant(&Result.ConstParticleCount,
                        &Result.ParticleMaxCount, 
                        sizeof(v4u), Usage_Default, Access_None);
    //Used By CSMain
    D3D11BufferConstant(&Result.ConstParticleParams,
                        &Result.ConstData.particle_params, 
                        sizeof(Result.ConstData.particle_params), Usage_Default, Access_None);
    //Used By GSMain
    D3D11BufferConstant(&Result.ConstUpdatedFrameData,
                        &Result.ConstData.transforms, 
                        sizeof(Result.ConstData.transforms), Usage_Default, Access_None);
    //Used By GSMain
    D3D11BufferConstant(&Result.ConstParticleRenderParams,
                        &Result.ConstData.particle_render_params, 
                        sizeof(Result.ConstData.particle_render_params), Usage_Default, Access_None);
  }
  {
    // checkerboard texture, with 50% transparency on black colors
    unsigned int pixels[] =
    {
      0x00000000, 0xffffffff,
      0xffffffff, 0xffffffff,
    };
    UINT width = 2;
    UINT height = 2;
    
    D3D11_TEXTURE2D_DESC Desc =
    {
      .Width = width,
      .Height = height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = { 1, 0 },
      .Usage = D3D11_USAGE_IMMUTABLE,
      .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };
    D3D11_SUBRESOURCE_DATA Data =
    {
      .pSysMem = pixels,
      .SysMemPitch = width * sizeof(unsigned int),
    };
    
    ID3D11Texture2D* Texture;
    ID3D11Device_CreateTexture2D(Device, &Desc, &Data, &Texture);
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, NULL, &Result.TextureView);
    ID3D11Texture2D_Release(Texture);
  }
  {
    D3D11_SAMPLER_DESC Desc =
    {
      .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
      .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    };
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.Sampler);
  }
  
  return Result;
}


#endif //PARTICLES_H
