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

ID3DBlob *CompileD3D11Shader(const char *ShaderFileDir, const char *ShaderEntry, const char *ShaderTypeAndVer)
{
  ID3DBlob *ShaderBlob, *Error;
  HRESULT Hr;
  FILE *ShaderFile;
  fopen_s(&ShaderFile, ShaderFileDir, "rb");
  str8 ShaderSrc = Str8FromFile(ShaderFile);
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
  
  {
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA Initial;
    Initial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &Desc, &Initial, &Test.VBuffer);
  }
  {
    // This RW buffer is a duplicate of vetex buffer that was used for the quad expansion
    // except it will get updated via the compute shader. Updated data won't be used for anything
    // beside get copied to the staging buffer has nothing to do with the vertex buffer.
    D3D11_BUFFER_DESC BufferDesc = {0};
    BufferDesc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    BufferDesc.StructureByteStride = sizeof(test_vert);
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA RWInitial;
    RWInitial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &BufferDesc, &RWInitial, &Test.RWStructBuffer);
    // This is a view for the RW buffers so it data is accessable from the shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC ViewDesc = {0};
    ViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    ViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    ViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
    ViewDesc.Buffer.FirstElement = 0;
    ViewDesc.Buffer.NumElements = Test.VertexMaxCount;
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Test.RWStructBuffer, &ViewDesc, &Test.RWStructBufferView);
    // This is a staging buffer so its data can be read from the cpu. The contents of the RWStructBuffer will be 
    // copied to this buffer. 
    D3D11_BUFFER_DESC Desc = {0};
    Desc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    Desc.BindFlags = 0; //Staging buffer are not ment to be bount to any stage in the pipeline
    Desc.Usage = D3D11_USAGE_STAGING;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    D3D11_SUBRESOURCE_DATA StageInitial;
    StageInitial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &Desc, &StageInitial, &Test.DbgStageBuffer);
  }
  {
    // Append Consume Buffer settup.
    D3D11_BUFFER_DESC AppendBufferDesc = {0};
    AppendBufferDesc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    AppendBufferDesc.StructureByteStride = sizeof(test_vert);
    AppendBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    AppendBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    AppendBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    AppendBufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA AppendInitial;
    AppendInitial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &AppendBufferDesc, &AppendInitial, &Test.AppendStructBuffer);
    D3D11_UNORDERED_ACCESS_VIEW_DESC AppendViewDesc = {0};
    AppendViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    AppendViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    AppendViewDesc.Buffer.FirstElement = 0;
    AppendViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    AppendViewDesc.Buffer.NumElements = Test.VertexMaxCount;
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Test.AppendStructBuffer, &AppendViewDesc, &Test.AppendView);
    D3D11_BUFFER_DESC ConsumeBufferDesc = {0};
    ConsumeBufferDesc.ByteWidth = Test.VertexMaxCount*sizeof(test_vert);
    ConsumeBufferDesc.StructureByteStride = sizeof(test_vert) ;
    ConsumeBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    ConsumeBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    ConsumeBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    ConsumeBufferDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA ConsumeInitial;
    ConsumeInitial.pSysMem = Test.Vertices;
    ID3D11Device_CreateBuffer(Device, &ConsumeBufferDesc, &ConsumeInitial, &Test.ConsumeStructBuffer);
    D3D11_UNORDERED_ACCESS_VIEW_DESC ConsumeViewDesc = {0};
    ConsumeViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    ConsumeViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    ConsumeViewDesc.Buffer.FirstElement = 0;
    ConsumeViewDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    ConsumeViewDesc.Buffer.NumElements = Test.VertexMaxCount;
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Test.ConsumeStructBuffer, &ConsumeViewDesc, &Test.ConsumeView);
  }
  ID3DBlob* VShaderBlob = CompileD3D11Shader("F:\\Dev\\ParticleSystem\\src\\testvshader.hlsl", "VSMAIN", "vs_5_0");
  ID3DBlob* PShaderBlob = CompileD3D11Shader("F:\\Dev\\ParticleSystem\\src\\testpshader.hlsl", "PSMAIN", "ps_5_0");
  ID3DBlob* GShaderBlob = CompileD3D11Shader("F:\\Dev\\ParticleSystem\\src\\testgshader.hlsl", "GSMAIN", "gs_5_0");
  ID3DBlob* CShaderBlob = CompileD3D11Shader("F:\\Dev\\ParticleSystem\\src\\testcshader.hlsl", "CSMAIN", "cs_5_0");
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

void TestDraw(testrend *Test, ID3D11DeviceContext * Context, b32 IsFirstFrame,
              D3D11_VIEWPORT *Viewport,
              ID3D11RasterizerState* RastState,
              ID3D11DepthStencilState* DepthState,
              ID3D11BlendState* BlendState, 
              ID3D11RenderTargetView* RTView,
              ID3D11DepthStencilView* DSView)
{
  //~ COMPUTATION
  ID3D11DeviceContext_CSSetShader(Context, Test->CShader, NULL, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Test->RWStructBufferView, &Test->VertexMaxCount);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Test->ConsumeView, &Test->VertexMaxCount);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Test->AppendView, 0);
  //ID3D11DeviceContext_CSSetShaderResources(); // Don't think this is needed. its a view but write only.
  // Only 1 thead group needed. The thread group in the shader will have 10 thread in a group. So one
  // thead can be used to process each of the 10 verticies.
  Assert(Test->VertexMaxCount == 10); //This test requires the the number of vertices is 10.
  ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
  //For staging/debug
  ID3D11DeviceContext_CopyResource(Context,
                                   (ID3D11Resource*)Test->DbgStageBuffer,
                                   (ID3D11Resource*)Test->RWStructBuffer);
  D3D11_MAPPED_SUBRESOURCE MappedBuffer =  {0};
  ID3D11DeviceContext_Map(Context, (ID3D11Resource *)Test->DbgStageBuffer, 0,
                          D3D11_MAP_READ, 0, &MappedBuffer);
  MemoryCopy(MappedBuffer.pData, sizeof(test_vert)*Test->VertexMaxCount,
             Test->Vertices    , sizeof(test_vert)*Test->VertexMaxCount);
  ID3D11DeviceContext_Unmap(Context, (ID3D11Resource *)Test->DbgStageBuffer, 0);
#if 0
  //For staging/debug
  ID3D11DeviceContext_CopyResource(Context,
                                   (ID3D11Resource*)Test->DbgStageBuffer,
                                   (ID3D11Resource*)Test->RWStructBuffer);
  D3D11_MAPPED_SUBRESOURCE MappedBuffer =  {0};
  ID3D11DeviceContext_Map(Context, (ID3D11Resource *)Test->DbgStageBuffer, 0,
                          D3D11_MAP_READ, 0, &MappedBuffer);
  MemoryCopy(MappedBuffer.pData, sizeof(test_vert)*Test->VertexMaxCount,
             Test->Vertices    , sizeof(test_vert)*Test->VertexMaxCount);
  ID3D11DeviceContext_Unmap(Context, (ID3D11Resource *)Test->DbgStageBuffer, 0);
#endif
  //~ RENDER
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
