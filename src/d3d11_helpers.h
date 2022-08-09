/* date = July 19th 2022 7:11 pm */

#ifndef D3D11_HELPERS_H
#define D3D11_HELPERS_H

#include "memory.h"


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

typedef enum gpu_mem_op gpu_mem_op;
enum gpu_mem_op
{
  GPU_MEM_READ,
  GPU_MEM_WRITE,
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
#ifndef NDEBUG
  {
    ID3D11InfoQueue* Info;
    ID3D11Device_QueryInterface(Base.Device, &IID_ID3D11InfoQueue, &Info);
    ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    ID3D11InfoQueue_Release(Info);
  }
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
  D3D11_MAP MapType = (Op==GPU_MEM_READ)?D3D11_MAP_READ:D3D11_MAP_WRITE;
  ID3D11DeviceContext_Map(Context,(ID3D11Resource *)GPUBuffer, 0, MapType, 0, &MappedBuffer);
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
fn void D3D11ConstantBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Size)
{
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Size;
  Desc.Usage          = D3D11_USAGE_DEFAULT;
  Desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  Desc.CPUAccessFlags = 0;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  ID3D11Device_CreateBuffer(Device, &Desc, &Initial, Buffer);
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
fn void D3D11SRV(ID3D11Device* Device, ID3D11ShaderResourceView  **SRV, ID3D11Buffer *Buffer, u32 Count)
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
fn ID3DBlob *D3D11LoadAndCompileShader(char *ShaderFileDir, const char *ShaderEntry, const char *ShaderTypeAndVer)
{
  ID3DBlob *ShaderBlob, *Error;
  HRESULT Status;
  u8 Buffer[4096*2];
  arena Arena     = ArenaInit(&Arena, 4096*2, &Buffer);
  arena_temp Temp = ArenaTempBegin(&Arena);
  str8 ShaderSrc  = OSFileRead(Str8(ShaderFileDir), Temp.Arena);
  ArenaTempEnd(Temp);
  UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR |
                D3DCOMPILE_ENABLE_STRICTNESS        |
                D3DCOMPILE_WARNINGS_ARE_ERRORS);
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  Status = D3DCompile(ShaderSrc.Data, ShaderSrc.Size, NULL, NULL, NULL,
                      ShaderEntry, ShaderTypeAndVer, flags, 0, &ShaderBlob, &Error);
  if (FAILED(Status))
  {
    const char* message = ID3D10Blob_GetBufferPointer(Error);
    OutputDebugStringA(message);
    ConsoleLog(Arena, "[TestCode]: Failed to load shader of type %s !!!", ShaderTypeAndVer);
    Assert(!"[TestCode]: Failed to load shader of type [meh] !!!");
  }
  return ShaderBlob;
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
  ID3D11DeviceContext_CSSetConstantBuffers     (Context, 0, 1, NullBuffer);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, NullUAV, 0);
  ID3D11DeviceContext_CSSetShader              (Context, NullCShader, NULL, 0);
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
#endif //D3D11_HELPERS_H
