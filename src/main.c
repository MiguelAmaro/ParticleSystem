#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

// MEDIA FOUNDATION - VIDEO CAPTURE
#include <mfapi.h>
#include <mfidl.h>
#pragma comment (lib, "mf.lib")
#pragma comment (lib, "mfplat.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "mfuuid.lib")

//PRNG
//#include "pcg32.h"
#pragma comment (lib, "advapi32.lib")


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

#include "sort.h"
#include "string.c"
#include "mmath.h"
#include "atomics.h"
#include "images.h"
#include "dx11.h"
#include "dx11.c"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"
#include "capture.h"

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

//PROJECTS
#include "particles/particles.h"
#include "test/test.h"
#include "mm.h"
#include "boids/boids.h"
#include "physarum/physarum.h"
#include "cca/cca.h"
#include "eoc/eoc.h"
#include "wfc/wfc.h"
#include "reactdiffuse/reactdiffuse.h"
#include "volumetric/volumetric.h"
#include "instancing/instancing.h"
#include "tex3d/tex3d.h"
#include "terrain/terrain.h"

//PROJECTS
// TODO(MIGUEL): Push function for Sys str table
// TODO(MIGUEL): Push function for Sys str table

#include "ui.h"
#include "ui.c"

/* NOTE(MIGUEL): 
*                LOG: (12/03/2022)
*                Today i fleshed out imgui stuff more. There is now the ablity to display shader compilation error messages.
               *                The ways it work is very sketchy for the following reasons:
*                - I just read the async_loader structure message string directly from ui code.
               *                - The loader system might be overwriting previous messages emited by previous shader compilations.
*                - 
               *                I am not sure if d3d11 Info query can get the state of a shader compiler. Regardless it seems that it can
               *                query the state of the pipeline and its error which will be usefull.
               *                
               *                My guess for how to compile messages should be stored is:
               *                - d3d11_shader types should each have a block allocated in their systems arena.
*                - they should have a pointer the their respective blocks int their str8s.
               *                - d3d11_base should have and arena and a function that can use d3d11 queryinfo api to get pipeline state
*                  and push whatever on to the base arena.
               *                - shader will all have their messages dumped after hotloading.
               *                
               *                Things to implement/fix:
               *                - The issue will be keep messages on screen and clearing them as appropriate.
               *                - Other things would be to a have the option render to an imgui window instad of main window.
               *                  This involves grabbing the frame buffer from d3d11 and some othe stuff
               *                
               *                - I should push forward more with media foundation.
               *                - I want a way to poast on instagram/twiter from the app.
               *                - I want working matix operations.
               *                - Make "systems" a dll that can be loaded.
               *                - make funcions of "systems" that are call in main loop exported module functions
               *                - metaprogramming. generating boiler plate files (should be a working example)
               *                
               *                Resources for D3D11InfoQuery
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/ne-d3d11sdklayers-d3d11_message_category
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/nn-d3d11sdklayers-id3d11infoqueue
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/nf-d3d11sdklayers-id3d11infoqueue-getmessage
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/nf-d3d11sdklayers-id3d11infoqueue-getnumstoredmessages
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/nf-d3d11sdklayers-id3d11infoqueue-getbreakonid
               *                https://learn.microsoft.com/en-us/windows/win32/api/d3d11sdklayers/ne-d3d11sdklayers-d3d11_message_id
               *                
               *                for imgui
               *                https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples
               *                https://www.youtube.com/watch?v=uQljBVVb-lU
               *                https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-display-an-image-what-is-imtextureid-how-does-it-work
               *                https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
               *                https://gamedev.stackexchange.com/questions/150214/render-in-a-imgui-window
               *                https://github.com/ocornut/imgui/wiki/Glossary
               *                
               *                
               *                for rendering to texture on d3d11 (search term: dx11 render to texture)
               *                https://www.rastertek.com/dx11tut22.html <--(look into this first)
               *                https://stackoverflow.com/questions/48969606/directx-11-render-to-texture
               *                https://stackoverflow.com/questions/10623787/directx-11-framebuffer-capture-c-no-win32-or-d3dx
               *                https://learnopengl.com/Advanced-OpenGL/Framebuffers
               *                
               *                for media foundation
               *                https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/multimedia/mediafoundation
*                
  *                video to file:
  *                https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/Win7Samples/multimedia/mediafoundation/MFCaptureToFile
*                guide: 
*                https://learn.microsoft.com/en-us/windows/win32/medfound/media-foundation-programming-guide
*                https://learn.microsoft.com/en-us/windows/win32/medfound/media-foundation-sdk-samples
*                
*                LOG: (12/04/2022)
*                Found out imgui's ImTextureId is a void * that is expected to point to an srv. I tried to capture 
*                the frame buffer but got garbage. There is another example on how to render to a texture described by
*                https://www.rastertek.com/dx11tut22.html. should give it a try.
*                Acually i seem to be copying the frame buffer somewhat correctly. The issue is that the application must
*                be maximized and in the the left monitor or else there's distortion.
*                
*                
*                
*                
*/               

