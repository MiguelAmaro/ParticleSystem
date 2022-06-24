#ifndef TEST_H
#define TEST_H

/* NOTE(MIGUEL): Thea are my goals for testing the rendering pipleind.
*                1. VShader -> Gshader communication
*                   Feed pointlist via input assemble and have geometry shader generate quads.
*                   then render normaly. with vertex buffer with position in clip space.
*                2. Staging buffer writing. Write to a staging buffer and printf result.
*                3. TBD...
*                \
*                
*/

typedef struct test_vert test_vert;
struct test_vert
{
  v4f Pos;
  v4f Color;
};

typedef struct testrend testrend;
struct testrend
{
  ID3D11Buffer *VBuffer;
  ID3D11VertexShader *VShader;
  ID3D11PixelShader *PShader;
  ID3D11GeometryShader *GShader;
  ID3D11InputLayout   *Layout;
  test_vert Vertices[10];
  u32 VertexMaxCount;
};

testrend CreateTestRenderer(ID3D11Device* Device, ID3D11DeviceContext * Context)
{
  testrend Test = {0};
  Test.VertexMaxCount = 10;
  for(u32 i=0; i<Test.VertexMaxCount; i++)
  {
    Test.Vertices[i].Pos = V4f(0.1f*i, 0.0f, 0.0f, 1.0f);
  }
  
  {
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Test.VBuffer);
  }
  FILE *VShaderFile;
  FILE *PShaderFile;
  FILE *GShaderFile;
  fopen_s(&VShaderFile, (const char *)"F:\\Dev\\ParticleSystem\\src\\testvshader.hlsl", "rb");
  fopen_s(&PShaderFile, (const char *)"F:\\Dev\\ParticleSystem\\src\\testpshader.hlsl", "rb");
  fopen_s(&GShaderFile, (const char *)"F:\\Dev\\ParticleSystem\\src\\testgshader.hlsl", "rb");
  str8 VShaderSrc = Str8FromFile(VShaderFile);
  str8 PShaderSrc = Str8FromFile(PShaderFile);
  str8 GShaderSrc = Str8FromFile(GShaderFile);
  HRESULT hr;
  ID3DBlob* error;
  ID3DBlob* VShaderBlob;
  ID3DBlob* PShaderBlob;
  ID3DBlob* GShaderBlob;
  UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  hr = D3DCompile(VShaderSrc.Data, VShaderSrc.Size, NULL, NULL, NULL, "VSMAIN", "vs_5_0", flags, 0, &VShaderBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main compute shader!");
  }
  hr = D3DCompile(PShaderSrc.Data, PShaderSrc.Size, NULL, NULL, NULL, "PSMAIN", "ps_5_0", flags, 0, &PShaderBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main compute shader!");
  }
  hr = D3DCompile(GShaderSrc.Data, GShaderSrc.Size, NULL, NULL, NULL, "GSMAIN", "gs_5_0", flags, 0, &GShaderBlob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile particle sys main compute shader!");
  }
  {
    D3D11_INPUT_ELEMENT_DESC Desc[] =
    {
      { "IAPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(test_vert, Pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "IACOL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(test_vert, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ID3D11Device_CreateInputLayout(Device, Desc, ARRAYSIZE(Desc), ID3D10Blob_GetBufferPointer(VShaderBlob), ID3D10Blob_GetBufferSize(VShaderBlob), &Test.Layout);
  }
  ID3D11Device_CreateVertexShader(Device, ID3D10Blob_GetBufferPointer(VShaderBlob), ID3D10Blob_GetBufferSize(VShaderBlob), NULL, &Test.VShader);
  ID3D11Device_CreatePixelShader(Device, ID3D10Blob_GetBufferPointer(PShaderBlob), ID3D10Blob_GetBufferSize(PShaderBlob), NULL, &Test.PShader);
#if 0
  {
    D3D11_SO_DECLARATION_ENTRY Decl[] =
    {
      // semantic name, semantic index, start component, component count, output slot
      {0, "SV_POSITION", 0, 0, 4, 0 },   // output all components of position
    };
    
    ID3D11Device_CreateGeometryShaderWithStreamOutput(Device, ID3D10Blob_GetBufferPointer(GShaderBlob), ID3D10Blob_GetBufferSize(GShaderBlob), Decl, 
                                                      sizeof(Decl), NULL, 0, 0, NULL, &Test.GShader );
  }
#else
  ID3D11Device_CreateGeometryShader(Device, ID3D10Blob_GetBufferPointer(GShaderBlob), ID3D10Blob_GetBufferSize(GShaderBlob), NULL, &Test.GShader);
#endif
  ID3D10Blob_Release(VShaderBlob);
  ID3D10Blob_Release(PShaderBlob);
  ID3D10Blob_Release(GShaderBlob);
  return Test;
}

void TestDraw(testrend *Test, ID3D11DeviceContext * Context, b32 IsFirstFrame,
              D3D11_VIEWPORT *Viewport,
              ID3D11RasterizerState* RastState,
              ID3D11DepthStencilState* DepthState,
              ID3D11BlendState* BlendState, 
              ID3D11RenderTargetView* RTView,
              ID3D11DepthStencilView* DSView)
{
  UINT Stride = sizeof(test_vert);
  UINT Offset = 0;
  //Input Assempler
  ID3D11DeviceContext_IASetInputLayout(Context, Test->Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Test->VBuffer, &Stride, &Offset);
  
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  
  //Shader
  ID3D11DeviceContext_VSSetShader(Context, Test->VShader, NULL, 0);
  ID3D11DeviceContext_GSSetShader(Context, Test->GShader, NULL, 0);
  ID3D11DeviceContext_PSSetShader(Context, Test->PShader, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  // draw 3 vertices
  ID3D11DeviceContext_Draw(Context, 10, 0);
  ID3D11DeviceContext_GSSetShader(Context, NULL, NULL, 0);
  return;
}

#endif //TEST_H
