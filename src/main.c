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
#define foreach(a, b) for(int a=0; a<b;a++)
#define AssertHR(hr) Assert(SUCCEEDED(hr))
//BASE
#define fn
#include "types.h"
#include "memory.h"
#include "string.h"
#include "os.h"
#include "memory.c"
#include "string.c"
#include "mmath.h"
#include "d3d11_helpers.h"

// replace this with your favorite Assert() implementation
#include <intrin.h>


#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define STR2(x) #x
#define STR(x) STR2(x)


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

#include "particles.h"
#include "test.h"
#include "mm.h"
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
  OSConsoleCreate();
  ConsoleLog("Welcome To D3D11 Playground!");
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
  v2s WindowDim = V2s(CW_USEDEFAULT, CW_USEDEFAULT);
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
  HWND Window = CreateWindowExW(
                                exstyle, wc.lpszClassName, L"D3D11 Window", style,
                                CW_USEDEFAULT, CW_USEDEFAULT, WindowDim.x, WindowDim.y,
                                NULL, NULL, wc.hInstance, NULL);
  Assert(Window && "Failed to create window");
  
  d3d11_base Base = D3D11InitBase(Window);
  time_measure   TimeData       = InitTimeMeasure();
  particlesystem ParticleSystem = CreateParticleSystem(Base.Device, 20, (f32)WindowDim.x, (f32)WindowDim.y);
  //mm_render      MMRender       = CreateMMRender(device, context);
  testrend       TestRenderer   = CreateTestRenderer(Base.Device, Base.Context);
  
  
  // show the window
  ShowWindow(Window, SW_SHOWDEFAULT);
  
  ParticleSystemLoadShaders(&ParticleSystem, Base.Device);
  
  LARGE_INTEGER freq, c1;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c1);
  
  b32 IsFirstFrame = 1;
  for (;;)
  {
    // process all incoming Windows messages
    if(OSProcessMessges() == 1) break; // If '1' the quit message recieved
    WindowDim = OSWindowGetSize(Window);
    ParticleSystem.ConstData.transforms.ProjMatrix = M4fOrtho(0.0f, (f32)WindowDim.x,
                                                              0.0f, (f32)WindowDim.y,
                                                              0.0f, 100.0f);
    ParticleSystem.ConstData.sim_params.TimeFactors = V4f((f32)TimeData.MSDelta, 0.0f, 0.0f, 0.0f);
    D3D11UpdateWindowSize(&Base, WindowDim);
    // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
    if (Base.RTView)
    {
      LARGE_INTEGER c2;
      QueryPerformanceCounter(&c2);
      float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
      c1 = c2;
      
      // output viewport covering all client area of window
      Base.Viewport.TopLeftX = 0;
      Base.Viewport.TopLeftY = 0;
      Base.Viewport.Width    =  (FLOAT)WindowDim.x;
      Base.Viewport.Height   = (FLOAT)WindowDim.y;
      Base.Viewport.MinDepth = 0;
      Base.Viewport.MaxDepth = 1;
      // clear screen
      v4f Color = V4f(0.1f, 0.1f, 0.12f, 1.f );
      ID3D11DeviceContext_ClearRenderTargetView(Base.Context, Base.RTView, Color.e);
      ID3D11DeviceContext_ClearDepthStencilView(Base.Context, Base.DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
      //Working!!!
      //MMDraw(&MMRender, height, width, delta, context, viewport, rasterizerState, depthState, blendState, rtView, dsView);
      TestDraw(&TestRenderer, &Base, IsFirstFrame);
      
      //Brokent!!!
      //ParticleSystemDraw(&ParticleSystem, context, IsFirstFrame, &viewport, rasterizerState, depthState, blendState, rtView, dsView);
      
      IsFirstFrame = 0;
    }
    
    // change to FALSE to disable vsync
    HRESULT Status;
    BOOL vsync = TRUE;
    Status = IDXGISwapChain1_Present(Base.SwapChain, vsync ? 1 : 0, 0);
    if (Status == DXGI_STATUS_OCCLUDED)
    {
      // window is minimized, cannot vsync - instead sleep a bit
      if (vsync)
      {
        Sleep(10);
      }
    }
    else if (FAILED(Status))
    {
      FatalError("Failed to present swap chain! Device lost?");
    }
    TimeMeasureElapsed(&TimeData);
  }
}