b32 CompareInts(void *a, void *b)
{
  u32 *numa = (u32 *)a;
  u32 *numb = (u32 *)b;
  b32 Result = *numa>*numb?1:0;
  return Result;
}
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR Args, int ArgCount)
{
  OSConsoleCreate();
  ConsoleLog("Welcome To D3D11 Playground!");
  v2s WindowDim = V2s(900, 1080);
  HWND Window = OSWindowCreate(Instance, WindowDim);
  Assert(Window && "Failed to create window");
  OSInitTimeMeasure(&TimeMeasure);
  {
    thread_ctx ThreadContext = {0};
    ThreadCtxInit(&ThreadContext, OSMemoryAlloc(Gigabytes(1)), Gigabytes(1));
    ThreadCtxSet(&ThreadContext);
  }
  
  d3d11_base D11Base = D3D11InitBase(Window);
  ImGuiIO *Io = NULL;
  
  UInit(&Io, Window, &D11Base);
  
  
  IMFMediaSource *VideoSource; 
  CreateVideoDeviceSource(&VideoSource);
  
  LARGE_INTEGER freq, c1;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c1);
  
  
  // NOTE(MIGUEL): After this assgne the this projects ui struct to the ui_state.
  //
  cca            Cca           = CcaInit(&D11Base);
#if 1
  eoc Eoc        = EocInit(&D11Base);
  reactdiffuse   ReactDiffuse = ReactDiffuseInit(&D11Base);
  wfc Wfc        = WfcInit(&D11Base);
  boids          Boids        = BoidsInit(&D11Base);
  instancing     Instancing    = InstancingInit(&D11Base);
  particlesystem ParticleSystem = CreateParticleSystem(&D11Base, 20, (f32)WindowDim.x, (f32)WindowDim.y);
  mm_render      MMRender       = CreateMMRender      (D11Base.Device, D11Base.Context);
  testrend       TestRenderer   = CreateTestRenderer(&D11Base);
  physarum       Physarum       = PhysarumInit(&D11Base);
#else
  instancing Instancing ;
  boids          Boids;
  reactdiffuse   ReactDiffuse ;
  wfc Wfc        ;
  eoc Eoc;
  particlesystem ParticleSystem;
  mm_render      MMRender      ;
  testrend       TestRenderer  ;
  //cca            Cca           ;
  physarum       Physarum      ;
