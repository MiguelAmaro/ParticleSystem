#ifndef DX11_HELPERS_H
#define DX11_HELPERS_H

typedef struct d3d11_base d3d11_base;
struct d3d11_base
{
  v2s WindowDim;
  ID3D11Device            *Device;
  ID3D11DeviceContext     *Context;
  ID3D11BlendState        *BlendState;
  IDXGISwapChain1         *SwapChain;
  ID3D11RasterizerState   *RasterizerState;
  ID3D11DepthStencilState *DepthState;
  ID3D11RenderTargetView  *RTView;
  ID3D11DepthStencilView  *DSView;
  D3D11_VIEWPORT           Viewport;
  
};
typedef enum cpu_access cpu_access;
enum cpu_access 
{
  Access_None  = 0,
  Access_Read  = D3D11_CPU_ACCESS_READ,
  Access_Write = D3D11_CPU_ACCESS_WRITE,
};
typedef enum buffer_usage buffer_usage;
enum buffer_usage 
{
  Usage_Default = D3D11_USAGE_DEFAULT,
  Usage_Dynamic = D3D11_USAGE_DYNAMIC,
};
typedef enum gpu_mem_op gpu_mem_op;
enum gpu_mem_op
{
  GPU_MEM_READ  = D3D11_MAP_READ,
  GPU_MEM_WRITE = D3D11_MAP_WRITE_DISCARD,
};
typedef enum tex_format tex_format;
enum tex_format
{
  Unorm_RGBA = DXGI_FORMAT_R8G8B8A8_UNORM,
  Unorm_R    = DXGI_FORMAT_R8_UNORM,
  Float_R    = DXGI_FORMAT_R32_FLOAT,
  Float_RG   = DXGI_FORMAT_R32G32_FLOAT,
  Float_RGBA = DXGI_FORMAT_R32G32B32A32_FLOAT,
};
typedef enum shaderkind shaderkind;
enum shaderkind
{
  ShaderKind_Vertex,
  ShaderKind_Pixel,
  ShaderKind_Compute,
  ShaderKind_Geometry,
};
typedef struct d3d11_shader d3d11_shader;
struct d3d11_shader
{
  shaderkind Kind;
  str8 Path;
  str8 EntryName;
  datetime LastRecordedWrite;
  union
  {
    ID3D11VertexShader   *VertexHandle;
    ID3D11PixelShader    *PixelHandle;
    ID3D11ComputeShader  *ComputeHandle;
    ID3D11GeometryShader *GeometryHandle;
  };
  ID3D11InputLayout *Layout;
  //async reloading stuff
  u32 Id;           // id given when submiting req to get back data when processing finishes
  datetime ReqTime; //used to see if the file was modified between when the req being made and it being fulfilled
};
/*// NOTE(MIGUEL): I want to keep D3D11ShaderHotReload() but i want an async variant that is essentially spliting
              *                  (check & compile & shader swap) to (async:check & compile then on each frame check if should swap & swap)
              *                  check if should hot load(file modification detected)
              *                  if yes submit async req else ignore
              *                  async req: ->shader obj with the new compiled shader handle & was (successfull, failed, proccessing)
                *                  failed: get error msg for dx11
                *                  succces: shader swap
                *                  processing: ignore/wait till anther frame
                *                  
                *                  
                *                  What is slow part of this? file loading or shader compilation. An nice api feature would be the ability
                *                  to push and active file that is know to contain multiple shader programs. There is no reason to do re-reads
                *                  for each shade load.
                *                  Memoymapped io? is it worth it??
                *                  takes: ~18.103418s for the pixelshader
                *                  takes: ~0.03s for the pixelshader
                *                  takes: ~0.008s for the vertex shader
  */
