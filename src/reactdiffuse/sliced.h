#ifndef SLICED_H
#define SLICED_H

#define AgnetCount 1000000000
agent Agents[AgentCount];
u32 SliceCounts[];
u32 SliceFirstVoxelIndices[]; //just data to index not index it self {SliceOffset, BB.minx, BB.minz, VoxelDim.x}
tex2<int> VoxelAgentIndices[] //store the indices of agents in voxels
tex2 IndexPoolTex;
u32 VoxelDim = 32; //d
u32 SliceOffset; //pi
v2u SliceVoxelDim; //(nxi, nxj)
u32 SliceVoxelArea; //ni voxelspace

v2s GetSliceVoxelDim(i2u32 SliceBB)
{
  v2s SliceVoxelDim = V2s((SliceBB.maxx-SliceBB.minx/VoxelDim.x)+1,  //x
                          (SliceBB.maxy-SliceBB.miny/VoxelDim.y)+1); //z
  return SliceVoxelDim;
}
u32 GetSliceVoxelArea(i2u32 SliceBB)
{
  v2s SliceVoxelDim  = GetSliceVoxelDim(SliceBB);
  u32 SliceVoxelArea = SliceVoxelDim.x*SliceVoxelDim.y;
  return SliceVoxelArea;
}
u32 GetSliceVoxelOffset(sorted_slice_list Slices, u32 SliceId)
{
  u32 VoxelOffset = 0;
  for(u32 i=0; i<SliceId; i++)
  {
    //Assume Slices is Sorted
    i2u32 SliceBB = Slices[i];
    u32 SliceVoxelArea = GetSliceArea(SliceBB);
    VoxelOffset += SliceVoxelArea;
  }
  return VoxelOffset;
}
fn v4u MakeIndexPoolTexelForSlice()
{
  u32 SliceId = SliceIndexFromPos(v3f Pos, i1u32 YBounds);
  u32 VoxelIndexFromPos(v3f Pos, slicebb_hash SliceBBHash);
  i2u32 SliceBB = GetSliceBBFromHash(SliceBBHash, AgentSliceId);
  u32 SliceVoxelOffset = GetSliceVoxelOffset();
  v4u V4u(SliceVoxelOffset, SliceBB.minx, SliceBB.maxx, )
    return;
}
fn u32 SliceIndexFromPos(v3f Pos, i1u32 YBounds)
{
  u32 SliceId = (Pos.y-YBounds.min)/VoxelDim;
  return SliceId;
}
fn u32 VoxelIndexFromPos(v3f Pos, slicebb_hash SliceBBHash)
{
  i2u32 SliceBB = GetSliceBBFromHash(SliceBBHash, AgentSliceId);
  u32 VoxelId = SliceVoxelOffset + ((Pos.x-Bounds.SliceBB.minx)/VoxelDim +
                                    (Pos.y-Bounds.SliceBB.miny)/VoxelDim*SliceVoxelArea);
  return VoxelId;
}

// Computational Region: n slices voxels over their bounding box
// Global Indices to calc index index of voxel in bounding box
// Global Indeces for each slice
fn Construct(TexRes)
{
  //O(n)
  i1u32 YBounds;
  for(AgentId, AgentCount)
  {
    Agent = Agents[AgentId];
    AgentSliceId = Agent.Pos.y/VexelDim;
    i1u32Union(YBounds, Agent.Pos.y);
    i2u32 SliceBB = GetSliceBBFromHash(SliceBBHash, AgentSliceId);
    i2u32Union(SliceBB, Agent.Pos.xz);
    
  }
  SliceCount = SliceBBHash.Count;
  SliceVoxelCountArray;
  
  //scatter 
  
  //gather
  return;
}

#endif //SLICED_H