#endif
  // AddSystem( "React Diffuse System"); //no more string table i just declare name here and have and store in buffer. ui can traverse array and pick whatever.
  
  
  // NOTE(MIGUEL): After this go to the main loops switch and assgin the ui req which should
  //               have be updated by imgu to the project's ui state struct. they are the same type.
  //               and dont forget to set the sys kind
  //IMGUI state
  ui_state UIState = {
    .SysKind = SysKind_Cca,
    .CcaReq = Cca.UIState,
#if 1
    .InstancingReq = Instancing.UIState,
    .EocReq = Eoc.UIState,
    .ReactDiffuseReq = ReactDiffuse.UIState,
    .WfcReq = Wfc.UIState,
    .BoidsReq = Boids.UIState,
    .InstancingReq = Instancing.UIState,
    .PhysarumReq = Physarum.UIState,
#endif
  };
  //ParticleSystemLoadShaders(&ParticleSystem, D11Base.Device);
  u64 FrameCount = 0;
  b32 Running = 1;
  arena MainArena = ArenaInit(NULL, Gigabytes(2), OSMemoryAlloc(Gigabytes(2)));
  
  
  //D3D11Tex2D()
  
  str8 D3D11Msg = {0};
  u32 *CapturedFB = NULL;
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
      
      
      arena_temp Scratch = MemoryGetScratch(NULL, 0);
      D3D11_MESSAGE *Message = {0};
      u64 MessageSize = 0;
      ID3D11InfoQueue_GetMessage(D11Base.Info, 0, NULL, &MessageSize);
      Message = ArenaPushBlock(Scratch.Arena, MessageSize);
      ID3D11InfoQueue_GetMessage(D11Base.Info, 0, Message, &MessageSize);
      if(
         Message->Category==D3D11_MESSAGE_CATEGORY_COMPILATION ||
         Message->Category==D3D11_MESSAGE_CATEGORY_SHADER
         )
      {
        ArenaReset(&MainArena);
        D3D11Msg = Str8FromArena(&MainArena, Message->DescriptionByteLength);
        MemoryCopy((void *)Message->pDescription, D3D11Msg.Size, D3D11Msg.Data, D3D11Msg.Size);
        ConsoleLog(*Scratch.Arena, "%s\n", D3D11Msg.Data);
      }
      MemoryReleaseScratch(Scratch);
      
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
          D3D11ShaderAsyncHotReload(&D11Base, &Cca.Reset);
          D3D11ShaderAsyncHotReload(&D11Base, &Cca.Step);
          D3D11ShaderAsyncHotReload(&D11Base, &Cca.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Cca.Pixel);
          CcaDraw(&Cca, &D11Base, UIState.CcaReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Boids:
        {
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.AgentsReset);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.AgentsMove);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.AgentsTrails);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.AgentsDebug);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.TexReset);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.TexDiffuse);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Boids.Pixel);
          BoidsDraw(&Boids, &D11Base, UIState.BoidsReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Physarum:
        {
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.AgentsReset);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.AgentsMove);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.AgentsTrails);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.AgentsDebug);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.TexReset);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.TexDiffuse);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Physarum.Pixel);
          PhysarumDraw(&Physarum, &D11Base, UIState.PhysarumReq, FrameCount, WindowDimu);
        } break;
        case SysKind_ReactDiffuse:
        {
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.ReactDiffuse);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Reset);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Pixel);
          D3D11ShaderAsyncHotReload(&D11Base, &ReactDiffuse.Bump);
          ReactDiffuseDraw(&ReactDiffuse, &D11Base, UIState.ReactDiffuseReq, FrameCount, WindowDimu);
        } break;
        case SysKind_Instancing:
        {
          Instancing.UIState = UIState.InstancingReq;
          InstancingUpdate(&Instancing, FrameCount, WindowDimu);
          D3D11ShaderAsyncHotReload(&D11Base, &Instancing.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Instancing.Pixel);
          D3D11ShaderAsyncHotReload(&D11Base, &Instancing.Step);
          D3D11ShaderAsyncHotReload(&D11Base, &Instancing.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &Instancing.Reset);
          InstancingDraw(&Instancing, &D11Base, FrameCount, WindowDimu);
        } break;
        case SysKind_Wfc:
        {
          D3D11ShaderAsyncHotReload(&D11Base, &Wfc.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Wfc.Pixel);
          WfcDraw(&Wfc, &D11Base, FrameCount, WindowDimu);
        } break;
        case SysKind_Eoc:
        {
          Eoc.UIState = UIState.EocReq;
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.Vertex);
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.Pixel);
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.Reset);
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.Render);
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.Step);
          D3D11ShaderAsyncHotReload(&D11Base, &Eoc.CopyDown);
          EocDraw(&Eoc, &D11Base, FrameCount, WindowDimu);
        } break;
        case SysKind_Particles:
        {
          //ParticleSystemDraw(&ParticleSystem, context, IsFirstFrame, &viewport, rasterizerState, depthState, blendState, rtView, dsView);
        } break;
        default:
        {} break;
      }
      // NOTE(MIGUEL): After add in shader reloading go to the ui.h file and .c file and make sure
      //               everything is hooked up.
      UIBegin(&UIState);
      UIControlCluster(&UIState);
      igBegin("Textures", NULL, ImGuiWindowFlags_None);
      switch(UIState.SysKind)
      {
        case SysKind_Cca: CcaDisplayTextures(&Cca); break;
        case SysKind_ReactDiffuse: ReactDiffuseDisplayTextures(&ReactDiffuse); break;
        default: break;
      }
      igEnd();
      ID3D11ShaderResourceView* SRVDisplayTex;
