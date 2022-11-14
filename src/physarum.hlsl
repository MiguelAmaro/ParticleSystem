
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
  uint  UTime; //UFrameCount;
  uint4 _padding;
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
RWStructuredBuffer<agent> Agents : register(u3);

//~ FUNCTIONS
float2 rand22(float2 p)
{
  float3 a = frac(p.xyx * float3(123.34, 234.34, 345.65));
  a += dot(a, a + 34.45);
  return frac(float2(a.x * a.y, a.y * a.z));
}
float2 hash2( float2 p ) // replace this by something better
{
  p = float2( dot(p,float2(127.1,311.7)), dot(p,float2(269.5,183.3)) );
  return -1.0 + 2.0*frac(sin(p)*43758.5453123);
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
  return length(Vec)>Mag?Mag*normalize(Vec):Vec;
}
uint2 ToroidalWrap(int2 p)
{
  p = int2(p.x<0?UTexRes.x-1:p.x,
           p.y<0?UTexRes.y-1:p.y);
  uint2 Result = fmod(uint2(p), UTexRes);
  return Result;
}
// Palettes - iq: https://www.shadertoy.com/view/ll2GD3
float3 Pallete( in float t, in float3 a, in float3 b, in float3 c, in float3 d )
{
  return a + b*cos( 6.28318*(c*t+d) );
}
float2 SimpleTurns(uint3 id, agent Agent)
{
  float4 Trail  = TexRead[round(Agent.Pos + Agent.Vel*2.0)];
  float2 NewVel = (Trail.x>0)?RandomDirection(id.xx*0.01 + sin(UTime)): Agent.Vel;
  return NewVel;
}
bool IsVisable(agent Agent, float2 Dir, float Dist)
{
  bool InField = (dot(Dir, normalize(Agent.Vel)) > UFieldOfView);
  bool InRange = Dist < USearchRange;
  return InRange & InField;
}
void DrawAgentSearchRange(agent Agent)
{
  int Range = USearchRange;
  for(int x=-Range; x<=Range; x++)
  {
    for(int y=-Range; y<=Range; y++)
    {
      if(x==0 && y == 0) { continue; } // Ignore Self
      
      float2 Delta = float2(x, y);
      if((dot(normalize(Delta), normalize(Agent.Vel))>UFieldOfView) &&
         (length(Delta) < USearchRange))
      {
        uint2 Pos = round(ToroidalWrap(Agent.Pos + Delta));
        if(UStepCount%1 == 0)
        {
          // Draw the search area
          TexDebug[Pos] += 0.1*float4(1.0/(float)UAgentCount, 0.0, 0.0, 0.0);
        }
      }
    }
  }
  return;
}
float2 Alignment(agent Agent, uint AgentId)
{
  float2 Steer = 0.0;
  uint   NeighborCount = 0;
  float2 AlignmentAvg  = 0.0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    if(IsVisable(Agent, Dist, Dist))
    {
      AlignmentAvg += Neighbor.Vel; 
      NeighborCount++;
    }
  }
  if(NeighborCount>0)
  {
    AlignmentAvg /= (float)NeighborCount;
    Steer = Limit(AlignmentAvg, Agent.MaxForce);
  }
  return Steer;
}
float2 Cohesion(agent Agent, uint AgentId)
{
  float2 Steer  = 0.0;
  float2 PosAvg = 0.0;
  uint   NeighborCount = 0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    if(IsVisable(Agent, Dist, Dist))
    {
      PosAvg += Neighbor.Pos;
      NeighborCount++;
    }
  }
  if(NeighborCount>0)
  {
    PosAvg /= (float)NeighborCount;
    float2 Target = PosAvg-Agent.Pos;
    Steer = Limit(Target, Agent.MaxForce);
  }
  return Steer;
}
float2 Seperation(agent Agent, uint AgentId)
{
  float2 Steer = 0.0;
  uint   NeighborCount = 0;
  float2 SeperAvg  = 0.0;
  for(uint NeighborId=0; NeighborId<UAgentCount; NeighborId++)
  {
    if(NeighborId == AgentId) { continue; }
    agent Neighbor = Agents[NeighborId];
    float2 Dir  = normalize(Neighbor.Pos - Agent.Pos);
    float  Dist = distance (Neighbor.Pos, Agent.Pos);   
    if((Dist < USearchRange) && (dot(Dir, normalize(Agent.Vel)) > UFieldOfView))
    {
      float2 Delta = Agent.Pos-Neighbor.Pos;
      Delta /= max(Dist, 0.001);
      SeperAvg += Neighbor.Vel; 
      NeighborCount++;
    }
  }
  if(NeighborCount>0)
  {
    SeperAvg /= (float)NeighborCount;
    float2 Desired = Agent.MaxSpeed*normalize(SeperAvg);
    Steer = Limit(Desired - Agent.Vel, Agent.MaxForce);
  }
  return Steer;
}
float sdBox( in float2 p, in float2 b )
{
  float2 d = abs(p)-b;
  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}
