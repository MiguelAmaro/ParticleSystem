#ifndef UI_H
#define UI_H

typedef enum system_kind system_kind;
enum system_kind
{
  SysKind_MM = 0,
  SysKind_Test,
  SysKind_Cca,
  SysKind_Boids,
  SysKind_Physarum,
  SysKind_ReactDiffuse,
  SysKind_Particles,
  SysKind_Instancing,
  SysKind_Tex3d,
  SysKind_Volumetric,
  SysKind_Wfc,
  SysKind_Eoc,
  SysKind_Count,
};
const char *SysStrTable[SysKind_Count] =
{ 
  "MM",
  "Test",
  "Cca",
  "Boids",
  "Physarum",
  "React Diffuse",
  "Particles",
  "Instancing",
  "Tex 3d",
  "Volumetric",
  "Wave function Collapse",
  "Eoc"
};
typedef struct ui_state ui_state;
struct ui_state
{
  v4f  ClearColor;
  bool ClearColorToggle;
  bool TexToggle;
  f32  Slider;
  // No MMReq
  // No TestReq
  cca_ui CcaReq;
  boids_ui BoidsReq;
  physarum_ui PhysarumReq;
  reactdiffuse_ui ReactDiffuseReq;
  instancing_ui InstancingReq;
  tex3d_ui Tex3dReq;
  wfc_ui WfcReq;
  eoc_ui EocReq;
  system_kind SysKind;
  u32 DimgColPushCount;
  u32 DimgVarPushCount;
};

// NOTE(MIGUEL): Meta: I can have something like META_PROJNAME(<name>) definded in a project to 
//                     generate this ui file and tell the user how the ui  stuct whould be called.
//                     This doesn't sovlve the problem of the defining ui functions that have imgui.
//                     calls...
//                     Ideally i would want to define my imgui in the <project>.h file.
//                     
//                     Im kind of leaning toward the idea of having a set of hooks that must be provided
//                     to the  application.
//                     The application can provide a vtable for it's api and the project is then compiled
//                     as a dll that can be reloaded.
//                     

#endif //UI_H
