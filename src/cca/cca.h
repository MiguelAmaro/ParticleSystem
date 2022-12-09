#ifndef CCA_H
#define CCA_H

#define CCA_MIN_TEX_RES (256)
#define CCA_MAX_TEX_RES (2048)
#define CCA_AGENTS_PER_THREADGROUP 64
#define CCA_PIXELS_PER_THREADGROUP 32
typedef struct cca_consts cca_consts;
struct16 cca_consts
{
  //CORE
  v2u UWinRes;
  v2u UTexRes;
  u32 UFrameCount;
  u32 UStepCount;
  //PRIMARY
  f32 UStateCountPrimary;
  f32 UThreasholdPrimary;
  s32 URangePrimary;
  b32 UDoMoorePrimary;
  //SECONDARY
  f32 UStateCountSecondary;
  f32 UThreasholdSecondary;
  s32 URangeSecondary;
  b32 UDoMooreSecondary;
};
typedef struct cca_ui cca_ui;
struct cca_ui
{
  //CORE
  s32 Res;
  s32 StepsPerFrame;
  s32 StepMod;
  b32 DoStep;
  b32 AutoStep;
  b32 DoReset;
  //PRIMARY
  s32 StateCountPrimary;
  s32 ThreasholdPrimary;
  s32 RangePrimary;
  b32 DoMoorePrimary;
  //SECONDARY
  s32 StateCountSecondary;
  s32 ThreasholdSecondary;
  s32 RangeSecondary;
  b32 DoMooreSecondary;
};
typedef struct cca cca;
struct cca
{
  ID3D11InputLayout        *Layout;
  ID3D11ShaderResourceView **SelectedTex;
  v2s TexRes;
  ID3D11Texture2D           *TexRead;
  ID3D11ShaderResourceView  *SRViewTexRead;
  ID3D11SamplerState        *SamTexRead;
  
  ID3D11Texture2D           *TexWrite;
  ID3D11ShaderResourceView  *SRViewTexWrite;
  ID3D11UnorderedAccessView *UAViewTexWrite;
  ID3D11SamplerState        *SamTexWrite;
  
  ID3D11Texture2D           *TexRender;
  ID3D11ShaderResourceView  *SRViewTexRender;
  ID3D11UnorderedAccessView *UAViewTexRender;
  ID3D11SamplerState        *SamTexRender;
  
  ID3D11Texture2D           *TexKusu;
  ID3D11ShaderResourceView  *SRViewTexKusu;
  ID3D11SamplerState        *SamTexKusu;
  
  ID3D11Texture2D           *TexSwapStage;
  
