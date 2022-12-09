#define PIXELS_PER_THREADGROUP 32
#define PI  3.14159265359
#define TAU 2.0*3.14159265359
//~ CONSTS
cbuffer cbuffer0 : register(b0)
{
  //CORE
  uint2 UWinRes; //1:1
  uint2 UTexRes; //1:1
  uint  UFrameCount;
  uint  UStepCount;
  //PRIMARY
  float UStateCountPrimary;
  float UThreasholdPrimary;
  int   URangePrimary;
  uint  UDoMoorePrimary;
  //SECONDARY
  float UStateCountSecondary;
  float UThreasholdSecondary;
  int   URangeSecondary;
  uint  UDoMooreSecondary;
};

Texture2D  <float> TexKusu   : register( t0 );
Texture2D  <float> TexRead   : register( t0 );
RWTexture2D<float> TexWrite  : register( u0 );
RWTexture2D<float4>TexRender : register( u1 );

//~ FUNCTIONS
float2 rand22(float2 p)
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
void Render( uint3 id, float State, float Count)
{
  float sm = State/UStateCountPrimary;
  float c = Count;
  float3 hsb = sm;
  float h = lerp(c, 4.0*sm, hsb.x*0.9);   // 1/3/4 M
  float s = lerp(h*0.8, 10.0, c*10.0);     // 8/14/2/N
  float b = 0.8;     // 8/14/2/N
  hsb = float3(h,s,b);
  
  //TexRender[id.xy] = sm;
  TexRender[id.xy] += hsb2rgb(hsb); // 1/3/4/M
  TexRender[id.xy] *= 0.3; // 1/3/4/M
  return;
}

