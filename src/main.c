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
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif
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

#include "particles.h"
#include "test.h"
#include "mm.h"

typedef struct ui_state ui_state;
struct ui_state 
{
  v4f  ClearColor;
  bool ClearColorToggle;
  f32  Slider;
};


void UIControlCluster(ui_state *State)
{
  f32 Framerate = igGetIO()->Framerate;
  ImGuiViewport* main_viewport = igGetMainViewport();
  igSetNextWindowPos(*ImVec2_ImVec2_Float(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver, *ImVec2_ImVec2_Float(0.0,0.0));
  igBegin("Hello, world!", NULL, 0);                          // Create a window called "Hello, world!" and append into it.
  igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f/Framerate, Framerate);
  igCheckbox("Toggle Background Color", &State->ClearColorToggle);      // Edit bools storing our window open/close state
  igColorEdit3("Clear Color", State->ClearColor.e, 0); // Edit 3 floats representing a color
  igSliderFloat("Slider", &State->Slider, 0.0f, 1.0f, NULL, 0);            // Edit 1 float using a slider from 0.0f to 1.0f
  igEnd();
  return;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR Args, int ArgCount)
{
  OSConsoleCreate();
  ConsoleLog("Welcome To D3D11 Playground!");
  v2s WindowDim = V2s(CW_USEDEFAULT, CW_USEDEFAULT);
  HWND Window = OSWindowCreate(Instance, WindowDim);
  Assert(Window && "Failed to create window");
  
  d3d11_base    D11Base = D3D11InitBase(Window);
  ImGuiContext* Ctx = igCreateContext(NULL);
  ImGuiIO *Io = igGetIO();
  Io->ConfigFlags = (ImGuiConfigFlags_NavEnableKeyboard |       // Enable Keyboard Controls
                     ImGuiConfigFlags_DockingEnable     |       // Enable Docking
                     ImGuiConfigFlags_ViewportsEnable   |
                     ImGuiConfigFlags_NavEnableSetMousePos);         // Enable Multi-Viewport / Platform Windows
  ImGui_ImplWin32_Init(Window);
  ImGui_ImplDX11_Init(D11Base.Device, D11Base.Context);
  igSetCurrentContext(Ctx);
  
  time_measure   TimeData       = OSInitTimeMeasure();
  particlesystem ParticleSystem = CreateParticleSystem(D11Base.Device, 20, (f32)WindowDim.x, (f32)WindowDim.y);
  //mm_render      MMRender       = CreateMMRender(device, context);
  testrend       TestRenderer   = CreateTestRenderer(D11Base.Device, D11Base.Context);
  
  // show the window
  ShowWindow(Window, SW_SHOWDEFAULT);
  
  
  ParticleSystemLoadShaders(&ParticleSystem, D11Base.Device);
  
  LARGE_INTEGER freq, c1;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c1);
  
  //IMGUI state
  ui_state UIState = {0};
  b32 IsFirstFrame = 1;
  b32 Running = 1;
  for (;Running;)
  {
    // process all incoming Windows messages
    if(OSProcessMessges() == 1) break; // If '1' the quit message recieved
    WindowDim = OSWindowGetSize(Window);
    ParticleSystem.ConstData.transforms.ProjMatrix = M4fOrtho(0.0f, (f32)WindowDim.x,
                                                              0.0f, (f32)WindowDim.y,
                                                              0.0f, 100.0f);
    ParticleSystem.ConstData.sim_params.TimeFactors = V4f((f32)TimeData.MSDelta, 0.0f, 0.0f, 0.0f);
    D3D11UpdateWindowSize(&D11Base, WindowDim);
    // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
    if (D11Base.RTView)
    {
      LARGE_INTEGER c2;
      QueryPerformanceCounter(&c2);
      float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
      c1 = c2;
      
      // output viewport covering all client area of window
      D11Base.Viewport.TopLeftX = 0;
      D11Base.Viewport.TopLeftY = 0;
      D11Base.Viewport.Width    =  (FLOAT)WindowDim.x;
      D11Base.Viewport.Height   = (FLOAT)WindowDim.y;
      D11Base.Viewport.MinDepth = 0;
      D11Base.Viewport.MaxDepth = 1;
      // clear screen
      v4f Color = UIState.ClearColorToggle==true?V4f(0.1f, 0.1f, 0.12f, 1.f ):UIState.ClearColor;
      ID3D11DeviceContext_ClearRenderTargetView(D11Base.Context, D11Base.RTView, Color.e);
      ID3D11DeviceContext_ClearDepthStencilView(D11Base.Context, D11Base.DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
      //Working!!!
      //MMDraw(&MMRender, height, width, delta, context, viewport, rasterizerState, depthState, blendState, rtView, dsView);
      TestDraw(&TestRenderer, &D11Base, IsFirstFrame);
      
      //Brokent!!!
      //ParticleSystemDraw(&ParticleSystem, context, IsFirstFrame, &viewport, rasterizerState, depthState, blendState, rtView, dsView);
      
      ImGui_ImplWin32_NewFrame();
      ImGui_ImplDX11_NewFrame();
      igNewFrame();
      UIControlCluster(&UIState);
      igRender();
      ImGui_ImplDX11_RenderDrawData(igGetDrawData());
      igEndFrame(); 
      if (Io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
      {
        igUpdatePlatformWindows();
        igRenderPlatformWindowsDefault(NULL, NULL);
      }
      IsFirstFrame = 0;
    }
    
    // change to FALSE to disable vsync
    HRESULT Status;
    BOOL vsync = TRUE;
    Status = IDXGISwapChain1_Present(D11Base.SwapChain, vsync ? 1 : 0, 0);
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
    OSTimeMeasureElapsed(&TimeData);
  }
}
