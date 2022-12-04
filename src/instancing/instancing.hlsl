
#define PIXELS_PER_THREADGROUP 32
#define PI  3.14159265359
#define TAU 2.0*3.14159265359

//~CONSTANTS
cbuffer cbuffer0 : register(b0)
{
  row_major matrix UProj;
  row_major matrix UModel;
  uint2 UWinRes;
  uint2 UTexRes;
  uint  UTime; //UFrameCount;
};
//~COMMON FUNCTIONS
float2 rand22(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.65));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
//~COMPUTE SHADER
Texture2D<int>      TexRead  : register(t0);
RWTexture2D<int>    TexWrite : register(u0);
RWTexture2D<float4> TexRender : register(u1);
//KERNELS START
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelReset( uint3 id : SV_DispatchThreadID  )
{
  // NOTE(MIGUEL): make sure that all TexRes components are assigned in the .h file
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  float2 uv = (float2)id.xy/(float2)UTexRes.xy;
  TexWrite[id.xy] = 10*rand22(uv*(float)UTime*0.0001).x;
  return;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelStep( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  TexWrite[id.xy] = TexRead[id.xy];
  return;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelRender( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  TexRender[id.xy] = (float)TexRead[id.xy]/10.0;
  return;
}
//KERNELS END

//~VERTEX SHADER
struct VS_INPUT
{
  //PASSED FROM THE INPUT ASSEMBLER
  float3 Pos : IAPOS;
  float3 UV  : IATEXCOORD;
};
struct PS_INPUT                                            
{                                                          
  float4 Pos   : SV_POSITION; // This is required to be float4 cause is a D311 SysValue
  float3 UV    : TEXCOORD;                                 
};
PS_INPUT VSMain(VS_INPUT Input, uint VertID : SV_VertexID)
{
  PS_INPUT Output;
  float4 Pos = mul(UProj, mul(UModel, float4(Input.Pos, 1.0)));
  Output.Pos = Pos;
  Output.UV  = Input.UV;
  return Output;
}

//~PIXEL SHADER
sampler           Sampler : register(s0); // s0 = sampler bound to slot 0
Texture2D<float4> Texture : register(t0); // t0 = shader resource bound to slot 0

float4 PSMain(PS_INPUT Input) : SV_TARGET                      
{  
  float2 uv = Input.Pos.xy/UWinRes.xy;
  float2 st = (2.0*Input.Pos.xy-UWinRes.xy)/UWinRes.y;
  
  float3 Texel = Texture.Sample(Sampler,st).xyz;
  float3 Color = Texel;
  return float4(Color, 1.0);
}
