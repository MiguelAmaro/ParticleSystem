# Questions

**Why are there so many constant buffers with similar data?**

**Is there any reason for there beeing duplicate strucured buffers in the different shaders*

**Vertex data is not introducet via input assembler but some mechanism in geometry shader? Why us IndrectDraw to and point primitive type?**

Particles are represente as float4s. That's it. Basically a sequence of points... We are calcing and updating pos of gpu side with minimal cpu guidance so vertex data needs to be generated at updated positions using particle float4s. Thats whey input topology point. The inderect draw call...

**What exactly is gpu threading and how is work divided among threads?**

**There are 2 buffers that where one hold last frames particle state and the other serves as storage for processed positions? Is is neccessary?**

**What do i get/can do with an unorderd access view? How does help me in processing the particle data?**

Well more specifically the algo makes use of append and consume structured buffers which is just a more convenient way off picking up data processing them and storing. HLSL has build in functions that let you do that. Append and Consume

**Do I need to set the input asssembler's topology to point primitive if it's getting bypassed?** 

**Which buffer do i tell the device context to use if strucuted buffers?**


**What is the input layout needed for. Input assembler? Which stage relies on it?**


**How does the input assembler know how many vertices there are?**

In this case it doesnt

**Can append consume buffer write past buffer size?**

I set the buffer size/max count on creation. Then with the uav i can set the count of data stored in the buffer.

# Lessons
* A resource view can't be bound to two stages at the same time. In needs to be unbounded before any subsequent setresourceblah calls are made with the same resourceview. 

* Make sure other resources (shaders, data, layouts) from a previouse draw call aren't bound to the stages. If those are not cleared it can affect other draw calls that expect different resources bound or resouces not to be bound in certain stages. Especially important for layouts.

* Input Assembler expects at leas 32bytes of per vertex data. You can't just do pos float3/float4 and be done. It will run but data wont load correctly because the stride needs to be 32bytes or more.

* Pixel shader requires the POS, TEXCOORD and COLOR semantics to be supplies by the previous shader(vertex, geometry ect)

* Don't expect to see any geometetry show up on the screen if you dont set the w component of the float4 to 1.0.

* Use staging buffer to debug the inderectdraw arg buffer

# Topic Search 
indirect args > pipeline exec methods > 

indirdraw invokes pipeline > gpu gens paricles>particle updates> copy sturct count method updates indirect args buffer >>

pg.572 Use of DrawInstancedIndirect >> pg.69 Expanation of DrawInstancedIndirect and AgrBuffer using Append/Consume Struct Bufffers>>pg.113 using copystructcount method >>

Gpu based Arg buffer modification(not used for the parti
simulation)
* stream output functionality
* writteng to as render target
* modified as unordered access view

cpu arg buffer modification
CopyStructCount will be used to copy updeated append/consume buffer's hidden count to the arg buffer for the rendering.

# Notes

**Compute Shader Overview**

The compute shader doesnt use semantic data passed from other pipeline stages but instead does fetches and outputs data via resources accesable via resource views.

Takes as input a flaot3 denoting how thread are grouped together using 3 dimensional scheme.

There are tradoff with data layout that will yield efficient access by the cs as opposed to the rendering pipeling.

Data sharing accross thread is possible but introduces overhead.

There are memory fencing funcions(atomic) interlockedadd that synch thread such that they work as if they are a single thread. Not sure exactly what this implies.

input attribute registers (v#)
texture registers (t#)
constant buffer registers (cb#)
unordered registers (u#)
and temporary registers (r#, x#)

For a gpu dynamic particle system i am using a compute shader to update particle positions using append/consume structured buffers, A and B and UOAccess views. I need feed particle position data back into the pipline. I'm trying use a shader resource view for buffer B so that the position data is visible to the vertex shader but im not able to bind it.

**Geometry Shader Overview**

TriangleStream is a d3d11 buffer type that allows you to pass mesh data to the following stages.

# Status

...

# Stash
```c
    // alternative to hlsl compilation at runtime is to precompile shaders offline
    // it improves startup time - no need to parse hlsl files at runtime!
    // and it allows to remove runtime dependency on d3dcompiler dll file
    
    // a) save shader source code into "shader.hlsl" file
    // b) run hlsl compiler to compile shader, these run compilation with optimizations and without debug info:
    //      fxc.exe /nologo /T vs_5_0 /E vs /O3 /WX /Zpc /Ges /Fh d3d11_vshader.h /Vn d3d11_vshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv shader.hlsl
    //      fxc.exe /nologo /T ps_5_0 /E ps /O3 /WX /Zpc /Ges /Fh d3d11_pshader.h /Vn d3d11_pshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv shader.hlsl
    //    they will save output to d3d11_vshader.h and d3d11_pshader.h files
    // c) change #if 0 above to #if 1
    
    // you can also use "/Fo d3d11_*shader.bin" argument to save compiled shader as binary file to store with your assets
    // then provide binary data for Create*Shader functions below without need to include shader bytes in C
```
