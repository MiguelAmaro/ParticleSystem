/*// NOTE(MIGUEL): This compute shader satisfied the need for the gpu to have the 
*                  ability to generate new particles dynamicall with out cpu. new
*                  particles are appended to the output buffer resource.
*                  
*                  
*/

struct particle
{
  float3 Pos;
  float3 Dir;
  float Time;
};

AppendStructuredBuffer<particle> NewSimState : register( u0 );

cbuffer particle_params
{
  float4 EmitterLocation;
  float4 RandomVector;
};

static const float3 Direction[8] =
{
  normalize( float3( 1.0f, 1.0f, 1.0f ) ),
  normalize( float3( -1.0f, 1.0f, 1.0f ) ),
  normalize( float3( -1.0f, -1.0f, 1.0f ) ),
  normalize( float3( 1.0f, -1.0f, 1.0f ) ),
  normalize( float3( 1.0f, 1.0f, -1.0f ) ),
  normalize( float3( -1.0f, 1.0f, -1.0f ) ),
  normalize( float3( -1.0f, -1.0f, -1.0f ) ),
  normalize( float3( 1.0f, -1.0f, -1.0f ) )
};

[numthreads( 8, 1, 1)]
void CSHELPER( uint3 GroupThreadID : SV_GroupThreadID )
{
  particle Particle;
  // Initialize position to the current emitter location
  Particle.Pos = EmitterLocation.xyz;
  // Initialize direction to a randomly reflected vector
  Particle.Dir = reflect( Direction[GroupThreadID.x], RandomVector.xyz ) * 5.0f;
  // Initialize the lifetime of the particle in seconds
  Particle.Time = 0.0f;
  // Append the new particle to the output buffer
  NewSimState.Append( Particle );
}

