//~ COMPUTE SHADER
struct particle
{
  float3 Pos;
  float3 Vel;
  float Time;
};

AppendStructuredBuffer<particle> NewSimState : register( u0 );
ConsumeStructuredBuffer<particle> CurrSimState : register( u1 );

cbuffer sim_params
{
  float4 EmitterLocation;
  float4 ConsumerLocation;
  float4 TimeFactors;
};

cbuffer particle_count
{
  uint4 NumParticles;
};

static const float G = 10.0f;
static const float m1 = 1.0f;
static const float m2 = 1000.0f;
static const float m1m2 = m1 * m2;
static const float EventHorizon = 5.0f;

[numthreads( 512, 1, 1)]
void CSMAIN( uint3 DispatchThreadID : SV_DispatchThreadID  )
{
  // Check for if this thread should run or not.
  uint MyID = DispatchThreadID.x
    + DispatchThreadID.y * 512
    + DispatchThreadID.z * 512 * 512;
  if ( MyID < NumParticles.x )
  {
    // Perform update process here...
    // Get the current particle
    particle Particle = CurrSimState.Consume();
    // Calculate the current gravitational force applied to it
    float3 d = ConsumerLocation.xyz - Particle.Pos;
    float r = length( d );
    float3 Force = ( G * m1m2 / (r*r) ) * normalize( d );
    // Calculate the new velocity, accounting for the acceleration from
    // the gravitational force over the current time step.
    Particle.Vel = Particle.Vel + ( Force / m1 ) * TimeFactors.x;
    // Calculate the new position, accounting for the new velocity value
    // over the current time step.
    Particle.Pos += Particle.Vel * TimeFactors.x;
    // Update the life time left for the particle,
    Particle.Time = Particle.Time + TimeFactors.x;
    // Test to see how close the particle is to the black hole, and
    // don't pass it to the output list if it is too close,
    if ( r > EventHorizon )
    {
      if ( Particle.Time < 10.0f )
      {
        NewSimState.Append( Particle );
      }
    }
  }
}

//~ VERTEX SHADER
struct VS_INPUT
{
  uint VertId : SV_VertexID;
};

struct GS_INPUT
{
  float4 position : SV_POSITION;
};

StructuredBuffer<particle> SimState;

GS_INPUT VSMAIN( in VS_INPUT input )
{
  GS_INPUT output;
  // NOTE(MIGUEL): Vertex data is not coming thru the input assembler. It is stored as point data in
  //               the structured buffer, SimState. 
  output.position = float4(SimState[input.VertId].Pos, 1.0);
  return output;
}


//~ GEOMETRY SHADER
#if 1
// NOTE(MIGUEL): (ignore)Data visiable via sim_params
// NOTE(MIGUEL): sim_params is bound to the compute shader stage so this copy seems neccisary
cbuffer particle_render_params
{
  float4 RenderEmitterLocation;
  float4 RenderConsumerLocation;
};
#endif

cbuffer transforms : register(b0)                             // b0 = constant buffer bound to slot 0
{                                                          
  float4x4 WorldViewMatrix;                                   
  float4x4 ProjMatrix;                                   
}                                                          

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float2 tex : TexCoord;
  float4 color : Color;
};

static const float4 GlobalPos[4] =
{
  float4(- 1, 1, 0, 0 ) ,
  float4( 1, 1, 0, 0 ),
  float4( -1, -1, 0, 0 ),
  float4( 1, -1, 0, 0 ),
};

static const float2 GlobalTexCoords[ 4 ] =
{
  float2( 0, 1 ),
  float2( 1, 1 ),
  float2( 0, 0 ),
  float2( 1, 0 ),
};

[maxvertexcount(4)]
void GSMAIN(point GS_INPUT input[1], inout TriangleStream<PS_INPUT> SpriteStream )
{
  PS_INPUT Output;
  float dist = saturate(length(input[0].position-RenderConsumerLocation)
                        /100.0f);
  float4 color = float4(0.2f,0.2f,1.0f,0.0f)*(dist) + float4(1.0f,0.2f,0.2f,0.0f)*(1.0f-dist);
  // Transform to view space
  float4 viewposition = mul(input[0].position,
                            WorldViewMatrix);
  // Emit two new triangles
  for(int i=0;i<4;i++)
  {
    // Transform to clip space
    Output.position = mul( viewposition + GlobalPos[i], ProjMatrix );
    Output.tex = GlobalTexCoords[i];
    Output.color = color;
    SpriteStream.Append(Output);
  }
  SpriteStream.RestartStrip();
}

//~ PIXEL SHADER
Texture2D ParticleTexture : register( t0 );
SamplerState LinearSampler : register( s0 );
float4 PSMAIN( in PS_INPUT input ) : SV_Target
{
  float4 color = ParticleTexture.Sample( LinearSampler, input.tex );
  color = color * input.color;
  return( color );
}
