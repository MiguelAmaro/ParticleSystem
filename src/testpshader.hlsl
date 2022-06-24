struct PS_INPUT                                            
{                                                          
  float4 Pos   : SV_POSITION;
  float4 UV    : TEXCOORD;                                 
  float4 Color : COLOR;
};                                                         

float4 PSMAIN(PS_INPUT Input) : SV_TARGET                      
{  
  return Input.Color;
}
