
//~ CONSTS
struct uniforms
{
  int Resolution; //1:1
};

//~COMPUTE SHADER
Texture2D  <float> OldState  : register( t0 );
RWTexture2D<float> NewState  : register( u0 );
RWTexture2D<float4>CCAResult : register( u1 );
void Render( uint3 ThreadID, float State)
{
  CCAResult[ThreadID.xy] = State;
}
[numthreads( 1, 1, 1)]
void ResetKernel( uint3 ThreadID : SV_DispatchThreadID  )
{
  NewState[ThreadID.xy] = 0;
}
[numthreads( 1, 1, 1)]
void StepKernel( uint3 ThreadID : SV_DispatchThreadID  )
{
  NewState[ThreadID.xy] = OldState[ThreadID.xy] + .1;
  Render(ThreadID, NewState[ThreadID.xy]);
}
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
  Output.Pos = float4(Input.Pos, 1.0);
  Output.UV  = Input.UV;
  return Output;
}

//~PIXEL SHADER

sampler           Sampler : register(s0); // s0 = sampler bound to slot 0
Texture2D<float4> Texture : register(t0); // t0 = shader resource bound to slot 0

float4 PSMain(PS_INPUT Input) : SV_TARGET                      
{  
  float4 Color = float4((Texture.Sample(Sampler, Input.UV.xy).xyz), 1.0);
  //float4 Color = float4(1.0, 0.0, 1.0, 1.0);
  return Color;
}
