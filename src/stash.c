{
  typedef struct axispos_id axispos_id;
  struct axispos_id
  {u32 Id; f32 Pos; };
  axispos_id *AxisPosToId[Axis2_Count];
  b32        *IsAgentInserted;
  AxisPosToId[Axis2_X] = ArenaPushArray(Arena, AgentCount, axispos_id);
  AxisPosToId[Axis2_Y] = ArenaPushArray(Arena, AgentCount, axispos_id);
  IsAgentInserted = ArenaPushArray(Arena, AgentCount, u32);
  foreach(AgentId, AgentCount, u32)
  {
    axispos_id *EntryX = AxisPosToId[Axis2_X] + AgentId;
    axispos_id *EntryY = AxisPosToId[Axis2_Y] + AgentId;
    *EntryX = (axispos_id){AgentId, Agents[AgentId].Pos.x};
    *EntryY = (axispos_id){AgentId, Agents[AgentId].Pos.y};
    IsAgentInserted[AgentId] = 0;
  }
  MergeSort(AxisPosToId[Axis2_X], AgentCount, axispos_id, BoidsAgentPosCompare, NULL, 0);
  
  ////
  
  // NOTE(MIGUEL): One of the big isssues it that once i recurse to a level where the SubAgentCount is 2
  //               the median get computed the later of the 2 elm gets considere while the firs is ignored.
  //               both need to be considrerd.
  //               Another problem that may aris is the redundent use of merge sort each level undos the sort
  //               by axis of the previouse level and must sort it sub array. big sad.
  // TODO(MIGUEL): Make a less horibble kd tree!
  
  //               Current Kd-tree builder is taking ~5 second to build a tree of 20,000 most of the time is taken 
  //               by redundent Merge sorts done on each level. 
  //               One solution would be to have for each axis an (axis pos, index) sorted list that is done at depth
  //               == 0 and kept aroung for binary querying of a
  // NOTE(MIGUEL): Current Solution is O(n^2) where n threads are in quasi parallel and each thread does an O(n)
  //               search. 
  typedef struct agent_pos_id agent_pos_id;
  struct agent_pos_id
  {
    v2f Pos;
    u32 AgentId;
  };
  b32 BoidsAgentPosCompareX(void *a, void *b)
  {
    b32 Result = 1;
    SortParamAsRef(agent_pos_id, AgentA, a);
    SortParamAsRef(agent_pos_id, AgentB, b);
    if(AgentA->Pos.x<AgentB->Pos.x) Result = 0;
    return Result;
  }
  b32 BoidsAgentPosCompareY(void *a, void *b)
  {
    b32 Result = 1;
    SortParamAsRef(agent_pos_id, AgentA, a);
    SortParamAsRef(agent_pos_id, AgentB, b);
    if(AgentA->Pos.y<AgentB->Pos.y) Result = 0;
    return Result;
  }
  fn u32 BoidsAgentsKdTreeBuildItrnl(boids_agent *Agents, agent_pos_id *AgentPosToId, u32 StartIdx, u32 AgentCount, u32 Depth)
  {
    u32 NullIdx = U32MAX;
    Assert(AgentCount!=U32MAX);
    if(AgentCount<=0) return NullIdx;
    // Select axis based on depth so that axis cycles through all valid values
    axis2 Axis = Depth%Axis2_Count;
    
    u32 MedianAgentIdx = StartIdx + (u32)floor(AgentCount/2); // NOTE(MIGUEL): Assumes that agents are sorted for balanced tree
    
    // Create node and construct subtree
    sort_comparator *Compare = (Axis==Axis2_X?BoidsAgentPosCompareX:
                                Axis==Axis2_Y?BoidsAgentPosCompareY:NULL);
    Assert(Compare);
    MergeSort(AgentPosToId + StartIdx, AgentCount, agent_pos_id, Compare, NULL, 0);
    u32 AgentId = AgentPosToId[MedianAgentIdx].AgentId;
    boids_agent *Agent = &Agents[AgentId];
    u32 SubTreeIdx = AgentId;
    f32 Median = Agent->Pos.e[Axis];
    u32 NewCountA = (MedianAgentIdx-StartIdx);
    u32 NewCountB = (AgentCount-1)-(MedianAgentIdx-StartIdx);
    u32 NewStartA = StartIdx;
    u32 NewStartB = MedianAgentIdx+1;
    u32 ChildA = BoidsAgentsKdTreeBuildItrnl(Agents, AgentPosToId, NewStartA, NewCountA, Depth+1);
    u32 ChildB = BoidsAgentsKdTreeBuildItrnl(Agents, AgentPosToId, NewStartB, NewCountB, Depth+1);
    Agent->MedianValue = Median;
    Agent->Left  = ChildA;
    Agent->Right = ChildB;
#if 1
    arena LArena; ArenaLocalInit(LArena, 1024);
    ConsoleLog(LArena,
               "agent-----------------\n"
               "depth: %d | subcount: %d | start idx: %d\n"
               "axis : '%c' | median val: %f\n"
               "pos  : (x:%f, y:%f)\n"
               "this ID: %d\n"
               "CHILDREN: left id: %d | right id: %d\n\n",
               Depth, AgentCount, StartIdx,
               Axis==Axis2_X?'x':Axis==Axis2_Y?'y':'n', Median,
               Agent->Pos.x, Agent->Pos.y,
               MedianAgentIdx,
               Agent->Left, Agent->Right);
#endif
    return SubTreeIdx;
  }
  fn u32 BoidsAgentsKdTreeBuild(boids_agent *Agents, u32 StartIdx, u32 AgentCount, u32 Depth, arena *Arena)
  {
    arena_temp Scratch = ArenaTempBegin(Arena);
    agent_pos_id *AgentPosToId = ArenaPushArray(Scratch.Arena, AgentCount, agent_pos_id);
    foreach(AgentId, AgentCount, u32)
    {
      AgentPosToId[AgentId] = (agent_pos_id){Agents[AgentId].Pos, AgentId};
    }
    u32 Root = BoidsAgentsKdTreeBuildItrnl(Agents, AgentPosToId, StartIdx, AgentCount, Depth);
    ArenaTempEnd(Scratch);
    return Root;
  }
  {
    //BOIDS SORT
    
    // NOTE(MIGUEL): Merge Sort is running a ~0.1s-0.09s for 20,000 boids
    arena Arena; ArenaLocalInit(Arena, 1024);
    //OSProfileLinesStart("Boids Kd Tree");
    arena_temp Scratch = ArenaTempBegin(&Boids->Arena);
    boids_agent *Agents = D3D11ReadBuffer(Context, Boids->Agents, Boids->AgentsStage,
                                          sizeof(boids_agent), Boids->AgentCount, Scratch.Arena);
    ConsoleLog("\n\n\n*start kd tree log*");
    u32 KdRoot = BoidsAgentsKdTreeBuild(Agents, 0, Boids->AgentCount, 0, Scratch.Arena);
    ConsoleLog(*Scratch.Arena, "Root: %d\n", KdRoot);
    ConsoleLog("*start kd tree end log*\n\n\n");
    //D3D11GPUMemoryWrite(Context, Boids->Agents, Agents,sizeof(boids_agent), Boids->AgentCount);
    ArenaTempEnd(Scratch);
  }
  ///shader
  uint ParentAgentIds[2000];
  float2 Min = float2(Agent.Pos.x-Range, Agent.Pos.y-Range);
  float2 Max = float2(Agent.Pos.x+Range, Agent.Pos.y+Range);
#if 1
  agent Root = Agents[URootAgentIdx];
  ParentAgentIds[0] = URootAgentIdx;
  uint ParentAgentIdsTop = 0;
  uint Depth = 0;
  // find closest subtree 
  while(1)
  {
    float MedianTestMin = Depth%2==0?Min.x:Max.x;
    float MedianTestMax = Depth%2==0?Min.y:Max.y;
    if(Root.MedianValue<=MedianTestMin)
    {
      Root = Agents[Root.Right];
      if(ParentAgentIdsTop >= 2000) break;
      ParentAgentIds[Depth + 1] = Root.Right;
    }
    else if(Root.MedianValue>=MedianTestMax)
    {
      Root = Agents[Root.Left];
      if(ParentAgentIdsTop >= 2000) break;
      ParentAgentIds[Depth + 1] = Root.Left;
    }
    else break;
    if(Depth > 10) break;
    Depth++;
  }
  
  //dfs on subtree
#else
  float  MedianValue;
  uint Left;
  uint Right;
  Agent.MedianValue = 0.0;
  Agent.Left = 0;
  Agent.Right = 0;
  f32 MedianValue;
  u32 Left;
  u32 Right;
  
  ID3DBlob *CompileD3D11Shader(char *ShaderFileDir, const char *ShaderEntry, const char *ShaderTypeAndVer)
  {
    ID3DBlob *ShaderBlob, *Error;
    HRESULT Hr;
    FILE *ShaderFile;
    fopen_s(&ShaderFile, ShaderFileDir, "rb");
    u8 Buffer[4096*2];
    arena Arena = ArenaInit(&Arena, 4096*2, &Buffer);
    str8 ShaderSrc = OSFileRead(Str8(ShaderFileDir), &Arena);
    UINT flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    Hr = D3DCompile(ShaderSrc.Data, ShaderSrc.Size, NULL, NULL, NULL,
                    ShaderEntry, ShaderTypeAndVer, flags, 0, &ShaderBlob, &Error);
    if (FAILED(Hr))
    {
      const char* message = ID3D10Blob_GetBufferPointer(Error);
      OutputDebugStringA(message);
      Assert(!"[TestCode]: Failed to load shader of type [meh] !!!");
    }
    return ShaderBlob;
  }
  
  ID3D11DeviceContext     *Context    = Base->Context;
  D3D11_VIEWPORT           Viewport   = Base->Viewport;
  ID3D11RasterizerState   *RastState  = Base->RasterizerState;
  ID3D11DepthStencilState *DepthState = Base->DepthState;
  ID3D11BlendState        *BlendState = Base->BlendState; 
  ID3D11RenderTargetView  *RTView     = Base->RTView;
  ID3D11DepthStencilView  *DSView     = Base->DSView;
  
}

