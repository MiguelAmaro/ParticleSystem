fn void UIControlCluster(ui_state *State)
{
  f32 Framerate = igGetIO()->Framerate;
  ImGuiViewport* MainView = igGetMainViewport();
  igPushStyleColor_U32(ImGuiCol_DockingEmptyBg, 0x00000000);
  igPushStyleColor_U32(ImGuiCol_WindowBg, 0x00000000); //ImGuiDockNodeFlags_PassthruCentralNode
  igDockSpaceOverViewport(MainView, ImGuiDockNodeFlags_None, ImGuiWindowClass_ImGuiWindowClass());
  igPopStyleColor(2);
  {   
    //COMMON
    igBegin("D3D11 Sys Controls", NULL, ImGuiWindowFlags_None);
    igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f/Framerate, Framerate);
    igSliderInt(SysStrTable[State->SysKind], (s32 *)&State->SysKind, 0, SysKind_Count-1, NULL, 0);
    igSeparator();
    igSpacing();
    switch(State->SysKind)
    {
      case SysKind_Cca:
      {
        cca_ui *Req = &State->CcaReq;
        CcaUI(Req);
      } break;
      case SysKind_Boids:
      {
        boids_ui *Req = &State->BoidsReq;
        BoidsUI(Req);
      } break;
      case SysKind_Physarum:
      {
        physarum_ui *Req = &State->PhysarumReq;
        PhysarumUI(Req);
      } break;
      case SysKind_ReactDiffuse:
      {
        reactdiffuse_ui *Req = &State->ReactDiffuseReq;
        ReactDiffuseUI(Req);
      } break;
      case SysKind_Instancing:
      {
        instancing_ui *Req = &State->InstancingReq;
        InstancingUI(Req);
      } break;
      case SysKind_Eoc:
      {
        eoc_ui *Req = &State->EocReq;
        EocUI(Req);
      } break;
      default: 
      {
        igTextWrapped("No UI avilable for this sys!\n");
      } break;
    }
    igEnd();
  }
  local_persist bool Enabled = 0;
  igBegin("Record Shader", NULL, ImGuiWindowFlags_None);
  igEnd();
  igBegin("Share", NULL, ImGuiWindowFlags_None);
  igEnd();
  
  igBegin("D3D11 Messages", NULL, ImGuiWindowFlags_None);
  igTextWrapped("%.*s\n", ShaderLoader.ShaderMsg.Size, ShaderLoader.ShaderMsg.Data);
  igEnd();
  return;
}
fn void UInit(ImGuiIO **Io, HWND Window, d3d11_base *D11Base)
{
  ImGuiContext* Ctx = igCreateContext(NULL);
  *Io = igGetIO();
  {
    ImGuiIO *ConfigIo = *Io;
    ConfigIo->ConfigFlags = (ImGuiConfigFlags_NavEnableKeyboard |       // Enable Keyboard Controls
                             ImGuiConfigFlags_ViewportsEnable   |
                             ImGuiConfigFlags_DockingEnable     |       // Enable Docking
                             ImGuiConfigFlags_NavEnableSetMousePos);    // Enable Multi-Viewport / Platform Windows
    ConfigIo->ConfigViewportsNoDefaultParent = true;
    ConfigIo->ConfigDockingTransparentPayload = true;
  }
  ImGui_ImplWin32_Init(Window);
  ImGui_ImplDX11_Init(D11Base->Device, D11Base->Context);
  igSetCurrentContext(Ctx);
  ShowWindow(Window, SW_SHOWDEFAULT);
}
fn void UIBegin(ui_state *State)
{
  State->DimgColPushCount = 0;
  State->DimgVarPushCount = 0;
  {
#define _CNTVAR(_call) (State->DimgVarPushCount++); (_call)
#define _CNTCOL(_call) (State->DimgColPushCount++); (_call)
    u32 LRed = igGetColorU32_Vec4(*ImVec4_ImVec4_Float(1.0f, 0.2f, 0.2f, 0.5f)); //active
    u32 Red = igGetColorU32_Vec4(*ImVec4_ImVec4_Float(0.8f, 0.0f, 0.0f, 0.5f)); //active or normal
    u32 Grey = igGetColorU32_Vec4(*ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 0.5f)); //normal
    u32 LGrey = igGetColorU32_Vec4(*ImVec4_ImVec4_Float(0.6f, 0.6f, 0.6f, 0.5f)); //hoverd
    f32 r = 5.0f;
    //Rounding
    _CNTVAR(igPushStyleVar_Float(ImGuiStyleVar_PopupRounding, r));
    _CNTVAR(igPushStyleVar_Float(ImGuiStyleVar_WindowRounding,r));
    _CNTVAR(igPushStyleVar_Float(ImGuiStyleVar_FrameRounding, r));
    _CNTVAR(igPushStyleVar_Float(ImGuiStyleVar_ScrollbarRounding, r));
    _CNTVAR(igPushStyleVar_Float(ImGuiStyleVar_GrabRounding, r));
    //Docking
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_DockingPreview, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_DockingEmptyBg, Grey));
    //Buttons
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_Button, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ButtonHovered, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ButtonActive, LRed));
    //Slider
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_SliderGrab, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_SliderGrabActive, LGrey));
    //Srollbar
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ScrollbarBg, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ScrollbarGrab, LGrey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ScrollbarGrabHovered, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ScrollbarGrabActive, LRed));
    //Tabs
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_Tab, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TabHovered, LGrey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TabActive, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TabUnfocused, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TabUnfocusedActive, Red));
    //Title
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TitleBg, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TitleBgActive, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_TitleBgCollapsed, Grey));
    //Header
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_Header, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_HeaderActive, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_HeaderHovered, LGrey));
    //Resize
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ResizeGrip, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ResizeGripHovered, LGrey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ResizeGripActive, Red));
    //Frame
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_FrameBg, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_FrameBgHovered, LGrey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_FrameBgActive, Red));
    //Seperator
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_Separator, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_SeparatorHovered, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_SeparatorActive, LRed));
    //Misc
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_CheckMark, Red));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_MenuBarBg, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_ChildBg, Grey));
    _CNTCOL(igPushStyleColor_U32(ImGuiCol_FrameBgHovered, LGrey));
#undef _CNTVAR
#undef _CNTCOL
  }
  ImGui_ImplWin32_NewFrame();
  ImGui_ImplDX11_NewFrame();
  igNewFrame();
  return;
};
fn void UIEnd(ui_state *State, ImGuiIO *Io)
{
  igRender();
  ImGui_ImplDX11_RenderDrawData(igGetDrawData());
  igEndFrame(); 
  if(Io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    igUpdatePlatformWindows();
    igRenderPlatformWindowsDefault(NULL, NULL);
  }
  igPopStyleVar(State->DimgVarPushCount);
  igPopStyleColor(State->DimgColPushCount);
  return;
}