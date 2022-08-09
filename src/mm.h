#ifndef MM_H
#define MM_H


typedef struct Vertex Vertex;
struct Vertex
{
  float position[2];
  float uv[2];
  float color[3];
};
typedef struct mm_render mm_render;
struct mm_render
{
  ID3D11SamplerState* sampler;
  ID3D11ShaderResourceView* textureView;
  ID3D11InputLayout* layout;
  ID3D11VertexShader* vshader;
  ID3D11PixelShader* pshader;
  ID3D11Buffer* vbuffer;
  ID3D11Buffer* ubuffer; 
};

fn mm_render CreateMMRender(ID3D11Device* device, ID3D11DeviceContext* context)
{
  HRESULT hr;
  mm_render Result = {0}; 
  {
    struct Vertex data[] =
    {
      { { -0.00f, +0.75f }, { 25.0f, 50.0f }, { 1, 0, 0 } },
      { { +0.75f, -0.50f }, {  0.0f,  0.0f }, { 0, 1, 0 } },
      { { -0.75f, -0.50f }, { 50.0f,  0.0f }, { 0, 0, 1 } },
    };
    D3D11_BUFFER_DESC desc =
    {
      .ByteWidth = sizeof(data),
      .Usage = D3D11_USAGE_IMMUTABLE,
      .BindFlags = D3D11_BIND_VERTEX_BUFFER,
    };
    D3D11_SUBRESOURCE_DATA initial = { .pSysMem = data };
    ID3D11Device_CreateBuffer(device, &desc, &initial, &Result.vbuffer);
  }
  
  // these must match vertex shader input layout (VS_INPUT in vertex shader source below)
  D3D11_INPUT_ELEMENT_DESC LayoutDesc[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct Vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  
  // vertex & pixel shaders for drawing triangle, plus input layout for vertex input
  
  
  {
    D3D11_BUFFER_DESC desc =
    {
      // space for 4x4 float matrix (cbuffer0 from pixel shader)
      .ByteWidth = 4 * 4 * sizeof(float),
      .Usage = D3D11_USAGE_DYNAMIC,
      .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
      .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    ID3D11Device_CreateBuffer(device, &desc, NULL, &Result.ubuffer);
  }
  {
    D3D11_SAMPLER_DESC desc =
    {
      .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
      .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    };
    
    ID3D11Device_CreateSamplerState(device, &desc, &Result.sampler);
  }
  
  {
    // checkerboard texture, with 50% transparency on black colors
    unsigned int pixels[] =
    {
      0x80000000, 0xffffffff,
      0xffffffff, 0x80000000,
    };
    UINT width = 2;
    UINT height = 2;
    
    D3D11_TEXTURE2D_DESC desc =
    {
      .Width = width,
      .Height = height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = { 1, 0 },
      .Usage = D3D11_USAGE_IMMUTABLE,
      .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    };
    
    D3D11_SUBRESOURCE_DATA data =
    {
      .pSysMem = pixels,
      .SysMemPitch = width * sizeof(unsigned int),
    };
    
    ID3D11Texture2D* texture;
    ID3D11Device_CreateTexture2D(device, &desc, &data, &texture);
    ID3D11Device_CreateShaderResourceView(device, (ID3D11Resource*)texture, NULL, &Result.textureView);
    ID3D11Texture2D_Release(texture);
  }
  
#if 0
  ID3D11Device_CreateVertexShader(device, d3d11_vshader, sizeof(d3d11_vshader), NULL, &Result.vshader);
  ID3D11Device_CreatePixelShader(device, d3d11_pshader, sizeof(d3d11_pshader), NULL, &Result.pshader);
  ID3D11Device_CreateInputLayout(device, LayoutDesc, ARRAYSIZE(LayoutDesc), d3d11_vshader, sizeof(d3d11_vshader), &Result.layout);
#else
  UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#ifndef NDEBUG
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
  //FILE *MainShaderFile;
  //fopen_s(&MainShaderFile          , (const char *)"F:\\Dev\\ParticleSystem\\src\\main.hlsl", "rb");
  //= Str8FromFile(MainShaderFile);
  //Allocate on the stack
  u8 Buffer[4096];
  arena Arena = ArenaInit(&Arena, 4096, Buffer);
  str8 MainShaderSrc = OSFileRead(Str8("F:\\Dev\\ParticleSystem\\src\\main.hlsl"), &Arena);
  
  ID3DBlob* error;
  ID3DBlob* vblob;
  hr = D3DCompile(MainShaderSrc.Data, MainShaderSrc.Size, NULL, NULL, NULL, "vs", "vs_5_0", flags, 0, &vblob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile vertex shader!");
  }
  
  ID3DBlob* pblob;
  hr = D3DCompile(MainShaderSrc.Data, MainShaderSrc.Size, NULL, NULL, NULL, "ps", "ps_5_0", flags, 0, &pblob, &error);
  if (FAILED(hr))
  {
    const char* message = ID3D10Blob_GetBufferPointer(error);
    OutputDebugStringA(message);
    Assert(!"Failed to compile pixel shader!");
  }
#if 1
  
  
#endif
  
  ID3D11Device_CreateVertexShader(device, ID3D10Blob_GetBufferPointer(vblob), ID3D10Blob_GetBufferSize(vblob), NULL, &Result.vshader);
  ID3D11Device_CreatePixelShader(device, ID3D10Blob_GetBufferPointer(pblob), ID3D10Blob_GetBufferSize(pblob), NULL, &Result.pshader);
  ID3D11Device_CreateInputLayout(device, LayoutDesc, ARRAYSIZE(LayoutDesc), ID3D10Blob_GetBufferPointer(vblob), ID3D10Blob_GetBufferSize(vblob), &Result.layout);
  ID3D10Blob_Release(pblob);
  ID3D10Blob_Release(vblob);
#endif
  
  return Result;
}
fn void MMDraw(mm_render *MMRender,
               float height, 
               float width,
               float delta,
               ID3D11DeviceContext * Context,
               D3D11_VIEWPORT viewport,
               ID3D11RasterizerState* rasterizerState,
               ID3D11DepthStencilState* depthState,
               ID3D11BlendState* blendState, 
               ID3D11RenderTargetView* rtView,
               ID3D11DepthStencilView* dsView)
{
  // setup 4x4c rotation matrix in uniform
  {
    static float angle = 0;
    angle += delta * 2.0f * (float)M_PI / 20.0f; // full rotation in 20 seconds
    angle = fmodf(angle, 2.0f * (float)M_PI);
    
    float aspect = (float)height / width;
    float matrix[16] =
    {
      cosf(angle) * aspect, -sinf(angle), 0, 0,
      sinf(angle) * aspect,  cosf(angle), 0, 0,
      0,            0, 0, 0,
      0,            0, 0, 1,
    };
    
    D3D11_MAPPED_SUBRESOURCE mapped;
    ID3D11DeviceContext_Map(Context, (ID3D11Resource*)MMRender->ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, matrix, sizeof(matrix));
    ID3D11DeviceContext_Unmap(Context, (ID3D11Resource*)MMRender->ubuffer, 0);
  }
  
  // Input Assembler
  ID3D11DeviceContext_IASetInputLayout(Context, MMRender->layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  UINT stride = sizeof(struct Vertex);
  UINT offset = 0;
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &MMRender->vbuffer, &stride, &offset);
  
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &MMRender->ubuffer);
  ID3D11DeviceContext_VSSetShader(Context, MMRender->vshader, NULL, 0);
  
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &viewport);
  ID3D11DeviceContext_RSSetState(Context, rasterizerState);
  
  // Pixel Shader
  ID3D11DeviceContext_PSSetSamplers(Context, 0, 1, &MMRender->sampler);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &MMRender->textureView);
  ID3D11DeviceContext_PSSetShader(Context, MMRender->pshader, NULL, 0);
  
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, blendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, depthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &rtView, dsView);
  
  // draw 3 vertices
  ID3D11DeviceContext_Draw(Context, 3, 0);
  D3D11ClearPipeline(Context);
  return;
}

#endif //MM_H
