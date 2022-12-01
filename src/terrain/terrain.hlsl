cbuffer cbuffer0 : register(b0)
{
  uint2 UWinRes;
  uint  UTime; //UFrameCount;
};

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
  
  Output.Pos = float4(Input.Pos, 0.0);
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
  
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.8);
  float3 Texel = Texture.Sample(Sampler, Input.UV.xy).xyz;
  float4 Color = float4(Texel, 1.0);
  
  return Color*Vignetting;
}
