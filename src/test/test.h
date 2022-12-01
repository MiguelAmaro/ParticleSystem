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
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  d3d11_shader Geometry;
  d3d11_shader Compute;
  test_vert Vertices[10];
  u32 VertexMaxCount;
};
fn testrend CreateTestRenderer(d3d11_base* Base)
{
  D3D11BaseDestructure(Base);
  testrend Test = {0};
  Test.VertexMaxCount = 10;
  for(u32 i=0; i<Test.VertexMaxCount; i++)
  { Test.Vertices[i].Pos = V4f(0.1f*i, 0.0f, 0.0f, 1.0f); }
  
  D3D11ScopedBase(Base)
  {
    D3D11BufferVertex(&Test.VBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
    // This RW buffer is a duplicate of vetex buffer that was used for the quad expansion
    // except it will get updated via the compute shader. Updated data won't be used for anything
    // beside get copied to the staging buffer has nothing to do with the vertex buffer.
    D3D11BufferStructUA(&Test.RWStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
    // This is a view for the RW buffers so it data is accessable from the shader
    D3D11BufferViewUA(Device, &Test.RWStructBufferView, Test.RWStructBuffer, Test.VertexMaxCount);
    
    // This is a staging buffer so its data can be read from the cpu. The contents of the RWStructBuffer will be 
    // copied to this buffer. 
    D3D11BufferStaging(&Test.DbgStageBuffer, Test.Vertices, Test.VertexMaxCount*sizeof(test_vert));
    // This is view of the RWBuffer for the vertex shader that s
    D3D11BufferViewSR(Device, &Test.VertexResView, Test.RWStructBuffer, Test.VertexMaxCount);
    
    // Append Consume Buffer settup.
    D3D11BufferStructUA(&Test.AppendStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
    D3D11BufferViewUAAppend(Device, &Test.AppendView, Test.AppendStructBuffer, Test.VertexMaxCount);
    D3D11BufferStructUA(&Test.ConsumeStructBuffer, Test.Vertices, sizeof(test_vert), Test.VertexMaxCount);
    D3D11BufferViewUAAppend(Device, &Test.ConsumeView, Test.ConsumeStructBuffer, Test.VertexMaxCount);
  }
  
  // Shaders
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(test_vert, Pos  ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IACOL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(test_vert, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 VSrc = Str8("F:\\Dev\\ParticleSystem\\src\\test\\testvshader.hlsl");
  str8 PSrc = Str8("F:\\Dev\\ParticleSystem\\src\\test\\testpshader.hlsl");
  str8 GSrc = Str8("F:\\Dev\\ParticleSystem\\src\\test\\testgshader.hlsl");
  str8 CSrc = Str8("F:\\Dev\\ParticleSystem\\src\\test\\testcshader.hlsl");
  Test.Vertex   = D3D11ShaderCreate(ShaderKind_Vertex, VSrc, Str8("VSMAIN"), Desc, ArrayCount(Desc), Base);
  Test.Pixel    = D3D11ShaderCreate(ShaderKind_Pixel, PSrc, Str8("PSMAIN"), NULL, 0, Base);
  Test.Geometry = D3D11ShaderCreate(ShaderKind_Geometry, GSrc, Str8("GSMAIN"), NULL, 0, Base);
  Test.Compute  = D3D11ShaderCreate(ShaderKind_Compute, CSrc, Str8("CSMAIN"), NULL, 0, Base);
  
  return Test;
}

void TestDraw(testrend *Test, d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  {
    //~ COMPUTE PASS
    ID3D11DeviceContext_CSSetShader(Context, Test->Compute.ComputeHandle, NULL, 0);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Test->RWStructBufferView, &Test->VertexMaxCount);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, &Test->ConsumeView, &Test->VertexMaxCount);
    ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Test->AppendView, 0);
    //ID3D11DeviceContext_CSSetShaderResources(); // Don't think this is needed. its a view but write only.
    // Only 1 thead group needed. The thread group in the shader will have 10 thread in a group. So one
    // thead can be used to process each of the 10 verticies.
    ID3D11DeviceContext_Dispatch(Context, 1, 1, 1);
    D3D11ClearComputeStage(Context);
  }
  Assert(Test->VertexMaxCount == 10); //This test requires the the number of vertices is 10.
  arena Arena; ArenaLocalInit(Arena, 4096*2);
  u32        Count = Test->VertexMaxCount;
  test_vert *Verts = D3D11BufferRead(Test->RWStructBuffer, Test->DbgStageBuffer, sizeof(test_vert), Test->VertexMaxCount, &Arena);
  ConsoleLog("D3D11 Debug Results:\n");
  foreach(VertId, Count, u32)
  {
    v4f Vert = Verts[VertId].Pos;
    ConsoleLog(Arena, "[%d] {x: %f, y: %f, z: %f, w: %f}\n", VertId, Vert.x, Vert.y, Vert.z, Vert.w);
  }
  // Compute Cleanup
  //ID3D11UnorderedAccessView *NullUAView = NULL;
  //ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &NullUAView, 0);
  //~ DRAW PASS
  UINT Stride = sizeof(test_vert);
  UINT Offset = 0;
  //Input Assempler
  ID3D11DeviceContext_IASetInputLayout(Context, Test->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Test->VBuffer, &Stride, &Offset);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  //Shader
  ID3D11DeviceContext_VSSetShaderResources(Context, 0, 1, &Test->VertexResView);
  ID3D11DeviceContext_VSSetShader(Context, Test->Vertex.VertexHandle, NULL, 0);
  ID3D11DeviceContext_GSSetShader(Context, Test->Geometry.GeometryHandle, NULL, 0);
  ID3D11DeviceContext_PSSetShader(Context, Test->Pixel.PixelHandle, NULL, 0);
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