{
#if 1
  fn void D3D11TexCubeViewSR(ID3D11Device* Device, ID3D11UnorderedAccessView **UAV, ID3D11Texture2D **GetTex, v2s TexDim, void *Data, u32 Stride, tex_format Format)
  {
    D3D11_TEXTURE2D_DESC TexDesc = {0};
    {
      TexDesc.Width          = TexDim.x;
      TexDesc.Height         = TexDim.y;
      TexDesc.MipLevels      = 1;
      TexDesc.ArraySize      = 6;
      TexDesc.Format         = Format;
      TexDesc.Usage          = D3D11_USAGE_DYNAMIC;
      TexDesc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS;
      TexDesc.CPUAccessFlags = 0;
      TexDesc.SampleDesc     = (DXGI_SAMPLE_DESC){1, 0};
    }
    D3D11_SUBRESOURCE_DATA Initial = {0};
    {
      Initial.pSysMem = Data;
      Initial.SysMemPitch = Stride*TexDim.x;
    }
    ID3D11Texture2D *Texture;
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {0};
    SRVDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Format              = Format;
    SRVDesc.Texture2D           =  (D3D11_TEXCUBE_SRV){0, 1};
    ID3D11Device_CreateShaderResourceView(Device, (ID3D11Resource*)Texture, &SRVDesc, SRV);
    if(GetTex != NULL) *GetTex = Texture;
    else ID3D11Texture2D_Release(Texture);
  }
#endif
}