#define PIXELS_PER_THREADGROUP 32
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
  uint  UStepCount;
  uint  UFrameCount;
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
//~COMPUTE SHADERS
Texture2D  <float> TexRead   : register( t0 );
RWTexture2D<float> TexWrite  : register( u0 );
RWTexture2D<float4>TexRender : register( u1 );
void Render( uint3 id, float State, float Count)
{
  float s = State/UMaxStates;
  float c = Count/UOverCount;
  
  float3 hsb = s;
  hsb.x = lerp(2., .3, hsb.x);   // 1/3/4 M
  hsb.y = lerp(.8, 1.0, 0.2);     // 8/14/2/N
  hsb.z += 0.7;     // 8/14/2/N
  TexRender[id.xy] += hsb2rgb(hsb); // 1/3/4/M
  TexRender[id.xy] *= 0.9*hsb2rgb(hsb); // 1/3/4/M
  return;
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void ResetKernel( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  TexWrite[id.xy] = floor(rand(float2(id.x*0.001+UFrameCount, id.y)).x*UMaxStates);
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void StepKernel( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  int LastState = (int)min(TexRead[id.xy], UMaxStates);
  int Range = URange;
  int Count = 0;
  for(int x=-Range;x<=Range;x++)
  {
    for(int y=-Range;y<=Range;y++)
    {
      uint2 Coord = ToridalWrap(id.xy + int2(x, y));
      if(Coord.x==0 && Coord.y==0) continue;
      Count = TexRead[Coord]<UThreashold?Count:Count+1;
    }
  }
  int NextState = Count<UOverCount?LastState:(LastState+1)%UMaxStates;
  TexWrite[id.xy] = (float)NextState;
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
  float theta = float(UStepCount*0.01);
  float2x2 r = float2x2(sin(theta), -cos(theta),
                        cos(theta), sin(theta));
  //Input.Pos.xy = mul(r, Input.Pos.xy);
  Output.Pos = float4(Input.Pos, 1.0);
  Output.UV  = Input.UV;
  return Output;
}

//~PIXEL SHADER

sampler           SamTexRendered : register(s0); // s0 = sampler bound to slot 0
Texture2D<float4> TexRendered    : register(t0); // t0 = shader resource bound to slot 0
sampler           SamTexState    : register(s1); // s0 = sampler bound to slot 0
Texture2D<float2>  TexState       : register(t1); // t0 = shader resource bound to slot 0

//MISC RAYMARCHING
struct cam { float3 x; float3 y; float3 z; };
struct hit { float  d; float3 p;    int m; };
struct ray { float3 o; float3 d;};

hit HitZero()
{
  hit h; h.d = 0.0; h.p = 0.0; h.m = 0;
  return h;
}
float4 TriPlanarMap(float3 p)
{
  float3 q = bias3(p); 
  float s = 0.1;//sin(UStepCount*0.00002);
  float4 tex = 0.0;
#if 1
  float3 bld = abs(p);
  bld = normalize(max(bld, 0.00001));
  float b = (bld.x + bld.y + bld.z);
  bld /= b;
  float3 x = TexRendered.Sample(SamTexRendered, s*q.yz).xyz;
  float3 y = TexRendered.Sample(SamTexRendered, s*q.xz).xyz;
  float3 z = TexRendered.Sample(SamTexRendered, s*q.xy).xyz;
  tex.xyz = x*bld.x + y*bld.y + z*bld.z;
  float xw = TexState.Sample(SamTexState, s*q.yz);
  float yw = TexState.Sample(SamTexState, s*q.xz);
  float zw = TexState.Sample(SamTexState, s*q.xy);
  tex.w = xw*bld.x + yw*bld.y + zw*bld.z;
#else
  tex.xyz = TexRendered.Sample(SamTexRendered, s*q.xy).xyz;
  tex.w   = TexState.Sample(SamTexState, s*q.xy);
#endif
  return tex;
}
float World(float3 p)
{
  float d = 10000.0;
  //Intersect Testing
  
  float3 q = abs(p)%4.5;
  float3 s = 4.5*0.5;
  float4 tex = TriPlanarMap(normalize(q));
  float dp = length(q-s)-0.95+tex.w/UMaxStates*0.1;
  d = min(d, dp);
  return d;
}
float3 Normal(float3 pos)
{
  float2 e = float2(0.001,0.0);
  return normalize(float3(World(pos+e.xyy)-World(pos-e.xyy),
                          World(pos+e.yxy)-World(pos-e.yxy),
                          World(pos+e.yyx)-World(pos-e.yyx)));
}
hit CastRay(ray r)
{
  hit h = HitZero();
  float t = 0.0;
  float d = 0.0;
  
  [unroll]
    for(int i=0;i<100;i++)
  {
    h.p  = r.d*t + r.o;
    d = World(h.p);
    h.d = d;
    t += d;
    if(0.001 > h.d || h.d > 20.00) break;
  }
  return h;
}
float4 PSMain(PS_INPUT Input) : SV_TARGET                      
{  
  float2 uv = Input.Pos.xy/UWinRes.xy;
  float2 st = (2.0*Input.Pos.xy-UWinRes.xy)/UWinRes.y;
  
  
  float Vignetting = smoothstep(0.0,0.3, length(st)-0.85);
  float3 Color = float3(1.0, 0.9, 0.9);
  
  //RAYMARCHING
  float OrbitRad = 1.0;
  float3 Target = float3(0.0, 0.0, 0.0);
  cam Cam;
  ray Ray;
  float spd = 0.01;
  Ray.o = float3(UFrameCount*0.02 +cos(UStepCount*spd)*OrbitRad, 0.0, sin(UStepCount*spd)*OrbitRad); 
  Cam.z = normalize(Target-Ray.o);
  Cam.x = normalize(cross(Cam.z, float3(0.0,1.0,0.0)));
  Cam.y = normalize(cross(Cam.x, Cam.z));
  float Focal = 1.0;
  float3 pix = float3(st, Focal);
  Ray.d = normalize(pix.x*Cam.x +
                    pix.y*Cam.y +
                    pix.z*Cam.z);
  hit Hit = CastRay(Ray);
  if(Hit.d < 0.001)
  {
    float3 Norm = Normal(Hit.p);
    float3 light = float3(0.0, 0.0,0.0);
    //float3 matte = 0.0*Hit.p;
    float3 matte = TriPlanarMap(Norm).xyz;
    Color = matte;
  }
  
  Color = TexRendered.Sample(SamTexRendered, 1.0*Input.UV.xy).xyz;
  Color = pow(max(0.0,Color), 0.4545);
  return float4(Color.x, Color.y, Color.z, 1.0);
}
