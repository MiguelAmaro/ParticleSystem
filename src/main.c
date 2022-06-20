//beep 

// example how to set up D3D11 rendering on Windows in C
// require Windows 7 Platform Update or newer Windows version

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

// replace this with your favorite Assert() implementation
#include <intrin.h>

#define AssertHR(hr) Assert(SUCCEEDED(hr))

#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define STR2(x) #x
#define STR(x) STR2(x)


#include "types.h"
u64 GetFileSizeFromCSTL(FILE *FileHandle)
{
  u64 Result = 0;
  fseek(FileHandle, 0, SEEK_END);
  Result = ftell(FileHandle);
  fseek(FileHandle, 0, SEEK_SET);
  return Result;
}


#include "str.h"
#include "mmath.h"
#include "particles.h"
static void FatalError(const char* message)
{
  MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
  ExitProcess(0);
}

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(wnd, msg, wparam, lparam);
}

typedef struct time_measure time_measure;
struct time_measure
{
  f64 MSDelta;
  f64 MSElapsed;
  u64 TickFrequency;
  u64 WorkStartTick;
  u64 WorkEndTick;
  u64 WorkTickDelta;
  f64 MicrosElapsedWorking;
  f64 TargetMicrosPerFrame;
  f64 TicksToMicros;
};

time_measure InitTimeMeasure(void)
{
  time_measure Result = {0};
  Result.MSDelta   = 0.0;
  Result.MSElapsed = 0.0;
  Result.TickFrequency = 0;
  Result.WorkStartTick = 0;
  Result.WorkEndTick = 0;
  Result.WorkTickDelta = 0;
  Result.MicrosElapsedWorking = 0.0;
  QueryPerformanceFrequency((LARGE_INTEGER *)&Result.TickFrequency);
  Result.TargetMicrosPerFrame = 16666.0;
  Result.TicksToMicros = 1000000.0/(f64)Result.TickFrequency;
  return Result;
}