  ID3D11Buffer             *VBuffer;
  ID3D11Buffer             *Consts;
  //Shader
  d3d11_shader Reset;
  d3d11_shader Step;
  d3d11_shader Vertex;
  d3d11_shader Pixel;
  arena Arena; //only textures
  image Image;
  cca_ui UIState;
};
cca_ui CCaUIStateInit(void)
{
  cca_ui Result =
  {
    //CORE
    .Res = 512, //CCA_MAX_TEX_RES,
    .StepMod = 1,
    .StepsPerFrame = 1,
    .AutoStep = 1,
    //PRIMARY
    .ThreasholdPrimary = 4,
    .StateCountPrimary = 10,
    .RangePrimary = 1,
    .DoMoorePrimary = 0,
    //SECONDARY
    .StateCountSecondary = 4,
    .ThreasholdSecondary = 10,
    .RangeSecondary = 1,
    .DoMooreSecondary = 0,
  };
  return Result;
}
#define CCA_UI_MAX_STATES 20
#define CCA_UI_MAX_THREASHOLD 25
#define CCA_UI_MAX_RANGE 10
fn void CcaUI(cca_ui *Req)
{
  // SIM PARAMS
  {
    //PRIMARY
    igText("Primary");
    igSliderInt("Search Range", (s32 *)&Req->RangePrimary, 1, CCA_UI_MAX_RANGE, NULL, 0);
    igSliderInt("Threashold", (s32 *)&Req->ThreasholdPrimary, 1, CCA_UI_MAX_THREASHOLD, NULL, 0);
    igSliderInt("MaxState", (s32 *)&Req->StateCountPrimary, 1, CCA_UI_MAX_STATES, NULL, 0);
    igCheckbox("Moore Neighborhood", (bool *)&Req->DoMoorePrimary);
    igSpacing();
    //SECONDARY
    igText("Secondary");
    igSliderInt("Search Range##2", (s32 *)&Req->RangeSecondary, 1, CCA_UI_MAX_RANGE, NULL, 0);
    igSliderInt("Threashold##2", (s32 *)&Req->ThreasholdSecondary, 1, CCA_UI_MAX_THREASHOLD, NULL, 0);
    igSliderInt("MaxState##2", (s32 *)&Req->StateCountSecondary, 1, CCA_UI_MAX_STATES, NULL, 0);
    igCheckbox("Moore Neighborhood###2", (bool *)&Req->DoMooreSecondary);
    igSpacing();
  }
  // SYS CONTROLS
  {
    ImVec2 dummy = *ImVec2_ImVec2_Float(0, 0);
    igSliderInt("Resolution", (s32 *)&Req->Res, CCA_MIN_TEX_RES, CCA_MAX_TEX_RES , NULL, 0);
    igSliderInt("Steps Per Frame", (s32 *)&Req->StepsPerFrame, 1, 50, NULL, 0);
    igSliderInt("Steps Mod", (s32 *)&Req->StepMod, 1, 120, NULL, 0);
    igCheckbox("Auto Step", (bool *)&Req->AutoStep);      // Edit bools storing our window open/close state
    Req->DoStep  = igButton("Step" , dummy)?true:false;
    Req->DoReset = igButton("Reset", dummy)?true:false;
  }
  return;
}
fn void CcaDisplayTextures(cca *Cca)
{
  ImVec2 Dim;
  igGetWindowSize(&Dim);
  igColumns(3, NULL, 0);
  igText("read");
  igImage(Cca->SRViewTexRead, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  igNextColumn();
  igText("write");
  igImage(Cca->SRViewTexWrite, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  igNextColumn();
  igText("render");
  igImage(Cca->SRViewTexRender, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  igImage(Cca->SRViewTexKusu, Dim,
          *ImVec2_ImVec2_Float(0.0f, 1.0f), *ImVec2_ImVec2_Float(1.0f, 0.0f),
          *ImVec4_ImVec4_Float(1.0f, 1.0f, 1.0f, 1.0f), 
          *ImVec4_ImVec4_Float(0.2f, 0.2f, 0.2f, 1.0f));
  return;
}
fn cca CcaInit(d3d11_base *Base)
{
  D3D11BaseDestructure(Base);
  cca Result = {0};
  Result.UIState = CCaUIStateInit();
  u64 MemSize = Gigabytes(2);
  Result.Arena = ArenaInit(NULL, MemSize, OSMemoryAlloc(MemSize));
  s32 Res = Result.UIState.Res;
  Result.TexRes = V2s(Res, Res);
  Result.Image = ImageLoad(Str8("F:\\Dev\\ParticleSystem\\res\\kusu.png"), &Result.Arena);
  struct vert { v3f Pos; v3f TexCoord; }; // NOTE(MIGUEL): update changes in draw.
  struct vert Data[6] =
  {
    //Tri A
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  0.0f, 0.0f } },
    { {  1.0f, -1.0f, 0.0f }, {  1.0f,  0.0f, 0.0f } },
    //Tri B
    { {  1.0f,  1.0f, 0.0f }, {  1.0f,  1.0f, 0.0f } },
    { { -1.0f, -1.0f, 0.0f }, {  0.0f,  0.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f }, {  0.0f,  1.0f, 0.0f } },
  };
  D3D11ScopedBase(Base)
  {
    D3D11BufferConstant(&Result.Consts, NULL, sizeof(cca_consts), Usage_Dynamic, Access_Write);
    D3D11BufferVertex(&Result.VBuffer, Data, sizeof(struct vert), 6);
    D3D11Tex2D(&Result.TexKusu, &Result.SRViewTexKusu, NULL,
               Result.Image.Dim, Result.Image.Data, Result.Image.Stride, Unorm_R, Usage_Default, 0);
    D3D11Tex2D(&Result.TexRead, &Result.SRViewTexRead, NULL,
               Result.TexRes, NULL, sizeof(s32), Float_R, Usage_Default, 0);
    D3D11Tex2D(&Result.TexWrite, &Result.SRViewTexWrite, &Result.UAViewTexWrite,
               Result.TexRes, NULL, sizeof(f32), Float_R, Usage_Default, 0);
    D3D11Tex2D(&Result.TexRender,
               &Result.SRViewTexRender, &Result.UAViewTexRender,
               Result.TexRes, NULL, sizeof(v4f), Float_RGBA, Usage_Default, 0);
    D3D11Tex2DStage(&Result.TexSwapStage, Result.TexRes, NULL, sizeof(f32), Float_R); // Swap Stage
  }
  //SAMPLERS
  {
    D3D11_SAMPLER_DESC Desc = {0};
    Desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexRead);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexWrite);
    ID3D11Device_CreateSamplerState(Device, &Desc, &Result.SamTexRender);
  }
  
  D3D11_INPUT_ELEMENT_DESC Desc[] =
  {
    { "IAPOS"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, Pos     ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "IATEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vert, TexCoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  str8 ShaderFile = Str8("F:\\Dev\\ParticleSystem\\src\\cca\\cca.hlsl");
  Result.Reset = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("ResetKernel"), NULL, 0, Base);
  Result.Step  = D3D11ShaderCreate(ShaderKind_Compute, ShaderFile, Str8("StepKernel"), NULL, 0, Base);
  Result.Vertex = D3D11ShaderCreate(ShaderKind_Vertex, ShaderFile, Str8("VSMain"), Desc, ArrayCount(Desc), Base);
  Result.Pixel  = D3D11ShaderCreate(ShaderKind_Pixel, ShaderFile, Str8("PSMain"), NULL, 0, Base);
  return Result;
}
fn void CcaStep(cca *Cca, d3d11_base *Base, cca_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Cca->Step.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Cca->Consts, &Consts, sizeof(cca_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_CSSetShaderResources     (Context, 0, 1, &Cca->SRViewTexRead);   // int
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->UAViewTexWrite, NULL);  // int
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 1, 1, &Cca->UAViewTexRender, NULL); // float4
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Cca->TexRead, &Cca->TexWrite, Cca->TexSwapStage);
  return;
}
fn void CcaReset(cca *Cca, d3d11_base *Base, cca_consts Consts)
{
  D3D11BaseDestructure(Base);
  u32 GroupCount = Max(1, Consts.UTexRes.x/BOIDS_PIXELS_PER_THREADGROUP);
  ID3D11DeviceContext_CSSetShader(Context, Cca->Reset.ComputeHandle, NULL, 0);
  D3D11GPUMemoryWrite(Context, Cca->Consts, &Consts, sizeof(cca_consts), 1);
  ID3D11DeviceContext_CSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_CSSetShaderResources(Context, 0, 1, &Cca->SRViewTexKusu); // int
  ID3D11DeviceContext_CSSetUnorderedAccessViews(Context, 0, 1, &Cca->UAViewTexWrite, NULL); // int
  ID3D11DeviceContext_Dispatch(Context, GroupCount, GroupCount, 1);
  D3D11ClearComputeStage(Context);
  D3D11Tex2DSwap(Context, &Cca->TexRead, &Cca->TexWrite, Cca->TexSwapStage);
  return;
}
fn void CcaDraw(cca *Cca, d3d11_base *Base, cca_ui UIReq, u64 FrameCount, v2u WinRes)
{
  D3D11BaseDestructure(Base);
  local_persist u32 StepCount = 0;
  
  // CCA PASS
  // TODO(MIGUEL): fix hlsl side to match this. the packing might not work how it is.
  cca_consts Consts = {
    //CORE
    .UWinRes = WinRes,
    .UTexRes = V2u((u32)Cca->TexRes.x, (u32)Cca->TexRes.y),
    .UStepCount = (u32)StepCount,
    .UFrameCount = (u32)FrameCount,
    //PRIMARY
    .UStateCountPrimary  = (f32)UIReq.StateCountPrimary,
    .URangePrimary      = UIReq.RangePrimary,
    .UThreasholdPrimary = (f32)UIReq.ThreasholdPrimary,
    .UDoMoorePrimary = UIReq.DoMoorePrimary,
    //SECONDARY
    .UStateCountSecondary  = (f32)UIReq.StateCountSecondary,
    .URangeSecondary      = UIReq.RangeSecondary,
    .UThreasholdSecondary = (f32)UIReq.ThreasholdSecondary,
    .UDoMooreSecondary = UIReq.DoMooreSecondary,
  };
  if((UIReq.DoStep || UIReq.AutoStep) && ((FrameCount%UIReq.StepMod)==0))
  {
    CcaStep(Cca, Base, Consts);
    StepCount++;
  }
  if(UIReq.DoReset)
  {
    CcaReset(Cca, Base, Consts);
    StepCount = 0;
  }
  // DRAW PASS
  //Input Assempler
  // NOTE(MIGUEL): the same struct is in the init function and may change. this is lazy.
  struct vert { v3f Pos; v3f TexCoord; };
  UINT Stride = sizeof(struct vert);
  UINT Offset = 0;
  ID3D11DeviceContext_IASetInputLayout(Context, Cca->Vertex.Layout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_IASetVertexBuffers(Context, 0, 1, &Cca->VBuffer, &Stride, &Offset);
  // Vertex Shader
  ID3D11DeviceContext_VSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_VSSetShader(Context, Cca->Vertex.VertexHandle, NULL, 0);
  // Rasterizer Stage
  ID3D11DeviceContext_RSSetViewports(Context, 1, &Viewport);
  ID3D11DeviceContext_RSSetState(Context, RastState);
  // Pixel Shader
  ID3D11DeviceContext_PSSetConstantBuffers(Context, 0, 1, &Cca->Consts);
  ID3D11DeviceContext_PSSetSamplers       (Context, 0, 1, &Cca->SamTexRender);
  ID3D11DeviceContext_PSSetShaderResources(Context, 0, 1, &Cca->SRViewTexRender);
  ID3D11DeviceContext_PSSetSamplers       (Context, 1, 1, &Cca->SamTexWrite);
  ID3D11DeviceContext_PSSetShaderResources(Context, 1, 1, &Cca->SRViewTexWrite);
  ID3D11DeviceContext_PSSetShader(Context, Cca->Pixel.PixelHandle, NULL, 0);
  // Output Merger
  ID3D11DeviceContext_OMSetBlendState(Context, BlendState, NULL, ~0U);
  ID3D11DeviceContext_OMSetDepthStencilState(Context, DepthState, 0);
  ID3D11DeviceContext_OMSetRenderTargets(Context, 1, &RTView, DSView);
  ID3D11DeviceContext_Draw(Context, 6, 0);
  D3D11ClearPipeline(Context);
  return;
}

/* NOTE(MIGUEL):  
*                (12/07/2022)
  *                It is important to take into account the orbit radius as the corner of the texture is drawn there.
  *                Some thing i want to think about is how to normalize a region of the plane and sample the texture
  *                so that the size and pos is arbitrary.
  *                - Currently im using 20.0 which is the escape distance of the ray marcher however i think making the
  *                  distance smalleer would be a lot better.
*                I want a function to take a hit position, a write position and a size and it will do the mapping
*                to the correct uv to sample the texture
*                Succesfull wrote the texture placing function.
*                Need to write a domain space repetition function that divides space such that there is a sphere over each
*                pixel.
*                
*                !!!! Befoere writing more stuff for the raymarch version i should complete the full port from unity!!!!!
*                
*                THIS IS WRONG!!!!
*                For CCa there need to be a percentage of cells in the neighborhood that are above a certain state 
*                for the state of the current cell to be promoted.
*                - UThreashold could be a [0.0,1.0] value that is the percentage of cells in the neighborhood needed for promotion
*                - UStateLine could be the boundary state in which cells over it are counted. 
*                Its supposted to be only consider the cell that are over the state of the current cell.
*                
*                If enough neighbor are over the current cell than promote the current.
*                It's kind of modeling influence of the environment.
*                
*                
*                
*                
*/

#endif //CCA_H
