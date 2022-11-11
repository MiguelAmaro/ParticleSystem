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
#include "atomics.h"
#include "dx11.h"
#include "dx11.c"
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
#include "boids.h"
#include "physarum.h"
#include "cca.h"
#include "reactdiffuse.h"
#include "instancing.h"
#include "tex3d.h"

// TODO(MIGUEL): Push function for Sys str table
// TODO(MIGUEL): Push function for Sys str table

#include "ui.h"
#include "ui.c"

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR Args, int ArgCount)
{
  OSConsoleCreate();
  ConsoleLog("Welcome To D3D11 Playground!");
  v2s WindowDim = V2s(900, 1080);
  HWND Window = OSWindowCreate(Instance, WindowDim);
  Assert(Window && "Failed to create window");
  OSInitTimeMeasure(&TimeMeasure);
  d3d11_base    D11Base = D3D11InitBase(Window);
  
  ImGuiIO *Io = NULL;
  {
    ImGuiContext* Ctx = igCreateContext(NULL);
    Io = igGetIO();
    Io->ConfigFlags = (ImGuiConfigFlags_NavEnableKeyboard |       // Enable Keyboard Controls
                       ImGuiConfigFlags_DockingEnable     |       // Enable Docking
                       ImGuiConfigFlags_ViewportsEnable   |
                       ImGuiConfigFlags_NavEnableSetMousePos);         // Enable Multi-Viewport / Platform Windows
    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX11_Init(D11Base.Device, D11Base.Context);
    igSetCurrentContext(Ctx);
    ShowWindow(Window, SW_SHOWDEFAULT);
  }
  
  LARGE_INTEGER freq, c1;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c1);
  
  boids          Boids        = BoidsInit(&D11Base);
#if 0
  particlesystem ParticleSystem = CreateParticleSystem(D11Base.Device, 20, (f32)WindowDim.x, (f32)WindowDim.y);
  mm_render      MMRender       = CreateMMRender      (D11Base.Device, D11Base.Context);
  testrend       TestRenderer   = CreateTestRenderer(D11Base.Device, D11Base.Context);
  cca            Cca           = CcaInit(&D11Base);
  physarum       Physarum       = PhysarumInit(&D11Base);
  reactdiffuse   ReactDiffuse = ReactDiffuseInit(&D11Base);
#else
  particlesystem ParticleSystem;
  mm_render      MMRender      ;
  testrend       TestRenderer  ;
  cca            Cca           ;
  physarum       Physarum      ;
  reactdiffuse   ReactDiffuse;