//~KERNELS
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void ResetKernel( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  float Rand = rand22(float2(id.x*0.001+UFrameCount, id.y*0.01)).x;
  TexWrite[id.xy] = (int)(Rand*UStateCountPrimary)*TexKusu[id.xy];
}
[numthreads( PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void StepKernel( uint3 id : SV_DispatchThreadID  )
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  int LastState = (int)TexRead[id.xy];
  int NextState = (LastState+1)==UStateCountPrimary?0:(LastState+1);
  
  int Range = URangePrimary;
  int DoMoore = UDoMoorePrimary;
  int Threashold = UThreasholdPrimary;
  int Count = 0;
  for(int x=-Range;x<=Range;x++)
  {
    for(int y=-Range;y<=Range;y++)
    {
      uint2 Coord = ToridalWrap(id.xy + int2(x, y));
      if(Coord.x==0 && Coord.y==0) continue;
      
      if(DoMoore || (x==0 || y==0))
      {
        int NeighState = (int)TexRead[Coord];
        Count += (int)(NeighState == NextState);
      }
    }
  }
  // NOTE(MIGUEL): UThreasholdPrimary might not be correct. was UOverCount(deleted)
  int State = LastState;
  if(Count >= Threashold)
  {
    State = NextState;
  }
  TexWrite[id.xy] = State;
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
Texture2D<float>  TexState       : register(t1); // t0 = shader resource bound to slot 0

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
  float s = 0.1;
  float4 tex = TexRendered.SampleLevel(SamTexRendered, s*p.xz, 0);
  return tex;
}
float3 PlaceTex(float2 hp, float2 p, float2 d)
{
  //centered pos
  float2 rmin = p - d/2.0;
  float2 rmax = p + d/2.0;
  float3 tex = 0.0;
  if((rmin.x <= hp.x && hp.x <= rmax.x) && 
     (rmin.y <= hp.y && hp.y <= rmax.y))
  {
    float2 uv = float2((hp.x-rmin.x)/d.x,
                       (hp.y-rmin.y)/d.y);
    tex = TexRendered.SampleLevel(SamTexRendered, uv, 0).xyz;
  }
  return tex;
}
float SdfBox(float3 p, float3 b)
{
  float3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}
float SphereGroup(float3 hp, float3 p, float3 d, float ogd)
{
  //centered pos
  float3 rmin = p - d/2.0;
  float3 rmax = p + d/2.0;
  float dist = 0.0;
  /*NOTE(MA): I initially only considered the world space occupancy of the texture and forgot
*           that the size of the texture in world space is divide by the texture res.
*           what ever the world space size of the texture divided by the texture res size is
*           is the cell size that each sdf object must reside in.
*           
*/
  
  //end height
  float sd = (float)UTexRes.y; //subdivision
  float3 cs = d/sd; //cell size: world size/tex res
  float3 cc = cs*0.5; //cell center
  float3 sdr = abs(hp)%cs; //space domain repetition
  // op rep lim
  float3 lim = UTexRes.x/2.0;
  
  //hight
  float3 pid = (hp-rmin)/cs.x/UTexRes.x; 
  float2 uv = float2(pid.x,
                     pid.z);
  float h = TexState.SampleLevel(SamTexState, uv, 0).r;
  hp.y += h*cs.y;
  //end height
  lim.y = 2.0;
  float3 lp = (hp+cc)-cs*clamp(round((hp+cc)/cs),-lim,lim);
  //end
  float ss = min(cs.x*0.5, cs.x*0.5); //sdf sphere size(caped by cell size)
  dist = length(lp)-ss;
  
  
  //d.y= 1.0;
  // NOTE(MIGUEL): im upsidedown
  //x
#if 0
  if(hp.x < rmin.x) dist = ogd;
  if(hp.x > rmax.x) dist = ogd;
  //z
  if(hp.z < rmin.z) dist = ogd;
  if(hp.z > rmax.z) dist = ogd;
  //y
  //if(hp.y < -1.0) dist = ogd;
  rmin.y = -1;
  rmax.y = 8;
  if(hp.y < rmin.y) dist = ogd;
  if(hp.y > rmax.y) dist = ogd;
#endif
  return dist;
}
hit World(float3 p)
{
  float dmax = 10000.0;
  //Intersect Testing
  hit h; h.d = dmax; h.m = 0.0; h.p = p;
  //Plane
  float dp = 2.0-p.y;
  //if(dp<h.d) { h.d = dp; h.m = 1.0; }
  //Sphere
  //p.y = min(p.y, 10.0);
  float ds = SphereGroup(p, 0.0, float3(100.0, 400.0, 100), h.d);
  if(ds<h.d) { h.d = ds; h.m = 0.0; }
  //+tex.w/UStateCountPrimary*0.1;
  return h;
}
float3 Normal(float3 pos)
{
  float2 e = float2(0.001,0.0);
  return normalize(float3(World(pos+e.xyy).d-World(pos-e.xyy).d,
                          World(pos+e.yxy).d-World(pos-e.yxy).d,
                          World(pos+e.yyx).d-World(pos-e.yyx).d));
}
hit CastRay(ray r)
{
  hit h = HitZero();
  float tmin = 1.0;
  float tmax = 128.0;
  float t = 0.0;
  float d = tmin;
  
  for(int i=0;i<64;i++)
  {
    float precis = 0.00005*t;
    h.p  = r.d*t + r.o;
    h = World(h.p);
    if(precis > h.d || t > tmax) break;
    t += h.d;
  }
  if(t>tmax) {t=-1.0;}
  h.d = t;
  return h;
}
float3 Fresnel(float CosTheta, float3 F0)
{
  float3 Result = F0 + (1.0 - F0) * pow(1.0 - CosTheta, 5.0);
  return Result;
}
float NormalDistribution(float3 Norm, float3 Half, float Roughness)
{
  float a2     = Roughness*Roughness*Roughness*Roughness;
  float NdotH  = max(dot(Norm, Half), 0.0);
  float Denom  = (NdotH*NdotH*(a2-1.0)+1.0);
  float Result = a2/(PI*Denom*Denom);
  return Result;
}
float GeometrySchlickGGX(float NdotV, float Roughness)
{
  float r  = (Roughness+1.0);
  float k  = (Roughness*Roughness)/8.0;
  float Result = NdotV/(NdotV*(1.0-Roughness)+Roughness);
  return Result;
}
float Geometry(float3 Norm, float3 V, float3 L, float Roughness)
{
  float NdotV = max(dot(Norm, V), 0.0);
  float NdotL = max(dot(Norm, L), 0.0);
  float ggx1 = GeometrySchlickGGX(NdotV, Roughness);
  float ggx2 = GeometrySchlickGGX(NdotL, Roughness);
  return ggx1 * ggx2;
}
float3 PBLighting(hit Hit, ray Ray, float3 CameraPos, float3 Norm, float3 Albedo)
{
  //Params
  float Roughness = 0.3;
  float3 F0 = 0.8;
  float3 LightPos = float3(10.0, 0.3,10.0);
  //Params End
  float3 LightCol = float3(1.0, 1.0,1.0);
  float3 LightDir = normalize(LightPos - Hit.p); //Wi //float3 Wi = hash3(i);
  float3 ViewDir  = normalize(CameraPos - Hit.p); //Wo
  float3 Half     = normalize(ViewDir + LightDir); 
  float3 LightDist = length(LightPos - Hit.p);
  float3 Atten     = 1.0/(LightDist*LightDist); //from irradiance? r^2 squared distance falloff? maybe not
  float3 Radiance = LightCol * Atten;
  float3 Normal = normalize(Hit.p);
  
  //BRDF
  float3 D = NormalDistribution(Norm, Half, Roughness);
  float3 G = Geometry(Norm, ViewDir, LightDir, Roughness);
  float3 F = Fresnel(max(0.0, dot(Half, ViewDir)), F0);
  float3 Ks = F;
  float3 Kd = 1.0 - Ks;
  
  float3 SNumer = D*G*F;
  float3 SDenom = 4.0 * max(dot(Norm, ViewDir), 0.0)*max(dot(Norm, LightDir), 0.0) + 0.0001;
  float3 Specular = SNumer/SDenom;
  
  float3 NdotL = max(0.0, dot(Norm, LightDir));
  float3 Result = (Kd * Albedo / PI + Specular) * Radiance * NdotL;
  return Result;
}
float3 Render(ray Ray, float3 rdx, float3 rdy, float3 FallbackColor)
{
  float3 Color = FallbackColor;
  
  //Only one light and excluding skylight so one loop
  hit Hit = CastRay(Ray);
  if(Hit.d > -0.5)
  {
    
    /*
float3 Norm = Normal(Hit.p);
    float3 light = float3(0.0, 0.0,0.0);
    //float3 matte = 0.0*Hit.p;
    float2 huv = clamp(Hit.p.xz/20.0, 0.0, 1.0);
    float3 matte = 0;
    if(Hit.m < 0.5)
    {
      matte = PlaceTex(Hit.p.xz, 0.0, 100.0);
      TexRendered.SampleLevel(SamTexRendered, huv, 0).xyz; //TriPlanarMap(Norm).xyz;
    }
    Color = matte;
*/
    
    float3 Norm = normalize(Hit.p);
    float3 lighting = 0.0;
    float3 matte = 0.00102;
    float3 ambient = 0.0;
    float3 ao = 2.0;
    if(Hit.m < 0.5)
    {
      matte *= PlaceTex(Hit.p.xz, 0.0, 100.0);
      float2 huv = clamp(Hit.p.xz/20.0, 0.0, 1.0);
      TexRendered.SampleLevel(SamTexRendered, huv, 0).xyz; //TriPlanarMap(Norm).xyz;
    }
    //PBL
    ambient = 0.05*matte*ao; //ambient
    lighting = ambient+PBLighting(Hit, Ray, Ray.o, Norm, matte);
    //Result
    Color = lighting;
  }
  
  return Color;
}

float4 PSMain(PS_INPUT Input) : SV_TARGET                      
{  
#if 1
  
  float2 uv = Input.Pos.xy/UWinRes.xy;
  float2 st = (2.0*Input.Pos.xy-UWinRes.xy)/UWinRes.y;
  
  //Color.xy = TexState.Sample(SamTexState, 1.0*Input.UV.xy).xy;
  float OrbitRad = max(20.0,sin((float)UStepCount*0.01)*100.0)+2.0*bias(sin(UFrameCount*0.005));
  float Vignetting = smoothstep(0.02,0.0, length(st)-0.05/OrbitRad)*0.8+0.222;
  float3 BackgroundColor = 0.0;
  BackgroundColor = TexRendered.Sample(SamTexRendered, 1.0*Input.UV.xy).xyz;
  BackgroundColor = Vignetting*lerp(float3(.4, 0.8, 0.85), float3(1.0, 0.9, 0.85), uv.y*0.0+1.);
  
  float3 Sun = float3(1.212, 1.3, 1.2);
  float3 Target = float3(0.0, 0.0, 0.0);
  cam Cam; ray Ray;
  float spd = 0.01;
  Ray.o = float3(0.01+sin(UStepCount*spd)*OrbitRad,
                 80.0,
                 0.01+cos(UStepCount*spd)*OrbitRad); 
  Cam.z = normalize(Target-Ray.o);
  Cam.x = normalize(cross(Cam.z, float3(0.0,1.0,0.0)));
  Cam.y = normalize(cross(Cam.z,Cam.x));
  
#define AA (2)
  float3 Total = 0.0;
  for(int m=0;m<AA;m++)
  {
    for(int n=0;n<AA;n++)
    {
      float2 o = float2(m, n)/(float)AA - 0.5;
      float2 p = (2.0*(Input.Pos.xy+o)-UWinRes.xy)/UWinRes.y; //offset version of st. so target other neigboring pixels and acculiate/blend
      float Focal = 2.0;
      
      float3 pix = float3(p, Focal); 
      Ray.d = normalize(pix.x*Cam.x + pix.y*Cam.y + pix.z*Cam.z);
      float2 px = (2.0*(Input.Pos.xy+float2(1.0,0.0))-UWinRes.xy)/UWinRes.y;
      float2 py = (2.0*(Input.Pos.xy+float2(0.0,1.0))-UWinRes.xy)/UWinRes.y;
      pix = float3(px, Focal); 
      float3 rdx = normalize(pix.x*Cam.x + pix.y*Cam.y + pix.z*Cam.z);
      pix = float3(py, Focal); 
      float3 rdy = normalize(pix.x*Cam.x + pix.y*Cam.y + pix.z*Cam.z);
      
      float3 Color = Render(Ray, rdx, rdy, BackgroundColor);
      Color = pow(clamp(Color, 0.0, 1.0), 0.4545);
      Total += Color;
    }
  }
  
  float3 Color = Total/AA*AA;
  
  return float4(Color.x, Color.y, Color.z, 1.0);
#endif
#if 0
  float2 uv = Input.Pos.xy/UWinRes.xy;
  float2 st = (2.0*Input.Pos.xy-UWinRes.xy)/UWinRes.y;
  
  
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.75);
  float3 Color = float3(1.0, 0.87, 0.8);
  
  //RAYMARCHING
  float OrbitRad = max(10, 90.0*cos(UStepCount*0.008));
  float3 Target = float3(0.0, 0.0, 0.0);
  cam Cam;
  ray Ray;
  float spd = 0.01;
  Ray.o = float3(UFrameCount*0.0 +cos(UStepCount*spd)*OrbitRad, -9.0, sin(UStepCount*spd)*OrbitRad); 
  //Ray.o = float3(PI*OrbitRad, -1.0, PI*OrbitRad); 
  Cam.z = normalize(Target-Ray.o);
  Cam.x = normalize(cross(Cam.z, float3(0.0,1.0,0.0)));
  Cam.y = normalize(cross(Cam.x, Cam.z));
  float Focal = 1.0;
  float3 pix = float3(st, Focal);
  Ray.d = normalize(pix.x*Cam.x +
                    pix.y*Cam.y +
                    pix.z*Cam.z);
  hit Hit = CastRay(Ray);
  if(Hit.d > -0.5)
  {
    float3 Norm = Normal(Hit.p);
    float3 light = float3(0.0, 0.0,0.0);
    //float3 matte = 0.0*Hit.p;
    float2 huv = clamp(Hit.p.xz/20.0, 0.0, 1.0);
    float3 matte = 0;
    if(Hit.m < 0.5)
    {
      matte = PlaceTex(Hit.p.xz, 0.0, 100.0);
      TexRendered.SampleLevel(SamTexRendered, huv, 0).xyz; //TriPlanarMap(Norm).xyz;
    }
    Color = matte;
  }
  
  //Color = TexRendered.Sample(SamTexRendered, 1.0*Input.UV.xy).xyz;
  Color = pow(max(0.0,Color), 0.4545);
  return float4(Color, 1.0)+0.0*Vignetting;
#endif
}
