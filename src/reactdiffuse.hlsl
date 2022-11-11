#define PIXELS_PER_THREADGROUP 32
#define PI  3.14159265359
#define TAU 2.0*3.14159265359
#define KindProduce  0
#define KindConsumer 1

//~ CONSTS
cbuffer cbuffer0 : register(b0)
{
  uint2 UWinRes; //1:1
  uint2 UTexRes; //1:1
  uint  UStepCount;
  uint  UFrameCount;
  float UBufferInit;
};

Texture2D  <float2> TexRead   : register( t0 );
RWTexture2D<float2> TexWrite  : register( u0 );
RWTexture2D<float4>TexRender : register( u1 );


//~ FUNCTIONS
float2 rand22(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.6));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
float3 hash3( uint n ) 
{
  // integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
  n = n * (n * n * 15731U + 789221U) + 1376312589U;
  uint3 k = n * uint3(n,n*16807U,n*48271U);
  return ( k & 0x7fffffffU)/(float)0x7fffffff;
}
float4 hsb2rgb(float3 c)
{
  float3 rgb = clamp(abs(((c.x * 6.0 + float3(0.0, 4.0, 2.0)) % 6.0) - 3.0) - 1.0, 0.0, 1.0);
  rgb = rgb * rgb * (3.0 - 2.0 * rgb);
  float3 o = c.z * lerp(float3(1.0, 1.0, 1.0), rgb, c.y);
  return float4(o.r, o.g, o.b, 1);
}
float sdBox( in float2 p, in float2 b )
{
  float2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
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
uint2 ToroidalWrap(int2 p)
{
  p = int2(p.x<0?UTexRes.x-1:p.x,
           p.y<0?UTexRes.y-1:p.y);
  uint2 Result = fmod(uint2(p), UTexRes);
  return Result;
}
float ChemFromTile(int2 Coord, uint Kind)
{
  Coord = ToroidalWrap(Coord);
#if 0
  uint BufferIdx = Coord.y*UTexRes + Coord.x;
  tile Tile      = BufferRead[BufferIdx];
  float Chem = Kind==KindProduce?Tile.Produce:Tile.Consumers;
#endif
  float Chem = Kind==KindProduce?TexRead[Coord].r:TexRead[Coord].g;
  return Chem;
}
float Laplacian(int2 id, uint Kind)
{
  //10:36
  float Result = 0.0;
  Result += ChemFromTile(id+int2(-1,-1), Kind)*0.05;
  Result += ChemFromTile(id+int2( 0,-1), Kind)*0.20;
  Result += ChemFromTile(id+int2( 1,-1), Kind)*0.05;
  
  Result += ChemFromTile(id+int2(-1, 0), Kind)*0.20;
  Result += ChemFromTile(id+int2( 0, 0), Kind)*-1.0;
  Result += ChemFromTile(id+int2( 1, 0), Kind)*0.20;
  
  Result += ChemFromTile(id+int2(-1, 1), Kind)*0.05;
  Result += ChemFromTile(id+int2( 0, 1), Kind)*0.20;
  Result += ChemFromTile(id+int2( 1, 1), Kind)*0.05;
  return Result;
}
float AverageDiff(int2 id, uint Kind)
{
  float Result = 0.0;
  Result += ChemFromTile(id+int2(-1,-1), Kind);
  Result += ChemFromTile(id+int2( 0,-1), Kind);
  Result += ChemFromTile(id+int2( 1,-1), Kind);
  
  Result += ChemFromTile(id+int2(-1, 0), Kind);
  //Result += ChemFromTile(id+int2( 0, 0), Kind);
  Result += ChemFromTile(id+int2( 1, 0), Kind);
  
  Result += ChemFromTile(id+int2(-1, 1), Kind);
  Result += ChemFromTile(id+int2( 0, 1), Kind);
  Result += ChemFromTile(id+int2( 1, 1), Kind);
  return Result/8.0;
}
void Test(int2 id)
{
#if 0
  tile Result;
  Result.Produce   = 0.545;
  Result.Consumers = AverageDiff(id.xy, KindConsumer);
  uint BufferIdx = id.y*UTexRes + id.x;
  BufferWrite[BufferIdx] = Result;
#endif
  return;
}

//~KERNELS
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelChemReset(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  
  float2 st   = (2.0*id.xy-UTexRes.xy)/UTexRes.y;
  float2 Seed = rand22(id.xy+UFrameCount);
  
  //TEXTURES
  float ring = smoothstep(0.01, 0.0, abs(length(st)-0.4)-0.01);
  float box = sdBox(st, float2(0.4,0.4));
  float2 Result = float2(0.8, UBufferInit*(ring+box));
  TexWrite[id.xy] = Result;
  return;
}
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelChemReactDiffuse(uint3 id : SV_DispatchThreadID)
{
  //// NOTE(MIGUEL): spatial radialy adjusted feed rate (lenght(st)).
  
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  float2 st   = (2.0*id.xy-UTexRes.xy)/UTexRes.y;
  //PARAMS
  float dt = 1.0;
  float DiffRateA = 1.0;
  float DiffRateB = 0.5;
  //https://www.karlsims.com/rdtool.html?s=0Nwog
  float FeedRate = 0.01200;
  float KillRate = 0.01413;
  //k=0.01413   f=0.00200
  //TEXTUREj
  float2 Result;
  float a = TexRead[id.xy].r;
  float b = TexRead[id.xy].g;
  a = a + (DiffRateA*Laplacian(id.xy, KindProduce ) - a*b*b + FeedRate*(1.0-a))*dt;
  b = b + (DiffRateB*Laplacian(id.xy, KindConsumer) + a*b*b - (KillRate+FeedRate)*b)*dt;
  Result = float2(a, b);
  TexWrite[id.xy] = Result;
  return;
}
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelRender(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.y) return;
  float2 st = (2.0*id.xy-UTexRes.xy)/UTexRes.y;
  float2 uv = id.xy/UTexRes.xy;
  
  float3 hsb = float3(TexRead[id.xy].rg, 1.0);
  float4 Result = float4(TexRead[id.xy].r, TexRead[id.xy].g, 1.0, 1.0);
  //lerp(a, b, t)
  hsb.x = lerp(3.0, sin(UFrameCount*0.02), Result.g);
  hsb.y = lerp(20.3, 55.8*Result.r, 10.1*hsb.z); //20, 100/40, 10/3
  
  TexRender[id.xy] = hsb2rgb(normalize(hsb));
  //float3 cl = sdBox(st, float2(0.9,0.9));//float3(smoothstep(0.0,0.01, st), 1.0);
  //TexRender[id.xy] = float4(0.01, 0.07, 0.2, 1.0);
  return;
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
//diffuse map
sampler           SamTexRendered : register(s0); // s0 = sampler bound to slot 0
Texture2D<float4> TexRendered    : register(t0); // t0 = shader resource bound to slot 0
//hight map
sampler           SamTexState : register(s1); // s1 = sampler bound to slot 0
Texture2D<float2> TexState    : register(t1); // t1 = shader resource bound to slot 0

//MISC RAYMARCHING
struct cam { float3 x; float3 y; float3 z; };
struct hit { float  d; float3 p;    int m; };
struct ray { float3 o; float3 d;};

hit HitZero()
{
  hit h; h.d = 0.0; h.p = 0.0; h.m = 0;
  return h;
}
#define TexelKind_Height (0)
#define TexelKind_Color  (1)
float3 GetTexel(float3 p, int id)
{
  float3 q = normalize(p);
  float  s  = 0.4*sin(UStepCount*0.002);
#if 0
  float2 uv = float2(atan2(q.x, q.z), asin(q.y));
#else
  float2 uv = 0.0;
  float hight = sin(PI/4);
  float lowt = sin(PI/4)/length(float3(1.0,1.0,1.0));
  q = abs(q);
  //clamp(normalize(abs(q)).y, lowt, hight)
  if(q.x>hight) uv = q.zy;
  if(q.y>hight) uv = q.xz;
  if(q.z>hight) uv = q.xy;
#endif
  
  float3 tex = 1.0;
  if(id==TexelKind_Color)
  {
    tex.xyz = TexRendered.SampleLevel(SamTexRendered, uv*s, 0).xyz;
  }
  if(id==TexelKind_Height)
  {
    tex.xy = TexState.SampleLevel(SamTexState, uv*s, 0).xy;
  }
  return tex;
}
float SdTexturedSphere(float3 p)
{
  float s = -0.04*clamp(GetTexel(normalize(p), TexelKind_Height).r,0.0,1.0);
  float d = length(p)-0.9-s;
  return d;
}
float World(float3 p)
{
  float d = 1000.0;
  //Intersect Testing
  float dp = SdTexturedSphere(p);
  //float d0 = max(0.0, p.y+0.8);
  d = min(d, dp);
  //d = min(d, d0);
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
  float tmin = 1.0;
  float tmax = 20.0;
  float d = 0.0;
  float t = tmin;
  
  for(int i=0;i<64;i++)
  {
    float precis = 0.0005*t;
    h.p  = r.d*t + r.o;
    d = World(h.p);
    if(precis > d || t > tmax) break;
    t += d;
  }
  
  if(t>tmax) { t=-1.0; }
  h.d = t;
  
  return h;
}
float SoftShadow(ray r, float mint, float maxt, float k)
{
  float res = 1.0;
  float ph = 1e20;
  
  for(float t=mint; t<maxt;)
  {
    float h = World(r.o + r.d*t);
    if(h<0.001) { return 0.0; }
    float y = h*h/(2.0*ph);
    float d = sqrt(h*h-y*y);
    res = min(res, k*d/max(0.0,t-y));
    ph = h;
    t += h;
  }
  return res;
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
  float Roughness = 0.1;
  float3 F0 = 0.8;
  float3 LightPos = 5.0;
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
float3 PhongLighting(hit Hit, float3 CameraPos, float3 Norm)
{
  float3 af = 0.1; //ambient light factor
  float3 alc = float3(0.8, 0.74, 0.3); //ambient light color
  float3 amb = af*alc; //ambiant
  float3 ld = normalize(float3(1.0, 0.2, 1.0)-Hit.p);
  float dif = max(0.0, dot(Norm, ld)); //diffuse
  float sf = 0.5;
  float3 vd = normalize(CameraPos - Hit.p); //view dir
  float3 rd = reflect(-ld, Norm); //reflect dir
  float spc = pow(max(dot(vd, rd), 0.0), 32);
  float3 col = (dif+amb+spc);
  return col;
}

float3 Render(ray Ray, float3 rdx, float3 rdy, float3 FallbackColor)
{
  float3 Color = FallbackColor;
  
  // Only one light and excluding skylight so one loop
  hit Hit = CastRay(Ray);
  if(Hit.d > -0.5)
  {
    float3 Norm = Normal(Hit.p);
    float3 lighting = 0.0;
    float3 matte = 0.0102;
    float3 ambient = 0.0;
    float3 ao = 2.0;
#define LIGHTING 1
#if LIGHTING == 0
    float3 diffuse = GetTexel(Hit.p, TexelKind_Color).rgb;
    matte = float3(0.5242,0.0,0.0)*0.0+diffuse;
    lighting = PhongLighting(Hit, Ray.o, Norm);
    //Result
    Color = matte*lighting;
#elif LIGHTING==1
    float3 diffuse = GetTexel(Hit.p, TexelKind_Color).rgb;
    matte = diffuse;
    ambient = 0.03*matte*ao;
    lighting = ambient + PBLighting(Hit, Ray, Ray.o, Norm, matte);
    //Result
    Color = lighting;
#endif
  }
  
  return Color;
}

float4 PSMain(PS_INPUT Input) : SV_TARGET                      
{  
  float2 uv = Input.Pos.xy/UWinRes.xy;
  float2 st = (2.0*Input.Pos.xy-UWinRes.xy)/UWinRes.y;
  
  //Color.xy = TexState.Sample(SamTexState, 1.0*Input.UV.xy).xy;
  float OrbitRad = 1.8;
  float Vignetting = smoothstep(0.32,0.0, length(st)-0.85/OrbitRad)*0.8+0.222;
  float3 BackgroundColor = 0.0;
  BackgroundColor = TexRendered.Sample(SamTexRendered, 1.0*Input.UV.xy).xyz;
  BackgroundColor *= Vignetting*lerp(float3(.4, 0.8, 0.85), float3(1.0, 0.9, 0.85), uv.y*0.0+1.);
  
  float3 Sun = float3(1.212, 1.3, 1.2);
  float3 Target = float3(0.0, 0.0, 0.0);
  cam Cam; ray Ray;
  float spd = 0.01;
  Ray.o = float3(0.01+sin(UStepCount*spd)*OrbitRad,
                 1.0,
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
}
