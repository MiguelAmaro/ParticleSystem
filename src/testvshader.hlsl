struct VS_INPUT
{
  float4 Pos   : IAPOS;
  float4 Color : IACOL;
};

struct GS_INPUT
{
  float4 Pos : SV_POSITION;
};

GS_INPUT VSMAIN(VS_INPUT Input)
{
  GS_INPUT Output;
  // NOTE(MIGUEL): Vertex data is not coming thru the input assembler. It is stored as point data in
  //               the structured buffer, SimState. 
  Output.Pos = Input.Pos;
  return Output;
}
