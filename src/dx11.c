
// NOTE(MIGUEL): should base fall under graphics settings that the programmer can control with ui
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
    ID3D11Device_QueryInterface(Base.Device, &IID_ID3D11InfoQueue, &Base.Info);
    ID3D11InfoQueue_SetBreakOnSeverity(Base.Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    ID3D11InfoQueue_SetBreakOnSeverity(Base.Info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
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
      
      D3D11_TEXTURE2D_DESC DepthDesc =
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
      ID3D11Device_CreateTexture2D(Base->Device, &DepthDesc, NULL, &Depth);
      ID3D11Device_CreateDepthStencilView(Base->Device, (ID3D11Resource*)Depth, NULL, &Base->DSView);
      ID3D11Texture2D_Release(Depth);
    }
    Base->WindowDim.x = WindowDim.x;
    Base->WindowDim.y = WindowDim.y;
  }
  return;
}
fn void D3D11GPUMemoryRead(ID3D11DeviceContext * Context, ID3D11Resource *GPUResource, void *CPUBuffer, u32 Stride, u32 Count)
{
  D3D11_MAPPED_SUBRESOURCE MappedBuffer =  {0};
  ID3D11DeviceContext_Map(Context, GPUResource, 0, GPU_MEM_READ, 0, &MappedBuffer);
  void *Dst = CPUBuffer;
  void *Src = MappedBuffer.pData;
  MemoryCopy(Src, Stride*Count, Dst, Stride*Count);
  ID3D11DeviceContext_Unmap(Context, (ID3D11Resource *)GPUResource, 0);
  return;
  
}
fn void D3D11GPUMemoryWrite(ID3D11DeviceContext * Context, ID3D11Buffer *GPUBuffer, void *CPUBuffer, u32 Stride, u32 Count)
{
  D3D11_MAPPED_SUBRESOURCE MappedBuffer =  {0};
  ID3D11DeviceContext_Map(Context,(ID3D11Resource *)GPUBuffer, 0, GPU_MEM_WRITE, 0, &MappedBuffer);
  void *Dst = MappedBuffer.pData;
  void *Src = CPUBuffer;
  MemoryCopy(Src, Stride*Count, Dst, Stride*Count);
  ID3D11DeviceContext_Unmap(Context, (ID3D11Resource *)GPUBuffer, 0);
  return;
  
}
/*// NOTE(MIGUEL): I'm going to attempt to formalize the api.
*                  I want the api to be organized by objects
*                  StructuredBuffer
*                  
*                  Api functions will used a global ID3D11Device
*                  They will verify that it does exist
*                  And skip function and report if it doesnt
*                  Only data needed to create d3d11 data will be passed ass args
*                  
*                  Allow user to set cpu access flags defalt dynamic
*/
//~ STATE MGMT
fn void D3D11SetGlobalBase(d3d11_base *Base)
{
  GlobalBase = Base;
  return;
}
fn void D3D11ClearGlobalBase(void)
{
  GlobalBase = NULL;
  return;
}
//~ Buffers
fn void D3D11BufferStructUA(ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_BUFFER_DESC Desc = {0};
  {
    Desc.ByteWidth           = Count*Stride;
    Desc.StructureByteStride = Stride;
    Desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    Desc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    Desc.Usage               = D3D11_USAGE_DEFAULT;
    Desc.CPUAccessFlags = 0;
  }
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, (Data==NULL)?NULL:&Initial, Buffer);
  return;
}
fn void D3D11BufferVertex(ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_BUFFER_DESC Desc = {0};
  {
    Desc.ByteWidth = Count*Stride;
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.Usage = D3D11_USAGE_DEFAULT;
  }
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
  return;
}
fn void D3D11BufferConstant(ID3D11Buffer **Buffer, void *Data, u32 Size, buffer_usage Usage, cpu_access Access)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_BUFFER_DESC Desc = {0};
  {
    Desc.ByteWidth      = Size;
    Desc.Usage          = Usage;
    Desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = Access;
  }
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, Data==NULL?NULL:&Initial, Buffer);
  return;
}
fn void D3D11BufferStaging(ID3D11Buffer **Buffer, void *Data, u32 Size)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_BUFFER_DESC Desc = {0};
  {
    Desc.ByteWidth      = Size;
    Desc.BindFlags      = 0;
    Desc.Usage          = D3D11_USAGE_STAGING;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  }
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, (Data!=NULL)?&Initial:NULL, Buffer);
  return;
}
fn void D3D11BufferArgs(ID3D11Buffer **Buffer, void *Args, u32 Size)
{
  D3D11ValidateAndDestructBase(GlobalBase);
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
fn void *D3D11BufferRead(ID3D11Buffer *TargetBuffer, ID3D11Buffer *StageBuffer, u32 Stride, u32 Count, arena *Arena)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  void *Result = ArenaPushBlock(Arena, Stride*Count);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource*)StageBuffer, (ID3D11Resource*)TargetBuffer);
  D3D11GPUMemoryRead(Context, (ID3D11Resource *)StageBuffer, Result, Stride, Count);
  return Result;
}
fn void D3D11BufferWrite(ID3D11Buffer *TargetBuffer, ID3D11Buffer *StageBuffer, void *Data, u32 Stride, u32 Count)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11GPUMemoryWrite(Context, StageBuffer, Data, Stride, Count);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource*)TargetBuffer, (ID3D11Resource*)StageBuffer);
  return;
}
//~
fn void D3D11Tex2DStage(ID3D11Texture2D **Texture, v2s TexDim,
                        void *Data, u32 Stride,
                        tex_format Format)
{
  D3D11ValidateAndDestructBase(GlobalBase);
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
  ID3D11Device_CreateTexture2D(Device, &Desc, Data==NULL?NULL:&Initial, Texture);
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
  {
    Desc.Width          = TexDim.x;
    Desc.Height         = TexDim.y;
    Desc.MipLevels      = 1;
    Desc.ArraySize      = 1;
    Desc.Format         = Format;
    Desc.Usage          = D3D11_USAGE_DEFAULT;
    Desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
    Desc.CPUAccessFlags = 0;
    Desc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  }
  D3D11_SUBRESOURCE_DATA Initial = {0};
  {
    Initial.pSysMem = Data;
    Initial.SysMemPitch = Stride*TexDim.x;
  }
  ID3D11Texture2D *Texture;
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Format        = Format;
    SRVDesc.Texture2D     = (D3D11_TEX2D_SRV){0, 1}; //Mip Level, Level Count
    ID3D11Device_CreateTexture2D(Device, &Desc, &Initial, &Texture);
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
  }
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
  return;
}
fn void D3D11Tex2DViewUA(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Texture2D **GetTex, v2s TexDim, void *Data, u32 Stride, tex_format Format)
{
  D3D11_TEXTURE2D_DESC TexDesc = {0};
  {
    TexDesc.Width          = TexDim.x;
    TexDesc.Height         = TexDim.y;
    TexDesc.MipLevels      = 1;
    TexDesc.ArraySize      = 1;
    TexDesc.Format         = Format;
    TexDesc.Usage          = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  }
  D3D11_SUBRESOURCE_DATA Initial = {0};
  {
    Initial.pSysMem = Data;
    Initial.SysMemPitch = Stride*TexDim.x;
  }
  ID3D11Texture2D *Texture;
  ID3D11Device_CreateTexture2D(Device, &TexDesc, Data!=NULL?&Initial:NULL, &Texture);
  D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {0};
  {
    UAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
    UAVDesc.Format              = Format;
    UAVDesc.Texture2D.MipSlice  = 0;
  }
  ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Texture, &UAVDesc, UAV);
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
  return;
}
/*// NOTE(MIGUEL): I just want one function that takes a pointer UAV and SRV and Tex and the function will
*                  will check which of the those pointers are null to understand what to give back to the caller
*                  so now itll be one funciont that that can give back anything tex, whatever view instead 
*                  of multiple functions for diffent views.
*/
fn void D3D11Tex2D(ID3D11Texture2D **GetTex,
                   ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV, 
                   v2s TexDim, void *Data, u32 Stride, tex_format Format, buffer_usage Usage, cpu_access Access)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_TEXTURE2D_DESC TexDesc = {0};
  {
    TexDesc.Width          = TexDim.x;
    TexDesc.Height         = TexDim.y;
    TexDesc.MipLevels      = 1;
    TexDesc.ArraySize      = 1;
    TexDesc.Format         = Format;
    TexDesc.Usage          = Usage;
    TexDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    TexDesc.CPUAccessFlags = Access;
    TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
  }
  D3D11_SUBRESOURCE_DATA Initial = {0};
  {
    Initial.pSysMem = Data;
    Initial.SysMemPitch = Stride*TexDim.x;
  }
  ID3D11Texture2D *Texture;
  ID3D11Device_CreateTexture2D(Device, &TexDesc, (Data!=NULL)?&Initial:NULL, &Texture);
  if(SRV != NULL)
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
    SRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Format              = Format;
    SRVDesc.Texture2D           =  (D3D11_TEX2D_SRV){0, 1};
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
  }
  if(UAV != NULL)
  {
    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {0};
    UAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
    UAVDesc.Format              = Format;
    UAVDesc.Texture2D           = (D3D11_TEX2D_UAV){0};
    ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Texture, &UAVDesc, UAV);
  }
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
}
fn void D3D11Tex2DCube(ID3D11Texture2D **GetTex,
                       ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV, 
                       v2s TexDim, void **Data, u32 Stride, tex_format Format, buffer_usage Usage)
{
  D3D11ValidateAndDestructBase(GlobalBase);
  D3D11_TEXTURE2D_DESC TexDesc = {0};
  {
    TexDesc.Width          = TexDim.x;
    TexDesc.Height         = TexDim.y;
    TexDesc.MipLevels      = 1;
    TexDesc.ArraySize      = 6;
    TexDesc.Format         = Format;
    TexDesc.Usage          = Usage;
    TexDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
    TexDesc.MiscFlags      = D3D11_RESOURCE_MISC_TEXTURECUBE;
  }
  D3D11_SUBRESOURCE_DATA Initial[6] = {0};
  if(Data!=NULL)
  {
    foreach(FaceId, 6, s32)
    {
      Initial[FaceId].pSysMem = Data[FaceId];
      Initial[FaceId].SysMemPitch = Stride*TexDim.x;
    }
  }
  ID3D11Texture2D *Texture;
  ID3D11Device_CreateTexture2D(Device, &TexDesc, (Data!=NULL)?&Initial[0]:NULL, &Texture);
  if(SRV != NULL)
  {
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
    SRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURECUBE;
    SRVDesc.Format              = Format;
    SRVDesc.TextureCube           =  (D3D11_TEXCUBE_SRV){0, 1};
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
  }
  if(UAV != NULL)
  {
    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {0};
    Assert(!"Texture UAV Not suppoerted!");
    //UAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURECUBE;
    //UAVDesc.Format              = Format;
    //UAVDesc.TextureCube           = (D3D11_TEXCUBE_UAV){0};
    //ID3D11Device_CreateUnorderedAccessView(Device, (ID3D11Resource*)Texture, &UAVDesc, UAV);
  }
  if(GetTex != NULL) *GetTex = Texture;
  else ID3D11Texture2D_Release(Texture);
}
fn void D3D11Tex2DSwap(ID3D11DeviceContext *Context, ID3D11Texture2D **TexA, ID3D11Texture2D **TexB, ID3D11Texture2D *Stage)
{
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)Stage, (ID3D11Resource *)*TexA);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)*TexA, (ID3D11Resource *)*TexB);
  ID3D11DeviceContext_CopyResource(Context, (ID3D11Resource *)*TexB, (ID3D11Resource *)Stage);
  return;
}
//~ SHADER LOADING & CREATION
// TODO(MIGUEL): This whole section needs testing.
// TODO(MIGUEL): copy the ErrorMsg to a str8 CompilerMsg, arena Arena and display in  imgui 
fn ID3DBlob *D3D11ShaderLoadAndCompile(str8 ShaderFileDir, str8 ShaderEntry, const char *ShaderTypeAndVer,
                                       str8 *CompilerMsg, arena *Arena)
{
  ID3DBlob *ShaderBlob = NULL;
  ID3DBlob *ErrorMsg = NULL;
  HRESULT Status;
  arena LocalArena; ArenaLocalInit(LocalArena, 4096*5);
  arena_temp Temp = ArenaTempBegin(&LocalArena);
  str8 ShaderSrc  = OSFileRead(ShaderFileDir, Temp.Arena);
  ArenaTempEnd(Temp);
  UINT DbgFlags    = (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS);
  UINT CommonFlags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS);
  //OSProfileLinesStart("D3DCompile");
  Status = D3DCompile(ShaderSrc.Data, ShaderSrc.Size, NULL, NULL, NULL,
                      (LPCSTR)ShaderEntry.Data, (LPCSTR)ShaderTypeAndVer,
                      CommonFlags | DbgFlags, 0, &ShaderBlob, &ErrorMsg);
  if(FAILED(Status))
  {
    const char* Message = ID3D10Blob_GetBufferPointer(ErrorMsg);
    
    if(!(CompilerMsg==NULL || Arena==NULL))
    {
      *CompilerMsg = Str8CopyToArena(Arena, (u8 *)Message);
    }
    OutputDebugStringA(Message);
    ConsoleLog(LocalArena, "[%s]: Failed to load shader of type !!!\n", ShaderTypeAndVer);
    //Assert(!"Failed to load shader! Look at console for details");
  }
  else
  {
    if(!(CompilerMsg==NULL || Arena==NULL))
    {
      CompilerMsg->Size = 0;
    }
  }
  return ShaderBlob;
}
global async_shader_load ShaderLoader = {0};

