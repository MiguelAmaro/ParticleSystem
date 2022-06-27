struct test_vert
{
  float4 Pos;
  float4 Color;
};

// NOTE(MIGUEL): 
// The vert count in the structured buffer is know to and guarenteed to be 
// the same(10 verts) to the app and this shader. If that wasn't the
// case the would need to be a constant buffer set by the app with the
// number of verts

RWStructuredBuffer<test_vert> VertState : register( u0 );
AppendStructuredBuffer<test_vert> VertStateApp : register( u1 );
ConsumeStructuredBuffer<test_vert> VertStateCon : register( u2 );

[numthreads( 10, 1, 1)]
void CSMAIN( uint3 DispatchThreadID : SV_DispatchThreadID  )
{
  uint CurrThreadID = (DispatchThreadID.x +
                       DispatchThreadID.y * 10 +
                       DispatchThreadID.z * 10 * 10);
  
  VertState[CurrThreadID].Pos += 10.0;
  test_vert Vert = VertStateCon.Consume();
  Vert.Pos += 30.0;
  VertStateApp.Append(Vert);
}