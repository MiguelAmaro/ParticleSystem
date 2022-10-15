
#define PI  3.14159265359
#define TAU 2.0*3.14159265359
//~ CONSTS
cbuffer cbuffer0 : register(b0)
{
  uint2 UWinRes; //1:1
  uint2 UTexRes; //1:1
  float UMaxStates;
  float UThreashold;
  int   URange;
  int   UOverCount;
  uint  UFrameCount;
  float3 _padding;
};

//~ FUNCTIONS
float2 rand(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.65));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
float4 hsb2rgb(float3 c)
{
  float3 rgb = clamp(abs(((c.x * 6.0 + float3(0.0, 4.0, 2.0)) % 6.0) - 3.0) - 1.0, 0.0, 1.0);
  rgb = rgb * rgb * (3.0 - 2.0 * rgb);
  float3 o = c.z * lerp(float3(1.0, 1.0, 1.0), rgb, c.y);
  return float4(o.r, o.g, o.b, 1);
}
uint2 ToridalWrap(int2 p)
{
  p = int2(p.x<0?UTexRes.x-1:p.x,
           p.y<0?UTexRes.y-1:p.y);
  uint2 Result = fmod(uint2(p), UTexRes);
  return Result;
}
//~COMPUTE SHADERS
Texture2D  <float> OldState  : register( t0 );
RWTexture2D<float> NewState  : register( u0 );
RWTexture2D<float4>CCAResult : register( u1 );
void Render( uint3 id, float State, float Count)
{
  float c = State/UMaxStates;
  float s = Count/UOverCount;
  float StateChanged;
  StateChanged = State==OldState[id.xy]?1.0:0.0;
  float3 hsb;
#if 0
  hsb.x = hsb.y = hsb.z = s;
  hsb.x = lerp(1., 4., hsb.x);   // 1/3/4 M
  //hsb.x = lerp(0, .1, hsb.x);     // 8/14/2/N
  hsb.y += 0.001;
  CCAResult[id.xy] += hsb2rgb(hsb)*StateChanged;
  CCAResult[id.xy] *= hsb2rgb(hsb)*.94*StateChanged; // 1/3/4/M
#endif
  hsb.x = hsb.y = hsb.z = c;
  hsb.y = lerp(1., 2., hsb.x);   // 1/3/4 M
  //hsb.x = lerp(0, .1, hsb.x);     // 8/14/2/N
  hsb.z = s;
  CCAResult[id.xy] = hsb2rgb(hsb);
  //CCAResult[id.xy] *= hsb2rgb(hsb)*.94; // 1/3/4/M
  
  return;
}
[numthreads( 1, 1, 1)]
void ResetKernel( uint3 id : SV_DispatchThreadID  )
{
  //(float)ThreadID.y/(float)URes.y*0.0 + 
  //step(0.1,0.9-)
  NewState[id.xy] = floor(UMaxStates*rand(float2(id.x*0.01, id.y)).x);
  printf("%d", UTexRes.x);
  //Render(id, NewState[id.xy]);
}
[numthreads( 1, 1, 1)]
void StepKernel( uint3 id : SV_DispatchThreadID  )
{
  float LastState = OldState[id.xy];
  int Range = URange;
  int Count = 0;
  for(int x=-Range;x<=Range;x++)
  {
    for(int y=-Range;y<=Range;y++)
    {
      uint2 Coord = ToridalWrap(id.xy + int2(x, y));
      if(Coord.x==0 && Coord.y==0) continue;
      Count = OldState[Coord]<UThreashold?Count:Count+1;
    }
  }
  float NextState = Count<UOverCount?LastState:fmod(LastState+1.0, UMaxStates);
  NewState[id.xy] = NextState;
  Render(id, NextState, (float)Count);
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
  float theta = float(UFrameCount*0.01);
  float2x2 r = float2x2(sin(theta), -cos(theta),
                        cos(theta), sin(theta));
  Input.Pos.xy = mul(r, Input.Pos.xy);
  Output.Pos = float4(Input.Pos, 1.0);
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
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.75);
  float4 Color = float4((Texture.Sample(Sampler, 1.0*Input.UV.xy).xyz), 1.0);
  return Color*Vignetting;
}