//~COMPUTE KERNELS
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsReset(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId > UAgentCount) return;       
  agent Agent;
  float  InitialSpeed = 3.0;
  float  InitialForce = 1.0;
  float2 InitialDir   = RandomDirection(id.xx*0.1 + sin(UTime));
  Agent.Pos = rand22(id.x*0.0001 + UTime*0.001)*UTexRes;
  Agent.Vel = InitialDir*InitialSpeed;
  Agent.MaxSpeed = InitialSpeed;
  Agent.MaxForce = InitialForce;
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
  float2 A = Alignment(Agent, AgentId); //UApplyAlignment *
  float2 C = Cohesion (Agent, AgentId); //UApplyCohesion  *
  float2 S = Seperation(Agent, AgentId); //UApplySeperation*
  float2 RawVel = A+C+S+Agent.Vel;
  float2 NewVel = Limit(RawVel, Agent.MaxSpeed);
  float2 NewPos = Agent.Pos + NewVel;
  Agent.Vel = NewVel;
  Agent.Pos = NewPos;
  Agents[AgentId] = Agent;
  DrawAgentSearchRange(Agent);
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
  float TrailDecay = 0.99;
  for(int x=-Range; x<=Range; x++) {
    for(int y=-Range; y<=Range; y++) 
    {
      float2 TexIdx = (id.xy + int2(x, y))/(float)UTexRes;
      Avg += TexRead.SampleLevel(TexReadSampler, TexIdx, 0.0).r;
    } }
  
  Avg /= (Range*2.0+1.0)*(Range*2.0+1.0);
  oc   = Avg*TrailDecay;
  oc   = clamp(oc, 0.0, 1.0);
  //Wierd Mousething ignored
  float2 st = (2.0*id.xy-float2(UTexRes.x, UTexRes.y))/UTexRes.y;
  float  px = 3.0/UTexRes.y;
  float t = UTime*0.001;
  st = mul(st, float2x2(cos(t), sin(t), -sin(t), cos(t)));
  float Poison = 1.0*smoothstep(0.0,0.1,abs(length(st+0.1)-0.5*sin(t)))-0.1-noise2(3.0*st-t)*0.9*sin(UTime*0.01);
  TexWrite[id.xy] = Poison<0.0?Poison:oc; //oc;
}
[numthreads(PIXELS_PER_THREADGROUP, PIXELS_PER_THREADGROUP, 1)]
void KernelRender(uint3 id : SV_DispatchThreadID)
{
  if(id.x >= UTexRes.x || id.y >= UTexRes.x) return;
  float3 col = 0.0;
  //ESSENTIAL
  float2 res = float2(UTexRes.x, UTexRes.y);
  float2 uv = id.xy/res.xy;
  float2 st = (2.0*id.xy-res.xy)/res.y;
  float  px = 2.0/UTexRes.y;
  //SHADING
  float Sample = TexRead[id.xy].r;
  float3 matte = Sample;
  col = matte;
  //Sample2+
  //POST
  col += smoothstep(0.0, px, sdBox(st, 0.999));
  col += TexDebug[id.xy].rgb*0.001;
  col  = pow(max(0.0,col), 0.4545); //gamma correction
  //OUTPUT
  TexRender[id.xy] = float4(col.r, col.g, col.b, 1.0);
  TexDebug [id.xy] = 0.0;
}
[numthreads(AGENTS_PER_THREADGROUP, 1, 1)]
void KernelAgentsDebug(uint3 id : SV_DispatchThreadID)
{
  uint AgentId = id.x;
  if(AgentId >= UAgentCount) return;
  agent Agent = Agents[AgentId];
  //TexRender[round(Agent.Pos)] = float4(0.3, 0.2+0.8*AgentId/UAgentCount, 0.4, 1.0);
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
  float theta = float(UTime*0.01);
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
  float Vignetting = smoothstep(0.3,0.0, length(st)-0.9);
  float4 Color = float4((Texture.Sample(Sampler, Input.UV.xy).xyz), 1.0);
  return Color*Vignetting;
}