#if 1
      {
        igBegin("Framebuffer", NULL, ImGuiWindowFlags_None);
        ID3D11Texture2D* FBTex;
        HRESULT Status = IDXGISwapChain1_GetBuffer(D11Base.SwapChain, 0, &IID_ID3D11Texture2D, &FBTex);
        
        ID3D11Texture2D* CapturedTex;
        D3D11ScopedBase(&D11Base)
        {
          D3D11Tex2DStage(&CapturedTex, WindowDim, NULL, sizeof(u32), DXGI_FORMAT_R8G8B8A8_UNORM);
          u32 PixelCount = WindowDim.x*WindowDim.y;
          
          Scratch = MemoryGetScratch(NULL, 0);
          CapturedFB = ArenaPushArray(Scratch.Arena, PixelCount, u32);
          
          ID3D11DeviceContext_CopyResource(D11Base.Context, (ID3D11Resource *)CapturedTex, (ID3D11Resource *)FBTex);
          D3D11GPUMemoryRead(D11Base.Context, (ID3D11Resource *)CapturedTex, CapturedFB, sizeof(u32), PixelCount);
          ID3D11Texture2D_Release(CapturedTex);
          ID3D11Texture2D_Release(FBTex);
          // Imgui add framebuffer to window
          
          D3D11Tex2D(NULL, &SRVDisplayTex, NULL, WindowDim, CapturedFB, sizeof(u32), Unorm_RGBA,
                     Usage_Default, 0);
          MemoryReleaseScratch(Scratch);
        }
        
        ImVec2 Pos;
        igGetCursorScreenPos(&Pos);
        ImVec2 Dim;
        igGetWindowSize(&Dim);
        //*ImVec2_ImVec2_Float(WindowDimf.x, WindowDimf.y)
        igImage(SRVDisplayTex, Dim,
                *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
                *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
                *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
        igEnd();
      }
#endif
      UIEnd(&UIState, Io);
      if(SRVDisplayTex) ID3D11ShaderResourceView_Release(SRVDisplayTex);
    }
    {
      // change to FALSE to disable vsync
      HRESULT Status;
      BOOL Vsync = TRUE;
      Status = IDXGISwapChain1_Present(D11Base.SwapChain, Vsync?1:0, 0);
      if (Status == DXGI_STATUS_OCCLUDED)
      {
        // window is minimized, cannot vsync - instead sleep a bit
        if (Vsync) { Sleep(10); }
      }
      else if (FAILED(Status))
      {
        FatalError("Failed to present swap chain! Device lost?");
      }
    }
    
    FrameCount++;
  }
}
