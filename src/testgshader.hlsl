struct PS_INPUT                                            
{                                                          
  float4 Pos   : SV_POSITION;
  float4 UV    : TEXCOORD;                                 
  float4 Color : COLOR;
};                                                         

struct GS_INPUT
{
  float4 Pos : SV_POSITION;
};

static const float4 QuadOffset[4] =
{
  float4(- 0.04,  0.04, 0.0, 0.0 ),
  float4(  0.04,  0.04, 0.0, 0.0 ),
  float4( -0.04, -0.04, 0.0, 0.0 ),
  float4(  0.04, -0.04, 0.0, 0.0 ),
};

static const float4 CornerColors[4] =
{
  float4( 1.0,  0.0, 0.0, 0.5 ),
  float4( 0.0,  1.0, 0.0, 0.8 ),
  float4( 0.0,  0.0, 1.0, 0.8 ),
  float4( 1.0,  0.0, 1.0, 0.5 ),
};

[maxvertexcount(4)]
void GSMAIN(point GS_INPUT Input[1], inout TriangleStream<PS_INPUT> SpriteStream )
{
  PS_INPUT Output;
  for(int i=0;i<4;i++)
  {
    Output.Pos    = Input[0].Pos + QuadOffset[i];
    Output.UV     = float4(0.0, 0.0, -1.0, 0.0);
    Output.Color  = CornerColors[i];
    SpriteStream.Append(Output);
  }
  SpriteStream.RestartStrip();
}