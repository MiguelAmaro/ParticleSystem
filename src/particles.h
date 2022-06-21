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
