// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \defgroup RForce
@{
\ingroup Raytracers
*/

/*! \file RaytracerRForce.h
\brief Declaration of RaytracerRForce class.
*/

#ifndef _RAYTRACERRFORCE_H_
#define _RAYTRACERRFORCE_H_

#include "Raytracer.h"

// include kernels
#include "KernelHitTest.h"
#include "KernelBVTest.h"
#include "KernelReduceHits.h"
#include "KernelShader.h"
#include "KernelTransformer.h"
#include "KernelRayGenerator.h"
#include "KernelBVBuilder.h"
#include "KernelDebug.h"

//! RForce static textures (encoded scene data).
enum E_RTRF_TEXTURE {
  RTRF_TEXTURE_TRIS_APOSITION = 0, //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_BPOSITION,     //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_CPOSITION,     //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_ANORMAL,       //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_BNORMAL,       //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_CNORMAL,       //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_COLOR,         //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_TRIS_UVRR,          //!< scene data: 2D GL_FLOAT_RGBA_NV [RTRF_TRIMAP_SX x RTRF_TRIMAP_SY]
  RTRF_TEXTURE_BOX_MAX,            //!< (debug) scene data: 2D GL_FLOAT_RGBA_NV [RTRF_BVMAP_SX x RTRF_BVMAP_SY]
  RTRF_TEXTURE_BOX_MIN,            //!< (debug) scene data: 2D GL_FLOAT_RGBA_NV [RTRF_BVMAP_SX x RTRF_BVMAP_SY]
  RTRF_TEXTURE_MATERIAL,           //!< scene texture: 2D GL_RGB8 [1024x1024]
  RTRF_TEXTURE_COUNT
};

//! RForce working PBuffers.
enum E_RTRF_PBUFFER {
  RTRF_PBUFFER_NONE = -1,          //!< special - use for switch to main context
  RTRF_PBUFFER_BOUNDTESTING = 0,   //!< here bounding volumes testing take place: r=16f texRECT [RTRF_BOUNDTESTING_SPACE_SX x RTRF_BOUNDTESTING_SPACE_SY]
  RTRF_PBUFFER_INTERSECTING,       //!< here main work takes place: rgba=32f doublebuffer depth texRECT depthTexRECT [RTRF_INTERSECTING_SPACE_SX x RTRF_INTERSECTING_SPACE_SY] 
  RTRF_PBUFFER_SHADING,            //!< here shading takes place: rgba=8 texRECT [RTRF_VIEWPORT_SX x RTRF_VIEWPORT_SY]
  RTRF_PBUFFER_COUNT
};

//! RForce working Kernels.
enum E_RTRF_KERNEL {
  RTRF_KERNEL_HITTEST = 0,
  RTRF_KERNEL_BVTEST,
  RTRF_KERNEL_BVBUILDER,
  RTRF_KERNEL_REDUCEHITS,
  RTRF_KERNEL_RAYGENERATOR,
  RTRF_KERNEL_SHADER,
  RTRF_KERNEL_TRANSFORMER,
  RTRF_KERNEL_DEBUG,
  RTRF_KERNEL_COUNT
};

//! Matrix shape modes.
enum E_MT_NORMAL {
  MT_NORMAL = 0, 
  MT_INVERSE, 
  MT_TRANSPOSE, 
  MT_INVERSETRANSPOSE
};

//! Implementation of RForce raytracer.
/*!
<b>This document aims to briefly describe method RForce used in NV38Box.</b><br>
<br>
<b>Background</b><br>
Maybe you have read Purcell's [1+3] or Carr's [2] papers on raytracing in GPU.<br>
This method is based on some of their ideas but is&nbsp;sligtly different in 
some cases.<br>
This method was developed under Inferno project (<a href="http://inferno.hildebrand.cz">http://inferno.hildebrand.cz</a>).<br>
I assume you know how ray tracing works and maybe you should understand
<br>
main concepts in papers [1] and [2] for better understanding of this method.<br>
<br>
<b>Features of RForce method:</b><br>
- dynamic GPU raytracer running in realtime for simple scenes and low 
resolutions<br>
- scene = 16 triangular objects per 64 triangles (total up to 1024 triangles)<br>
- using AABB as an acceleration structure<br>
- per pixel shading for both primary and secondary rays<br>
- no dynamic control flow as opposite to Purcell's method<br>
- hybrid algorithm (primary rays = classic rasterization)<br>
- nVidia only, NV38 path, NV40 path, brute force for comparison<br>
- implementation performs only secondary reflection rays,
<br>
but method is more flexible in general<br>
- tested on WinXP, GeForce 5950 Ultra, ForceWare 60.xx<br>
- slow as hell (but working)<br>
<br>
<b>What do we want ?</b><br>
We want triangular scenes (no procedural "one-shader-spheres+plane" tricks!).<br>
We want to trace array of rays in the GPU.<br>
We want to shade their hits in the GPU.<br>
We want to display final picture in the&nbsp;GPU.<br>
We want to do it effective even for dynamic scenes !<br>
<br>
<b>Global algorithm</b><br>
<br>
1. build static data structures (textures and VBOs)<br>
2. for every frame<br>
frame preprocess:<br>
2.1. transform scene geometry and store results in set of textures<br>
2.2. compute bounding boxes in GPU using transfored geometry in textures<br>
frame steps:<br>
2.3. render primary rays using classic scene rasterization (with custom 
shading)<br>
---<br>
2.4. generate rays (secondary reflection, refractions, shadow rays, whatever 
you want)<br>
2.5. trace rays (hard work)<br>
2.6. shade rays (per-pixel)<br>
2.7. blend results to main image (according to rays contributions)<br>
---<br>
2.8. display final image<br>
<br>
Steps 2.4. - 2.7. can be looped more times. Contribution is blended to the 
framebuffer after every step.<br>
Current implementation does have only one loop generating secondary reflection rays only.<br>
<br>
See it in pictures:<br>
<a href="example-datastructures.png"><img src="texample-datastructures.png"></a>&nbsp;
<a href="example-primary.png"><img src="texample-primary.png"></a>&nbsp;
<a href="example-raystart.png"><img src="texample-raystart.png"></a>&nbsp;
<a href="example-rayend.png"><img src="texample-rayend.png"></a>&nbsp;
<a href="example-mask.png"><img src="texample-mask.png"></a>&nbsp;
<a href="example-workspace.png"><img src="texample-workspace.png"></a>&nbsp;
<a href="example-hitinfos.png"><img src="texample-hitinfos.png"></a>&nbsp;
<a href="example-contribution.png"><img src="texample-contribution.png"></a>&nbsp;
<a href="example-final.png"><img src="texample-final.png"></a>&nbsp;
<br>
Legend (from left):<br>
1. data structures generated in 2.1 and 2.2<br>
2. primary rays from 2.3<br>
3. rays' starts from 2.4<br>
4. rays' ends from 2.4<br>
5. computation mask for accelerating raytracing<br>
6. tracing space (huge buffer - this is only a part of it) - yellow points are incomming hits<br>
7. reduced tracing space (all segments) - reduction = find best hit per ray<br>
8. shaded hits = contribution to the final image<br>
9. final image composited from 1 and 8<br>

<br>
<b>How is defined scene ?</b><br>
Scene consists from 16 objects. Each object can have up to 64 triangles.<br>
Although we have tweakable constants in sources, we will talk here about these 
numbers directly.<br>
Every object has its own transformation matrix (= OpenGL-like scene).<br>
<br>
We store all scene related data in texture with this block structure:<br>
This is picture of 4x4 blocks. Whole picture represents data for whole scene.<br>
Here is a block of 16 sub-blocks representing objects 1 to 16.
<br>
Every sub-block is 8x8 elements corresponding to triangles 1-64 of given 
object.
<br>
Every element is texel in 32-bit floating point texture with 4 components.<br>
<font size="1">
<br>
0102030405060708 0202020202020202 0303030303030303 0404040404040404<br>
0910111213141516 0202020202020202 0303030303030303 0404040404040404<br>
1718192021222324 0202020202020202 0303030303030303 0404040404040404<br>
2526272829303132 0202020202020202 0303030303030303 0404040404040404<br>
3334353637383940 0202020202020202 0303030303030303 0404040404040404<br>
4142434445464748 0202020202020202 0303030303030303 0404040404040404<br>
4950515253545556 0202020202020202 0303030303030303 0404040404040404<br>
5758596061626364 0202020202020202 0303030303030303 0404040404040404<br>
<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
0505050505050505 0606060606060606 0707070707070707 0808080808080808<br>
<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
0909090909090909 1010101010101010 1111111111111111 1212121212121212<br>
<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
1313131313131313 1414141414141414 1515151515151515 1616161616161616<br>
</font>
<br>
In every such a&nbsp;picture we are able to store per triangle information in 4 
components.<br>
We will call this structure "block structure" for later.<br>
<br>
We store these static scene textures:<br>
- face colors: [r,g,b,packed(mat1, mat2)]<br>
- material and uv: [packed(u1v1), packed(u2v2), packed(u3v3), packed(mat1, 
mat2)]<br>
<br>
mat1 = material coefficient<br>
mat2 = reflection coefficient<br>
mat3 = refraction coefficient (unused)<br>
mat4 = eta for refraction (unused)<br>
<br>
uv = vertex texture coordinates into image texture map.<br>
for simplicity we have baked all object textures into one big texture.<br>
<br>
packed means using pack_2half (see Cg manual).<br>
<br>
<br>
<b>Building static data structures</b><br>
a) We need to store scene geometry as static VBO, there are<br>
- position [x,y,z]<br>
- normal [x,y,z]<br>
- material [mat1, mat2, mat3, mat4]<br>
- texture coordinates [tu, tv]<br>
- triangle color [r,g,b,counter]<br>
- optimization info [?,?,?,?] some precomputed data for custom shaders<br>
<br>
We store these data in 32bit floating point textures.<br>
This is interleaved buffer. We use stride to glPointer* them correctly.<br>
<br>
b) VBO storing mark point positions (later on this)<br>
<br>
c) All static textures (e.g. face colors &amp; material and uv).<br>
<br>
<b>Transform scene geometry</b><br>
Neat trick:
<br>
Use vertex engine to do the transformation of positions and normals.<br>
But do not rasterize primitives = &gt; store transformed data into texture 
using block structure.<br>
<br>
For positions:<br>
Send scene into pipeline as GL_POINTS with size 1 pixel (use static VBO with 
scene data)<br>
Vertex program transforms vertex position into world space as normally would<br>
BUT writes it as texture coordinates for fragment program.<br>
AND writes position according to block structure into output fragment position 
INSTEAD real position.
<br>
Fragment program simply stores incomming texture.<br>
<br>
For normals:<br>
Same trick, but transforms normal into world space.<br>
<br>
Result: we have 3+3 textures using block structure.<br>
First three are transformed positions for vertices A,B and C respectively<br>
Second three are transformed normals for vertices A,B and C respectively<br>
<br>
<b>Compute bounding boxes</b><br>
We have 3 textures with transformed world positions from previous passes.<br>
Perform reduction steps on theses textures to get minimum and maximum 
coordinates for every sub-block (object).<br>
Sub-block is 8x8 so we need 3 reduction passes. And one more extra pass for 
merging results from all three maps.<br>
<br>
Fragment program simply compares coordinates and writes min or max to the 
output.
<br>
<br>
Result: We get two textures holding min and max values for every object (=we 
have AABB computed on the GPU).<br>
This texture is using "reduced block structure", because sub-blocks are reduced 
to one pixel.<br>
<br>
<b>Render primary rays</b><br>
Use classic OpenGL with custom shading (per pixel lighting).<br>
Use static scene VBO. Nothing special here. We are rendering in 512x512 buffer.<br>
<br>
<b>Generate rays</b><br>
We are raytracing in resolution 128x128 (or others).<br>
We need to generate
<br>
128x128 buffer with ray START positions and
<br>
128x128 buffer with ray END position.<br>
<br>
Render scene two times (again using static VBO).<br>
1st time use shader to generate rays STARTS<br>
2nd time use shader to generate rays ENDS (here takes place reflection formula)<br>
<br>
Results: we get buffers with rays definition for trace part.<br>
<br>
<b>Trace rays</b><br>
How to trace ?
<br>
For every pixel: Test hits in block structure + reduce them into one point<br>
Draw fullscreen quad executing ray-triangle intersection test for every pixel.<br>
=&gt; One pixel corresponds to one pair ray-triangle ! For one ray we have 
32x32 ray-triangle pairs.<br>
Then do 5 reduction steps to retrieve hit with minimal time.<br>
<br>
How many pixels (pairs) do we need ?<br>
For every ray from 128x128 buffer we need 32x32 triangles.<br>
We need working buffer 128x32 x 128x32. That is 4096x4096 !
<br>
Such a 32-bit floating point buffer does not fit into GPU memory.<br>
So we trace per partes (one part is called "segment").
<br>
More in part Segmentation.<br>
<br>
How to speed this up ?<br>
More in part Raytracing acceleration.<br>
<br>
<b>Shade rays</b><br>
Incomming hits (with their info) are used for shading.<br>
This shader is quite complicated (doing per-pixel shading of secondary 
reflected points)<br>
Scene data structures in textures are used.<br>
<br>
<b>Blend results</b><br>
Shaded image from previous represents contribution of actual groups of rays.<br>
Contribution of every pixel is stored in alpha channel.<br>
We use blending to add this contribution to the final imge using alpha.<br>
We did raytracing in lower resolution than is final image resolution,<br>
so we do texture stretching + filtering.<br>
<br>
<b>Display final image</b><br>
We did accumulation of all contributions in BACK buffer of main context.<br>
Do just glSwapBuffers() :-)<br>
<br>
<b>Segmentation</b><br>
Because there is not enough GPU memory to do ray tracing in one go,<br>
we need to trace per blocks. These blocks are called segments and correspond<br>
to tiles of final images. Tiling is dependent on how much memory is available<br>
and what resolution do we ray trace.<br>
<br>
Because we are producing tiles from left to right, from top to bottom.<br>
We can move over the buffer without destroying previous data.<br>
At the time there is only one tile in progress (occupies buffer).<br>
But previous are there in reduced form. See picture.<br>
<br>
<b>Raytracing acceleration</b><br>
We use method called "computation mask". It is not needed to call intersection
<br>
of all pairs ray-triangle. That is why we build bounding boxes.<br>
First pass goes and performs 4x4 tests of AABB bounding box intersection for 
every ray.
<br>
These tests are stored in buffer 512x512 and this buffer is used as computation 
mask.<br>
<br>
How to reject fragments in Trace rays part ?<br>
This method differs between NV38 and NV40. We use early-z functionality 
available
<br>
on these GPUs.<br>
<br>
We have implemented three methods:<br>
1. brute force - no mask is used at all<br>
2. NV38 - mask must be read into PBO, then rendered as GL_POINTS using PBO 
rebound as VBO<br>
3. NV40 - mask is produced using one-quad fragment program with discard 
operation<br>
<br>
See detailed code reference for more info.<br>
<br>
<b>References:</b><br>
[1] T. J. Purcell, I. Buck, W. R. Mark, and P. Hanrahan. Ray Tracing on 
Programmable Graphics Hardware. In to appear in Proc. SIGGRAPH, 2002.<br>
[2] Nathan A. Carr, Jesse D. Hall and John C. Hart, The Ray Engine, 
UIUCDCS-R-2002-2269 March 2002<br>
[3] Timothy J. Purcell, Ray Tracing on a Stream Processor, Ph.D. dissertation, 
Stanford University, March 2004.
<br>

*/
class RaytracerRForce : public Raytracer
{
public:
  //! RaytracerRForce constructor.
  RaytracerRForce ();
  //! RaytracerRForce destructor.
  ~RaytracerRForce ();

  //////////////////////////////////////////////////////////////////////////
  /*! \name General interface
  @{ */

  //! Prepares RForce.
  bool Prepare();
  //! Builds RForce.
  bool Build();
  //! Executes RForce frame (active schema).
  bool Execute();
  //! Shutdowns RForce.
  bool Shutdown();

  //! @} 
protected:
  //////////////////////////////////////////////////////////////////////////
  /*! \name Scheme paths
  @{ */

  //! A scheme path
  bool RenderA();
  //! B scheme path
  bool RenderB();
  //! C scheme path
  bool RenderC();
  //! D scheme path
  bool RenderD();
  //! E scheme path
  bool RenderE();

  //! F+G scheme paths
  bool RenderEarlyZRejection(bool bEnable);
  //! H+I scheme paths
  bool RenderEarlySRejection(bool bEnable);

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Initialization and preprocessing
  @{ */
  
  //! Encodes scene data into custom GPU friendly buffers.
  bool PrepareTriData(float** pAPositions, float** pBPositions, float** pCPositions, float** pANormals, float** pBNormals,float** pCNormals, float** pColors, float** pUVRR);
  //! Computes bounding volumes in CPU and encodes them into buffers (for debug purposes).
  bool PrepareBoundingVolumes(float* pAPositions, float* pBPositions, float* pCPositions, float** pBoxMin, float** pBoxMax);

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Scene rendering
  @{ */

  //! Sets scene projection settings into OpenGL state.
  void PrepareSceneProjection();
  //! Binds VBO buffers for scene rendering.
  void BindSceneBuffers();
  //! Renders scene geometry in given mode (GL_TRIANGLES, GL_POINTS, etc.).
  void RenderScene(GLenum mode);
  //! Releases VBO buffers after BindSceneBuffers().
  void UnbindSceneBuffers();

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Scheme helpers
  @{ */

  //! Renders primary rays using classic OpenGL rasterization with selected shading.
  void RenderPrimary();
  
  //! Performs trace pass - brute force modification.
  void TracePassBruteForce();
  //! Performs trace pass - NV38 modification.
  void TracePassNV38();
  //! Performs trace pass - NV40 modification.
  void TracePassNV40();

  //! Generates rays to be traced.
  void GenerateRays();

  //! Transforms geometry into world space and stores it as textures.
  void GenerateTransformedGeometry();
  //! Builds bounding volumes in GPU (using AABB).
  void BuildBoundingVolumes();

  //! Performs testing bounding boxes against rays, result is .
  void TestBoundingVolumes();
  //! Reads bounding boxes test results into PBO
  void ReadTestDataAsPBO();

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Tracing core 
  @{ */

  //! Creates computation mask for NV38 path using test data VBO in render of GL_POINTS.
  void MarkHitSpacePoints(int iSegmentX, int iSegmentY);

  //! Creates computation mask for NV40 using fullscreen quad and fragment program discard.
  void ComputationMask(int iSegmentX, int iSegmentY);
  //! Performs testing ray vs tringle for given segment.
  void TestHits(int iSegmentX, int iSegmentY);
  //! Performs hits reduction.
  void ReduceHits(int iSegmentX, int iSegmentY);

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Shading + Composition
  @{ */

  //! Performs shading of incomming hits.
  void Shade();
  //! Does final composition of shaded colors with previous image (blending + texture stretching & filtering).
  void RenderFinal();

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Bounding Box support
  @{ */

  //! Displays bounding boxes.
  void DisplayBoundingBoxes();
  //! Renders bounding boxes (helper function).
  void RenderBoundingBoxes();
  //! Renders specified bounding box.
  void RenderBoundingBox(Vector3f kMin, Vector3f kMax);
  //! Reads bounding box data back to CPU.
  void ReadbackBoundingBoxes();

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Gizmo support
  @{ */
  
  //! Displays all gizmos.
  void DisplayGizmos();
  //! Helper function for displaying scene gizmos (lights, objects ticks).
  void DisplaySceneGizmos();

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  /*! \name Common parameters support
  @{ */

  //! Sets common object parameters into bound Cg program.
  void SetCommonObjectParams(ParamsCache* pkParamsCache);
  //! Sets common scene parameters into bound Cg program.
  void SetCommonSceneParams(ParamsCache* pkParamsCache);

  //! @} 
  //////////////////////////////////////////////////////////////////////////
  // data members

  GLuint m_uiVBOScene;                  //!< VBO handle for scene geometry and related data.
  
  GLuint m_uiBOMarks;                   //!< VBO/PBO handle for mask points.
  GLuint m_uiVBOMarkPoints;             //!< VBO handle for static part of mask points.
  
  float* m_afBoundingBoxesMins;         //!< Array of bounding boxes data from readback (for display)
  float* m_afBoundingBoxesMaxs;         //!< Array of bounding boxes data from readback (for display)

  Matrix4f m_kWM;                       //!< actual world matrix (used in SetCommonObjectParams)
};

#endif

//! @}      //doxygen group