f64 TimeMeasureElapsed(time_measure *Time)
{
  QueryPerformanceCounter((LARGE_INTEGER *)&Time->WorkEndTick);
  Time->WorkTickDelta  = Time->WorkEndTick - Time->WorkStartTick;
  Time->MicrosElapsedWorking = (f64)Time->WorkTickDelta*Time->TicksToMicros;
  u64 IdleTickDelta = 0;
  u64 IdleStartTick = Time->WorkEndTick;
  u64 IdleEndTick = 0;
  f64 MicrosElapsedIdle = 0.0;
  while((Time->MicrosElapsedWorking+MicrosElapsedIdle)<Time->TargetMicrosPerFrame)
  {
    QueryPerformanceCounter((LARGE_INTEGER *)&IdleEndTick);
    IdleTickDelta = IdleEndTick-IdleStartTick;
    MicrosElapsedIdle = (f64)IdleTickDelta*Time->TicksToMicros;
  }
  f64 FrameTimeMS = (Time->MicrosElapsedWorking+MicrosElapsedIdle)/1000.0;
  Time->MSDelta    = FrameTimeMS;
  Time->MSElapsed += FrameTimeMS;
  return Time->MSDelta;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
  // register window class to have custom WindowProc callback
  WNDCLASSEXW wc =
  {
    .cbSize = sizeof(wc),
    .lpfnWndProc = WindowProc,
    .hInstance = instance,
    .hIcon = LoadIcon(NULL, IDI_APPLICATION),
    .hCursor = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = L"d3d11_window_class",
  };
  ATOM atom = RegisterClassExW(&wc);
  Assert(atom && "Failed to register window class");
  
  // window properties - width, height and style
  int width = CW_USEDEFAULT;
  int height = CW_USEDEFAULT;
  // WS_EX_NOREDIRECTIONBITMAP flag here is needed to fix ugly bug with Windows 10
  // when window is resized and DXGI swap chain uses FLIP presentation model
  // DO NOT use it if you choose to use non-FLIP presentation model
  DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  DWORD style = WS_OVERLAPPEDWINDOW;
  
  // uncomment in case you want fixed size window
  //style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
  //RECT rect = { 0, 0, 1280, 720 };
  //AdjustWindowRectEx(&rect, style, FALSE, exstyle);
  //width = rect.right - rect.left;
  //height = rect.bottom - rect.top;
  
  // create window
  HWND window = CreateWindowExW(
                                exstyle, wc.lpszClassName, L"D3D11 Window", style,
                                CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                                NULL, NULL, wc.hInstance, NULL);
  Assert(window && "Failed to create window");
  
  HRESULT hr;
  
  ID3D11Device* device;
  ID3D11DeviceContext* context;
  
  // create D3D11 device & context
  {
    UINT flags = 0;
#ifndef NDEBUG
    // this enables VERY USEFUL debug messages in debugger output
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    hr = D3D11CreateDevice(
                           NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, ARRAYSIZE(levels),
                           D3D11_SDK_VERSION, &device, NULL, &context);
    // make sure device creation succeeeds before continuing
    // for simple applciation you could retry device creation with
    // D3D_DRIVER_TYPE_WARP driver type which enables software rendering
    // (could be useful on broken drivers or remote desktop situations)
    AssertHR(hr);
  }
  
#ifndef NDEBUG
  // for debug builds enable VERY USEFUL debug break on API errors
  {
    ID3D11InfoQueue* info;
    ID3D11Device_QueryInterface(device, &IID_ID3D11InfoQueue, &info);
    ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
    ID3D11InfoQueue_Release(info);
  }
  // after this there's no need to check for any errors on device functions manually
  // so all HRESULT return  values in this code will be ignored
  // debugger will break on errors anyway
#endif
  
  // create DXGI swap chain
  IDXGISwapChain1* swapChain;
  {
    // create DXGI 1.2 factory for creating swap chain
    IDXGIFactory2* factory;
    hr = CreateDXGIFactory(&IID_IDXGIFactory2, &factory);
    AssertHR(hr);
    
    DXGI_SWAP_CHAIN_DESC1 desc =
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
    
    hr = IDXGIFactory2_CreateSwapChainForHwnd(factory, (IUnknown*)device, window, &desc, NULL, NULL, &swapChain);
    // make sure swap chain creation succeeds before continuing
    AssertHR(hr);
    
    IDXGIFactory_Release(factory);
  }
  
  // disable silly Alt+Enter changing monitor resolution to match window size
  {
    IDXGIFactory* factory;
    IDXGISwapChain1_GetParent(swapChain, &IID_IDXGIFactory, &factory);
    IDXGIFactory_MakeWindowAssociation(factory, window, DXGI_MWA_NO_ALT_ENTER);
    IDXGIFactory_Release(factory);
  }
  
  struct Vertex
  {
    float position[2];
    float uv[2];
    float color[3];
  };
  
  ID3D11Buffer* vbuffer;
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
    ID3D11Device_CreateBuffer(device, &desc, &initial, &vbuffer);
  }
  
  // vertex & pixel shaders for drawing triangle, plus input layout for vertex input
  ID3D11InputLayout* layout;
  ID3D11VertexShader* vshader;
  ID3D11PixelShader* pshader;
  //paratilc systems
  ID3D11ComputeShader  *PSysCMShader;
  ID3D11ComputeShader  *PSysCHShader;
  ID3D11VertexShader   *PSysVShader;
  ID3D11GeometryShader *PSysGShader;
  ID3D11PixelShader    *PSysPShader;
  {
    // these must match vertex shader input layout (VS_INPUT in vertex shader source below)
    D3D11_INPUT_ELEMENT_DESC desc[] =
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct Vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
#if 0
    
    
    ID3D11Device_CreateVertexShader(device, d3d11_vshader, sizeof(d3d11_vshader), NULL, &vshader);
    ID3D11Device_CreatePixelShader(device, d3d11_pshader, sizeof(d3d11_pshader), NULL, &pshader);
    ID3D11Device_CreateInputLayout(device, desc, ARRAYSIZE(desc), d3d11_vshader, sizeof(d3d11_vshader), &layout);
    
#else
    FILE *MainShaderFile;
    FILE *ParticleShaderFile;
    FILE *ParticleHelperShaderFile;
    fopen_s(&MainShaderFile          , (const char *)"F:\\Dev\\ParticleSystem\\src\\main.hlsl", "rb");
    fopen_s(&ParticleShaderFile      , (const char *)"F:\\Dev\\ParticleSystem\\src\\particles.hlsl", "rb");
    fopen_s(&ParticleHelperShaderFile, (const char *)"F:\\Dev\\ParticleSystem\\src\\particleshelper.hlsl", "rb");
    str8 MainShaderSrc           = Str8FromFile(MainShaderFile);
    str8 ParticleShaderSrc       = Str8FromFile(ParticleShaderFile);
    str8 ParticleHelperShaderSrc = Str8FromFile(ParticleHelperShaderFile);
    
    UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
    
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
    //Particle
    ID3DBlob* PSysCMBlob;
    ID3DBlob* PSysCHBlob;
    ID3DBlob* PSysVBlob;
    ID3DBlob* PSysGblob;
    ID3DBlob* PSysPBlob;
    hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "CSMAIN", "cs_5_0", flags, 0, &PSysCMBlob, &error);
    if (FAILED(hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(error);
      OutputDebugStringA(message);
      Assert(!"Failed to compile particle sys main compute shader!");
    }
    hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "VSMAIN", "vs_5_0", flags, 0, &PSysVBlob, &error);
    if (FAILED(hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(error);
      OutputDebugStringA(message);
      Assert(!"Failed to compile particle sys main vertex shader!");
    }
    hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "GSMAIN", "gs_5_0", flags, 0, &PSysGblob, &error);
    if (FAILED(hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(error);
      OutputDebugStringA(message);
      Assert(!"Failed to compile particle sys main geometry shader!");
    }
    hr = D3DCompile(ParticleShaderSrc.Data, ParticleShaderSrc.Size, NULL, NULL, NULL, "PSMAIN", "ps_5_0", flags, 0, &PSysPBlob, &error);
    if (FAILED(hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(error);
      OutputDebugStringA(message);
      Assert(!"Failed to compile particle sys main pixels shader!");
    }
    //ParticleHelper
    hr = D3DCompile(ParticleHelperShaderSrc.Data, ParticleHelperShaderSrc.Size, NULL, NULL, NULL, "CSHELPER", "cs_5_0", flags, 0, &PSysCHBlob, &error);
    if (FAILED(hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(error);
      OutputDebugStringA(message);
      Assert(!"Failed to compile helper compute shader!");
    }
    
    
#endif
    
    ID3D11Device_CreateVertexShader(device, ID3D10Blob_GetBufferPointer(vblob), ID3D10Blob_GetBufferSize(vblob), NULL, &vshader);
    ID3D11Device_CreatePixelShader(device, ID3D10Blob_GetBufferPointer(pblob), ID3D10Blob_GetBufferSize(pblob), NULL, &pshader);
    ID3D11Device_CreateInputLayout(device, desc, ARRAYSIZE(desc), ID3D10Blob_GetBufferPointer(vblob), ID3D10Blob_GetBufferSize(vblob), &layout);
    ID3D10Blob_Release(pblob);
    ID3D10Blob_Release(vblob);
    //particle sys
    ID3D11Device_CreateComputeShader(device, ID3D10Blob_GetBufferPointer(PSysCMBlob), ID3D10Blob_GetBufferSize(PSysCMBlob), NULL, &PSysCMShader);
    ID3D11Device_CreateComputeShader(device, ID3D10Blob_GetBufferPointer(PSysCHBlob), ID3D10Blob_GetBufferSize(PSysCHBlob), NULL, &PSysCHShader);
    ID3D11Device_CreateVertexShader  (device, ID3D10Blob_GetBufferPointer(PSysVBlob), ID3D10Blob_GetBufferSize(PSysVBlob), NULL, &PSysVShader);
    ID3D11Device_CreateGeometryShader(device, ID3D10Blob_GetBufferPointer(PSysGblob), ID3D10Blob_GetBufferSize(PSysGblob), NULL, &PSysGShader);
    ID3D11Device_CreatePixelShader(device, ID3D10Blob_GetBufferPointer(PSysPBlob), ID3D10Blob_GetBufferSize(PSysPBlob), NULL, &PSysPShader);
    ID3D10Blob_Release(PSysCMBlob);
    ID3D10Blob_Release(PSysCHBlob);
    ID3D10Blob_Release(PSysVBlob);
    ID3D10Blob_Release(PSysGblob);
    ID3D10Blob_Release(PSysPBlob);
    
#endif
  }
  
  ID3D11Buffer* ubuffer;
  {
    D3D11_BUFFER_DESC desc =
    {
      // space for 4x4 float matrix (cbuffer0 from pixel shader)
      .ByteWidth = 4 * 4 * sizeof(float),
      .Usage = D3D11_USAGE_DYNAMIC,
      .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
      .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    };
    ID3D11Device_CreateBuffer(device, &desc, NULL, &ubuffer);
  }
  time_measure TimeData = InitTimeMeasure();
  particlesystem ParticleSystem = CreateParticleSystem(device, 100, (f32)width, (f32)height);
  ID3D11ShaderResourceView* textureView;
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
    ID3D11Device_CreateShaderResourceView(device, (ID3D11Resource*)texture, NULL, &textureView);
    ID3D11Texture2D_Release(texture);
  }
  
  ID3D11SamplerState* sampler;
  {
    D3D11_SAMPLER_DESC desc =
    {
      .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
      .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
      .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    };
    
    ID3D11Device_CreateSamplerState(device, &desc, &sampler);
  }
  
  ID3D11BlendState* blendState;
  {
    // enable alpha blending
    D3D11_BLEND_DESC desc =
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
    ID3D11Device_CreateBlendState(device, &desc, &blendState);
  }
  
  ID3D11RasterizerState* rasterizerState;
  {
    // disable culling
    D3D11_RASTERIZER_DESC desc =
    {
      .FillMode = D3D11_FILL_SOLID,
      .CullMode = D3D11_CULL_NONE,
    };
    ID3D11Device_CreateRasterizerState(device, &desc, &rasterizerState);
  }
  
  ID3D11DepthStencilState* depthState;
  {
    // disable depth & stencil test
    D3D11_DEPTH_STENCIL_DESC desc =
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
    ID3D11Device_CreateDepthStencilState(device, &desc, &depthState);
  }
  
  ID3D11RenderTargetView* rtView = NULL;
  ID3D11DepthStencilView* dsView = NULL;
  
  // show the window
  ShowWindow(window, SW_SHOWDEFAULT);
  
  LARGE_INTEGER freq, c1;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c1);
  
  float angle = 0;
  DWORD currentWidth = 0;
  DWORD currentHeight = 0;
  
  for (;;)
  {
    // process all incoming Windows messages
    MSG msg;
    if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      continue;
    }
    
    // get current size for window client area
    RECT rect;
    GetClientRect(window, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    ParticleSystem.ConstData.transforms.ProjMatrix = M4fOrtho(0.0f, (f32)width,
                                                              0.0f, (f32)height,
                                                              0.0f, 100.0f);
    ParticleSystem.ConstData.sim_params.TimeFactors = V4f((f32)TimeData.MSDelta, 0.0f, 0.0f, 0.0f);
    
    // resize swap chain if needed
    if (rtView == NULL || width != currentWidth || height != currentHeight)
    {
      if (rtView)
      {
        // release old swap chain buffers
        ID3D11DeviceContext_ClearState(context);
        ID3D11RenderTargetView_Release(rtView);
        ID3D11DepthStencilView_Release(dsView);
        rtView = NULL;
      }
      
      // resize to new size for non-zero size
      if (width != 0 && height != 0)
      {
        hr = IDXGISwapChain1_ResizeBuffers(swapChain, 0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
        {
          FatalError("Failed to resize swap chain!");
        }
        
        // create RenderTarget view for new backbuffer texture
        ID3D11Texture2D* backbuffer;
        IDXGISwapChain1_GetBuffer(swapChain, 0, &IID_ID3D11Texture2D, &backbuffer);
        ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)backbuffer, NULL, &rtView);
        ID3D11Texture2D_Release(backbuffer);
        
        D3D11_TEXTURE2D_DESC depthDesc =
        {
          .Width = width,
          .Height = height,
          .MipLevels = 1,
          .ArraySize = 1,
          .Format = DXGI_FORMAT_D32_FLOAT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
          .SampleDesc = { 1, 0 },
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        };
        
        // create new depth stencil texture & DepthStencil view
        ID3D11Texture2D* depth;
        ID3D11Device_CreateTexture2D(device, &depthDesc, NULL, &depth);
        ID3D11Device_CreateDepthStencilView(device, (ID3D11Resource*)depth, NULL, &dsView);
        ID3D11Texture2D_Release(depth);
      }
      
      currentWidth = width;
      currentHeight = height;
    }
    
    // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
    if (rtView)
    {
      LARGE_INTEGER c2;
      QueryPerformanceCounter(&c2);
      float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
      c1 = c2;
      
      // output viewport covering all client area of window
      D3D11_VIEWPORT viewport =
      {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width = (FLOAT)width,
        .Height = (FLOAT)height,
        .MinDepth = 0,
        .MaxDepth = 1,
      };
      
      // clear screen
      FLOAT color[] = { 0.392f, 0.584f, 0.929f, 1.f };
      ID3D11DeviceContext_ClearRenderTargetView(context, rtView, color);
      ID3D11DeviceContext_ClearDepthStencilView(context, dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
      
      // setup 4x4c rotation matrix in uniform
      {
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
        ID3D11DeviceContext_Map(context, (ID3D11Resource*)ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, matrix, sizeof(matrix));
        ID3D11DeviceContext_Unmap(context, (ID3D11Resource*)ubuffer, 0);
      }
      
      // Input Assembler
      ID3D11DeviceContext_IASetInputLayout(context, layout);
      ID3D11DeviceContext_IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      UINT stride = sizeof(struct Vertex);
      UINT offset = 0;
      ID3D11DeviceContext_IASetVertexBuffers(context, 0, 1, &vbuffer, &stride, &offset);
      
      // Vertex Shader
      ID3D11DeviceContext_VSSetConstantBuffers(context, 0, 1, &ubuffer);
      ID3D11DeviceContext_VSSetShader(context, vshader, NULL, 0);
      
      // Rasterizer Stage
      ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
      ID3D11DeviceContext_RSSetState(context, rasterizerState);
      
      // Pixel Shader
      ID3D11DeviceContext_PSSetSamplers(context, 0, 1, &sampler);
      ID3D11DeviceContext_PSSetShaderResources(context, 0, 1, &textureView);
      ID3D11DeviceContext_PSSetShader(context, pshader, NULL, 0);
      
      // Output Merger
      ID3D11DeviceContext_OMSetBlendState(context, blendState, NULL, ~0U);
      ID3D11DeviceContext_OMSetDepthStencilState(context, depthState, 0);
      ID3D11DeviceContext_OMSetRenderTargets(context, 1, &rtView, dsView);
      
      // draw 3 vertices
      ID3D11DeviceContext_Draw(context, 3, 0);
      
      
      //~ Particle System
      /*TODOs
1. Supply a view and proj matrix (done)
2. Unordered access view for defining particle count (done)
3. Work on provideing nessaary unifrom data (done)
4. Do const buffer mapping (done)
5. Understand draw inderect
*/
      ID3D11UnorderedAccessView* NullUAV[1] = {NULL};
      ID3D11ShaderResourceView* NullSRV[1] = {NULL};
      // Compute Shader Helper
      ID3D11DeviceContext_CSSetConstantBuffers(context, 0, 1, &ParticleSystem.ConstParticleParams);
      ID3D11DeviceContext_CSSetUnorderedAccessViews(context, 0, 1, &ParticleSystem.UOAccessViewA,
                                                    &ParticleSystem.ParticleMaxCount.x);
      ID3D11DeviceContext_CSSetShader(context, PSysCHShader, NULL, 0);
      ID3D11DeviceContext_Dispatch(context, 1, 1, 1);
      // Compute Shader Main
      ID3D11DeviceContext_CSSetConstantBuffers(context, 1, 1, &ParticleSystem.ConstSimParams);
      ID3D11DeviceContext_CSSetConstantBuffers(context, 2, 1, &ParticleSystem.ConstParticleCount);
      // NOTE(MIGUEL): UOAccessViewA is already bound
      //ID3D11DeviceContext_CSSetUnorderedAccessViews(context, 1, 1, &ParticleSystem.UOAccessViewA,
      //&ParticleSystem.ParticleMaxCount.x);
      ID3D11DeviceContext_CSSetUnorderedAccessViews(context, 1, 1, &ParticleSystem.UOAccessViewB,
                                                    &ParticleSystem.ParticleMaxCount.x);
      ID3D11DeviceContext_CSSetShader(context, PSysCMShader, NULL, 0);
      ID3D11DeviceContext_Dispatch(context, 1, 1, 1);
      //UBIND for render
      ID3D11DeviceContext_CSSetUnorderedAccessViews(context, 1, 1, NullUAV,
                                                    &ParticleSystem.ParticleMaxCount.x);
      // Input Assembler
      ID3D11DeviceContext_IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
      
      // Vertex Shader
      ID3D11DeviceContext_VSSetConstantBuffers(context, 0, 1, &ParticleSystem.ConstUpdatedFrameData);
      ID3D11DeviceContext_VSSetShaderResources(context, 1, 1, &ParticleSystem.ShaderResViewB);
      ID3D11DeviceContext_VSSetShader(context, PSysVShader, NULL, 0);
      // Geometry Shader
      ID3D11DeviceContext_GSSetConstantBuffers(context, 0, 1, &ParticleSystem.ConstUpdatedFrameData);
      ID3D11DeviceContext_GSSetConstantBuffers(context, 1, 1, &ParticleSystem.ConstParticleRenderParams);
      ID3D11DeviceContext_GSSetShader(context, PSysGShader, NULL, 0);
      // Rasterizer Stage
      ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
      ID3D11DeviceContext_RSSetState(context, rasterizerState);
      // Pixel Shader
      ID3D11DeviceContext_PSSetSamplers(context, 0, 1, &sampler);
      ID3D11DeviceContext_PSSetShaderResources(context, 1, 1, &textureView);
      ID3D11DeviceContext_PSSetShader(context, pshader, NULL, 0);
      // Output Merger
      ID3D11DeviceContext_OMSetBlendState(context, blendState, NULL, ~0U);
      ID3D11DeviceContext_OMSetDepthStencilState(context, depthState, 0);
      ID3D11DeviceContext_OMSetRenderTargets(context, 1, &rtView, dsView);
      // draw 3 vertices
      ID3D11DeviceContext_DrawInstancedIndirect(context, ParticleSystem.IndirectDrawArgs, 0);
      
      //UNBIND for next frame
      ID3D11DeviceContext_VSSetShaderResources(context, 1, 1, NullSRV);
    }
    
    // change to FALSE to disable vsync
    BOOL vsync = TRUE;
    hr = IDXGISwapChain1_Present(swapChain, vsync ? 1 : 0, 0);
    if (hr == DXGI_STATUS_OCCLUDED)
    {
      // window is minimized, cannot vsync - instead sleep a bit
      if (vsync)
      {
        Sleep(10);
      }
    }
    else if (FAILED(hr))
    {
      FatalError("Failed to present swap chain! Device lost?");
    }
    TimeMeasureElapsed(&TimeData);
  }
}
