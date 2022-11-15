#ifndef UI_H
#define UI_H

typedef enum system_kind system_kind;
enum system_kind
{
  SysKind_Test,
  SysKind_MM,
  SysKind_Cca,
  SysKind_Boids,
  SysKind_Physarum,
  SysKind_ReactDiffuse,
  SysKind_Particles,
  SysKind_Instancing,
  SysKind_Tex3d,
  SysKind_Volumetric,
  SysKind_Count,
};
const char *SysStrTable[] =
{ "Test", "MM", "Cca", "Boids", "Physarum", "React Diffuse", "Particles", "Instancing", "Tex 3d", "Volumetric"};
typedef struct ui_state ui_state;
struct ui_state 
{
  v4f  ClearColor;
  bool ClearColorToggle;
  bool TexToggle;
  f32  Slider;
  cca_ui CcaReq;
  boids_ui BoidsReq;
  physarum_ui PhysarumReq;
  reactdiffuse_ui ReactDiffuseReq;
  instancing_ui InstancingReq;
  tex3d_ui Tex3dReq;
  system_kind SysKind;
};

#endif //UI_H