#endif
  // AddSystem( "React Diffuse System"); //no more string table i just declare name here and have and store in buffer. ui can traverse array and pick whatever.
  
  //IMGUI state
  ui_state UIState = {
    .SysKind = SysKind_Boids,
    .BoidsReq = Boids.UIState,
#if 0
    .CcaReq = Cca.UIState,
    .PhysarumReq = Physarum.UIState,
    .ReactDiffuseReq = ReactDiffuse.UIState,
#endif
  };
  //ParticleSystemLoadShaders(&ParticleSystem, D11Base.Device);
  u64 FrameCount = 0;
  b32 Running = 1;
  for (;Running;)
  {
    // process all incoming Windows messages
    if(OSProcessMessges() == 1) break; // If '1' the quit message recieved
    WindowDim = OSWindowGetSize(Window);
    /*
    ParticleSystem.ConstData.transforms.ProjMatrix = M4fOrtho(0.0f, (f32)WindowDim.x,
                                                              0.0f, (f32)WindowDim.y,
                                                                0.0f, 100.0f);
    */
    D3D11UpdateWindowSize(&D11Base, WindowDim);
    // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
    if (D11Base.RTView)
    {
      LARGE_INTEGER c2;
      QueryPerformanceCounter(&c2);
      float delta = (float)((double)(c2.QuadPart - c1.QuadPart) / freq.QuadPart);
      c1 = c2;
      
      v2f WindowDimf = V2f((f32)WindowDim.x, (f32)WindowDim.y);
      v2u WindowDimu = V2u((u32)WindowDim.x, (u32)WindowDim.y);
      // output viewport covering all client area of window
      D11Base.Viewport.TopLeftX = 0;
      D11Base.Viewport.TopLeftY = 0;
      D11Base.Viewport.Width    = WindowDimf.x;
      D11Base.Viewport.Height   = WindowDimf.y;
      D11Base.Viewport.MinDepth = 0;
      D11Base.Viewport.MaxDepth = 1;
      // clear screen
      v4f Color = UIState.ClearColorToggle==true?V4f(0.1f, 0.1f, 0.12f, 1.f ):UIState.ClearColor;
      ID3D11DeviceContext_ClearRenderTargetView(D11Base.Context, D11Base.RTView, Color.e);
      ID3D11DeviceContext_ClearDepthStencilView(D11Base.Context, D11Base.DSView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
      
      /*FOOD FOR THOUGHT
for(sysid=0;sysid<??.Syscount;sysid++ )
{
  if(UIState.SysKind == SysId)
{
for(.Syscount)
{

      D3D11ShaderAsyncHotReload(??.Syss.shaders[shaderid]);

  }
  */
      switch(UIState.SysKind)
      {
        case SysKind_MM:
        {
          MMDraw(&MMRender, &D11Base, WindowDimf, delta);
        } break;
        case SysKind_Test:
        {
          TestDraw(&TestRenderer, &D11Base);
        } break;
        case SysKind_Cca:
        {
          D3D11ShaderHotReload(&D11Base, &Cca.Reset);
          D3D11ShaderHotReload(&D11Base, &Cca.Step);
          D3D11ShaderHotReload(&D11Base, &Cca.Vertex);
          D3D11ShaderHotReload(&D11Base, &Cca.Pixel);
          CcaDraw(&Cca, &D11Base, UIState.CcaReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Boids:
        {
          D3D11ShaderHotReload(&D11Base, &Boids.AgentsReset);
          D3D11ShaderHotReload(&D11Base, &Boids.AgentsMove);
          D3D11ShaderHotReload(&D11Base, &Boids.AgentsTrails);
          D3D11ShaderHotReload(&D11Base, &Boids.AgentsDebug);
          D3D11ShaderHotReload(&D11Base, &Boids.TexReset);
          D3D11ShaderHotReload(&D11Base, &Boids.TexDiffuse);
          D3D11ShaderHotReload(&D11Base, &Boids.Render);
          D3D11ShaderHotReload(&D11Base, &Boids.Vertex);
          D3D11ShaderHotReload(&D11Base, &Boids.Pixel);
          BoidsDraw(&Boids, &D11Base, UIState.BoidsReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Physarum:
        {
          D3D11ShaderHotReload(&D11Base, &Physarum.AgentsReset);
          D3D11ShaderHotReload(&D11Base, &Physarum.AgentsMove);
          D3D11ShaderHotReload(&D11Base, &Physarum.AgentsTrails);
          D3D11ShaderHotReload(&D11Base, &Physarum.AgentsDebug);
          D3D11ShaderHotReload(&D11Base, &Physarum.TexReset);
          D3D11ShaderHotReload(&D11Base, &Physarum.TexDiffuse);
          D3D11ShaderHotReload(&D11Base, &Physarum.Render);
          D3D11ShaderHotReload(&D11Base, &Physarum.Vertex);
          D3D11ShaderHotReload(&D11Base, &Physarum.Pixel);
          PhysarumDraw(&Physarum, &D11Base, UIState.PhysarumReq, FrameCount, WindowDimu);
        } break;
        case SysKind_ReactDiffuse:
        {
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Reset);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.ReactDiffuse);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Pixel);
          ReactDiffuseDraw(&ReactDiffuse, &D11Base, UIState.ReactDiffuseReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Particles:
        {
          //ParticleSystemDraw(&ParticleSystem, context, IsFirstFrame, &viewport, rasterizerState, depthState, blendState, rtView, dsView);
        } break;
        default:
        {} break;
      }
      UIBegin();
      UIControlCluster(&UIState);
      UIEnd(Io);
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
    FrameCount++;
  }
}
