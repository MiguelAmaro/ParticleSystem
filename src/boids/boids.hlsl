
#define AGENTS_PER_THREADGROUP 64
#define PIXELS_PER_THREADGROUP 32
#define PI  3.14159265359
#define TAU 2.0*3.14159265359

//~ CONSTS/STRUCTS
cbuffer cbuffer0 : register(b0)
{
  uint2 UWinRes;
  uint2 UTexRes;
  uint  UAgentCount;
  uint  UStepCount;
  float USearchRange;
  float UFieldOfView;
  float UApplyAlignment;
  float UApplyCohesion;
  float UApplySeperation;
  float UMaxForce;
  float UMaxSpeed;
  uint  UTime; //UFrameCount;
};
struct agent
{
  float2 Pos;
  float2 Vel;
  float  MaxSpeed;
  float  MaxForce;
};

Texture2D<float4>   TexRead        : register(t0);
sampler             TexReadSampler : register(s0); // s0 = sampler bound to slot 0
RWTexture2D<float4> TexWrite : register(u0); // t0 = shader resource bound to slot 0
RWTexture2D<float4> TexDebug : register(u1);
RWTexture2D<float4> TexRender: register(u2);
//RWTexture2D<float>  TexLastPos: register(u3);
RWStructuredBuffer<agent> Agents : register(u3);

//~ FUNCTIONS
float2 rand22(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.651));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
float2 hash2( float2 p ) // replace this by something better
{
  p = float2( dot(p,float2(127.1,311.7)), dot(p,float2(269.5,183.3)) );
  return -1.0 + 2.0*frac(sin(p)*43758.545312);
}

