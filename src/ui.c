fn void UIReactDiffuseSection(ui_state *State)
{
  reactdiffuse_ui *Req = &State->ReactDiffuseReq;
  {   // SIM PARAMS
    igSpacing();
  }
  {   // SYS CONTROLS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->TexRes,REACTDIFFUSE_MIN_TEX_RES, REACTDIFFUSE_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);
    if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
fn void UIInstancingSection(ui_state *State)
{
  instancing_ui *Req = &State->InstancingReq;
  {   // SIM PARAMS
    igSpacing();
  }
  {   // SYS CONTROLS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->TexRes, INSTANCING_TEX_MIN_RES, INSTANCING_TEX_MAX_RES, NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);
    if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
fn void UIEocSection(ui_state *State)
{
  eoc_ui *Req = &State->EocReq;
  {   // SIM PARAMS
    igSliderFloat("Lambda", (f32 *)&Req->Lambda, 0.0, 1.0, NULL, 0);
    igSliderInt("State Count", (s32 *)&Req->StateCount, 1, EOC_STATE_MAX_COUNT, NULL, 0);
    igSpacing();
  }
  {   // SYS CONTROLS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->TexRes,EOC_TEX_MIN_RES, EOC_TEX_MAX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);
    if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
fn void UICcaSection(ui_state *State)
{
  cca_ui *Req = &State->CcaReq;
  // SIM PARAMS
  {
    igSliderInt("MaxState", (s32 *)&Req->MaxStates, 1, 20, NULL, 0);
    igSliderInt("Threashold", (s32 *)&Req->Threashold, 1, Req->MaxStates, NULL, 0);
    igSliderInt("Search Range", (s32 *)&Req->Range, 1, 5, NULL, 0);
    igSliderInt("OverCount", (s32 *)&Req->OverCount, 1, (2*Req->Range+1)*(2*Req->Range+1), NULL, 0);
    igSpacing();
  }
  // SYS CONTROLS
  {
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->Res, CCA_MIN_TEX_RES, CCA_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
fn void UIBoidsSection(ui_state *State)
{
  boids_ui *Req = &State->BoidsReq;
  {  //SYS PARAMS
    igSliderInt("Agent Count", (s32 *)&Req->AgentCount, 1, BOIDS_MAX_AGENTCOUNT, NULL, 0);
    igSliderInt("Search Range", (s32 *)&Req->SearchRange, 1, 100, NULL, 0);
    igSliderFloat("FieldOfView", (f32 *)&Req->FieldOfView, 0.0, 1.0, NULL, 0);
    igSliderFloat("AlignmentFactor", (f32 *)&Req->AlignmentFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("CohesionFactor", (f32 *)&Req->CohesionFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("SeperationFactor", (f32 *)&Req->SeperationFactor, 0.0, 5.0, NULL, 0);
    igSliderFloat("Max Speed", (f32 *)&Req->MaxSpeed, 0.0, 5.0, NULL, 0);
    igSliderFloat("Max Force", (f32 *)&Req->MaxForce, 0.0, 5.0, NULL, 0);
    igSpacing();
  }
  {  //SYS PARAMS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->Res, BOIDS_MIN_TEX_RES, BOIDS_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    if(igButton("Step", *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
fn void UIPhysarumSection(ui_state *State)
{
  physarum_ui *Req = &State->PhysarumReq;
  {  //SYS PARAMS
    igSliderInt("Agent Count", (s32 *)&Req->AgentCount, 1, 10000, NULL, 0);
    igSliderInt("Search Range", (s32 *)&Req->SearchRange, 1, 5, NULL, 0);
    igSliderFloat("FieldOfView", (f32 *)&Req->FieldOfView, 1, Pi32*2.0, NULL, 0);
    igSpacing();
  }
  {  //SYS PARAMS
    Req->DoStep  = false;
    Req->DoReset = false;
    igSliderInt("Resolution", (s32 *)&Req->Res, BOIDS_MIN_TEX_RES, BOIDS_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    if(igButton("Step", *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
    if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  }
  return;
}
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
    local_persist bool Enabled = 0;
    //igBeginMenu("Record Shader", &Enabled);
    //igEndMenu();
    
    igSeparator();
    igSpacing();
    switch(State->SysKind)
    {
      case SysKind_Cca: UICcaSection(State); break;
      case SysKind_Boids:UIBoidsSection(State); break;
      case SysKind_Physarum:UIPhysarumSection(State); break;
      case SysKind_ReactDiffuse:UIReactDiffuseSection(State); break;
      case SysKind_Instancing:UIInstancingSection(State); break;
      case SysKind_Eoc:UIEocSection(State); break;
      default: 
      {
        igTextWrapped("No UI avilable for this sys!\n");
      } break;
    }
    igEnd();
  }
  
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