fn static DWORD WINAPI AsyncShaderLoader(void *Param)
{
  async_shader_load *Load = (async_shader_load *)Param;
  arena Arena = ArenaInit(NULL, 4096, Load->ArenaBuffer);
  D3D11BaseDestructure(Load->Base);
  while(1)
  {
    for(u32 EntryId = 0; EntryId<ASYNC_SHADER_BUFFER_SIZE; EntryId++)
    {
      async_shader_load_entry *Entry = &Load->Entries[EntryId];
      if(Entry->Status == ASYNC_SHADER_STATUS_Processing)
      {
        HRESULT Status;
        ID3DBlob *Blob = NULL;
        switch(Entry->Shader.Kind)
        {
          case ShaderKind_Vertex:
          {
            Blob = D3D11ShaderLoadAndCompile(Entry->Shader.Path, Entry->Shader.EntryName, "vs_5_0", 
                                             &Load->ShaderMsg, &Arena);
            if(Blob==NULL)
            {
              // NOTE(MIGUEL): Interlocked exchange here
              AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_Unable);
              continue;
            }
            Status = ID3D11Device_CreateVertexShader(Device,
                                                     ID3D10Blob_GetBufferPointer(Blob),
                                                     ID3D10Blob_GetBufferSize(Blob), NULL,
                                                     &Entry->Shader.VertexHandle);
          } break;
          case ShaderKind_Pixel:
          {
            Blob = D3D11ShaderLoadAndCompile(Entry->Shader.Path, Entry->Shader.EntryName, "ps_5_0",
                                             &Load->ShaderMsg, &Arena);
            if(Blob==NULL)
            {
              // NOTE(MIGUEL): Interlocked exchange here
              AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_Unable);
              continue;
            }
            Status = ID3D11Device_CreatePixelShader(Device,
                                                    ID3D10Blob_GetBufferPointer(Blob),
                                                    ID3D10Blob_GetBufferSize(Blob), NULL,
                                                    &Entry->Shader.PixelHandle);
          } break;
          case ShaderKind_Compute:
          {
            Blob = D3D11ShaderLoadAndCompile(Entry->Shader.Path, Entry->Shader.EntryName, "cs_5_0",
                                             &Load->ShaderMsg, &Arena);
            if(Blob==NULL)
            {
              // NOTE(MIGUEL): Interlocked exchange here
              AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_Unable);
              continue;
            }
            Status = ID3D11Device_CreateComputeShader(Device,
                                                      ID3D10Blob_GetBufferPointer(Blob),
                                                      ID3D10Blob_GetBufferSize(Blob), NULL,
                                                      &Entry->Shader.ComputeHandle);
          } break;
          default:
          {
            Assert(!"Invalide Codepath");
            Status = 0;
          }
        }
        if(Blob) ID3D10Blob_Release(Blob);
        
        if(SUCCEEDED(Status))
        {
          //EntyBlob assign
          AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_Finished);
        }
        else
        {
          // NOTE(MIGUEL): Interlocked exchange here
          AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_Unable);
          continue;
        }
        //Update status interlocked
        Load->Count--;
      }
    }
  }
  ExitThread(0);
}
fn void D3D11AsyncShaderLoaderInit(async_shader_load *Loader, d3d11_base *Base)
{
  static u32 CallCounter = 0;
  if(!Loader->IsInitialized)
  {
    foreach(EntryId, ASYNC_SHADER_BUFFER_SIZE, u32)
    {
      Loader->Entries[EntryId].Status = ASYNC_SHADER_STATUS_NULL;
    }
    Loader->Count = 0;
    Loader->Base = Base;
    Loader->IsInitialized = 1;
    {
      DWORD ThreadId = 0;
      HANDLE ThreadHandle = CreateThread(0, 0, AsyncShaderLoader, &ShaderLoader,0, &ThreadId);
    }
    CallCounter++;
  }
  Assert(CallCounter<2);
  return;
}
fn async_shader_status D3D11ShaderAsyncLoadAndCompile(d3d11_base *Base, d3d11_shader *Shader, datetime ReqTime)
{
  //decide weather to submit req wait or handle request result
  //first load or finish some other load
  // NOTE(MIGUEL): Key is invalid meaning it's not associeated with an ongoing request so
  //               we are free to submit a new request and get a valid key to retrieve data 
  //               after request is fulfilled.
  if(Shader->Id == ASYNC_SHADER_LOADER_invalidKey)
  {
    async_shader_load_entry *Entry = ShaderLoader.Entries;
    u32 EntryId = 0;
    for(; EntryId<ASYNC_SHADER_BUFFER_SIZE; EntryId++)
    {
      Entry = ShaderLoader.Entries+EntryId;
      if(Entry->Status == ASYNC_SHADER_STATUS_NULL) break;
    }
    //make a deep copy for file path and stuff. shader handle will be overwriten with new handle.
    //if compile is successfull
    Shader->Id      = EntryId;
    Shader->ReqTime = ReqTime;
    Entry->Status = ASYNC_SHADER_STATUS_Processing;
    Entry->Shader = *Shader;
    return ASYNC_SHADER_STATUS_Processing;
  }
  // NOTE(MIGUEL): the key is asoccieated with an ongoing request so the req status needs
  //               to be looked at. was is fulfilled or not or is it still processing?.
  async_shader_load_entry *Entry = &ShaderLoader.Entries[Shader->Id];
  if(Entry->Status == ASYNC_SHADER_STATUS_Finished)
  {
    Entry->Status = ASYNC_SHADER_STATUS_NULL;
    // NOTE(MIGUEL): req fulfilled do the swap
    switch(Shader->Kind)
    {
      case ShaderKind_Vertex:
      {
        ID3D11VertexShader_Release(Shader->VertexHandle);
        Shader->VertexHandle = Entry->Shader.VertexHandle;
      } break;
      case ShaderKind_Pixel:
      {
        ID3D11VertexShader_Release(Shader->PixelHandle);
        Shader->PixelHandle = Entry->Shader.PixelHandle;
      } break;
      case ShaderKind_Compute:
      {
        ID3D11VertexShader_Release(Shader->ComputeHandle);
        Shader->ComputeHandle = Entry->Shader.ComputeHandle;
      } break;
      default: { } break;
    };
    Shader->Id = ASYNC_SHADER_LOADER_invalidKey;
    AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_NULL);
    return ASYNC_SHADER_STATUS_Finished;
  }
  else if(Entry->Status == ASYNC_SHADER_STATUS_Processing)
  {
    // NOTE(MIGUEL): need to keep waiting.
    return ASYNC_SHADER_STATUS_Processing;
  }
  else if(Entry->Status == ASYNC_SHADER_STATUS_Unable)
  {
    Shader->Id = ASYNC_SHADER_LOADER_invalidKey;
    AtomicExchange((u64 *)&Entry->Status, ASYNC_SHADER_STATUS_NULL);
    return ASYNC_SHADER_STATUS_Unable;
  }
  else
  {
    Assert(!"Invalid Codepath");
    return ASYNC_SHADER_STATUS_NULL;
  }
}
fn void D3D11ShaderAsyncHotReload(d3d11_base *Base, d3d11_shader *Shader)
{
  D3D11BaseDestructure(Base);
  datetime LastWrite = OSFileLastWriteTime(Shader->Path);
  if(IsEqual(&LastWrite, &Shader->LastRecordedWrite, datetime)) return;
  D3D11AsyncShaderLoaderInit(&ShaderLoader, Base);
  async_shader_status Status = D3D11ShaderAsyncLoadAndCompile(Base, Shader, LastWrite);
  switch(Status)
  {
    case ASYNC_SHADER_STATUS_Processing: break;
    case ASYNC_SHADER_STATUS_Unable:
    case ASYNC_SHADER_STATUS_Finished:
    {
      // NOTE(MIGUEL): If we fail to compile the shader update the last write.
      //               in this case it is assumed that the user needs to modify the file
      //               again to bring to compilable state. Ideally when compilation faild the 
      //               compile error should be grabed and displayed.
      Shader->LastRecordedWrite = LastWrite;
    }break;
    case ASYNC_SHADER_STATUS_NULL:
    { 
      Assert(!"Invalid Codepath");
    } break;
  }
  return;
}
#if 0
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
#endif
fn d3d11_shader D3D11ShaderCreate(shaderkind Kind, str8 Path, str8 EntryName, D3D11_INPUT_ELEMENT_DESC *VElemDesc,
                                  u32 VElemDescCount, d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  //arena Arena
  d3d11_shader Result = {0};
  Result.Path      = Path;
  Result.EntryName = EntryName;
  Result.Kind      = Kind;
  Result.LastRecordedWrite = OSFileLastWriteTime(Path);
  Result.Id = ASYNC_SHADER_LOADER_invalidKey;
  MemoryZero(&Result.ReqTime, sizeof(Result.ReqTime));
  ID3DBlob *Blob = NULL;
  arena Arena; ArenaLocalInit(Arena, 256);
  // NOTE(MIGUEL): This codepath is going to assume that if the shader code is invalid and compiltation fails that
  //               the sync/async hotloading functions can take care of reporting error messages and compilining
  //               until compilation succeds using initialized data provided by this function.
  switch(Kind)
  {
    case ShaderKind_Vertex:
    {
      ID3D11VertexShader *NewVertexShader = NULL;
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "vs_5_0", NULL, NULL);
      if(Blob != NULL)
      {
        HRESULT Status = ID3D11Device_CreateVertexShader(Device,
                                                         ID3D10Blob_GetBufferPointer(Blob),
                                                         ID3D10Blob_GetBufferSize(Blob), NULL,
                                                         &Result.VertexHandle);
        ID3D11Device_CreateInputLayout(Device, VElemDesc, VElemDescCount, ID3D10Blob_GetBufferPointer(Blob), ID3D10Blob_GetBufferSize(Blob), &Result.Layout);
      }
    } break;
    case ShaderKind_Pixel:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "ps_5_0", NULL, NULL);
      if(Blob != NULL)
      {
        HRESULT Status = ID3D11Device_CreatePixelShader(Device,
                                                        ID3D10Blob_GetBufferPointer(Blob),
                                                        ID3D10Blob_GetBufferSize(Blob), NULL,
                                                        &Result.PixelHandle);
      }
    } break;
    case ShaderKind_Compute:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "cs_5_0", NULL, NULL);
      if(Blob != NULL)
      {
        HRESULT Status = ID3D11Device_CreateComputeShader(Device,
                                                          ID3D10Blob_GetBufferPointer(Blob),
                                                          ID3D10Blob_GetBufferSize(Blob), NULL,
                                                          &Result.ComputeHandle);
      }
    } break;
    case ShaderKind_Geometry:
    {
      Blob = D3D11ShaderLoadAndCompile(Path, EntryName, "gs_5_0", NULL, NULL);
      if(Blob != NULL)
      {
        HRESULT Status = ID3D11Device_CreateGeometryShader(Device,
                                                           ID3D10Blob_GetBufferPointer(Blob),
                                                           ID3D10Blob_GetBufferSize(Blob), NULL,
                                                           &Result.GeometryHandle);
      }
    } break;
    default:
    {
      Assert(!"Invalid Codepath");
    }
  }
  ConsoleLog(Arena, "Creating Shader\n");
  if(Blob != NULL) ID3D10Blob_Release(Blob);
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
  //UAV
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 2, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 3, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 4, 1, NullUAV, 0);
  //SRV
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 2, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 3, 1, NullSRV);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 4, 1, NullSRV);
  //Samplers
  ID3D11DeviceContext_CSSetSamplers            (Context, 0, 1, NullSampler);
  ID3D11DeviceContext_CSSetSamplers            (Context, 1, 1, NullSampler);
  ID3D11DeviceContext_CSSetSamplers            (Context, 2, 1, NullSampler);
  ID3D11DeviceContext_CSSetSamplers            (Context, 3, 1, NullSampler);
  ID3D11DeviceContext_CSSetSamplers            (Context, 4, 1, NullSampler);
  //CB
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
  ID3D11DeviceContext_PSSetSamplers            (Context, 1, 1, NullSampler);
  ID3D11DeviceContext_PSSetSamplers            (Context, 2, 1, NullSampler);
  ID3D11DeviceContext_PSSetSamplers            (Context, 3, 1, NullSampler);
  ID3D11DeviceContext_PSSetSamplers            (Context, 4, 1, NullSampler);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 0, 1, NullSRV);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 1, 1, NullSRV);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 2, 1, NullSRV);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 3, 1, NullSRV);
  ID3D11DeviceContext_PSSetShaderResources     (Context, 4, 1, NullSRV);
  ID3D11DeviceContext_PSSetShader              (Context, NullPShader, NULL, 0);
  return;
}