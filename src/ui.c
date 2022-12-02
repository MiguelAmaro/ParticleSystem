fn void UIBegin(void)
{
  ImGui_ImplWin32_NewFrame();
  ImGui_ImplDX11_NewFrame();
  igNewFrame();
  return;
};
fn void UIEnd(ImGuiIO *Io)
{
  igRender();
  ImGui_ImplDX11_RenderDrawData(igGetDrawData());
  igEndFrame(); 
  if (Io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    igUpdatePlatformWindows();
    igRenderPlatformWindowsDefault(NULL, NULL);
  }
  return;
}
fn void UIReactDiffuseSection(ui_state *State)
{
  reactdiffuse_ui *Req = &State->ReactDiffuseReq;
  igSliderInt("Resolution", (s32 *)&Req->TexRes,REACTDIFFUSE_MIN_TEX_RES, REACTDIFFUSE_MAX_TEX_RES , NULL, 0);
  igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
  igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
  igCheckbox("Auto Step", (bool *)&Req->AutoStep);
  Req->DoStep  = false;
  Req->DoReset = false;
  if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
  if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  return;
}
fn void UIInstancingSection(ui_state *State)
{
  instancing_ui *Req = &State->InstancingReq;
  // SIM PARAMS
  {
    
  }
  // SYS CONTROLS
  {
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
  igSliderInt("Resolution", (s32 *)&Req->TexRes,EOC_TEX_MIN_RES, EOC_TEX_MAX_RES , NULL, 0);
  igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
  igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
  igSliderFloat("Lambda", (f32 *)&Req->Lambda, 0.0, 1.0, NULL, 0);
  igSliderInt("State Count", (s32 *)&Req->StateCount, 1, EOC_STATE_MAX_COUNT, NULL, 0);
  
  
  igCheckbox("Auto Step", (bool *)&Req->AutoStep);
  Req->DoStep  = false; Req->DoReset = false;
  if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
  if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  return;
}
fn void UICcaSection(ui_state *State)
{
  cca_ui *Req = &State->CcaReq;
  Req->DoStep  = false;
  Req->DoReset = false;
  igSliderInt("Resolution", (s32 *)&Req->Res, CCA_MIN_TEX_RES, CCA_MAX_TEX_RES , NULL, 0);
  igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
  igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
  igSliderInt("MaxState", (s32 *)&Req->MaxStates, 1, 20, NULL, 0);
  igSliderInt("Threashold", (s32 *)&Req->Threashold, 1, Req->MaxStates, NULL, 0);
  igSliderInt("Search Range", (s32 *)&Req->Range, 1, 5, NULL, 0);
  igSliderInt("OverCount", (s32 *)&Req->OverCount, 1, (2*Req->Range+1)*(2*Req->Range+1), NULL, 0);
  igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
  if(igButton("Step" , *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
  if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  return;
}
fn void UIBoidsSection(ui_state *State)
{
  boids_ui *Req = &State->BoidsReq;
  Req->DoStep  = false;
  Req->DoReset = false;
  igSliderInt("Resolution", (s32 *)&Req->Res, BOIDS_MIN_TEX_RES, BOIDS_MAX_TEX_RES , NULL, 0);
  igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
  igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
  igSliderInt("Agent Count", (s32 *)&Req->AgentCount, 1, BOIDS_MAX_AGENTCOUNT, NULL, 0);
  igSliderInt("Search Range", (s32 *)&Req->SearchRange, 1, 100, NULL, 0);
  igSliderFloat("FieldOfView", (f32 *)&Req->FieldOfView, 0.0, 1.0, NULL, 0);
  
  igSliderFloat("Max Force", (f32 *)&Req->MaxForce, 0.0, 5.0, NULL, 0);
  igSliderFloat("Max Speed", (f32 *)&Req->MaxSpeed, 0.0, 5.0, NULL, 0);
  
  igSliderFloat("AlignmentFactor", (f32 *)&Req->AlignmentFactor, 0.0, 5.0, NULL, 0);
  igSliderFloat("CohesionFactor", (f32 *)&Req->CohesionFactor, 0.0, 5.0, NULL, 0);
  igSliderFloat("SeperationFactor", (f32 *)&Req->SeperationFactor, 0.0, 5.0, NULL, 0);
  
  igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
  if(igButton("Step", *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
  if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  return;
}
fn void UIPhysarumSection(ui_state *State)
{
  physarum_ui *Req = &State->PhysarumReq;
  Req->DoStep  = false;
  Req->DoReset = false;
  igSliderInt("Resolution", (s32 *)&Req->Res, BOIDS_MIN_TEX_RES, BOIDS_MAX_TEX_RES , NULL, 0);
  igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
  igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
  igSliderInt("Agent Count", (s32 *)&Req->AgentCount, 1, 10000, NULL, 0);
  igSliderInt("Search Range", (s32 *)&Req->SearchRange, 1, 5, NULL, 0);
  igSliderFloat("FieldOfView", (f32 *)&Req->FieldOfView, 1, Pi32*2.0, NULL, 0);
  igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
  if(igButton("Step", *ImVec2_ImVec2_Float(0, 0))) { Req->DoStep = true; }
  if(igButton("Reset", *ImVec2_ImVec2_Float(0, 0))) { Req->DoReset = true; }
  return;
}
fn void UIControlCluster(ui_state *State)
{
  f32 Framerate = igGetIO()->Framerate;
  ImGuiViewport* main_viewport = igGetMainViewport();
  //COMMON
  igSetNextWindowPos(*ImVec2_ImVec2_Float(0, 0), 0, *ImVec2_ImVec2_Float(0.0,0.0));
  igBegin("Hello, world!", NULL, 0);
  igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f/Framerate, Framerate);
  igCheckbox("Toggle Background Color", &State->ClearColorToggle);
  igCheckbox("Toggle Tex", &State->TexToggle);
  igColorEdit3("Clear Color", State->ClearColor.e, 0);
  igSliderFloat("Slider", &State->Slider, 0.0f, 1.0f, NULL, 0);
  igSliderInt(SysStrTable[State->SysKind], (s32 *)&State->SysKind, 0, SysKind_Count-1, NULL, 0);
  //END COMMON
  switch(State->SysKind)
  {
    case SysKind_Cca: UICcaSection(State); break;
    case SysKind_Boids:UIBoidsSection(State); break;
    case SysKind_Physarum:UIPhysarumSection(State); break;
    case SysKind_ReactDiffuse:UIReactDiffuseSection(State); break;
    case SysKind_Instancing:UIInstancingSection(State); break;
    case SysKind_Eoc:UIEocSection(State); break;
    default: ConsoleLog("No UI avilable for this sys!\n");
  }
  igEnd();
  return;
}