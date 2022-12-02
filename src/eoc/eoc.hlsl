
//~CONSTANTS
#define PIXELS_PER_THREADGROUP 32
#define PI  3.14159265359
#define TAU 2.0*3.14159265359
#define STATE_MAX_COUNT 15

cbuffer cbuffer0 : register(b0)
{
  uint2 UWinRes;
  uint2 UTexRes; //1:1
  uint  UTime; //UFrameCount;
  uint  UStepCount;
  int   UStateCount;
};
//~COMMON FUNCTIONS
float4 hsb2rgb(float3 c)
{
  float3 rgb = clamp(abs(((c.x * 6.0 + float3(0.0, 4.0, 2.0)) % 6.0) - 3.0) - 1.0, 0.0, 1.0);
  rgb = rgb * rgb * (3.0 - 2.0 * rgb);
  float3 o = c.z * lerp(float3(1.0, 1.0, 1.0), rgb, c.y);
  return float4(o.r, o.g, o.b, 1);
}
float2 rand22(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.65));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
float bias(float a)
{
  return 0.5+0.5*a;
}
float2 bias2(float2 a)
{
  return 0.5+0.5*a;
}
float3 bias3(float3 a)
{
  return 0.5+0.5*a;
}
uint2 ToridalWrap(int2 p)
{
  p = int2(p.x<0?UTexRes.x-1:p.x,
           p.y<0?UTexRes.y-1:p.y);
  uint2 Result = fmod(uint2(p), UTexRes);
  return Result;
}
//~COMPUTE SHADER
RWTexture2D<int> TexState  : register( u0 );
RWTexture2D<int> TexStateCopy  : register( u1 );
RWTexture2D<float4>TexRender : register( u1 );
RWStructuredBuffer<int>TransitionLUT : register( u2 );
void Render( uint3 id, float State, float Count)
{
  float s = State/STATE_MAX_COUNT;
  //float c = Count/UOverCount;
  
  float3 hsb = s;
  hsb.x = lerp(2., .3, hsb.x);   // 1/3/4 M
  hsb.y = lerp(.8, 1.0, 0.2);     // 8/14/2/N
  hsb.z += 0.7;     // 8/14/2/N
  TexRender[id.xy] += hsb2rgb(hsb); // 1/3/4/M
  TexRender[id.xy] *= 0.9*hsb2rgb(hsb); // 1/3/4/M
  return;
}
//KERNELS START
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelReset( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  int NewState = 0;
  if(id.y < 1)
  {
    float2 Seed      = float2(id.x, UTexRes.x);
    float  Random    = rand22(Seed*UTime*0.0001).x;
    NewState  = (int)floor((float)UStateCount*Random);
  }
  TexState [id.xy] = NewState;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelStep( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= 1) return;
  uint CurrLine = 0;//(uint)(UStepCount+1%UTexRes.x);
  uint PrevLine = max(CurrLine+1, 0);
  int LeftId  = !(id.x>0)?(UTexRes.x - 1):(id.x - 1);
  int RightId = (id.x + 1)%UTexRes.x;
  int L = TexState[uint2(LeftId , PrevLine)];
  int S = TexState[uint2(id.x   , PrevLine)];
  int R = TexState[uint2(RightId, PrevLine)];
  uint LUTid = L*UStateCount*UStateCount + S*UStateCount + R;
  int NewState = TransitionLUT[LUTid];
  TexState [uint2(id.x, CurrLine)] = NewState;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelCopyDown( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  uint CurrLine = id.y;
  uint NextLine = CurrLine+1;
  TexStateCopy[uint2(id.x, NextLine)] = TexState[uint2(id.x, CurrLine)];
  TexState[uint2(id.x, CurrLine)] = 0;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelRender( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  float State = float(TexState[id.xy]);
  float L = float(TexState[id.xy+1]);
  float R = float(TexState[id.xy-1]);
  
  float h = State/(float)UStateCount;
  float s = L/(float)UStateCount;
  float b = R/(float)UStateCount;
  float3 hsb = float3(h,s,h);
  TexRender[uint2(id.x, id.y)] = hsb2rgb(hsb);
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
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.8);
  //Input.UV.xy
  float3 Texel = Texture.Sample(Sampler, 0.3*uv).xyz;
  //float2 p = float2(atan2(st.x, st.y), length(st));
  
  //float2 w = float2(p.x*0.5*cos(0.123*(float)UTime)+0.5, 0.5*cos(0.1*(float)UTime)+1.0*p.y);
  //Texel = smoothstep(0.01, 0.4,Texel-0.55)*float3(w, 1.0);
  float4 Color = float4(Texel, 1.0);
  return Color*Vignetting;
}
