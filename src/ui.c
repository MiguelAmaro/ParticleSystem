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
  igCheckbox("Alignment", (bool *)&Req->ApplyAlignment);      // Edit bools storing our window open/close state
  igCheckbox("Cohesion", (bool *)&Req->ApplyCohesion);      // Edit bools storing our window open/close state
  igCheckbox("Seperation", (bool *)&Req->ApplySeperation);      // Edit bools storing our window open/close state
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
  igSetNextWindowPos(*ImVec2_ImVec2_Float(0, 0), 0, *ImVec2_ImVec2_Float(0.0,0.0));
  igBegin("Hello, world!", NULL, 0);                          // Create a window called "Hello, world!" and append into it.
  igText("Application average %.3f ms/frame (%.1f FPS)", 1000.0f/Framerate, Framerate);
  igCheckbox("Toggle Background Color", &State->ClearColorToggle);      // Edit bools storing our window open/close state
  igCheckbox("Toggle Tex", &State->TexToggle);      // Edit bools storing our window open/close state
  igColorEdit3("Clear Color", State->ClearColor.e, 0); // Edit 3 floats representing a color
  igSliderFloat("Slider", &State->Slider, 0.0f, 1.0f, NULL, 0);            // Edit 1 float using a slider from 0.0f to 1.0f
  igSliderInt(SysStrTable[State->SysKind], (s32 *)&State->SysKind, 0, SysKind_Count, NULL, 0);            // Edit 1 float using a slider from 0.0f to 1.0f
  //arena Arena; ArenaLocalInit(Arena, 256);
  switch(State->SysKind)
  {
    case SysKind_Cca: UICcaSection(State); break;
    case SysKind_Boids:UIBoidsSection(State); break;
    case SysKind_Physarum:UIPhysarumSection(State); break;
    case SysKind_ReactDiffuse:UIReactDiffuseSection(State); break;
    default: ConsoleLog("No UI avilable for this sys!\n");
  }
  igEnd();
  return;
}