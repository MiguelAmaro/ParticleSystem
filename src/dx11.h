#ifndef DX11_HELPERS_H
#define DX11_HELPERS_H

typedef struct d3d11_base d3d11_base;
struct d3d11_base
{
  v2s WindowDim;
  ID3D11Device            *Device;
  ID3D11DeviceContext     *Context;
  ID3D11BlendState        *BlendState;
  IDXGISwapChain1         *SwapChain;
  ID3D11RasterizerState   *RasterizerState;
  ID3D11DepthStencilState *DepthState;
  ID3D11RenderTargetView  *RTView;
  ID3D11DepthStencilView  *DSView;
  D3D11_VIEWPORT           Viewport;
  
};

#define D3D11BaseDestructure(BasePtr) \
ID3D11Device            *Device     = BasePtr->Device;          \
ID3D11DeviceContext     *Context    = BasePtr->Context;         \
ID3D11BlendState        *BlendState = BasePtr->BlendState;      \
IDXGISwapChain1         *SwapChain  = BasePtr->SwapChain;       \
ID3D11RasterizerState   *RastState  = BasePtr->RasterizerState; \
ID3D11DepthStencilState *DepthState = BasePtr->DepthState;      \
ID3D11RenderTargetView  *RTView     = BasePtr->RTView;          \
ID3D11DepthStencilView  *DSView     = BasePtr->DSView;          \
D3D11_VIEWPORT           Viewport   = BasePtr->Viewport;        

typedef enum cpu_access cpu_access;
enum cpu_access 
{
  Access_None  = 0,
  Access_Read  = D3D11_CPU_ACCESS_READ,
  Access_Write = D3D11_CPU_ACCESS_WRITE,
};
typedef enum buffer_usage buffer_usage;
enum buffer_usage 
{
  Usage_Default = D3D11_USAGE_DEFAULT,
  Usage_Dynamic = D3D11_USAGE_DYNAMIC,
};
typedef enum gpu_mem_op gpu_mem_op;
enum gpu_mem_op
{
  GPU_MEM_READ  = D3D11_MAP_READ,
  GPU_MEM_WRITE = D3D11_MAP_WRITE_DISCARD,
};
typedef enum tex_format tex_format;
enum tex_format
{
  Unorm_RGBA = DXGI_FORMAT_R8G8B8A8_UNORM,
  Unorm_R    = DXGI_FORMAT_R8_UNORM,
  Float_R    = DXGI_FORMAT_R32_FLOAT,
  Float_RG   = DXGI_FORMAT_R32G32_FLOAT,
  Float_RGBA = DXGI_FORMAT_R32G32B32A32_FLOAT,
};
typedef enum shaderkind shaderkind;
enum shaderkind
{
  ShaderKind_Vertex,
  ShaderKind_Pixel,
  ShaderKind_Compute,
  ShaderKind_Geometry,
};
typedef struct d3d11_shader d3d11_shader;
struct d3d11_shader
{
  shaderkind Kind;
  str8 Path;
  str8 EntryName;
  datetime LastRecordedWrite;
  union
  {
    ID3D11VertexShader   *VertexHandle;
    ID3D11PixelShader    *PixelHandle;
    ID3D11ComputeShader  *ComputeHandle;
    ID3D11GeometryShader *GeometryHandle;
  };
  ID3D11InputLayout *Layout;
};
fn  d3d11_base D3D11InitBase(HWND Window)
{
  d3d11_base Base = {0};
  HRESULT Status;
  Base.RTView = NULL;
  Base.DSView = NULL;
  Base.WindowDim = V2s(0,0);
  
  // create D3D11 device & context
  {
    UINT Flags = 0;
#ifndef NDEBUG
    // this enables VERY USEFUL debug messages in debugger output
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL Levels[] = { D3D_FEATURE_LEVEL_11_0 };
    Status = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, Flags, Levels, ARRAYSIZE(Levels),
                               D3D11_SDK_VERSION, &Base.Device, NULL, &Base.Context);
    // make sure device creation succeeeds before continuing
    // for simple applciation you could retry device creation with
    // D3D_DRIVER_TYPE_WARP driver type which enables software rendering
    // (could be useful on broken drivers or remote desktop situations)
    AssertHR(Status);
  }
  // for debug builds enable VERY USEFUL debug break on API errors
  {
    ID3D11InfoQueue* Info;
    ID3D11Device_QueryInterface(Base.Device, &IID_ID3D11InfoQueue, &Info);
    ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    ID3D11InfoQueue_Release(Info);
  }
