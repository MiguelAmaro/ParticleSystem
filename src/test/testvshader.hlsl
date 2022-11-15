struct test_vert
{
  float4 Pos;
  float4 Color;
};

struct VS_INPUT
{
  float4 Pos   : IAPOS;
  float4 Color : IACOL;
};

struct GS_INPUT
{
  float4 Pos : SV_POSITION;
};

StructuredBuffer<test_vert> VertState : register( t0 );

GS_INPUT VSMAIN(VS_INPUT Input, uint VertID : SV_VertexID)
{
  GS_INPUT Output;
  // NOTE(MIGUEL): Vertex data is not coming thru the input assembler. It is stored as point data in
  //               the structured buffer, SimState. 
  test_vert Vert = VertState[VertID];
  Vert.Pos.y = sin(Vert.Pos.y);
  Output.Pos = float4(Vert.Pos.xyz, 1.0);
  return Output;
}