float noise2( in float2 p )
{
  float K1 = 0.366025404; // (sqrt(3)-1)/2;
  float K2 = 0.211324865; // (3-sqrt(3))/6;
  
  float2  i = floor( p + (p.x+p.y)*K1 );
  float2  a = p - i + (i.x+i.y)*K2;
  float   m = step(a.y,a.x); 
  float2  o = float2(m,1.0-m);
  float2  b = a - o + K2;
  float2  c = a - 1.0 + 2.0*K2;
  float3  h = max( 0.5-float3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
  float3  n = h*h*h*h*float3( dot(a,hash2(i+0.0)), dot(b,hash2(i+o)), dot(c,hash2(i+1.0)) );
  return dot( n, 70.0);
}
#define NUM_OCTAVES 8
float fbm ( in float2 _uv) {
  float v = 0.0;
  float a = 0.8;
  float2 shift = 10.0;
  // Rotate to reduce axial bias
  float2x2 rot = float2x2(cos(0.5), sin(0.5),
                          -sin(0.5), cos(0.50));
  for (int i = 0; i < NUM_OCTAVES; ++i) {
    v += a * noise2(_uv).x;
    _uv = mul(rot, _uv) * 2.0 + shift;
    a *= 0.5;
  }
  return v;
}
float bias(float a)
{
  return 0.5 + 0.5*a;
}
float2 RandomDirection(float2 p)
{
  return normalize(2.0*(rand22(p) - 0.5));
}
float2 Limit(float2 Vec, float Mag)
{
  return (length(Vec)>Mag)?Mag*normalize(Vec):Vec;
}
uint2 ToroidalWrap(int2 p)
{
  uint2 Result = uint2(p.x<1?UTexRes.x-1-abs(p.x):p.x,
                       p.y<1?UTexRes.y-1-abs(p.y):p.y);
  Result %= UTexRes;
  return Result;
}
// Palettes - iq: https://www.shadertoy.com/view/ll2GD3
float3 Pallete( in float t, in float3 a, in float3 b, in float3 c, in float3 d )
{
  return a + b*cos( 6.28318*(c*t+d) );
}
float2 SimpleTurns(agent Agent, uint3 id)
{
  // TODO(MIGUEL): Test with fbm
  // This probably really expensive
  float SeedX = fbm((float2)((float)id+0.022)*UTime+.00132)*sin(UTime*0.001302);
  float SeedY = fbm((float2)id*UTime*0.001+0.002324)*cos(UTime*0.00123);
  float2 RandDir = normalize(float2(SeedX, SeedY));
  float2 NewVel  = normalize(lerp(RandDir, Agent.Vel, 0.5))+Agent.Vel;
  return NewVel;
}
void DrawAgentSearchRange(agent Agent, uint id)
{
  int Range = USearchRange;
  for(int x=-Range; x<=Range; x++)
  {
    for(int y=-Range; y<=Range; y++)
    {
      if(x==0 && y == 0) { continue; } // Ignore Self
      
      float2 Delta = float2(x, y);
      if((dot(normalize(-Delta), normalize(Agent.Vel))<UFieldOfView) &&
         (length(Delta) < USearchRange))
      {
        if(UStepCount%1 == 0)
        {
          uint2 Pos = ToroidalWrap(Agent.Pos + Delta);
          float alpha = 0.2+0.8*(float)id/(float)UAgentCount;
          float3 green = float3(1.0, 0.3, 0.8);
          float3 blue = float3(0.5, 0.0, 0.0);
          float3 pink = float3(0.003, 0.05, 0.03);
          TexDebug[Pos] += float4(pink, alpha);
          ///(float)UAgentCount
          // Draw the search area
        }
      }
    }
  }
  return;
}
bool IsVisable(agent Agent, float2 Dir, float Dist)
{
  bool InField = (dot(Dir, normalize(Agent.Vel)) > UFieldOfView);
  bool InRange = Dist < USearchRange;
  return InRange & InField;
}
float2 Alignment(agent Agent, uint AgentId)
{
  float2 Steer = 0.0;
  uint   NeighborCount = 0;
  float AlignFactor = UApplyAlignment;
  float2 AlignmentAvg  = 0.0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    //&& 
    if(IsVisable(Agent, Dir, Dist))
    {
      AlignmentAvg += Neighbor.Vel; 
      NeighborCount++;
    }
  }
  if(NeighborCount>0)
  {
    AlignmentAvg /= (float)NeighborCount;
    Steer = Limit(AlignmentAvg, UMaxForce*Agent.MaxForce*AlignFactor);
  }
  return Steer;
}
float2 Cohesion(agent Agent, uint AgentId)
{
  float2 Steer  = 0.0;
  float2 PosAvg = 0.0;
  float CohesionFactor = UApplyCohesion;
  uint   NeighborCount = 0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    if(IsVisable(Agent, Dir, Dist))
    {
      PosAvg += Neighbor.Pos;
      NeighborCount++;
    }
  }
  if(NeighborCount>0)
  {
    PosAvg /= (float)NeighborCount;
    float2 Target = PosAvg-Agent.Pos;
    Steer = Limit(Target, UMaxForce*Agent.MaxForce*CohesionFactor );
  }
  return Steer;
}
float2 Seperation(agent Agent, uint AgentId)
{
  float2 Steer = 0.0;
  float2 AvoidDir = 0.0;
  float AvoidFactor = UApplySeperation;
  float SafeDist = 10.0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    
    if(Dist < SafeDist || IsVisable(Agent, Dir, Dist))
    {
      AvoidDir += Agent.Pos-Neighbor.Pos; 
    }
  }
  Steer = Limit(AvoidDir, UMaxForce*Agent.MaxForce*AvoidFactor);
  return Steer;
}
float sdBox( in float2 p, in float2 b )
{
  float2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}