#ifndef NDEBUG
  // after this there's no need to check for any errors on device functions manually
  // so all HRESULT return  values in this code will be ignored
  // debugger will break on errors anyway
#endif
  // create DXGI swap chain
  {
    // create DXGI 1.2 factory for creating swap chain
    IDXGIFactory2* Factory;
    Status = CreateDXGIFactory(&IID_IDXGIFactory2, &Factory);
    AssertHR(Status );
    DXGI_SWAP_CHAIN_DESC1 Desc =
    {
      // default 0 value for width & height means to get it from HWND automatically
      //.Width = 0,
      //.Height = 0,
      
      // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      // FLIP presentation model does not allow MSAA framebuffer
      // if you want MSAA then you'll need to render offscreen and manually
      // resolve to non-MSAA framebuffer
      .SampleDesc = { 1, 0 },
      
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = 2,
      
      // we don't want any automatic scaling of window content
      // this is supported only on FLIP presentation model
      .Scaling = DXGI_SCALING_NONE,
      
      // use more efficient FLIP presentation model
      // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
      // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
      // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    };
    Status = IDXGIFactory2_CreateSwapChainForHwnd(Factory, (IUnknown*)Base.Device, Window, &Desc, NULL, NULL, &Base.SwapChain);
    // make sure swap chain creation succeeds before continuing
    AssertHR(Status);
    IDXGIFactory_Release(Factory);
  }
  // disable silly Alt+Enter changing monitor resolution to match window size
  {
    IDXGIFactory* Factory;
    IDXGISwapChain1_GetParent(Base.SwapChain, &IID_IDXGIFactory, &Factory);
    IDXGIFactory_MakeWindowAssociation(Factory, Window, DXGI_MWA_NO_ALT_ENTER);
    IDXGIFactory_Release(Factory);
  }
  {
    // enable alpha blending
    D3D11_BLEND_DESC Desc =
    {
      .RenderTarget[0] =
      {
        .BlendEnable = TRUE,
        .SrcBlend = D3D11_BLEND_SRC_ALPHA,
        .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
        .BlendOp = D3D11_BLEND_OP_ADD,
        .SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA,
        .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
        .BlendOpAlpha = D3D11_BLEND_OP_ADD,
        .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
      },
    };
    ID3D11Device_CreateBlendState(Base.Device, &Desc, &Base.BlendState);
  }
  {
    // disable culling
    D3D11_RASTERIZER_DESC Desc =
    {
      //.FillMode = D3D11_FILL_WIREFRAME,
      .FillMode = D3D11_FILL_SOLID,
      .CullMode = D3D11_CULL_NONE,
    };
    ID3D11Device_CreateRasterizerState(Base.Device, &Desc, &Base.RasterizerState);
  }
  {
    // disable depth & stencil test
    D3D11_DEPTH_STENCIL_DESC Desc =
    {
      .DepthEnable = FALSE,
      .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
      .DepthFunc = D3D11_COMPARISON_LESS,
      .StencilEnable = FALSE,
      .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
      .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
      // .FrontFace = ... 
      // .BackFace = ...
    };
    ID3D11Device_CreateDepthStencilState(Base.Device, &Desc, &Base.DepthState);
  }
  return Base;
}
fn void D3D11UpdateWindowSize(d3d11_base *Base, v2s WindowDim)
{
  HRESULT Status;
  // resize swap chain if needed
  if (Base->RTView == NULL ||
      WindowDim.x != Base->WindowDim.x ||
      WindowDim.y != Base->WindowDim.y)
  {
    if (Base->RTView)
    {
      // release old swap chain buffers
      ID3D11DeviceContext_ClearState(Base->Context);
      ID3D11RenderTargetView_Release(Base->RTView);
      ID3D11DepthStencilView_Release(Base->DSView);
      Base->RTView = NULL;
    }
    
    // resize to new size for non-zero size
    if (WindowDim.x != 0 && WindowDim.y != 0)
    {
      Status = IDXGISwapChain1_ResizeBuffers(Base->SwapChain, 0, WindowDim.x, WindowDim.y, DXGI_FORMAT_UNKNOWN, 0);
      if (FAILED(Status))
      {    
        FatalError("Failed to resize swap chain!");
      }
      
      // create RenderTarget view for new backbuffer texture
      ID3D11Texture2D* Backbuffer;
      IDXGISwapChain1_GetBuffer(Base->SwapChain, 0, &IID_ID3D11Texture2D, &Backbuffer);
      ID3D11Device_CreateRenderTargetView(Base->Device, (ID3D11Resource*)Backbuffer, NULL, &Base->RTView);
      ID3D11Texture2D_Release(Backbuffer);
      
      D3D11_TEXTURE2D_DESC depthDesc =
      {
        .Width  = WindowDim.x,
        .Height = WindowDim.y,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_D32_FLOAT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
        .SampleDesc = { 1, 0 },
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
      };
      
      // create new depth stencil texture & DepthStencil view
      ID3D11Texture2D* Depth;
      ID3D11Device_CreateTexture2D(Base->Device, &depthDesc, NULL, &Depth);
      ID3D11Device_CreateDepthStencilView(Base->Device, (ID3D11Resource*)Depth, NULL, &Base->DSView);
      ID3D11Texture2D_Release(Depth);
    }
    Base->WindowDim.x = WindowDim.x;
    Base->WindowDim.y = WindowDim.y;
  }
  return;
}
fn void D3D11GPUMemoryOp(ID3D11DeviceContext * Context, ID3D11Buffer *GPUBuffer, void *CPUBuffer, u32 Stride, u32 Count, gpu_mem_op Op)
{
  D3D11_MAPPED_SUBRESOURCE MappedBuffer =  {0};
  ID3D11DeviceContext_Map(Context,(ID3D11Resource *)GPUBuffer, 0, Op, 0, &MappedBuffer);
  void *Dst = (Op==GPU_MEM_READ)?CPUBuffer:MappedBuffer.pData;
  void *Src = (Op==GPU_MEM_READ)?MappedBuffer.pData:CPUBuffer;
  MemoryCopy(Src, Stride*Count, Dst, Stride*Count);
  ID3D11DeviceContext_Unmap(Context, (ID3D11Resource *)GPUBuffer, 0);
  return;
  
}
fn void D3D11StructuredBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count)
{
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth           = Count*Stride;
  Desc.StructureByteStride = Stride;
  Desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  Desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  Desc.Usage               = D3D11_USAGE_DEFAULT;
  Desc.CPUAccessFlags = 0;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
  return;
}
fn void D3D11VertexBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count)
{
  // Normal vertex buffer fed in via the input assembler as point topology
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth = Count*Stride;
  Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  Desc.Usage = D3D11_USAGE_DEFAULT;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
  return;
}
fn void D3D11ConstantBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Size, buffer_usage Usage, cpu_access Access)
{
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Size;
  Desc.Usage          = Usage;
  Desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  Desc.CPUAccessFlags = Access;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, Data==NULL?NULL:&Initial, Buffer);
  return;
}
fn void D3D11StageBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Size)
{
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Size;
  Desc.BindFlags      = 0; //Staging buffers are not meant to be bound to any stage in the pipeline.
  Desc.Usage          = D3D11_USAGE_STAGING;
  Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
  return;
}
fn void D3D11Tex2DStage(ID3D11Device* Device, ID3D11Texture2D **Texture, v2s TexDim,
                        void *Data, u32 Stride, tex_format Format)
{
  D3D11_TEXTURE2D_DESC Desc = {0};
  Desc.Width          = TexDim.x;
  Desc.Height         = TexDim.y;
  Desc.MipLevels      = 1;
  Desc.ArraySize      = 1;
  Desc.Format         = Format;
  Desc.Usage          = D3D11_USAGE_STAGING;
  Desc.BindFlags      = 0;
  Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  Desc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0}; //do i need?
  D3D11_SUBRESOURCE_DATA Initial = {0};
  Initial.pSysMem = Data;
  Initial.SysMemPitch = Stride*TexDim.x;
  ID3D11Device_CreateTexture2D(Device, &Desc, &Initial, Texture);
  return;
}
fn void D3D11BufferViewSR(ID3D11Device* Device, ID3D11ShaderResourceView  **SRV, ID3D11Buffer *Buffer, u32 Count)
{
  D3D11_SHADER_RESOURCE_VIEW_DESC Desc = {0};
  Desc.Format              = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.NumElements  = Count;
  ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Buffer, &Desc, SRV);
  return;
}
fn void D3D11ArgsBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Args, u32 Size)
{
  Assert(Size%4 == 0);
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth           = Size; //Draw Args are all 32bit variables so size must be a 4byte mutliple.
  Desc.StructureByteStride = 0;    // No structures just packes 32bit values.
  Desc.MiscFlags           = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
  Desc.Usage               = D3D11_USAGE_DEFAULT;
  Desc.BindFlags           = 0;
  Desc.CPUAccessFlags      = 0;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Args;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
  return;
}
fn void D3D11BufferViewUA(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Buffer *Buffer, u32 Count )
{
  D3D11_UNORDERED_ACCESS_VIEW_DESC Desc = {0};
  Desc.Format              = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
  Desc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_COUNTER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.NumElements = Count;
  ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Buffer, &Desc, UAV);
  return;
}
fn void D3D11BufferViewUAAppend(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Buffer *Buffer, u32 Count )
{
  //Used by CSMain
  D3D11_UNORDERED_ACCESS_VIEW_DESC Desc = {0};
  Desc.Format              = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_APPEND;
  Desc.Buffer.NumElements  = Count;
  ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Buffer, &Desc, UAV);
  return;
}
fn void D3D11Tex2DViewSR(ID3D11Device* Device, ID3D11ShaderResourceView **SRV, ID3D11Texture2D **GetTex, v2s TexDim,
                         void *Data, u32 Stride, tex_format Format)
{
  D3D11_TEXTURE2D_DESC Desc = {0};
  Desc.Width          = TexDim.x;
  Desc.Height         = TexDim.y;
  Desc.MipLevels      = 1;
  Desc.ArraySize      = 1;
  Desc.Format         = Format;
  Desc.Usage          = D3D11_USAGE_DEFAULT;
  Desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
  Desc.CPUAccessFlags = 0;
  Desc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  D3D11_SUBRESOURCE_DATA Initial = {0};
  Initial.pSysMem = Data;
  Initial.SysMemPitch = Stride*TexDim.x;
  ID3D11Texture2D *Texture;
  D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
  SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SRVDesc.Format        = Format;
  SRVDesc.Texture2D     = (D3D11_TEX2D_SRV){0, 1}; //Mip Level, Level Count
  ID3D11Device_CreateTexture2D(Device, &Desc, &Initial, &Texture);
  ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
  return;
}
fn void D3D11Tex2DViewUA(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Texture2D **GetTex, v2s TexDim, void *Data, u32 Stride, tex_format Format)
{
  D3D11_TEXTURE2D_DESC TexDesc = {0};
  TexDesc.Width          = TexDim.x;
  TexDesc.Height         = TexDim.y;
  TexDesc.MipLevels      = 1;
  TexDesc.ArraySize      = 1;
  TexDesc.Format         = Format;
  TexDesc.Usage          = D3D11_USAGE_DEFAULT;
  TexDesc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
  TexDesc.CPUAccessFlags = 0;
  TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  D3D11_SUBRESOURCE_DATA Initial = {0};
  Initial.pSysMem = Data;
  Initial.SysMemPitch = Stride*TexDim.x;
  ID3D11Texture2D *Texture;
  ID3D11Device_CreateTexture2D(Device, &TexDesc, &Initial, &Texture);
  D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {0};
  UAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
  UAVDesc.Format              = Format;
  UAVDesc.Texture2D.MipSlice  = 0;
  ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Texture, &UAVDesc, UAV);
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
}
fn void D3D11Tex2DViewSRAndUA(ID3D11Device* Device, ID3D11Texture2D **GetTex,
                              ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV, 
                              v2s TexDim, void *Data, u32 Stride, tex_format Format)
{
  D3D11_TEXTURE2D_DESC TexDesc = {0};
  TexDesc.Width          = TexDim.x;
  TexDesc.Height         = TexDim.y;
  TexDesc.MipLevels      = 1;
  TexDesc.ArraySize      = 1;
  TexDesc.Format         = Format;
  TexDesc.Usage          = D3D11_USAGE_DEFAULT;
  TexDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  TexDesc.CPUAccessFlags = 0;
  TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  D3D11_SUBRESOURCE_DATA Initial = {0};
  Initial.pSysMem = Data;
  Initial.SysMemPitch = Stride*TexDim.x;
  ID3D11Texture2D *Texture;
  ID3D11Device_CreateTexture2D(Device, &TexDesc, &Initial, &Texture);
  D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
  SRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
  SRVDesc.Format              = Format;
  SRVDesc.Texture2D           =  (D3D11_TEX2D_SRV){0, 1};
  D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {0};
  UAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
  UAVDesc.Format              = Format;
  UAVDesc.Texture2D           = (D3D11_TEX2D_UAV){0};
  ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
  ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Texture, &UAVDesc, UAV);
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
}
fn ID3DBlob *D3D11ShaderLoadAndCompile(str8 ShaderFileDir, str8 ShaderEntry,
                                       const char *ShaderTypeAndVer, const char *CallerName)
{
  ID3DBlob *ShaderBlob = NULL;
  ID3DBlob *Error = NULL;
  HRESULT Status;
  arena Arena; ArenaLocalInit(Arena, 4096*5);
  arena_temp Temp = ArenaTempBegin(&Arena);
  str8 ShaderSrc  = OSFileRead(ShaderFileDir, Temp.Arena);
  ArenaTempEnd(Temp);
  UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR |
                D3DCOMPILE_ENABLE_STRICTNESS        |
                D3DCOMPILE_WARNINGS_ARE_ERRORS);
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  Status = D3DCompile(ShaderSrc.Data, ShaderSrc.Size, NULL, NULL, NULL,
                      (LPCSTR)ShaderEntry.Data, (LPCSTR)ShaderTypeAndVer, flags, 0, &ShaderBlob, &Error);
  if (FAILED(Status))
  {
    const char* message = ID3D10Blob_GetBufferPointer(Error);
    OutputDebugStringA(message);
    ConsoleLog(Arena, "[%s]: Failed to load shader of type %s !!!\n", ShaderTypeAndVer, CallerName);
    //Assert(!"Failed to load shader! Look at console for details");
  }
  return ShaderBlob;
}
fn void D3D11ShaderHotReload(d3d11_base *Base, d3d11_shader *Shader)
{
  D3D11BaseDestructure(Base);
  datetime LastWrite = OSFileLastWriteTime(Shader->Path);
  if(IsEqual(&LastWrite, &Shader->LastRecordedWrite, datetime)) return;
  ID3DBlob *Blob = NULL;
  switch(Shader->Kind)
  {
    case ShaderKind_Vertex:
    {
      ID3D11VertexShader *NewVertexShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Shader->Path, Shader->EntryName, "vs_5_0", "Shader HotLoader");
      if(Blob==NULL) return;
      HRESULT Status = ID3D11Device_CreateVertexShader(Device,
                                                       ID3D10Blob_GetBufferPointer(Blob),
                                                       ID3D10Blob_GetBufferSize(Blob), NULL,
                                                       &NewVertexShader);
      if(SUCCEEDED(Status));
      {
        ID3D11VertexShader_Release(Shader->VertexHandle);
        Shader->VertexHandle = NewVertexShader;
      }
    } break;
    case ShaderKind_Pixel:
    {
      ID3D11PixelShader *NewPixelShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Shader->Path, Shader->EntryName, "ps_5_0", "Shader HotLoader");
      if(Blob==NULL) return;
      HRESULT Status = ID3D11Device_CreatePixelShader(Device,
                                                      ID3D10Blob_GetBufferPointer(Blob),
                                                      ID3D10Blob_GetBufferSize(Blob), NULL,
                                                      &NewPixelShader);
      if(SUCCEEDED(Status));
      {
        ID3D11PixelShader_Release(Shader->PixelHandle);
        Shader->PixelHandle = NewPixelShader;
      }
    } break;
    case ShaderKind_Compute:
    {
      ID3D11ComputeShader  *NewComputeShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Shader->Path, Shader->EntryName, "cs_5_0", "Shader HotLoader");
      if(Blob==NULL) return;
      HRESULT Status = ID3D11Device_CreateComputeShader(Device,
                                                        ID3D10Blob_GetBufferPointer(Blob),
                                                        ID3D10Blob_GetBufferSize(Blob), NULL,
                                                        &NewComputeShader);
      if(SUCCEEDED(Status));
      {
        ID3D11ComputeShader_Release(Shader->ComputeHandle);
        Shader->ComputeHandle = NewComputeShader;
      }
    } break;
    case ShaderKind_Geometry:
    {
      ID3D11GeometryShader  *NewGeometryShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Shader->Path, Shader->EntryName, "gs_5_0", "Shader HotLoader");
      if(Blob==NULL) return;
      HRESULT Status = ID3D11Device_CreateGeometryShader(Device,
                                                         ID3D10Blob_GetBufferPointer(Blob),
                                                         ID3D10Blob_GetBufferSize(Blob), NULL,
                                                         &NewGeometryShader);
      if(SUCCEEDED(Status));
      {
        ID3D11GeometryShader_Release(Shader->GeometryHandle);
        Shader->GeometryHandle = NewGeometryShader;
      }
    } break;
    default:
    {
      Assert(!"Invalid Codepath");
    }
  }
  arena Arena; ArenaLocalInit(Arena, 256);
  ConsoleLog(Arena, "Reloaded Shader\n");
  Shader->LastRecordedWrite = LastWrite;
  return;
}
fn d3d11_shader D3D11ShaderCreate(shaderkind Kind,
                                  str8 Path,
                                  str8 EntryName,
                                  D3D11_INPUT_ELEMENT_DESC *VElemDesc,
                                  u32 VElemDescCount,
                                  d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  //arena Arena
  d3d11_shader Result = {0};
  Result.Path      = Path;
  Result.EntryName = EntryName;
  Result.Kind      = Kind;
  Result.LastRecordedWrite = OSFileLastWriteTime(Path);
  ID3DBlob *Blob = NULL;
  arena Arena; ArenaLocalInit(Arena, 256);
  switch(Kind)
  {
    case ShaderKind_Vertex:
    {
      ID3D11VertexShader *NewVertexShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "vs_5_0", "Shader Create");
      HRESULT Status = ID3D11Device_CreateVertexShader(Device,
                                                       ID3D10Blob_GetBufferPointer(Blob),
                                                       ID3D10Blob_GetBufferSize(Blob), NULL,
                                                       &Result.VertexHandle);
      ID3D11Device_CreateInputLayout(Device, VElemDesc, VElemDescCount, ID3D10Blob_GetBufferPointer(Blob), ID3D10Blob_GetBufferSize(Blob), &Result.Layout);
    } break;
    case ShaderKind_Pixel:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "ps_5_0", "Shader Create");
      HRESULT Status = ID3D11Device_CreatePixelShader(Device,
                                                      ID3D10Blob_GetBufferPointer(Blob),
                                                      ID3D10Blob_GetBufferSize(Blob), NULL,
                                                      &Result.PixelHandle);
    } break;
    case ShaderKind_Compute:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "cs_5_0", "Shader Create");
      HRESULT Status = ID3D11Device_CreateComputeShader(Device,
                                                        ID3D10Blob_GetBufferPointer(Blob),
                                                        ID3D10Blob_GetBufferSize(Blob), NULL,
                                                        &Result.ComputeHandle);
    } break;
    case ShaderKind_Geometry:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "gs_5_0", "Shader Create");
      HRESULT Status = ID3D11Device_CreateGeometryShader(Device,
                                                         ID3D10Blob_GetBufferPointer(Blob),
                                                         ID3D10Blob_GetBufferSize(Blob), NULL,
                                                         &Result.GeometryHandle);
    } break;
    default:
    {
      Assert(!"Invalid Codepath");
    }
  }
  ConsoleLog(Arena, "Creating Shader\n");
  // TODO(MIGUEL): handle no blob case. It is possible that complilation fails and there is no blob
  ID3D10Blob_Release(Blob);
  return Result;
}
fn void D3D11ClearComputeStage(ID3D11DeviceContext *Context)
{
  ID3D11ComputeShader       *NullCShader = NULL;
  ID3D11SamplerState        *NullSampler[1] = {NULL};
  ID3D11ShaderResourceView  *NullSRV[1]     = {NULL};
  ID3D11UnorderedAccessView *NullUAV[1]     = {NULL};
  ID3D11Buffer              *NullBuffer[1]  = {NULL};
  // Compute Shader
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 2, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 3, 1, NullSRV);
  ID3D11DeviceContext_CSSetSamplers            (Context, 0, 1, NullSampler);
  ID3D11DeviceContext_CSSetSamplers            (Context, 1, 1, NullSampler);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, NullBuffer);
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 1, 1, NullBuffer);
  ID3D11DeviceContext_CSSetShader              (Context, NullCShader, NULL, 0);
  return;
}
fn void D3D11ClearPipeline(ID3D11DeviceContext *Context)
{
  ID3D11UnorderedAccessView *NullUAV[1]     = {NULL};
  ID3D11ShaderResourceView  *NullSRV[1]     = {NULL};
  ID3D11InputLayout         *NullLayout[1]  = {NULL};
  ID3D11Buffer              *NullBuffer[1]  = {NULL};
  ID3D11SamplerState        *NullSampler[1] = {NULL};
  ID3D11ComputeShader       *NullCShader = NULL;
  ID3D11VertexShader        *NullVShader = NULL;
  ID3D11GeometryShader      *NullGShader = NULL;
  ID3D11PixelShader         *NullPShader = NULL;
  // Input Assembler
  ID3D11DeviceContext_IASetInputLayout         (Context, NullLayout[0]);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers     (Context, 0, 1, NullBuffer);
  ID3D11DeviceContext_VSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_VSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_VSSetShader              (Context, NullVShader, NULL, 0);
  // Geometry Shader
  ID3D11DeviceContext_GSSetConstantBuffers     (Context, 0, 1, NullBuffer);
  ID3D11DeviceContext_GSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_GSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_GSSetShader              (Context, NullGShader, NULL, 0);
  // Compute Shader
  D3D11ClearComputeStage(Context);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers     (Context, 0, 1, NullBuffer);
  ID3D11DeviceContext_PSSetSamplers            (Context, 0, 1, NullSampler);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_PSSetShader              (Context, NullPShader, NULL, 0);
  return;
}
fn void *D3D11ReadBuffer(ID3D11DeviceContext *Context, ID3D11Buffer *TargetBuffer, ID3D11Buffer *StageBuffer, u32 Stride, u32 Count, arena *Arena)
{
  void *Result = ArenaPushBlock(Arena, Stride*Count);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource*)StageBuffer, (ID3D11Resource*)TargetBuffer);
  D3D11GPUMemoryOp(Context, StageBuffer, Result, Stride, Count, GPU_MEM_READ);
  return Result;
}
fn void D3D11Tex2DSwap(ID3D11DeviceContext *Context, ID3D11Texture2D **TexA, ID3D11Texture2D **TexB, ID3D11Texture2D *Stage)
{
#if 1
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)Stage, (ID3D11Resource *)*TexA);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)*TexA, (ID3D11Resource *)*TexB);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)*TexB, (ID3D11Resource *)Stage);
#else
  // NOTE(MIGUEL): Think doing this when these handles are associated with srv's is bad.
  //               
  ID3D11Texture2D *Temp = *TexA;
  *TexA = *TexB;
  *TexB = Temp;
#endif
  return;
}
#endif //DX11_HELPERS_H