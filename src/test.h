#ifndef TEST_H
#define TEST_H

/* NOTE(MIGUEL): Thea are my goals for testing the rendering pipleind.
*                1. VShader -> Gshader communication
*                   Feed pointlist via input assemble and have geometry shader generate quads.
*                   then render normaly. with vertex buffer with position in clip space.
*                2. Staging buffer writing. Write to a staging buffer and printf result.
*                3. Compute shader update with read RW structured buffer
*                3. Compute shader update Append Consume. Verify that write are happening to the buffer
*                
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
  ID3D11Buffer  *VBuffer;
  ID3D11Buffer  *RWStructBuffer;
  ID3D11Buffer  *AppendStructBuffer;
  ID3D11Buffer  *ConsumeStructBuffer;
  ID3D11Buffer  *DbgStageBuffer;
  ID3D11ShaderResourceView  *VertexResView;
  ID3D11UnorderedAccessView *RWStructBufferView;
  ID3D11UnorderedAccessView *AppendView;
  ID3D11UnorderedAccessView *ConsumeView;
  ID3D11ComputeShader *CShader;
  ID3D11VertexShader *VShader;
  ID3D11PixelShader *PShader;
  ID3D11GeometryShader *GShader;
  ID3D11InputLayout   *Layout;
  test_vert Vertices[10];
  u32 VertexMaxCount;
};

ID3DBlob *CompileD3D11Shader(char *ShaderFileDir, const char *ShaderEntry, const char *ShaderTypeAndVer)
{
  ID3DBlob *ShaderBlob, *Error;
  HRESULT Hr;
  FILE *ShaderFile;
  fopen_s(&ShaderFile, ShaderFileDir, "rb");
  u8 Buffer[4096*2];
  arena Arena = ArenaInit(&Arena, 4096*2, &Buffer);
  str8 ShaderSrc = OSFileRead(Str8(ShaderFileDir), &Arena);
  UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  Hr = D3DCompile(ShaderSrc.Data, ShaderSrc.Size, NULL, NULL, NULL,
                  ShaderEntry, ShaderTypeAndVer, flags, 0, &ShaderBlob, &Error);
  if (FAILED(Hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(Error);
    OutputDebugStringA(message);
    Assert(!"[TestCode]: Failed to load shader of type [meh] !!!");
  }
  return ShaderBlob;
}

testrend CreateTestRenderer(ID3D11Device* Device, ID3D11DeviceContext * Context)
{
  testrend Test = {0};
  Test.VertexMaxCount = 10;
  for(u32 i=0; i<Test.VertexMaxCount; i++)
  { Test.Vertices[i].Pos = V4f(0.1f*i, 0.0f, 0.0f, 1.0f); }
  
  
  D3D11VertexBuffer(Device, &Test.VBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
  // This RW buffer is a duplicate of vetex buffer that was used for the quad expansion
  // except it will get updated via the compute shader. Updated data won't be used for anything
  // beside get copied to the staging buffer has nothing to do with the vertex buffer.
  D3D11StructuredBuffer(Device, &Test.RWStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
  // This is a view for the RW buffers so it data is accessable from the shader
  D3D11BufferViewUA(Device, &Test.RWStructBufferView, Test.RWStructBuffer, Test.VertexMaxCount);
  
  // This is a staging buffer so its data can be read from the cpu. The contents of the RWStructBuffer will be 
  // copied to this buffer. 
  D3D11StageBuffer(Device, &Test.DbgStageBuffer, Test.Vertices, Test.VertexMaxCount*sizeof(test_vert));
  // This is view of the RWBuffer for the vertex shader that s
  D3D11BufferViewSR(Device, &Test.VertexResView, Test.RWStructBuffer, Test.VertexMaxCount);
  
  // Append Consume Buffer settup.
  D3D11StructuredBuffer  (Device, &Test.AppendStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
  D3D11BufferViewUAAppend(Device, &Test.AppendView, Test.AppendStructBuffer, Test.VertexMaxCount);
  D3D11StructuredBuffer(Device, &Test.ConsumeStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
  D3D11BufferViewUAAppend(Device, &Test.ConsumeView, Test.ConsumeStructBuffer, Test.VertexMaxCount);
  // Shaders
  ID3DBlob* VShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\testvshader.hlsl", "VSMAIN", "vs_5_0", "Test System");
  ID3DBlob* PShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\testpshader.hlsl", "PSMAIN", "ps_5_0", "Test System");
  ID3DBlob* GShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\testgshader.hlsl", "GSMAIN", "gs_5_0", "Test System");
  ID3DBlob* CShaderBlob = D3D11LoadAndCompileShader("F:\\Dev\\ParticleSystem\\src\\testcshader.hlsl", "CSMAIN", "cs_5_0", "Test System");
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
  ID3D11Device_CreateGeometryShader(Device, ID3D10Blob_GetBufferPointer(GShaderBlob), ID3D10Blob_GetBufferSize(GShaderBlob), NULL, &Test.GShader);
  ID3D11Device_CreateComputeShader(Device, ID3D10Blob_GetBufferPointer(CShaderBlob), ID3D10Blob_GetBufferSize(CShaderBlob), NULL, &Test.CShader);
  ID3D10Blob_Release(VShaderBlob);
  ID3D10Blob_Release(PShaderBlob);
  ID3D10Blob_Release(GShaderBlob);
  ID3D10Blob_Release(CShaderBlob);
  return Test;
}

void TestDraw(testrend *Test, d3d11_base *Base)
{
  ID3D11DeviceContext     *Context    = Base->Context;
  D3D11_VIEWPORT           Viewport   = Base->Viewport;
  ID3D11RasterizerState   *RastState  = Base->RasterizerState;
  ID3D11DepthStencilState *DepthState = Base->DepthState;
  ID3D11BlendState        *BlendState = Base->BlendState; 
  ID3D11RenderTargetView  *RTView     = Base->RTView;
  ID3D11DepthStencilView  *DSView     = Base->DSView;
  //~ COMPUTE PASS
  ID3D11DeviceContext_CSSetShader(Context, Test->CShader, NULL, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Test->RWStructBufferView, &Test->VertexMaxCount);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Test->ConsumeView, &Test->VertexMaxCount);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Test->AppendView, 0);
  //ID3D11DeviceContext_CSSetShaderResources(); // Don't think this is needed. its a view but write only.
  // Only 1 thead group needed. The thread group in the shader will have 10 thread in a group. So one
  // thead can be used to process each of the 10 verticies.
  ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
  Assert(Test->VertexMaxCount == 10); //This test requires the the number of vertices is 10.
  u8 Buffer[4096*2];
  arena Arena     = ArenaInit(&Arena, 4096*2, &Buffer);
  u32        Count = Test->VertexMaxCount;
  test_vert *Verts = D3D11ReadBuffer(Context, Test->RWStructBuffer, Test->DbgStageBuffer,
                                     sizeof(test_vert), Test->VertexMaxCount, &Arena);
  ConsoleLog("D3D11 Debug Results:\n");
  foreach(VertId, (s32)Count)
  {
    v4f Vert = Verts[VertId].Pos;
    ConsoleLog(Arena, "[%d] {x: %f, y: %f, z: %f, w: %f}\n", VertId, Vert.x, Vert.y, Vert.z, Vert.w);
  }
  
  // Compute Cleanup
  ID3D11UnorderedAccessView *NullUAView = NULL;
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &NullUAView, 0);
  //~ DRAW PASS
  UINT Stride = sizeof(test_vert);
  UINT Offset = 0;
  //Input Assempler
  ID3D11DeviceContext_IASetInputLayout(Context, Test->Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Test->VBuffer, &Stride, &Offset);
  
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  
  //Shader
  ID3D11DeviceContext_VSSetShaderResources(Context, 0, 1, &Test->VertexResView);
  ID3D11DeviceContext_VSSetShader(Context, Test->VShader, NULL, 0);
  ID3D11DeviceContext_GSSetShader(Context, Test->GShader, NULL, 0);
  ID3D11DeviceContext_PSSetShader(Context, Test->PShader, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  // draw 3 vertices
  ID3D11DeviceContext_Draw(Context, 10, 0);
  // Render Cleanup
  ID3D11ShaderResourceView *NullResView = NULL;
  ID3D11DeviceContext_VSSetShaderResources(Context, 0, 1, &NullResView);
  ID3D11DeviceContext_GSSetShader(Context, NULL, NULL, 0);
  D3D11ClearPipeline(Context);
  return;
}

#endif //TEST_H