//~COMPUTE SHADERS
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsReset(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId > UAgentCount) return;       
  agent Agent;
  float  InitialSpeed = 3.0;
  float  InitialForce = 1.2;
  float2 InitialDir   = RandomDirection(id.xx*0.1 + sin(UTime));
  Agent.Pos = floor(rand22(id.x*0.0001 + UTime*0.001)*UTexRes); // NOTE(MIGUEL): DEBUGGING KD
  Agent.Vel = InitialDir*InitialSpeed;
  float2 FS = rand22(id.x*0.0001 + UTime*0.001);
  Agent.MaxSpeed = FS.x;
  Agent.MaxForce = FS.y;
  Agents[AgentId] = Agent;
}
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsMove(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId >= UAgentCount) return;       
  agent Agent = Agents[AgentId];
  /*
  I was assuming delta time to be ~16.6ms or 16.5/1000secs. Pluging that into 
  that UAE scale Acceleration or Force so much that it was insignificant. The max
  force must be higher for smaller time steps.

  I thought i could calc a ratio representing how much past the maxspeed (maxspeed*vel/|vel|)
  was the computed vel was and scale it down appropriately however that means that
  if velocity is under max speed it will be scaled it up.
  */
  float dt = 1.0;
  DrawAgentSearchRange(Agent, id.x);
  float2 A = Alignment (Agent, AgentId);
  float2 C = Cohesion  (Agent, AgentId);
  float2 S = Seperation(Agent, AgentId);
  float2 Wander = SimpleTurns(Agent, id);
  float2 RawVel = (A+C+S)+Wander;
  float2 NewVel = Limit(RawVel, Agent.MaxSpeed*UMaxSpeed);
  float2 NewPos = ToroidalWrap(Agent.Pos+NewVel);
  Agent.Vel = NewVel;
  Agent.Pos = NewPos;
  //Agent.MaxSpeed = UMaxSpeed;
  //Agent.MaxForce = UMaxForce;
  Agents[AgentId] = Agent;
  return;
}
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsTrails(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId >= UAgentCount) return;     
  agent Agent = Agents[AgentId];
  TexWrite[round(Agent.Pos - Agent.Vel)] = 1.0;
  return;
}
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelTexReset(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  TexWrite[id.xy] = 0.0;
}

[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelTexDiffuse(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  float4 oc = TexRead[id.xy];
  int Range = 2;
  float Avg = 0;
  float TrailDecay = 0.9;
  for(int x=-Range; x<=Range; x++) {
    for(int y=-Range; y<=Range; y++) 
    {
      float2 TexIdx = (id.xy + int2(x, y))/(float)UTexRes;
      Avg += TexRead.SampleLevel(TexReadSampler, TexIdx, 0.0).r;
    } }
  
  Avg /= (Range*2.0+1.0)*(Range*2.0+1.0);
  oc   = Avg*TrailDecay;
  oc   = clamp(oc, 0.0, 1.0);
  TexWrite[id.xy] = oc;
}
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelRender(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  float3 col = 0.0;
  //ESSENTIAL
  float2 res = float2(UTexRes.x, UTexRes.y);
  float2 uv = id.xy/res.xy;
  float  px = 1.0/UTexRes.y;
  float2 st = (2.0*id.xy-res.xy)/res.y;
  //SHADING
  TexRender[id.xy] = 1.0;
  float3 Sample  = TexRender[id.xy].rgb;
  float3 Sample1 = TexRead[id.xy].rgb;
  float3 Sample2 = TexDebug[id.xy].rgb;
  float3 matte = Sample2-Sample*0.02;
  //Sample2+
  //POST
  col = matte;
  col += smoothstep(0.0, px, sdBox(st, 0.9999));
  col = pow(max(0.0,col), 0.4545); //gamma correction
  //OUTPUT
  TexRender[id.xy] = 1.0-float4(col.r, col.g, col.b, 1.0);
  TexDebug [id.xy] *= 0.98;
}
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsDebug(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId >= UAgentCount) return;
  agent Agent = Agents[AgentId];
  TexRender[round(Agent.Pos)] = float4(0.1, 1.0/UAgentCount, 0.1, 1.0);
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
  float theta = float(UTime*0.001);
  float2x2 r = float2x2(sin(theta), -cos(theta),
                        cos(theta), sin(theta));
  Input.Pos.xy = mul(r, Input.Pos.xy);
  Output.Pos = float4(Input.Pos, 0.7);
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
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.9);
  float4 Color = float4((Texture.Sample(Sampler, 2.0*Input.UV.xy).xyz), 1.0);
  return Color*Vignetting; //*float4(noise2(st.xy), 0.0, 0.0, 0.0);
}