#define ASYNC_SHADER_BUFFER_SIZE 32
typedef enum async_shader_status async_shader_status;
enum async_shader_status
{
  //enums [0: BUFFERSIZE-1] reserved for indexing into the shader array
  //enum  [BUFFERSIZE] reseved for invalid key
  ASYNC_SHADER_STATUS_Processing = (ASYNC_SHADER_BUFFER_SIZE + 1),
  ASYNC_SHADER_STATUS_Finished   = (ASYNC_SHADER_BUFFER_SIZE + 2),
  ASYNC_SHADER_STATUS_Unable     = (ASYNC_SHADER_BUFFER_SIZE + 3),
  ASYNC_SHADER_STATUS_NULL       = (ASYNC_SHADER_BUFFER_SIZE + 4),
};
#define ASYNC_SHADER_STATUS_RequestSubmitted(code) (code<ASYNC_SHADER_BUFFER_SIZE)
#define ASYNC_SHADER_LOADER_invalidKey ASYNC_SHADER_BUFFER_SIZE
typedef struct async_shader_load_entry async_shader_load_entry;
struct async_shader_load_entry
{
  d3d11_shader Shader;
  volatile async_shader_status Status;
};
typedef struct async_shader_load async_shader_load;
struct async_shader_load
{
  // NOTE(MIGUEL): shader to load aand compiles
  async_shader_load_entry Entries[ASYNC_SHADER_BUFFER_SIZE];
  u32 Count;
  d3d11_base *Base;
  b32 IsInitialized;
};
#define D3D11BaseDestructure(BasePtr) \
ID3D11Device            *Device     = BasePtr->Device;          \
ID3D11DeviceContext     *Context    = BasePtr->Context;         \
ID3D11BlendState        *BlendState = BasePtr->BlendState;      \
IDXGISwapChain1         *SwapChain  = BasePtr->SwapChain;       \
ID3D11RasterizerState   *RastState  = BasePtr->RasterizerState; \
ID3D11DepthStencilState *DepthState = BasePtr->DepthState;      \
ID3D11RenderTargetView  *RTView     = BasePtr->RTView;          \
ID3D11DepthStencilView  *DSView     = BasePtr->DSView;          \
D3D11_VIEWPORT           Viewport   = BasePtr->Viewport;        

// UTILS
fn  d3d11_base D3D11InitBase(HWND Window);
fn void D3D11UpdateWindowSize(d3d11_base *Base, v2s WindowDim);
fn void D3D11GPUMemoryOp(ID3D11DeviceContext * Context, ID3D11Buffer *GPUBuffer, void *CPUBuffer, u32 Stride, u32 Count, gpu_mem_op Op);
// BUFFERS
fn void D3D11StructuredBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count);
fn void D3D11VertexBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Stride, u32 Count);
fn void D3D11ConstantBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Size, buffer_usage Usage, cpu_access Access);
fn void D3D11StageBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data, u32 Size);
fn void D3D11BufferViewSR(ID3D11Device* Device, ID3D11ShaderResourceView  **SRV, ID3D11Buffer *Buffer, u32 Count);
fn void D3D11ArgsBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Args, u32 Size);
fn void D3D11BufferViewUA(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Buffer *Buffer, u32 Count );
fn void D3D11BufferViewUAAppend(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Buffer *Buffer, u32 Count );
fn void *D3D11ReadBuffer(ID3D11DeviceContext *Context, ID3D11Buffer *TargetBuffer, ID3D11Buffer *StageBuffer, u32 Stride, u32 Count, arena *Arena);
// TEXTURES
fn void D3D11Tex2DStage(ID3D11Device* Device, ID3D11Texture2D **Texture, v2s TexDim, void *Data, u32 Stride, tex_format Format);
fn void D3D11Tex2DViewSR(ID3D11Device* Device, ID3D11ShaderResourceView **SRV, ID3D11Texture2D **GetTex, v2s TexDim, void *Data, u32 Stride, tex_format Format);
fn void D3D11Tex2DViewUA(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Texture2D **GetTex, v2s TexDim, void *Data, u32 Stride, tex_format Format);
fn void D3D11Tex2DViewSRAndUA(ID3D11Device* Device, ID3D11Texture2D **GetTex, ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV, 
                              v2s TexDim, void *Data, u32 Stride, tex_format Format);
fn void D3D11Tex2DSwap(ID3D11DeviceContext *Context, ID3D11Texture2D **TexA, ID3D11Texture2D **TexB, ID3D11Texture2D *Stage);
// SHADER COMPILATION
fn ID3DBlob *D3D11ShaderLoadAndCompile(str8 ShaderFileDir, str8 ShaderEntry, const char *ShaderTypeAndVer, const char *CallerName);
fn static DWORD WINAPI AsyncShaderLoader(void *Param);
fn void D3D11AsyncShaderLoaderInit(async_shader_load *Loader, d3d11_base *Base);
fn async_shader_status D3D11ShaderAsyncLoadAndCompile(d3d11_base *Base, d3d11_shader *Shader, datetime ReqTime);
fn void D3D11ShaderAsyncHotReload(d3d11_base *Base, d3d11_shader *Shader);
fn void D3D11ShaderHotReload(d3d11_base *Base, d3d11_shader *Shader);
fn d3d11_shader D3D11ShaderCreate(shaderkind Kind, str8 Path, str8 EntryName, D3D11_INPUT_ELEMENT_DESC *VElemDesc,
                                  u32 VElemDescCount, d3d11_base *Base);
// PIPELINE MGMT
fn void D3D11ClearComputeStage(ID3D11DeviceContext *Context);
fn void D3D11ClearPipeline(ID3D11DeviceContext *Context);

#endif //DX11_HELPERS_H
