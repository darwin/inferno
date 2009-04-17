// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \defgroup Raytracers
\ingroup NV38Box
*/
/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file RaytracerRForce.cpp
\brief Implementation of RaytracerRForce class.
*/

#include "base.h"

#include "RaytracerRForce.h"

static Matrix4f
TransformMatrix(const Matrix4f& rkMat, int iTransform)
{
  switch (iTransform)
  {
  case MT_NORMAL:
    return rkMat.Transpose();
    break;
  case MT_INVERSETRANSPOSE:
    return rkMat.Inverse();
    break;
  case MT_INVERSE:
    return rkMat.Inverse().Transpose();
    break;
  case MT_TRANSPOSE:
  default:
    // do nothing
    return rkMat;
    break;
  }
}

RaytracerRForce::RaytracerRForce()
{
  m_iActiveKernel = -1;
  m_iActivePBuffer = -1;
  m_iActiveTexture = -1;

  m_uiScreenWidth = RTRF_VIEWPORT_SX; 
  m_uiScreenHeight = RTRF_VIEWPORT_SY; 
  
  m_afBoundingBoxesMins = NULL;
  m_afBoundingBoxesMaxs  = NULL;

  m_bInitialized = false;
}

RaytracerRForce::~RaytracerRForce()
{
  Shutdown();
}

/*! 
Releases all resources.
*/
bool RaytracerRForce::Shutdown()
{
  if (!m_bInitialized) return false;

  log_write(LOG_LEVEL_INFO, "Releasing kernels ...\n");
  m_kKernelManager.Release();

  log_write(LOG_LEVEL_INFO, "Deleting textures ...\n");
  m_kTextureManager.Release();

  log_write(LOG_LEVEL_INFO, "Releasing pbuffers ...\n");
  m_kPBufferManager.Release();

  log_write(LOG_LEVEL_INFO, "Releasing read-back buffers ...\n");
  delete[] m_afBoundingBoxesMins;
  delete[] m_afBoundingBoxesMaxs;

  log_write(LOG_LEVEL_INFO, "Releasing PBO/VBO buffers ...\n");
  glDeleteBuffersARB(1, &m_uiBOMarks);

  log_write(LOG_LEVEL_INFO, "Shuting down Cg runtime ...\n");
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return false;
  pCgManager->Done();

  m_bInitialized = false;
  return true;
}

/*! 
Allocates all needed resources:
- Cg runtime
- VBO/PBO buffers
- pbuffers
- textures
- kernels

Preapres scene data structures for GPU as textures.
Also loads png image map from disk.
*/
bool RaytracerRForce::Prepare()
{
  if (m_bInitialized) return false;

  /////////////////////////
  // init Cg runtime

  log_write(LOG_LEVEL_INFO, "Initializing Cg runtime ...\n");

  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return false;

  pCgManager->Init();

  /////////////////////////
  // alloc data buffers

  log_write(LOG_LEVEL_INFO, "Preparing PBO/VBO buffers ...\n");

  // USE PBOs !
  glGenBuffersARB(1, &m_uiBOMarks);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_uiBOMarks);
  glBufferDataARB(GL_PIXEL_PACK_BUFFER_EXT, RTRF_MARKMEMORY_SIZE, NULL, GL_STREAM_DRAW_ARB);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);

  /////////////////////////
  // alloc readback buffers

  log_write(LOG_LEVEL_INFO, "Preparing read-back buffers ...\n");

  m_afBoundingBoxesMins = new float[3*RTRF_BVMAP_SIZE];
  m_afBoundingBoxesMaxs = new float[3*RTRF_BVMAP_SIZE];

  /////////////////////////
  // alloc pbuffers

  log_write(LOG_LEVEL_INFO, "Preparing pbuffers ...\n");
  log_indent_increase();

  m_kPBufferManager.resize(RTRF_PBUFFER_COUNT);

  PBuffer* pPBuffer; 

  // prepare RTRF_PBUFFER_SHADING
  pPBuffer = new PBuffer("rgba=8 texRECT");
  if (!pPBuffer) return false; // safety net
  pPBuffer->Initialize(RTRF_VIEWPORT_SX, RTRF_VIEWPORT_SY);
  SetupPBuffer(pPBuffer);

  m_kPBufferManager[RTRF_PBUFFER_SHADING] = pPBuffer;

  // prepare RTRF_PBUFFER_INTERSECTING
  pPBuffer = new PBuffer("rgba=32f doublebuffer depth texRECT depthTexRECT");
  if (!pPBuffer) return false; // safety net
  pPBuffer->Initialize(RTRF_INTERSECTING_SPACE_SX, RTRF_INTERSECTING_SPACE_SY);
  SetupPBuffer(pPBuffer);

  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING] = pPBuffer;

  // prepare RTRF_PBUFFER_BOUNDTESTING
  pPBuffer = new PBuffer("r=16f texRECT");
  if (!pPBuffer) return false; // safety net
  pPBuffer->Initialize(RTRF_BOUNDTESTING_SPACE_SX, RTRF_BOUNDTESTING_SPACE_SY);
  SetupPBuffer(pPBuffer);

  m_kPBufferManager[RTRF_PBUFFER_BOUNDTESTING] = pPBuffer;

  log_indent_decrease();
  /////////////////////////
  // alloc textures

  log_write(LOG_LEVEL_INFO, "Preparing textures ...\n");

  m_kTextureManager.resize(RTRF_TEXTURE_COUNT);
  Texture* pTexture;

  // If the data argument of TexImage1D, TexImage2D, or TexImage3D is a null
  // pointer (a zero-valued pointer in the C implementation), a one-, two-, or threedimensional
  // texture array is created with the specified target, level, internalformat,
  // width, height, and depth, but with unspecified image contents. In this case no pixel
  // values are accessed in client memory, and no pixel processing is performed. Errors
  // are generated, however, exactly as though the data pointer were valid.

  float* pAPositions;
  float* pBPositions;
  float* pCPositions;
  float* pANormals;
  float* pBNormals;
  float* pCNormals;
  float* pColors;
  float* pUVRR;
  if (!PrepareTriData(&pAPositions, 
                      &pBPositions, 
                      &pCPositions, 
                      &pANormals, 
                      &pBNormals, 
                      &pCNormals, 
                      &pColors,
                      &pUVRR)) return false;

  float* pBoxMins;
  float* pBoxMaxs;

  if (!PrepareBoundingVolumes(pAPositions, 
    pBPositions, 
    pCPositions, 
    &pBoxMins, 
    &pBoxMaxs)) return false;

  // prepare texture RTRF_TEXTURE_TRIS_APOSITION
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pAPositions);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  //exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_APOSITION] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_BPOSITION
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pBPositions);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_BPOSITION] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_CPOSITION
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pCPositions);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_CPOSITION] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_ANORMAL
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pANormals);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_ANORMAL] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_BNORMAL
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pBNormals);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_BNORMAL] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_CNORMAL
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pCNormals);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_CNORMAL] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_COLOR
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pColors);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_COLOR] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_TRIS_UVRR
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_TRIMAP_SX, RTRF_TRIMAP_SY, 0, GL_RGBA, GL_FLOAT, pUVRR);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_TRIS_UVRR] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_BOX_MAX
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_BVMAP_SX, RTRF_BVMAP_SY, 0, GL_RGBA, GL_FLOAT, pBoxMaxs);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_BOX_MAX] = pTexture;
  pTexture->Release();

  // prepare texture RTRF_TEXTURE_BOX_MIN
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_RECTANGLE_NV);
  pTexture->Bind();
  pTexture->SetSafeFPParameters();

  glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA_NV, RTRF_BVMAP_SX, RTRF_BVMAP_SY, 0, GL_RGBA, GL_FLOAT, pBoxMins);
#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, pTexture->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  m_kTextureManager[RTRF_TEXTURE_BOX_MIN] = pTexture;
  pTexture->Release();

  delete[] pAPositions;
  delete[] pBPositions;
  delete[] pCPositions;
  delete[] pANormals;
  delete[] pBNormals;
  delete[] pCNormals;
  delete[] pColors;
  delete[] pUVRR;

  delete[] pBoxMins;
  delete[] pBoxMaxs;

  // prepare texture RTRF_TEXTURE_MATERIAL
  pTexture = new Texture();
  if (!pTexture) return false; // safety net

  pTexture->Generate();
  pTexture->SetTarget(GL_TEXTURE_2D);
  pTexture->Bind();

  unsigned int uiWidth;
  unsigned int uiHeight;
  unsigned char* pucData;

  char* acTextureName = strdup(m_pkSimpleScene->m_kFileName.c_str());
  // dirty part
  size_t iLen = strlen(acTextureName);
  if (iLen<=3) return false; // safety net
  acTextureName[iLen-3] = 'p';
  acTextureName[iLen-2] = 'n';
  acTextureName[iLen-1] = 'g';
  log_write(LOG_LEVEL_INFO, "Loading '%s'\n", acTextureName);

  corona::Image* pkImage = corona::OpenImage(acTextureName, corona::PF_R8G8B8);
  if (!pkImage) {
    log_write(LOG_LEVEL_ERROR, "  Error: failed to load '%s'\n", acTextureName);
    return false;
  }

  uiWidth  = pkImage->getWidth();
  uiHeight = pkImage->getHeight();

  if (!(uiWidth==1024 && uiHeight==1024))
  {
    log_write(LOG_LEVEL_ERROR, "  Error: invalid dimensions of '%s' (expected 1024x1024)\n", acTextureName);
    free(acTextureName);
    delete pkImage;
    return false;
  }

  pucData = (unsigned char*)pkImage->getPixels();

  free(acTextureName);

#if 0
  // debug
  imdebug("rgb=8 w=%d h=%d %p", uiWidth, uiHeight, pucData);
  exit(1);
#endif

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, uiWidth, uiHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pucData);
  pTexture->SetImageParameters();

  m_kTextureManager[RTRF_TEXTURE_MATERIAL] = pTexture;

  delete pkImage;

#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_2D, pTexture->GetId(), GL_RGB, 0, NULL);
  //exit(1);
#endif

  pTexture->Release();

  /////////////////////////
  // alloc kernels

  log_write(LOG_LEVEL_INFO, "Preparing kernels ...\n");

  m_kKernelManager.resize(RTRF_KERNEL_COUNT);

  Kernel* pKernel;
  
  pKernel = new KernelHitTest();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_HITTEST] = pKernel;

  pKernel = new KernelBVTest();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_BVTEST] = pKernel;

  pKernel = new KernelBVBuilder();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_BVBUILDER] = pKernel;

  pKernel = new KernelReduceHits();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_REDUCEHITS] = pKernel;

  pKernel = new KernelRayGenerator();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_RAYGENERATOR] = pKernel;

  pKernel = new KernelShader();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_SHADER] = pKernel;

  pKernel = new KernelTransformer();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_TRANSFORMER] = pKernel;

  pKernel = new KernelDebug();
  if (!pKernel) return false;
  m_kKernelManager[RTRF_KERNEL_DEBUG] = pKernel;

  /////////////////////////
  // prepare kernels

  m_kKernelManager.Prepare();

  m_bInitialized = true;
  return true;
}

bool RaytracerRForce::PrepareBoundingVolumes(float* pAPositions, float* pBPositions, float* pCPositions, float** pBoxMin, float** pBoxMax)
{
  *pBoxMin = new float[4*RTRF_BVMAP_SX*RTRF_BVMAP_SY];
  *pBoxMax = new float[4*RTRF_BVMAP_SX*RTRF_BVMAP_SY];

  memset(*pBoxMin, 0, 4*RTRF_BVMAP_SX*RTRF_BVMAP_SY*sizeof(float));
  memset(*pBoxMax, 0, 4*RTRF_BVMAP_SX*RTRF_BVMAP_SY*sizeof(float));

  for (int ey=0; ey<RTRF_BVMAP_SY; ey++)
  {
    for (int ex=0; ex<RTRF_BVMAP_SX; ex++)
    {
      int base = (ex*RTRF_BVELEMENT_SX + (ey * RTRF_BVELEMENT_SY * RTRF_TRIMAP_SX)) * 4;

      float fMinX = CG_FLT_MAX;
      float fMinY = CG_FLT_MAX;
      float fMinZ = CG_FLT_MAX;

      float fMaxX = CG_FLT_MIN;
      float fMaxY = CG_FLT_MIN;
      float fMaxZ = CG_FLT_MIN;

      SimpleElement& rkE = m_pkSimpleScene->m_kElements[RTRF_BVMAP_SX*ey+ex];

      unsigned int counter = 0;
      for (int ty=0; ty<RTRF_BVELEMENT_SY; ty++)
      {
        for (int tx=0; tx<RTRF_BVELEMENT_SX; tx++)
        {
          if (counter<rkE.size())
          {
            int offset = (tx + ty * RTRF_TRIMAP_SX) * 4;
            int addr = base + offset;

            fMinX = MIN(fMinX, pAPositions[addr+0]);
            fMinX = MIN(fMinX, pBPositions[addr+0]);
            fMinX = MIN(fMinX, pCPositions[addr+0]);

            fMinY = MIN(fMinY, pAPositions[addr+1]);
            fMinY = MIN(fMinY, pBPositions[addr+1]);
            fMinY = MIN(fMinY, pCPositions[addr+1]);

            fMinZ = MIN(fMinZ, pAPositions[addr+2]);
            fMinZ = MIN(fMinZ, pBPositions[addr+2]);
            fMinZ = MIN(fMinZ, pCPositions[addr+2]);

            fMaxX = MAX(fMaxX, pAPositions[addr+0]);
            fMaxX = MAX(fMaxX, pBPositions[addr+0]);
            fMaxX = MAX(fMaxX, pCPositions[addr+0]);

            fMaxY = MAX(fMaxY, pAPositions[addr+1]);
            fMaxY = MAX(fMaxY, pBPositions[addr+1]);
            fMaxY = MAX(fMaxY, pCPositions[addr+1]);

            fMaxZ = MAX(fMaxZ, pAPositions[addr+2]);
            fMaxZ = MAX(fMaxZ, pBPositions[addr+2]);
            fMaxZ = MAX(fMaxZ, pCPositions[addr+2]);
          }

          counter++;
        }
      }

      int eaddr = (ex + ey * RTRF_BVMAP_SX) * 4;

      (*pBoxMin)[eaddr+0] = fMinX;
      (*pBoxMin)[eaddr+1] = fMinY;
      (*pBoxMin)[eaddr+2] = fMinZ;
      (*pBoxMin)[eaddr+3] = 0.0f;

      (*pBoxMax)[eaddr+0] = fMaxX;
      (*pBoxMax)[eaddr+1] = fMaxY;
      (*pBoxMax)[eaddr+2] = fMaxZ;
      (*pBoxMax)[eaddr+3] = 0.0f;
    }
  }
  return true;       
}

/*!
  Prepares set of buffers to be uploaded as textures to the GPU.
  - vertex positions: 3x[x,y,z,mat?]
  - vertex normals: 3x[x,y,z,?]
  - face colors: [r,g,b,packed(mat1, mat2)]
  - material and uv: [packed(u1v1), packed(u2v2), packed(u3v3), packed(mat1, mat2)]

  mat1 = material coefficient
  mat2 = reflection coefficient
  mat3 = refraction coefficient (unused)
  mat4 = eta for refraction (unused)
*/
bool RaytracerRForce::PrepareTriData(float** pAPositions, 
                                      float** pBPositions, 
                                      float** pCPositions, 
                                      float** pANormals, 
                                      float** pBNormals,
                                      float** pCNormals, 
                                      float** pColors, 
                                      float** pUVRR)
{
//  assert(m_pkSimpleScene->m_kElements.size()<=RTRF_BVMAP_SX*RTRF_BVMAP_SY);
  m_pkSimpleScene->m_kElements.resize(RTRF_BVMAP_SX*RTRF_BVMAP_SY);

  // allocate space
  *pAPositions = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pBPositions = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pCPositions = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pANormals   = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pBNormals   = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pCNormals   = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pColors     = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];
  *pUVRR       = new float[4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY];

  memset(*pAPositions, 0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pBPositions, 0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pCPositions, 0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pANormals,   0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pBNormals,   0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pCNormals,   0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pColors,     0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));
  memset(*pUVRR,       0, 4*RTRF_TRIMAP_SX*RTRF_TRIMAP_SY*sizeof(float));

  int ex = 0;
  int ey = 0;
  TSimpleElementContainer::iterator i = m_pkSimpleScene->m_kElements.begin();
  while (i!=m_pkSimpleScene->m_kElements.end())
  {
    SimpleElement& rkE = *i;
    assert(rkE.size()<=RTRF_BVELEMENT_SX*RTRF_BVELEMENT_SY);

    int base = (ex*RTRF_BVELEMENT_SX + (ey * RTRF_BVELEMENT_SY * RTRF_TRIMAP_SX)) * 4;
    int tx = 0;
    int ty = 0;

    // compute element transform
    Matrix4f kWM;
    kWM = Matrix4f::GetTranslationMatrix(rkE.m_kPosition) * (Matrix4f)rkE.m_kRotation;
    kWM.ApplyScale(rkE.m_kScale, Quaternionf::IDENTITY);

    SimpleElement::iterator t = rkE.begin();
    while (t!=rkE.end())
    {
      SimpleTriangle& kTriangle = *t;

      int offset = (tx + ty * RTRF_TRIMAP_SX) * 4;
      int addr = base + offset;

      Vector4f kVA = kWM * (Vector4f)kTriangle.m_akVertices[0];
      Vector4f kVB = kWM * (Vector4f)kTriangle.m_akVertices[1];
      Vector4f kVC = kWM * (Vector4f)kTriangle.m_akVertices[2];

      (*pAPositions)[addr+0] = kVA[0];
      (*pAPositions)[addr+1] = kVA[1];
      (*pAPositions)[addr+2] = kVA[2];
      (*pAPositions)[addr+3] = kTriangle.m_kMaterial[1]; // reflection_k

      (*pBPositions)[addr+0] = kVB[0];
      (*pBPositions)[addr+1] = kVB[1];
      (*pBPositions)[addr+2] = kVB[2];
      (*pBPositions)[addr+3] = kTriangle.m_kMaterial[2]; // refraction_k

      (*pCPositions)[addr+0] = kVC[0];
      (*pCPositions)[addr+1] = kVC[1];
      (*pCPositions)[addr+2] = kVC[2];
      (*pCPositions)[addr+3] = kTriangle.m_kMaterial[3]; // eta

      (*pANormals)[addr+0] = kTriangle.m_akNormals[0][0];
      (*pANormals)[addr+1] = kTriangle.m_akNormals[0][1];
      (*pANormals)[addr+2] = kTriangle.m_akNormals[0][2];
      (*pANormals)[addr+3] = 0.0f; // unused

      (*pBNormals)[addr+0] = kTriangle.m_akNormals[1][0];
      (*pBNormals)[addr+1] = kTriangle.m_akNormals[1][1];
      (*pBNormals)[addr+2] = kTriangle.m_akNormals[1][2];
      (*pBNormals)[addr+3] = 0.0f; // unused

      (*pCNormals)[addr+0] = kTriangle.m_akNormals[2][0];
      (*pCNormals)[addr+1] = kTriangle.m_akNormals[2][1];
      (*pCNormals)[addr+2] = kTriangle.m_akNormals[2][2];
      (*pCNormals)[addr+3] = 0.0f; // unused

      (*pColors)[addr+0] = kTriangle.m_kColor[0];
      (*pColors)[addr+1] = kTriangle.m_kColor[1];
      (*pColors)[addr+2] = kTriangle.m_kColor[2];
      (*pColors)[addr+3] = gpu_pack(kTriangle.m_kMaterial[0], kTriangle.m_kMaterial[1]); // base & reflection coef

      (*pUVRR)[addr+0] = gpu_pack(kTriangle.m_akTexCoords[0][0], kTriangle.m_akTexCoords[0][1]);
      (*pUVRR)[addr+1] = gpu_pack(kTriangle.m_akTexCoords[1][0], kTriangle.m_akTexCoords[1][1]);
      (*pUVRR)[addr+2] = gpu_pack(kTriangle.m_akTexCoords[2][0], kTriangle.m_akTexCoords[2][1]);
      (*pUVRR)[addr+3] = gpu_pack(kTriangle.m_kMaterial[0], kTriangle.m_kMaterial[1]); // base & reflection coef

      // go to next triangle
      tx++;
      if (tx>=RTRF_BVELEMENT_SX)
      {
        ty++;
        tx = 0;
      }
      t++;
    }

    SimpleTriangle kDummyTriangle;
    kDummyTriangle.m_akVertices[0] = Vector3f(0,0,0);
    kDummyTriangle.m_akVertices[1] = Vector3f(0,0,0);
    kDummyTriangle.m_akVertices[2] = Vector3f(0,0,0);
    kDummyTriangle.m_akNormals[0] = Vector3f(0,0,0);
    kDummyTriangle.m_akNormals[1] = Vector3f(0,0,0);
    kDummyTriangle.m_akNormals[2] = Vector3f(0,0,0);
    kDummyTriangle.m_akTexCoords[0] = Vector3f(0,0,0);
    kDummyTriangle.m_akTexCoords[1] = Vector3f(0,0,0);
    kDummyTriangle.m_akTexCoords[2] = Vector3f(0,0,0);
    kDummyTriangle.m_kMaterial = Vector4f(0,0,0,0);
    kDummyTriangle.m_kColor = ColorRGB(0,0,0);

    // fill degenerated triangles
    while (!(tx==0 && ty==RTRF_BVELEMENT_SY))
    {
      int offset = (tx + ty * RTRF_TRIMAP_SX) * 4;
      int addr = base + offset;

      (*pAPositions)[addr+0] = kDummyTriangle.m_akVertices[0][0];
      (*pAPositions)[addr+1] = kDummyTriangle.m_akVertices[0][1];
      (*pAPositions)[addr+2] = kDummyTriangle.m_akVertices[0][2];
      (*pAPositions)[addr+3] = kDummyTriangle.m_kMaterial[1]; // reflection_k

      (*pBPositions)[addr+0] = (*pAPositions)[addr+0];
      (*pBPositions)[addr+1] = (*pAPositions)[addr+1];
      (*pBPositions)[addr+2] = (*pAPositions)[addr+2];
      (*pBPositions)[addr+3] = (*pAPositions)[addr+3];

      (*pCPositions)[addr+0] = (*pAPositions)[addr+0];
      (*pCPositions)[addr+1] = (*pAPositions)[addr+1];
      (*pCPositions)[addr+2] = (*pAPositions)[addr+2];
      (*pCPositions)[addr+3] = (*pAPositions)[addr+3];

      (*pANormals)[addr+0] = kDummyTriangle.m_akNormals[0][0];
      (*pANormals)[addr+1] = kDummyTriangle.m_akNormals[0][1];
      (*pANormals)[addr+2] = kDummyTriangle.m_akNormals[0][2];
      (*pANormals)[addr+3] = 0.0f; // unused

      (*pBNormals)[addr+0] = kDummyTriangle.m_akNormals[1][0];
      (*pBNormals)[addr+1] = kDummyTriangle.m_akNormals[1][1];
      (*pBNormals)[addr+2] = kDummyTriangle.m_akNormals[1][2];
      (*pBNormals)[addr+3] = 0.0f; // unused

      (*pCNormals)[addr+0] = kDummyTriangle.m_akNormals[2][0];
      (*pCNormals)[addr+1] = kDummyTriangle.m_akNormals[2][1];
      (*pCNormals)[addr+2] = kDummyTriangle.m_akNormals[2][2];
      (*pCNormals)[addr+3] = 0.0f; // unused

      (*pColors)[addr+0] = kDummyTriangle.m_kColor[0];
      (*pColors)[addr+1] = kDummyTriangle.m_kColor[1];
      (*pColors)[addr+2] = kDummyTriangle.m_kColor[2];
      (*pColors)[addr+3] = gpu_pack(kDummyTriangle.m_kMaterial[0], kDummyTriangle.m_kMaterial[1]); // base & reflection coef

      (*pUVRR)[addr+0] = gpu_pack(kDummyTriangle.m_akTexCoords[0][0], kDummyTriangle.m_akTexCoords[0][1]);
      (*pUVRR)[addr+1] = gpu_pack(kDummyTriangle.m_akTexCoords[1][0], kDummyTriangle.m_akTexCoords[1][1]);
      (*pUVRR)[addr+2] = gpu_pack(kDummyTriangle.m_akTexCoords[2][0], kDummyTriangle.m_akTexCoords[2][1]);
      (*pUVRR)[addr+3] = gpu_pack(kDummyTriangle.m_kMaterial[0], kDummyTriangle.m_kMaterial[1]); // base & reflection coef

      // go to next triangle
      tx++;
      if (tx>=RTRF_BVELEMENT_SX)
      {
        ty++;
        tx = 0;
      }
    }

    // go to the next element
    ex++;
    if (ex>=RTRF_BVMAP_SX) 
    {
      ey++;
      ex = 0;
    }
    i++;
  }

  return true;
}

/*!
Prepares and uploads static VBOs.
- VBO with scene geometry and attributes
- VBO with positions of mark points 

Scene VBO is packed for glDrawBuffer(GL_TRIANGLES, ...)
That means 3 consequent vertex infos built up one triangle.
My vertex info is:
- position [x,y,z]
- normal [x,y,z]
- material [mat1, mat2, mat3, mat4]
- texture coordinates [tu, tv]
- triangle color [r,g,b,counter]
- optimization info [?,?,?,?] some precomputed data for custom shaders
*/
bool RaytracerRForce::Build()
{
  log_write(LOG_LEVEL_INFO, "Building scene objects VBO ...\n");

  unsigned int iCount = RTRF_TRIMAP_SX*RTRF_TRIMAP_SY;
  int iElementSize = RTRF_BVELEMENT_SIZE;
  unsigned int iSize = 3 * RTRF_VBO_SCENE_VERTEX_SIZE; // V+N+Mat+UV+COLOR+ADDR
  unsigned int iBufferSize = iSize*iCount*sizeof(float);
  
  // interleave data
  float* pBuffer = new float[iBufferSize];
  memset(pBuffer, 0, iBufferSize*sizeof(float));

  // pack buffer
  float fCounter = 0;
  float * p = pBuffer;

  int ex = 0;
  int ey = 0;
  TSimpleElementContainer::iterator e = m_pkSimpleScene->m_kElements.begin();
  while (e != m_pkSimpleScene->m_kElements.end())
  {
    int tx = 0;
    int ty = 0;
    int counter = 0;
    SimpleElement& kElement = *e;
    SimpleElement::iterator i = kElement.begin();
    while (i != kElement.end())
    {
      fCounter += 1.0f;
      SimpleTriangle& kTriangle = *i;

      float ax = ex * RTRF_BVELEMENT_SX + tx;
      float ay = ey * RTRF_BVELEMENT_SY + ty;

      float addrx = (ax / RTRF_TRIMAP_SX)*2.0f - 1.0f;
      float addry = ((ay / RTRF_TRIMAP_SY)*2.0f) / 3.0f;
      float biasx = (2.0f / RTRF_TRIMAP_SX) / 2.0f;
      float biasy = ((2.0f / RTRF_TRIMAP_SY) / 3.0f) / 2.0f;

      // pack i-th triangle

      // V.A
      *p++ = kTriangle.m_akVertices[0][0];
      *p++ = kTriangle.m_akVertices[0][1];
      *p++ = kTriangle.m_akVertices[0][2];

      *p++ = kTriangle.m_akNormals[0][0];
      *p++ = kTriangle.m_akNormals[0][1];
      *p++ = kTriangle.m_akNormals[0][2];

      *p++ = kTriangle.m_kMaterial[0];
      *p++ = kTriangle.m_kMaterial[1];
      *p++ = kTriangle.m_kMaterial[2];
      *p++ = kTriangle.m_kMaterial[3];

      *p++ = kTriangle.m_akTexCoords[0][0];
      *p++ = kTriangle.m_akTexCoords[0][1];

      *p++ = kTriangle.m_kColor[0];
      *p++ = kTriangle.m_kColor[1];
      *p++ = kTriangle.m_kColor[2];
      *p++ = fCounter;

      *p++ = addrx + biasx;
      *p++ = -1 + 0*(2.0f/3.0f)+ addry + biasy;
      *p++ = gpu_pack(ax, ay); // possible problem with precision !
      *p++ = gpu_pack(kTriangle.m_kMaterial[0], kTriangle.m_kMaterial[1]);

      // V.B
      *p++ = kTriangle.m_akVertices[1][0];
      *p++ = kTriangle.m_akVertices[1][1];
      *p++ = kTriangle.m_akVertices[1][2];

      *p++ = kTriangle.m_akNormals[1][0];
      *p++ = kTriangle.m_akNormals[1][1];
      *p++ = kTriangle.m_akNormals[1][2];

      *p++ = kTriangle.m_kMaterial[0];
      *p++ = kTriangle.m_kMaterial[1];
      *p++ = kTriangle.m_kMaterial[2];
      *p++ = kTriangle.m_kMaterial[3];

      *p++ = kTriangle.m_akTexCoords[1][0];
      *p++ = kTriangle.m_akTexCoords[1][1];

      *p++ = kTriangle.m_kColor[0];
      *p++ = kTriangle.m_kColor[1];
      *p++ = kTriangle.m_kColor[2];
      *p++ = fCounter;

      *p++ = addrx + biasx;
      *p++ = -1 + 1*(2.0f/3.0f) + addry + biasy;
      *p++ = gpu_pack(ax, ay); // possible problem with precision !
      *p++ = gpu_pack(kTriangle.m_kMaterial[0], kTriangle.m_kMaterial[1]);

      // V.C
      *p++ = kTriangle.m_akVertices[2][0];
      *p++ = kTriangle.m_akVertices[2][1];
      *p++ = kTriangle.m_akVertices[2][2];

      *p++ = kTriangle.m_akNormals[2][0];
      *p++ = kTriangle.m_akNormals[2][1];
      *p++ = kTriangle.m_akNormals[2][2];

      *p++ = kTriangle.m_kMaterial[0];
      *p++ = kTriangle.m_kMaterial[1];
      *p++ = kTriangle.m_kMaterial[2];
      *p++ = kTriangle.m_kMaterial[3];

      *p++ = kTriangle.m_akTexCoords[2][0];
      *p++ = kTriangle.m_akTexCoords[2][1];

      *p++ = kTriangle.m_kColor[0];
      *p++ = kTriangle.m_kColor[1];
      *p++ = kTriangle.m_kColor[2];
      *p++ = fCounter;

      *p++ = addrx + biasx;
      *p++ = -1 + 2*(2.0f/3.0f) + addry + biasy;
      *p++ = gpu_pack(ax, ay); // possible problem with precision !
      *p++ = gpu_pack(kTriangle.m_kMaterial[0], kTriangle.m_kMaterial[1]);

      // go to next triangle
      tx++;
      if (tx>=RTRF_BVELEMENT_SX)
      {
        ty++;
        tx = 0;
      }

      i++;
      counter++;
    }

    assert(iElementSize>=counter);
    p += iSize * (iElementSize-counter); // skip to next element memory block
    
    // go to next element
    ex++;
    if (ex>=RTRF_BVMAP_SX) 
    {
      ey++;
      ex = 0;
    }

    e++;
  }

  // we have some static data to store in VBO
  glGenBuffersARB(1, &m_uiVBOScene);

  // create buffer object
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiVBOScene);

  // initialize data store of buffer object
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, iBufferSize, pBuffer, GL_STATIC_DRAW_ARB);

  // done
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // for safety

  delete[] pBuffer;

  /////////////////////////////////////////////////////////////////////
  // create markpoints geometry VBO

  log_write(LOG_LEVEL_INFO, "Building markers VBO ...\n");

  iCount = (RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y);
  iSize = 1 * (2); // V2
  iBufferSize = iCount*iSize*sizeof(float);

  // interleave data
  pBuffer = new float[iBufferSize];
  p = pBuffer;
  const float iSizeX = RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X;
  const float iSizeY = RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y;
  const float iHalfSizeX = (RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)/2;
  const float iHalfSizeY = (RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)/2;
  for (int y=0; y<iSizeY; y++)
  {
    for (int x=0; x<iSizeX; x++)
    {
      // v0
      *p++ = ((float)(x-iHalfSizeX)/iHalfSizeX+(2.0f/iSizeX)/2.0f);
      *p++ = ((float)(y-iHalfSizeY)/iHalfSizeY+(2.0f/iSizeY)/2.0f);
    }
  }

  // we have some static data to store in VBO
  glGenBuffersARB(1, &m_uiVBOMarkPoints);

  // create buffer object
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiVBOMarkPoints);

  // initialize data store of buffer object
  glBufferDataARB(GL_ARRAY_BUFFER_ARB, iBufferSize, pBuffer, GL_STATIC_DRAW_ARB);

  // done
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // for safety

  delete[] pBuffer;

  return true;
}

/*!
Binds scene VBO and sets its pointers.
See RaytracerRForce::Build() for memory layout.
*/
void RaytracerRForce::BindSceneBuffers()
{
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_uiVBOScene);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3,GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(3));
  glClientActiveTextureARB(GL_TEXTURE2_ARB);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
  glTexCoordPointer(4,GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(3 + 3));
  glClientActiveTextureARB(GL_TEXTURE1_ARB);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
  glTexCoordPointer(4,GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(3 + 3 + 4 + 2 + 4));
  glClientActiveTextureARB(GL_TEXTURE0_ARB);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
  glTexCoordPointer(2,GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(3 + 3 + 4));
  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(4,GL_FLOAT,BUFFER_STRIDE(RTRF_VBO_SCENE_VERTEX_SIZE),BUFFER_OFFSET(3 + 3 + 4 + 2));
}

/*!
Releases scene VBO and disables its pointers.
*/
void RaytracerRForce::UnbindSceneBuffers()
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glClientActiveTextureARB(GL_TEXTURE2_ARB);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glClientActiveTextureARB(GL_TEXTURE1_ARB);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glClientActiveTextureARB(GL_TEXTURE0_ARB);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // for safety
}

/*!
Sets GL_PROJECTION and GL_MODELVIEW matrices according to active camera.
*/
void RaytracerRForce::PrepareSceneProjection()
{
  // init projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  ApplyCameraProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  ApplyCameraTransform();

  vec3 e = m_pkSimpleScene->m_kCameras[GetActiveCamera()].m_kManipulator.get_eye_pos();
  m_kEyePos[0] = e.x;
  m_kEyePos[1] = e.y;
  m_kEyePos[2] = e.z;

  vec3 f = m_pkSimpleScene->m_kCameras[GetActiveCamera()].m_kManipulator.get_focus_pos();
  m_kFocusPos[0] = f.x;
  m_kFocusPos[1] = f.y;
  m_kFocusPos[2] = f.z;
}

/*!
Sets common scene parameters into actually bound Cg program.
Common parameters are parameters shared between many shaders.
This code is just for convenience.
*/
void RaytracerRForce::SetCommonSceneParams(ParamsCache* pkCache)
{
  // set world-space eye position
  if (pkCache->m_kWEyePosParam)
    cgGLSetParameter4f(pkCache->m_kWEyePosParam, m_kEyePos[0], m_kEyePos[1], m_kEyePos[2], 1.0f);

  if (pkCache->m_kWFocusPosParam)
    cgGLSetParameter4f(pkCache->m_kWFocusPosParam, m_kFocusPos[0], m_kFocusPos[1], m_kFocusPos[2], 1.0f);

  if (pkCache->m_kWEyeVecParam)
  {
    Vector3f v = m_kEyePos - m_kFocusPos;
    v.Normalize();
    cgGLSetParameter4f(pkCache->m_kWEyeVecParam, v[0], v[1], v[2], 1.0f);
  }

  // set world-space light positions
  if (pkCache->m_kWLightPosParam)
  {
    unsigned int iCounter = 0;
    TParamsArray::iterator i = pkCache->m_kWLightPosParams.begin();
    while (i!=pkCache->m_kWLightPosParams.end())
    {
      if (iCounter>=m_pkSimpleScene->m_kLights.size()) break; // safety net

      CGparameter& kElement = *i;
      SimpleLight* pkLight = &m_pkSimpleScene->m_kLights[iCounter];

      cgGLSetParameter4f(kElement, pkLight->m_kPosition[0], pkLight->m_kPosition[1], pkLight->m_kPosition[2], 1.0f);

      iCounter++;
      i++;
    }
  }
}

/*!
Sets common object parameters into actually bound Cg program.
Common parameters are parameters shared between many shaders.
This code is just for convenience.
*/
void RaytracerRForce::SetCommonObjectParams(ParamsCache* pkCache)
{
  
  if (pkCache->m_kModelParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelParam, TransformMatrix(m_kWM, MT_NORMAL)[0]);

  if (pkCache->m_kModelIParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelIParam, TransformMatrix(m_kWM, MT_INVERSE)[0]);

  if (pkCache->m_kModelITParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelITParam, TransformMatrix(m_kWM, MT_INVERSETRANSPOSE)[0]);

  if (pkCache->m_kModelTParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelTParam, TransformMatrix(m_kWM, MT_TRANSPOSE)[0]);

  if (pkCache->m_kModelViewParam || 
      pkCache->m_kModelViewIParam || 
      pkCache->m_kModelViewITParam || 
      pkCache->m_kModelViewTParam)
  {
    Matrix4f kTemp;
    glGetFloatv(GL_MODELVIEW_MATRIX, &kTemp[0][0]);

    if (pkCache->m_kModelViewParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewParam, TransformMatrix(kTemp, MT_NORMAL)[0]);

    if (pkCache->m_kModelViewIParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewIParam, TransformMatrix(kTemp, MT_INVERSE)[0]);

    if (pkCache->m_kModelViewITParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewITParam, TransformMatrix(kTemp, MT_INVERSETRANSPOSE)[0]);

    if (pkCache->m_kModelViewTParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewTParam, TransformMatrix(kTemp, MT_TRANSPOSE)[0]);
  }

  if (pkCache->m_kModelViewProjParam || 
      pkCache->m_kModelViewProjIParam || 
      pkCache->m_kModelViewProjITParam || 
      pkCache->m_kModelViewProjTParam)
  {
    Matrix4f kModView, kProj;
    glGetFloatv(GL_PROJECTION_MATRIX, &kProj[0][0]);
    glGetFloatv(GL_MODELVIEW_MATRIX, &kModView[0][0]);
    Matrix4f kTemp = kModView * kProj;

    if (pkCache->m_kModelViewProjParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewProjParam, TransformMatrix(kTemp, MT_NORMAL)[0]);

    if (pkCache->m_kModelViewProjIParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewProjIParam, TransformMatrix(kTemp, MT_INVERSE)[0]);

    if (pkCache->m_kModelViewProjITParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewProjITParam, TransformMatrix(kTemp, MT_INVERSETRANSPOSE)[0]);

    if (pkCache->m_kModelViewProjTParam)
      cgGLSetMatrixParameterfr(pkCache->m_kModelViewProjTParam, TransformMatrix(kTemp, MT_TRANSPOSE)[0]);
  }

  if (pkCache->m_kProjParam || 
      pkCache->m_kProjIParam || 
      pkCache->m_kProjITParam || 
      pkCache->m_kProjTParam)
  {
    Matrix4f kTemp;
    glGetFloatv(GL_PROJECTION_MATRIX, &kTemp[0][0]);

    if (pkCache->m_kProjParam)
      cgGLSetMatrixParameterfr(pkCache->m_kProjParam, TransformMatrix(kTemp, MT_NORMAL)[0]);

    if (pkCache->m_kProjIParam)
      cgGLSetMatrixParameterfr(pkCache->m_kProjIParam, TransformMatrix(kTemp, MT_INVERSE)[0]);

    if (pkCache->m_kProjITParam)
      cgGLSetMatrixParameterfr(pkCache->m_kProjITParam, TransformMatrix(kTemp, MT_INVERSETRANSPOSE)[0]);

    if (pkCache->m_kProjTParam)
      cgGLSetMatrixParameterfr(pkCache->m_kProjTParam, TransformMatrix(kTemp, MT_TRANSPOSE)[0]);
  }
  /*

  if (pkCache->m_kModelParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelParam, TransformMatrix(m_kWM, MT_NORMAL)[0]);

  if (pkCache->m_kModelIParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelIParam, TransformMatrix(m_kWM, MT_INVERSE)[0]);

  if (pkCache->m_kModelITParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelITParam, TransformMatrix(m_kWM, MT_INVERSETRANSPOSE)[0]);

  if (pkCache->m_kModelTParam)
    cgGLSetMatrixParameterfr(pkCache->m_kModelTParam, TransformMatrix(m_kWM, MT_TRANSPOSE)[0]);

  if (pkCache->m_kModelViewParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);

  if (pkCache->m_kModelViewIParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewIParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE);

  if (pkCache->m_kModelViewITParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewITParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);

  if (pkCache->m_kModelViewTParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewTParam, CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_TRANSPOSE);

  if (pkCache->m_kModelViewProjParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

  if (pkCache->m_kModelViewProjIParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewProjIParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);

  if (pkCache->m_kModelViewProjITParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewProjITParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);

  if (pkCache->m_kModelViewProjTParam)
    cgGLSetStateMatrixParameter(pkCache->m_kModelViewProjTParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);

  if (pkCache->m_kProjParam)
    cgGLSetStateMatrixParameter(pkCache->m_kProjParam, CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

  if (pkCache->m_kProjIParam)
    cgGLSetStateMatrixParameter(pkCache->m_kProjIParam, CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE);

  if (pkCache->m_kProjITParam)
    cgGLSetStateMatrixParameter(pkCache->m_kProjITParam, CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_INVERSE_TRANSPOSE);

  if (pkCache->m_kProjTParam)
    cgGLSetStateMatrixParameter(pkCache->m_kProjTParam, CG_GL_PROJECTION_MATRIX, CG_GL_MATRIX_TRANSPOSE);
    */
}

/*!
Renders whole scene in given mode.
Assumes GL_PROJECTION matrix is properly set.
Assumes GL_MODELVIEW matrix is set to scene transform from world space to view space.
Assumes custom OpenGL state is set.

Sets common parameters and renders scene elements with their respective transformations.
*/
void RaytracerRForce::RenderScene(GLenum mode = GL_TRIANGLES)
{
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // now, matrix is set to be world-space -> viewspace

  /////////////////////////////////////////////////
  // process known per-scene params

  ParamsCache* pkVPCache = ActiveKernel()->GetVertexProgramParamsCache();
  if (!pkVPCache) return; // safety net

  ParamsCache* pkFPCache = ActiveKernel()->GetFragmentProgramParamsCache();
  if (!pkFPCache) return; // safety net

  SetCommonSceneParams(pkVPCache);
  SetCommonSceneParams(pkFPCache);

  glPopMatrix();

  for (int i=0; i<RTRF_BVMAP_SIZE; i++)
  {
    SimpleElement& kE = m_pkSimpleScene->m_kElements[i];

    // push current matrix state
    glPushMatrix();

    // compute element transform
    Matrix4f kSM; // scene transformation
    GetSceneTransform(kSM);
    m_kWM = Matrix4f::GetTranslationMatrix(kE.m_kPosition) * (Matrix4f)kE.m_kRotation;
    m_kWM.ApplyScale(kE.m_kScale, Quaternionf::IDENTITY);
    m_kWM = m_kWM.Transpose() * kSM;

    // apply transform
    glMultMatrixf(m_kWM[0]);

    //////////////////////////////////////////////////////////////////////////
    // process known per-object params

    SetCommonObjectParams(pkVPCache);
    SetCommonObjectParams(pkFPCache);

    // draw element
    glDrawArrays(mode, i*3*RTRF_BVELEMENT_SIZE, 3*(GLsizei)kE.size());

    // pop matrix state
    glPopMatrix();
  }
}

/*!
This code is for NV38 raytracing path.
Aim is to mark in z-buffer parts which should be computed in ray-triangle testing.
The rest should be rejected by early-z testing.
There is hardware limitation in NV38 and we cannot use fragment program for
z-buffer modification. This would disable early-z functionality.

Shader uses computation mask prepared by testing bounding volumes against rays (see TestBoundingVolumes).
Computation mask is stored RTRF_PBUFFER_BOUNDTESTING pbuffer and was read as PBO.
This PBO is here set as texture with one component.
Another static VBO is set and defines point X,Y positions.

Points are rendered into RTRF_PBUFFER_INTERSECTING pbuffer.
Texture coordinates are used as point Z value.
Point is rendered with glPointSize(RTRF_BVELEMENT_SX).
So we cover all triangles of one element with one point.

This function supports work per segment.
*/
void RaytracerRForce::MarkHitSpacePoints(int iSegmentX, int iSegmentY)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  SetActiveKernel(RTRF_KERNEL_HITTEST);
  ActiveKernel()->SetMode(KERNEL_HITTEST_MODE_MARKPOINTS);
  ActiveKernel()->Activate();

  glPointSize(RTRF_BVELEMENT_SX);

  glBindBufferARB(GL_ARRAY_BUFFER, m_uiVBOMarkPoints);
  glEnableClientState(GL_VERTEX_ARRAY); 
  glVertexPointer(2, GL_FLOAT, BUFFER_STRIDE(0), BUFFER_OFFSET(0));

  int iSegment = iSegmentX + iSegmentY * RTRF_SEGMENTS_X;

  glBindBufferARB(GL_ARRAY_BUFFER, m_uiBOMarks);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
  glTexCoordPointer(1, GL_FLOAT, BUFFER_STRIDE(0), BUFFER_OFFSET(iSegment*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y))));

  glViewport(iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);
  glDrawArrays(GL_POINTS, 0, (RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y));

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glBindBufferARB(GL_ARRAY_BUFFER, 0);
  
  ActiveKernel()->Deactivate();

  glPointSize(1.0f);
}

/*!
This code is for NV40 raytracing path.
This is method as described by M.Harris at http://www.gpgpu.org/forums/viewtopic.php?t=256
Aim is to mark in z-buffer parts which should be computed in ray-triangle testing.
The rest should be rejected by early-z testing.

Shader uses computation mask prepared by testing bounding volumes against rays (see TestBoundingVolumes).
Computation mask is stored RTRF_PBUFFER_BOUNDTESTING pbuffer and was bound as texture.

We execute simple fragment program which does discard for parts which 
shouldn't be computed and should be rejected early.

Discard is executed according to texture value in RTRF_PBUFFER_BOUNDTESTING pbuffer.
We do stretch of this texture by factor 8 (RTRF_BVELEMENT_SX) in both axis, because
we need to cover all triangles with same value from bound volume testing.

This function supports work per segment.
*/
void RaytracerRForce::ComputationMask(int iSegmentX, int iSegmentY)
{
  CGprogram fp;
  CGparameter param;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetActiveKernel(RTRF_KERNEL_BVTEST);

  ActiveKernel()->SetMode(KERNEL_BVTEST_MODE_COMPUTATIONMASK);
  ActiveKernel()->Activate();

  fp = ActiveKernel()->GetFragmentProgram(KERNEL_BVTEST_MODE_COMPUTATIONMASK);

  param = cgGetNamedParameter(fp, "tests");
  m_kPBufferManager[RTRF_PBUFFER_BOUNDTESTING]->BindBuffer(WGL_FRONT_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_BOUNDTESTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, m_kTextureManager[RTRF_TEXTURE_BOX_MAX]->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  glViewport(iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);

  float B = 0;

  float S0X = iSegmentX*(RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X);
  float S0Y = iSegmentY*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y);
  float D0X = RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X;
  float D0Y = RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y;

  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B,   0+S0Y+B);
  glVertex3f(-1,-1, HARDDEPTH(0));
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B,   0+S0Y+B);
  glVertex3f( 1,-1, HARDDEPTH(0));
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B, D0Y+S0Y+B);
  glVertex3f( 1, 1, HARDDEPTH(0));
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B, D0Y+S0Y+B);
  glVertex3f(-1, 1, HARDDEPTH(0));
  glEnd();

  param = cgGetNamedParameter(fp, "tests");
  cgGLDisableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
Fundamental routine testing rays against triangles.
Every triangle corresponds to one pixel.
Every block of RTRF_TRIMAP_SX x RTRF_TRIMAP_SY pixels corresponds to one ray.

We execute one quad over whole workspace area to compute hit and their data (especially time).

This function supports work per segment because of possible memory limitations.
*/
void RaytracerRForce::TestHits(int iSegmentX, int iSegmentY)
{
  CGprogram fp;
  CGparameter param;

  glViewport(iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetActiveKernel(RTRF_KERNEL_HITTEST);
  ActiveKernel()->SetMode(KERNEL_HITTEST_MODE_TEST);
  ActiveKernel()->Activate();

  fp = ActiveKernel()->GetFragmentProgram(KERNEL_HITTEST_MODE_TEST);

  param = cgGetNamedParameter(fp, "rays");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "tris_posx");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "tris_posy");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "tris_posz");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);
  
  float B = 0;
  float S0X = RTRF_WORKSPACE_SX + iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X);
  float S0Y =                0 + iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y);
  float D0X = RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X;
  float D0Y = RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y;

  float S1X = RTRF_WORKSPACE_SX + iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X);
  float S1Y =  RTRF_VIEWPORT_SY + iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y);
  float D1X = RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X;
  float D1Y = RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y;

  float S2X =               0 + iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X);
  float S2Y =               0 + iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y);
  float D2X = (RTRF_SEGMENT_SX) / RTRF_TRIMAP_SX;
  float D2Y = (RTRF_SEGMENT_SY) / RTRF_TRIMAP_SY;

  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B,   0+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2,   0+S2X+B,   0+S2Y+B);
  glVertex3f(-1,-1, HARDDEPTH(0.75));
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B,   0+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2, D2X+S2X+B,   0+S2Y+B);
  glVertex3f( 1,-1, HARDDEPTH(0.75));
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B, D1Y+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2, D2X+S2X+B, D2Y+S2Y+B);
  glVertex3f( 1, 1, HARDDEPTH(0.75));
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B, D1Y+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2,   0+S2X+B, D2Y+S2Y+B);
  glVertex3f(-1, 1, HARDDEPTH(0.75));
  glEnd();

  param = cgGetNamedParameter(fp, "tris_posz");
  cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "tris_posy");
  cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "tris_posx");
  cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "rays");
  cgGLDisableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
Read bound volumes tests as PBO.
Bound volumes testing is done in RaytracerRForce::TestBoundingVolumes().
This PBO is used as VBO later in RaytracerRForce::MarkHitSpacePoints()

Function loads all segments to one PBO using different offsets.

Assumes RTRF_PBUFFER_BOUNDTESTING pbuffer is active context.
*/
void RaytracerRForce::ReadTestDataAsPBO()
{
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_uiBOMarks);
  glRasterPos2f(-1, -1);
  glReadBuffer(GL_FRONT);

  for (int j=0; j<RTRF_SEGMENTS_Y; j++)
  {
    for (int i=0; i<RTRF_SEGMENTS_X; i++)
    {
      int iSegment = i + j * RTRF_SEGMENTS_X;
      glReadPixels(i*(RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X), j*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y), RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X, RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y, GL_LUMINANCE, GL_FLOAT, BUFFER_OFFSET(iSegment*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)))); 
    }
  }
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);

#if 0
  // debug - display buffer
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, m_uiBOMarks);
  float* p = (float*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_EXT, GL_READ_ONLY_ARB);
  imdebug("lum b=32f w=%d h=%d %p", RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X, RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y, p + 0*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)));
  imdebug("lum b=32f w=%d h=%d %p", RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X, RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y, p + 1*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)));
  imdebug("lum b=32f w=%d h=%d %p", RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X, RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y, p + 2*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)));
  imdebug("lum b=32f w=%d h=%d %p", RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X, RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y, p + 3*((RTRF_BOUNDTESTING_SPACE_SX/RTRF_SEGMENTS_X)*(RTRF_BOUNDTESTING_SPACE_SY/RTRF_SEGMENTS_Y)));
  glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_EXT);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT, 0);
  exit(1);
#endif
}

/*!
Function is responsible for testing rays against bounding volumes of objects.
This is optimization step for future ray x triangle tests.
We have 16 (4x4) objects so we need to do 16 tests per ray.
We are drawing quad scaled by factor 4. In every pixel is done one test boundig volume against ray.

All is executed in RTRF_PBUFFER_BOUNDTESTING pbuffer.
Results are then used for optimization of ray triangle tests/
Different path is available for NV38 and NV40 hardware.
*/
void RaytracerRForce::TestBoundingVolumes()
{
  CGprogram fp;
  CGparameter param;

  glViewport(0, 0, RTRF_BOUNDTESTING_SPACE_SX, RTRF_BOUNDTESTING_SPACE_SY);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetActiveKernel(RTRF_KERNEL_BVTEST);
  ActiveKernel()->Activate();

  fp = ActiveKernel()->GetFragmentProgram(0);

  param = cgGetNamedParameter(fp, "rays");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "boxmins");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "boxmaxs");
  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);
  cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
  cgGLEnableTextureParameter(param);

/* DEBUG - static version
  param = cgGetNamedParameter(fp, "boxmins");
  cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_BOX_MIN]->GetId());
  cgGLEnableTextureParameter(param);

  param = cgGetNamedParameter(fp, "boxmaxs");
  cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_BOX_MAX]->GetId());
  cgGLEnableTextureParameter(param);
*/

#if 0
  // debug
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, m_kTextureManager[RTRF_TEXTURE_BOX_MAX]->GetId(), GL_RGBA, 0, NULL);
  exit(1);
#endif

  float B = 0;
  
  float S0X = RTRF_WORKSPACE_SX;
  float S0Y = 0;
  float D0X = RTRF_VIEWPORT_SX;
  float D0Y = RTRF_VIEWPORT_SY;

  float S1X = RTRF_WORKSPACE_SX;
  float S1Y = RTRF_VIEWPORT_SY;
  float D1X = RTRF_VIEWPORT_SX;
  float D1Y = RTRF_VIEWPORT_SY;

  float S2X = 0;
  float S2Y = 0;
  float D2X = RTRF_BOUNDTESTING_SPACE_SX / RTRF_BVMAP_SX;
  float D2Y = RTRF_BOUNDTESTING_SPACE_SY / RTRF_BVMAP_SY;

  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B,   0+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2,   0+S2X+B,   0+S2Y+B);
  glVertex2f(-1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B,   0+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2, D2X+S2X+B,   0+S2Y+B);
  glVertex2f( 1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B, D1Y+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2, D2X+S2X+B, D2Y+S2Y+B);
  glVertex2f( 1, 1);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B, D1Y+S1Y+B);
  glMultiTexCoord2f(GL_TEXTURE2,   0+S2X+B, D2Y+S2Y+B);
  glVertex2f(-1, 1);
  glEnd();

  param = cgGetNamedParameter(fp, "boxmaxs");
  cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "boxmins");
  cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "rays");
  cgGLEnableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
Hits reduction is process when first hit is collected across ray area.
In our intersecting buffer we have 32x32 blocks corresponding to every ray.
First component of image stores time of hit, if there was no hit time is FLT_CG_MAX.

Reduce kernel is simple kernel which fetches 4 neighbour values and writes
minimal hit from them according to time. This is called reduction.

We use pbuffer FRONT-BACK pingpong scheme to reduce 32x32 to 1x1.
5 reduction steps is needed for this and 
finally, we result with minimal hits per ray in back buffer.
*/
void RaytracerRForce::ReduceHits(int iSegmentX, int iSegmentY)
{
  CGprogram fp; 
  CGparameter param;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetActiveKernel(RTRF_KERNEL_REDUCEHITS);

  ActiveKernel()->SetMode(KERNEL_REDUCEHITS_MODE_REDUCE);
  ActiveKernel()->Activate();

  // pass data as texture parameter
  fp = ActiveKernel()->GetFragmentProgram(0);
  param = cgGetNamedParameter(fp, "data0");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data1");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data2");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data3");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(64,64,2048,2048,GL_RGB);
 // exit(1);
#endif

  bool swap = false;
  int resolution_x = RTRF_SEGMENT_SX;
  int resolution_y = RTRF_SEGMENT_SY;
  for (int k=0; k<5; k++) // do 5 reduction steps
  {
    // map other surface as input texture
    if (swap)
    {
      glDrawBuffer(GL_FRONT);
      ActivePBuffer()->BindBuffer(WGL_BACK_LEFT_ARB);
    }
    else
    {
      glDrawBuffer(GL_BACK);
      ActivePBuffer()->BindBuffer(WGL_FRONT_LEFT_ARB);
    }

    int S0X = iSegmentX*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X);
    int S0Y = iSegmentY*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y);

    int half_resolution_x = resolution_x/2;
    int half_resolution_y = resolution_y/2;
    float bias = -1.0;

    // perform reduction
    glViewport(S0X, S0Y, half_resolution_x, half_resolution_y);
    glBegin(GL_QUADS);
      glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+0+0+bias);
      glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+0+1+bias);
      glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+0+0+bias);
      glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+0+1+bias);
      glVertex2f(-1,-1);
      glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+0+0+bias);
      glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+0+1+bias);
      glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+0+0+bias);
      glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+0+1+bias);
      glVertex2f( 1,-1);
      glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+resolution_y+0+bias);
      glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+resolution_y+1+bias);
      glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+resolution_y+0+bias);
      glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+resolution_y+1+bias);
      glVertex2f( 1, 1);
      glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+resolution_y+0+bias);
      glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+resolution_y+1+bias);
      glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+resolution_y+0+bias);
      glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+resolution_y+1+bias);
      glVertex2f(-1, 1);
    glEnd();

    resolution_x /= 2;
    resolution_y /= 2;
    swap = !swap;
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(64,64,64,64,GL_RGBA);
  exit(1);
#endif

  cgGLDisableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
For every ray to be traced we need to know START and END point in world space coordinates.
We render whole scene geometry two times into two buffers.
In first buffer special fragment program writes ray starts.
In second buffer special fragment program writes ray ends (using ray reflection formula for surface point).
*/
void RaytracerRForce::GenerateRays()
{
  // setup scene rendering
  PrepareSceneProjection();
  BindSceneBuffers();

  SetActiveKernel(RTRF_KERNEL_RAYGENERATOR);

  glViewport(RTRF_BUFFER_RAYSTART_PX, RTRF_BUFFER_RAYSTART_PY, RTRF_BUFFER_RAYSTART_SX, RTRF_BUFFER_RAYSTART_SY);

  // output rays starts
  ActiveKernel()->SetMode(KERNEL_RAYGENERATOR_MODE_RAYSTARTS);
  ActiveKernel()->Activate();
  RenderScene(GL_TRIANGLES);
  ActiveKernel()->Deactivate();

  glViewport(RTRF_BUFFER_RAYDIRECTION_PX, RTRF_BUFFER_RAYDIRECTION_PY, RTRF_BUFFER_RAYDIRECTION_SX, RTRF_BUFFER_RAYDIRECTION_SY);

  // output rays ends
  ActiveKernel()->SetMode(KERNEL_RAYGENERATOR_MODE_RAYENDS);
  ActiveKernel()->Activate();
  RenderScene(GL_TRIANGLES);
  ActiveKernel()->Deactivate();

  // cleanup scene rendering
  UnbindSceneBuffers();
}
/*!
  Shading kernel is responsible for shading of incomming hits.
  There can be more versions of this shader (per vertex or per pixel, with or without lights etc.)
*/
void RaytracerRForce::Shade()
{
  CGprogram fp;
  CGparameter param;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  SetActiveKernel(RTRF_KERNEL_SHADER);
  ActiveKernel()->SetMode(KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY);
  ActiveKernel()->Activate();

  fp = ActiveKernel()->GetFragmentProgram(KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY);

  m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->BindBuffer(WGL_BACK_LEFT_ARB);

  param = cgGetNamedParameter(fp, "rays");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
    cgGLEnableTextureParameter(param);
  }

  param = cgGetNamedParameter(fp, "basic");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
    cgGLEnableTextureParameter(param);
  }

  param = cgGetNamedParameter(fp, "colors");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_TRIS_COLOR]->GetId());
    cgGLEnableTextureParameter(param);
  }

  param = cgGetNamedParameter(fp, "material");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_MATERIAL]->GetId());
    cgGLEnableTextureParameter(param);
  }

  param = cgGetNamedParameter(fp, "uvinfo");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_TRIS_UVRR]->GetId());
    cgGLEnableTextureParameter(param);
  }

  param = cgGetNamedParameter(fp, "tridata");
  if (param)
  {
    cgGLSetTextureParameter(param, m_kPBufferManager[RTRF_PBUFFER_INTERSECTING]->GetTextureID());
    cgGLEnableTextureParameter(param);
  }

  ParamsCache* pkFPCache = ActiveKernel()->GetFragmentProgramParamsCache();
  if (!pkFPCache) return; // safety net

  SetCommonSceneParams(pkFPCache); // light configuration

  glViewport(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY); 

  float B = 0;
  float S0X =               0 + 0;
  float S0Y =               0 + 0;
  float S1X = RTRF_VIEWPORT_SX + 0;
  float S1Y =               0 + 0;
  float D0X = RTRF_VIEWPORT_SX;
  float D0Y = RTRF_VIEWPORT_SY;
  float D1X = RTRF_VIEWPORT_SX;
  float D1Y = RTRF_VIEWPORT_SY;

  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B,   0+S1Y+B);
  glVertex2f(-1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B,   0+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B,   0+S1Y+B);
  glVertex2f( 1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1, D1X+S1X+B, D1Y+S1Y+B);
  glVertex2f( 1, 1);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B, D0Y+S0Y+B);
  glMultiTexCoord2f(GL_TEXTURE1,   0+S1X+B, D1Y+S1Y+B);
  glVertex2f(-1, 1);
  glEnd();

  param = cgGetNamedParameter(fp, "rays");
  if (param) cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "basic");
  if (param) cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "colors");
  if (param) cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "material");
  if (param) cgGLDisableTextureParameter(param);
  param = cgGetNamedParameter(fp, "uvinfo");
  if (param) cgGLDisableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
This routine is responsible for composition of final shaded hits with intermediate image.
Blending eqaution is assumed to be set outside of this function.
This function usually does stretching and filtering of shaded hits texture because
due to performance reasons we raytrace in lower resolution than is final image.
*/
void RaytracerRForce::RenderFinal()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight); 

  glEnable(GL_TEXTURE_RECTANGLE_NV);
  m_kPBufferManager[RTRF_PBUFFER_SHADING]->BindBuffer(WGL_FRONT_LEFT_ARB);
  m_kOptions.ApplyImageFilter();

  float B = 0;
  float S0X =               0 + 0;
  float S0Y =               0 + 0;
  float D0X = RTRF_VIEWPORT_SX;
  float D0Y = RTRF_VIEWPORT_SY;

  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B,   0+S0Y+B);
  glVertex2f(-1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B,   0+S0Y+B);
  glVertex2f( 1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, D0X+S0X+B, D0Y+S0Y+B);
  glVertex2f( 1, 1);
  glMultiTexCoord2f(GL_TEXTURE0,   0+S0X+B, D0Y+S0Y+B);
  glVertex2f(-1, 1);
  glEnd();

  glDisable(GL_TEXTURE_RECTANGLE_NV);
}

/*!
This function sends scene geometry trough special set of shaders.
Shader transforms points into world space and stores their positions in texture conseqentialy. 
Similary are handled normals -> world space normals.
We get three texture for vertices and three textures of normals.
They correspond to triangle points A,B and C respectively.
These texture is then used by raytracer for queries on scene geomtery.
THIS MAKES DYNAMIC RAYTRACING POSSIBLE
*/
void RaytracerRForce::GenerateTransformedGeometry()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  SetActiveKernel(RTRF_KERNEL_TRANSFORMER);

  // transform positions
  ActiveKernel()->SetMode(KERNEL_TRANSFORMER_MODE_POSITIONS);
  ActiveKernel()->Activate();

  glViewport(RTRF_BUFFER_TRANSFORMEDAREA_PX,RTRF_BUFFER_TRANSFORMEDAREA_PY,RTRF_BUFFER_TRANSFORMEDAREA_SX,RTRF_BUFFER_TRANSFORMEDAREA_HALF_SY); 

  BindSceneBuffers();
  RenderScene(GL_POINTS);
  UnbindSceneBuffers();

  ActiveKernel()->Deactivate();

  // transform normals
  ActiveKernel()->SetMode(KERNEL_TRANSFORMER_MODE_NORMALS);
  ActiveKernel()->Activate();

  glViewport(RTRF_BUFFER_TRANSFORMEDAREA_PX,RTRF_BUFFER_TRANSFORMEDAREA_PY+RTRF_BUFFER_TRANSFORMEDAREA_HALF_SY,RTRF_BUFFER_TRANSFORMEDAREA_SX,RTRF_BUFFER_TRANSFORMEDAREA_HALF_SY); 

  BindSceneBuffers();
  RenderScene(GL_POINTS);
  UnbindSceneBuffers();

  ActiveKernel()->Deactivate();
}

/*!
This is dynamic computation of bounding volumes. We use AABB boxes.
We render set of quads for reduction on world-space data generated by RaytracerRForce::GenerateTransformedGeometry().

Element is 8x8 so we need
3 reduction steps + 1 extra step are done for obtaining box MAX for every element.
3 reduction steps + 1 extra step are done for obtaining box MIN for every element.

These bounding box data are left as textures in intersection pbuffer.
*/
void RaytracerRForce::BuildBoundingVolumes()
{
  CGprogram fp; 
  CGparameter param;

  int S0X;
  int S0Y;
  float bias;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  SetActiveKernel(RTRF_KERNEL_BVBUILDER);

  // pass data as texture parameter
  fp = ActiveKernel()->GetFragmentProgram(KERNEL_BVBUILDER_MODE_MINS);
  param = cgGetNamedParameter(fp, "data0");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data1");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data2");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data3");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);

  fp = ActiveKernel()->GetFragmentProgram(KERNEL_BVBUILDER_MODE_MAXS);
  param = cgGetNamedParameter(fp, "data0");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data1");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data2");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);
  param = cgGetNamedParameter(fp, "data3");
  cgGLSetTextureParameter(param, ActivePBuffer()->GetTextureID());
  cgGLEnableTextureParameter(param);

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX,RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY-RTRF_BUFFER_TRANSFORMEDAREA_SY,RTRF_TRIMAP_SX,6*RTRF_TRIMAP_SX,GL_RGBA);
  // exit(1);
#endif

  bool swap = true;
  int resolution_x = RTRF_TRIMAP_SX;
  int resolution_y = 3*RTRF_TRIMAP_SY;
  for (int k=0; k<3; k++)
  {
    if (swap)
    {
      glDrawBuffer(GL_FRONT);
      ActivePBuffer()->BindBuffer(WGL_BACK_LEFT_ARB);
    }
    else
    {
      glDrawBuffer(GL_BACK);
      ActivePBuffer()->BindBuffer(WGL_FRONT_LEFT_ARB);
    }

    int half_resolution_x = resolution_x/2;
    int half_resolution_y = resolution_y/2;
    bias = -1.0;

    float SHY;
    
    ActiveKernel()->SetMode(KERNEL_BVBUILDER_MODE_MINS);
    ActiveKernel()->Activate();

    S0X = RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX;
    S0Y = RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY;

    SHY = 0;
    if (k==0)
    {
      SHY = -RTRF_BUFFER_TRANSFORMEDAREA_SY;
    }

    glViewport(S0X, S0Y, half_resolution_x, half_resolution_y);
    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+0+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+0+1+bias+SHY);
    glVertex2f(-1,-1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+0+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+0+1+bias+SHY);
    glVertex2f( 1,-1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+resolution_y+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+resolution_y+1+bias+SHY);
    glVertex2f( 1, 1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+resolution_y+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+resolution_y+1+bias+SHY);
    glVertex2f(-1, 1);
    glEnd();

    ActiveKernel()->SetMode(KERNEL_BVBUILDER_MODE_MAXS);
    ActiveKernel()->Activate();

    S0X = RTRF_BUFFER_BOUNDINGVOLUMESMAX_PX;
    S0Y = RTRF_BUFFER_BOUNDINGVOLUMESMAX_PY;

    SHY = 0;
    if (k==0)
    {
      SHY = - RTRF_BUFFER_TRANSFORMEDAREA_SY - 3*RTRF_TRIMAP_SY;
    }

    glViewport(S0X, S0Y, half_resolution_x, half_resolution_y);
    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+0+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+0+1+bias+SHY);
    glVertex2f(-1,-1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+0+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+0+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+0+1+bias+SHY);
    glVertex2f( 1,-1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+resolution_x+0+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+resolution_x+0+bias, S0Y+resolution_y+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+resolution_x+1+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+resolution_x+1+bias, S0Y+resolution_y+1+bias+SHY);
    glVertex2f( 1, 1);
    glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+resolution_y+1+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE2, S0X+0+1+bias, S0Y+resolution_y+0+bias+SHY);
    glMultiTexCoord2f(GL_TEXTURE3, S0X+0+1+bias, S0Y+resolution_y+1+bias+SHY);
    glVertex2f(-1, 1);
    glEnd();

#if 0
    // debug
    if (swap)
    {
      glReadBuffer(GL_FRONT);
    }
    else
    {
      glReadBuffer(GL_BACK);
    }
    imdebugPixelsf(RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX,RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY,RTRF_TRIMAP_SX,6*RTRF_TRIMAP_SX,GL_RGBA);
    //exit(1);
#endif

    resolution_x /= 2;
    resolution_y /= 2;
    swap = !swap;
  }

  glDrawBuffer(GL_BACK);
  ActivePBuffer()->BindBuffer(WGL_FRONT_LEFT_ARB);

  ActiveKernel()->SetMode(KERNEL_BVBUILDER_MODE_MINS);
  ActiveKernel()->Activate();

  S0X = RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX;
  S0Y = RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY;
  bias = 0; //-1.0;

  glViewport(S0X, S0Y, RTRF_BVMAP_SX, RTRF_BVMAP_SY);
  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+0+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+0+0+bias, S0Y+0+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+0+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glVertex2f(-1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glVertex2f( 1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glVertex2f( 1, 1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glVertex2f(-1, 1);
  glEnd();

  ActiveKernel()->SetMode(KERNEL_BVBUILDER_MODE_MAXS);
  ActiveKernel()->Activate();

  S0X = RTRF_BUFFER_BOUNDINGVOLUMESMAX_PX;
  S0Y = RTRF_BUFFER_BOUNDINGVOLUMESMAX_PY;
  bias = 0; //-1.0;

  glViewport(S0X, S0Y, RTRF_BVMAP_SX, RTRF_BVMAP_SY);
  glBegin(GL_QUADS);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+0+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+0+0+bias, S0Y+0+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+0+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glVertex2f(-1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+RTRF_BVMAP_SX+0+bias, S0Y+0+0*RTRF_BVMAP_SY+bias);
  glVertex2f( 1,-1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+RTRF_BVMAP_SX+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glVertex2f( 1, 1);
  glMultiTexCoord2f(GL_TEXTURE0, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE1, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+1*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE2, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+2*RTRF_BVMAP_SY+bias);
  glMultiTexCoord2f(GL_TEXTURE3, S0X+0+0+bias, S0Y+RTRF_BVMAP_SY+0*RTRF_BVMAP_SY+bias);
  glVertex2f(-1, 1);
  glEnd();

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX,RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY,RTRF_TRIMAP_SX,6*RTRF_TRIMAP_SX,GL_RGBA);
  exit(1);
#endif

#if 0
  // debug - check results with CPU path
  glReadBuffer(GL_BACK);
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, m_kTextureManager[RTRF_TEXTURE_BOX_MIN]->GetId(), GL_RGBA, 0, NULL);
  imdebugPixelsf(RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX,RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY,RTRF_BUFFER_BOUNDINGVOLUMESMIN_SX,RTRF_BUFFER_BOUNDINGVOLUMESMIN_SY,GL_RGBA);
  imdebugTexImagef(GL_TEXTURE_RECTANGLE_NV, m_kTextureManager[RTRF_TEXTURE_BOX_MAX]->GetId(), GL_RGBA, 0, NULL);
  imdebugPixelsf(RTRF_BUFFER_BOUNDINGVOLUMESMAX_PX,RTRF_BUFFER_BOUNDINGVOLUMESMAX_PY,RTRF_BUFFER_BOUNDINGVOLUMESMAX_SX,RTRF_BUFFER_BOUNDINGVOLUMESMAX_SY,GL_RGBA);
  exit(1);
#endif

  cgGLDisableTextureParameter(param);

  ActiveKernel()->Deactivate();
}

/*!
Helper function for bounding box visualization.
*/
void 
RaytracerRForce::RenderBoundingBox(Vector3f kMin, Vector3f kMax)
{
  Vector3f A(kMin[0], kMin[1], kMin[2]);
  Vector3f B(kMax[0], kMin[1], kMin[2]);
  Vector3f C(kMax[0], kMax[1], kMin[2]);
  Vector3f D(kMin[0], kMax[1], kMin[2]);

  Vector3f E(kMin[0], kMin[1], kMax[2]);
  Vector3f F(kMax[0], kMin[1], kMax[2]);
  Vector3f G(kMax[0], kMax[1], kMax[2]);
  Vector3f H(kMin[0], kMax[1], kMax[2]);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 

  glBegin(GL_QUADS);
    glVertex3fv(D);
    glVertex3fv(C);
    glVertex3fv(B);
    glVertex3fv(A);
    
    glVertex3fv(E);
    glVertex3fv(F);
    glVertex3fv(G);
    glVertex3fv(H);

    glVertex3fv(A);
    glVertex3fv(B);
    glVertex3fv(F);
    glVertex3fv(E);

    glVertex3fv(C);
    glVertex3fv(D);
    glVertex3fv(H);
    glVertex3fv(G);

    glVertex3fv(B);
    glVertex3fv(C);
    glVertex3fv(G);
    glVertex3fv(F);

    glVertex3fv(D);
    glVertex3fv(A);
    glVertex3fv(E);
    glVertex3fv(H);
  glEnd();
}

void 
RaytracerRForce::RenderBoundingBoxes()
{
  m_kOptions.ApplyBoundingBoxesColor();

  int index = 0;
  for (int i=0; i<RTRF_BVMAP_SIZE; i++)
  {
    float* pMin = &m_afBoundingBoxesMins[index];
    float* pMax = &m_afBoundingBoxesMaxs[index];

    RenderBoundingBox(Vector3f(pMin[0], pMin[1], pMin[2]), Vector3f(pMax[0], pMax[1], pMax[2]));

    index += 3;
  }
}

void 
RaytracerRForce::DisplayBoundingBoxes()
{
  CGprogram kVertexProgram;
  CGparameter kModelViewProjParam;

  PrepareSceneProjection();
  glMatrixMode(GL_MODELVIEW);

  SetActiveKernel(RTRF_KERNEL_SHADER);
  ActiveKernel()->SetMode(KERNEL_SHADER_MODE_BOUNDINGBOXES);

  kVertexProgram = ActiveKernel()->GetVertexProgram(KERNEL_SHADER_MODE_BOUNDINGBOXES);
  kModelViewProjParam = cgGetNamedParameter(kVertexProgram, "ModelViewProj");
  cgGLSetStateMatrixParameter(kModelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX , CG_GL_MATRIX_IDENTITY);

  ActiveKernel()->Activate();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  RenderBoundingBoxes();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  ActiveKernel()->Deactivate();
}

void 
RaytracerRForce::DisplaySceneGizmos()
{
  CGprogram kVertexProgram;
  CGparameter kModelViewProjParam;

  glLineWidth(1.0f);

  // light gizmos
  int aLight = GetActiveLight();
  int iCounter = 0;
  TSimpleLightContainer::iterator iL = m_pkSimpleScene->m_kLights.begin();
  while (iL!=m_pkSimpleScene->m_kLights.end())
  {
    Vector3f& kP = (*iL).m_kPosition;

    if (aLight == iCounter)
      glColor3f(1.0,1.0,0.0);
    else
      glColor3f(1.0,1.0,1.0);

    glPushMatrix();
    glMultMatrixf( (*iL).m_kManipulator.get_mat().mat_array );

    kVertexProgram = ActiveKernel()->GetVertexProgram(KERNEL_SHADER_MODE_BOUNDINGBOXES);
    kModelViewProjParam = cgGetNamedParameter(kVertexProgram, "ModelViewProj");
    cgGLSetStateMatrixParameter(kModelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX , CG_GL_MATRIX_IDENTITY);
    DisplayLightGizmo();
    glPopMatrix();

    iCounter++;
    iL++;
  }

  // element gizmos
  int aElement = GetActiveElement();
  iCounter = 0;
  TSimpleElementContainer::iterator iE = m_pkSimpleScene->m_kElements.begin();
  while (iE!=m_pkSimpleScene->m_kElements.end())
  {
    if (aElement == iCounter)
    {
      glColor3f(0.0,1.0,0.0);
      glPushMatrix();
      glMultMatrixf((*iE).m_kManipulator.get_mat().mat_array);
      kVertexProgram = ActiveKernel()->GetVertexProgram(KERNEL_SHADER_MODE_BOUNDINGBOXES);
      kModelViewProjParam = cgGetNamedParameter(kVertexProgram, "ModelViewProj");
      cgGLSetStateMatrixParameter(kModelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX , CG_GL_MATRIX_IDENTITY);
      DisplayLightGizmo();
      glPopMatrix();
    }

    iCounter++;
    iE++;
  }

}

void 
RaytracerRForce::DisplayGizmos()
{
  glMatrixMode(GL_MODELVIEW);

  SetActiveKernel(RTRF_KERNEL_SHADER);
  ActiveKernel()->SetMode(KERNEL_SHADER_MODE_BOUNDINGBOXES);

  ActiveKernel()->Activate();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  DisplaySceneGizmos();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  ActiveKernel()->Deactivate();
}

void 
RaytracerRForce::ReadbackBoundingBoxes()
{
  glReadBuffer(GL_BACK);
  glReadPixels(RTRF_BUFFER_BOUNDINGVOLUMESMIN_PX, RTRF_BUFFER_BOUNDINGVOLUMESMIN_PY, RTRF_BUFFER_BOUNDINGVOLUMESMIN_SX, RTRF_BUFFER_BOUNDINGVOLUMESMIN_SY, GL_RGB, GL_FLOAT, m_afBoundingBoxesMins);
  glReadPixels(RTRF_BUFFER_BOUNDINGVOLUMESMAX_PX, RTRF_BUFFER_BOUNDINGVOLUMESMAX_PY, RTRF_BUFFER_BOUNDINGVOLUMESMAX_SX, RTRF_BUFFER_BOUNDINGVOLUMESMAX_SY, GL_RGB, GL_FLOAT, m_afBoundingBoxesMaxs);
}

/*!
Primary hits are produced by classical rasterization with custom shading.
*/
void 
RaytracerRForce::RenderPrimary()
{
  CGprogram fp;
  CGparameter param;

  // setup scene for rendering
  PrepareSceneProjection();
  BindSceneBuffers();

  // setup kernel
  SetActiveKernel(RTRF_KERNEL_SHADER);
  int iActiveShader = KERNEL_SHADER_MODE_BASIC_FIXED_FLAT + m_kOptions.m_iRenderMode;
  ActiveKernel()->SetMode(iActiveShader);

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight);

  fp = ActiveKernel()->GetFragmentProgram(iActiveShader);
  param = cgGetNamedParameter(fp, "material");
  cgGLSetTextureParameter(param, m_kTextureManager[RTRF_TEXTURE_MATERIAL]->GetId());
  cgGLEnableTextureParameter(param);

  if (m_kOptions.m_iRenderMode==KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING)
    glEnable(GL_TEXTURE_2D);

  // render scene with custom SHADER
  ActiveKernel()->Activate();
  RenderScene(GL_TRIANGLES);
  ActiveKernel()->Deactivate();

  glDisable(GL_TEXTURE_2D);

  param = cgGetNamedParameter(fp, "material");
  cgGLDisableTextureParameter(param);

  // scene rendering cleanup
  UnbindSceneBuffers();
}

/*!
Brute force raytracing does not use bouding volumes to reject some ray-triangle tests.
We save on pbuffer switch but overhead of brute force solution is too high.
Method implemented for comparison with NV38 and NV40 paths.

Code takes these steps:
- for every segment test and reduce hits
*/
void 
RaytracerRForce::TracePassBruteForce()
{
  // assume INTERSECTING pbuffer is active

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  exit(1);
#endif

  for (int j=0; j<RTRF_SEGMENTS_Y; j++)
  {
    for (int i=0; i<RTRF_SEGMENTS_X; i++)
    {
      //i = 1; j = 1; // debug
      int iSegment = i + j * RTRF_SEGMENTS_X;

      glDrawBuffer(GL_FRONT);

      // initialize depth buffer - needed this reset for early-z
      glScissor(i*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), j*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);
      glEnable(GL_SCISSOR_TEST);
      SetupColorControl(true);
      glClearColor(CG_FLT_MAX, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      glDisable(GL_SCISSOR_TEST);

      // brute force testing
      SetupColorControl(true);
      SetupDepthControl(false, false);
      TestHits(i,j);

#if 0
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif

      SetupColorControl(true);
      SetupDepthControl(false, false);
      ReduceHits(i,j); // segment independent

#if 0
      // debug
      glReadBuffer(GL_BACK);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif
      // result is in BACK
    }
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  //exit(1);
#endif
}


/*!
Raytracing uses bouding volumes to reject some ray-triangle tests.
Rejection is performed by early-z test. 

We need to setup z-buffer special way to enable early-z culling.
We read computation mask produced by RaytracerRForce::TestBoundingVolumes() into PBO.
We rebind this PBO as VBO and uses these data as z-value for GL_POINTS.
Rendering points forces updates in z-buffer.

Code takes these steps:
- produce computation mask in RTRF_PBUFFER_BOUNDTESTING
- readback mask as PBO
- switch to RTRF_PBUFFER_INTERSECTING
- for every segment
-- clear z-buffer
-- mark mask using GL_POINTS and VBO
-- test hits
-- reduce hits
*/
void 
RaytracerRForce::TracePassNV38()
{
  /////////////////////////////////
  // switch to BOUNDTESTING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_BOUNDTESTING);

  glDrawBuffer(GL_FRONT);
  SetupColorControl(true);
  SetupDepthControl(false, false);
  TestBoundingVolumes();
  ReadTestDataAsPBO();

#if SHOW_STRUCTURE
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,512,512,GL_LUMINANCE);
  //exit(1);
#endif

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,512,512,GL_LUMINANCE);
  exit(1);
#endif

  /////////////////////////////////
  // switch to INTERSECTING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  exit(1);
#endif

  for (int j=0; j<RTRF_SEGMENTS_Y; j++)
  {
    for (int i=0; i<RTRF_SEGMENTS_X; i++)
    {
      //i = 1; j = 1; // debug
      int iSegment = i + j * RTRF_SEGMENTS_X;

      glDrawBuffer(GL_FRONT);

      // initialize depth buffer - needed this reset for early-z
      glScissor(i*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), j*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);
      glEnable(GL_SCISSOR_TEST);
      SetupColorControl(true);
      glClearColor(CG_FLT_MAX, 0, 0, 0);
      SetupDepthControl(false, true);
      glClearDepth(1); // THIS MUST BE HERE FOR EARLY-Z
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glDisable(GL_SCISSOR_TEST);

      SetupDepthControl(false, true);
      SetupColorControl(false);
      MarkHitSpacePoints(i,j);

#if 0
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_DEPTH_COMPONENT);
      //exit(1);
#endif

      SetupColorControl(true);
      SetupDepthControl(true, false, GL_LEQUAL);
      TestHits(i,j);

#if SHOW_STRUCTURE
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      //exit(1);
#endif

#if 0
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif

      SetupColorControl(true);
      SetupDepthControl(false, false);
      ReduceHits(i,j); // segment independent

#if 0
      // debug
      glReadBuffer(GL_BACK);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif
      // result is in BACK
    }
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  //exit(1);
#endif
}

/*!
Raytracing uses bouding volumes to reject some ray-triangle tests.
Rejection is performed by early-z test. 

We setup z-buffer using fragment program with conditional discard.
We read computation mask produced by RaytracerRForce::TestBoundingVolumes() as texture to evaluate conditional.
Rendering quad forces updates in z-buffer.

Code takes these steps:
- produce computation mask in RTRF_PBUFFER_BOUNDTESTING
- switch to RTRF_PBUFFER_INTERSECTING
- for every segment
-- clear z-buffer
-- produce computation mask
-- test hits
-- reduce hits
*/
void 
RaytracerRForce::TracePassNV40()
{
  /////////////////////////////////
  // switch to BOUNDTESTING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_BOUNDTESTING);

  glDrawBuffer(GL_FRONT);
  SetupColorControl(true);
  SetupDepthControl(false, false);
  TestBoundingVolumes();

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,512,512,GL_LUMINANCE);
  //exit(1);
#endif

  /////////////////////////////////
  // switch to INTERSECTING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  exit(1);
#endif

  for (int j=0; j<RTRF_SEGMENTS_Y; j++)
  {
    for (int i=0; i<RTRF_SEGMENTS_X; i++)
    {
      int iSegment = i + j * RTRF_SEGMENTS_X;

      glDrawBuffer(GL_FRONT);

      // initialize depth buffer - needed this reset for early-z
      glScissor(i*(RTRF_VIEWPORT_SX/RTRF_SEGMENTS_X), j*(RTRF_VIEWPORT_SY/RTRF_SEGMENTS_Y), RTRF_SEGMENT_SX, RTRF_SEGMENT_SY);
      glEnable(GL_SCISSOR_TEST);
      SetupColorControl(true);
      glClearColor(CG_FLT_MAX, 0, 0, 0);
      SetupDepthControl(false, true);
      glClearDepth(1); // THIS MUST BE HERE FOR EARLY-Z
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glDisable(GL_SCISSOR_TEST);

      SetupDepthControl(false, true);
      SetupColorControl(false);
      ComputationMask(i,j);

#if 0
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_DEPTH_COMPONENT);
      exit(1);
#endif

      SetupColorControl(true);
      SetupDepthControl(true, false, GL_LEQUAL);
      TestHits(i,j);

#if 0
      // debug
      glReadBuffer(GL_FRONT);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif

      SetupColorControl(true);
      SetupDepthControl(false, false);
      ReduceHits(i,j); // segment independent

#if 0
      // debug
      glReadBuffer(GL_BACK);
      imdebugPixelsf(0,0,2048,2048,GL_RGBA);
      exit(1);
#endif
      // result is in BACK
    }
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,2048,2048,GL_RGBA);
  //exit(1);
#endif
}

/*!
<b>Scheme A</b>
Basic scheme renders whole scene using classic OpenGL with custom shading.

Code takes these steps:
- clear buffers
- render scene
- display gizmos
*/
bool 
RaytracerRForce::RenderA()
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);

  SetupColorControl(true);
  glClearColor(0, 0, 0, 1);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  SetupSceneLighting();
  RenderPrimary();

  SetupColorControl(true);
  SetupDepthControl(false, false);
  DisplayGizmos();

  CHECK_OPENGL_ERROR;

  return true;
}

/*!
<b>Scheme B</b>
Scheme renders whole scene using classic OpenGL with custom shading.
Plus builds data structures in the GPU.
Data structures are
- world space vertex positions.
- world space normals
- bounding boxes

Code takes these steps:
- clear buffers
- render scene
- switch to RTRF_PBUFFER_INTERSECTING
- clear buffers
- transform geometry
- build bounding volumes
- switch back to main context
- display bounding boxes if enabled
- display gizmos
*/
bool 
RaytracerRForce::RenderB()
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);

  SetupColorControl(true);
  glClearColor(0, 0, 0, 1);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupDepthControl(true, true, GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 
  SetupSceneLighting();
  RenderPrimary();

  /////////////////////////////////
  // switch to INTERSECTING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

  glDrawBuffer(GL_BACK);
  glScissor(RTRF_WORKSPACE_SX, 0, RTRF_VIEWPORT_SX, RTRF_WORKSPACE_SY);
  glEnable(GL_SCISSOR_TEST);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(false, false);
  GenerateTransformedGeometry();
  BuildBoundingVolumes();

  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // readback boundingboxes data
    ReadbackBoundingBoxes();
  }

  /////////////////////////////////
  // switch to MAIN context

  SwitchToPBuffer(RTRF_PBUFFER_NONE);

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight);
  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // display bounding box
    SetupColorControl(true);
    SetupDepthControl(true, false, GL_LEQUAL);
    DisplayBoundingBoxes();
  }

  SetupColorControl(true);
  SetupDepthControl(false, false);
  DisplayGizmos();

  CHECK_OPENGL_ERROR;

  return true;
}

/*!
<b>Scheme C</b>
Performs scheme B. Plus traces bucket of reflection rays using brute force path.

Code takes these steps:
- clear buffers
- render scene (=primary rays)
- switch to RTRF_PBUFFER_INTERSECTING
- clear buffers
- generate rays
- transform geometry
- build bounding volumes
- TRACE RAYS using BRUTE FORCE method
- switch to shading context
- perform shading of incomming hits
- switch back to main context
- do final image composition
- display bounding boxes if enabled
- display gizmos
*/
bool 
RaytracerRForce::RenderC()
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 
  SetupSceneLighting();
  RenderPrimary();
  DisableSceneLighting();

  /////////////////////////////////
  // switch to INTERSECTING PBuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

  glDrawBuffer(GL_BACK);
  glScissor(RTRF_WORKSPACE_SX, 0, RTRF_VIEWPORT_SX, RTRF_WORKSPACE_SY);
  glEnable(GL_SCISSOR_TEST);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  GenerateRays();

  SetupColorControl(true);
  SetupDepthControl(false, false);
  GenerateTransformedGeometry();
  BuildBoundingVolumes();

  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // readback boundingboxes data
    ReadbackBoundingBoxes();
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  //imdebugPixelsf(RTRF_WORKSPACE_SX,0,128,512,GL_RGBA);
  imdebugPixelsf(RTRF_WORKSPACE_SX,0,64,64,GL_RGBA);
  //exit(1);
#endif

  TracePassBruteForce();

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGBA);
  //exit(1);
#endif

  /////////////////////////////////
  // switch to SHADING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_SHADING);

  glDrawBuffer(GL_FRONT);
  SetupColorControl(true);
  SetupDepthControl(false, false);
  Shade();

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixels(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGB);
  exit(1);
#endif

  /////////////////////////////////
  // switch to MAIN context

  SwitchToPBuffer(RTRF_PBUFFER_NONE);

  SetupColorControl(true);
  SetupDepthControl(false, false);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  RenderFinal();
  glDisable(GL_BLEND);

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight);
  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // display bounding box
    SetupColorControl(true);
    SetupDepthControl(true, false, GL_LEQUAL);
    DisplayBoundingBoxes();
  }

  SetupColorControl(true);
  SetupDepthControl(false, false);
  PrepareSceneProjection();
  DisplayGizmos();

  CHECK_OPENGL_ERROR;

  return true;
}

/*!
<b>Scheme D</b>
Performs scheme B. Plus traces bucket of reflection rays using NV38 path.

Code takes these steps:
- clear buffers
- render scene (=primary rays)
- switch to RTRF_PBUFFER_INTERSECTING
- clear buffers
- generate rays
- transform geometry
- build bounding volumes
- TRACE RAYS using NV38 method
- switch to shading context
- perform shading of incomming hits
- switch back to main context
- do final image composition
- display bounding boxes if enabled
- display gizmos
*/
bool 
RaytracerRForce::RenderD()
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 
  SetupSceneLighting();
  RenderPrimary();
  DisableSceneLighting();

#if SHOW_STRUCTURE
  // show structure
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,512,512,GL_RGBA);
  //exit(1);
#endif

  /////////////////////////////////
  // switch to INTERSECTING PBuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

  glDrawBuffer(GL_BACK);
  glScissor(RTRF_WORKSPACE_SX, 0, RTRF_VIEWPORT_SX, RTRF_WORKSPACE_SY);
  glEnable(GL_SCISSOR_TEST);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  GenerateRays();

#if SHOW_STRUCTURE
  // show structure
  glReadBuffer(GL_BACK);
  imdebugPixelsf(RTRF_WORKSPACE_SX,0,128,128,GL_RGBA);
  imdebugPixelsf(RTRF_WORKSPACE_SX,128,128,128,GL_RGBA);
  //exit(1);
#endif

  SetupColorControl(true);
  SetupDepthControl(false, false);
  GenerateTransformedGeometry();
  BuildBoundingVolumes();

#if SHOW_STRUCTURE
  // show structure
  glReadBuffer(GL_BACK);
  imdebugPixelsf(RTRF_WORKSPACE_SX,256,32,256,GL_RGBA);
  //exit(1);
#endif

  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // readback boundingboxes data
    ReadbackBoundingBoxes();
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  //imdebugPixelsf(RTRF_WORKSPACE_SX,0,128,512,GL_RGBA);
  imdebugPixelsf(RTRF_WORKSPACE_SX,0,64,64,GL_RGBA);
  //exit(1);
#endif

  TracePassNV38();

#if SHOW_STRUCTURE
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGBA);
  //exit(1);
#endif

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGBA);
  exit(1);
#endif

  /////////////////////////////////
  // switch to SHADING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_SHADING);

  glDrawBuffer(GL_FRONT);
  SetupColorControl(true);
  SetupDepthControl(false, false);
  Shade();

#if SHOW_STRUCTURE
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixels(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGB);
  //exit(1);
#endif

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixels(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGB);
  exit(1);
#endif

  /////////////////////////////////
  // switch to MAIN context

  SwitchToPBuffer(RTRF_PBUFFER_NONE);

  SetupColorControl(true);
  SetupDepthControl(false, false);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  RenderFinal();
  glDisable(GL_BLEND);

#if SHOW_STRUCTURE
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixels(0,0,512,512,GL_RGB);
  exit(1);
#endif

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight);
  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // display bounding box
    SetupColorControl(true);
    SetupDepthControl(true, false, GL_LEQUAL);
    DisplayBoundingBoxes();
  }

  SetupColorControl(true);
  SetupDepthControl(false, false);
  PrepareSceneProjection();
  DisplayGizmos();

  CHECK_OPENGL_ERROR;

  return true;
}

/*!
<b>Scheme E</b>
Performs scheme B. Plus traces bucket of reflection rays using NV40 path.

Code takes these steps:
- clear buffers
- render scene (=primary rays)
- switch to RTRF_PBUFFER_INTERSECTING
- clear buffers
- generate rays
- transform geometry
- build bounding volumes
- TRACE RAYS using NV40 method
- switch to shading context
- perform shading of incomming hits
- switch back to main context
- do final image composition
- display bounding boxes if enabled
- display gizmos
*/
bool 
RaytracerRForce::RenderE()
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 
  SetupSceneLighting();
  RenderPrimary();
  DisableSceneLighting();

  /////////////////////////////////
  // switch to INTERSECTING PBuffer

  SwitchToPBuffer(RTRF_PBUFFER_INTERSECTING);

  glDrawBuffer(GL_BACK);
  glScissor(RTRF_WORKSPACE_SX, 0, RTRF_VIEWPORT_SX, RTRF_WORKSPACE_SY);
  glEnable(GL_SCISSOR_TEST);
  SetupColorControl(true);
  glClearColor(0, 0, 0, 0);
  SetupDepthControl(false, true);
  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetupColorControl(true);
  SetupDepthControl(true, true, GL_LEQUAL);
  GenerateRays();

  SetupColorControl(true);
  SetupDepthControl(false, false);
  GenerateTransformedGeometry();
  BuildBoundingVolumes();

  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // readback boundingboxes data
    ReadbackBoundingBoxes();
  }

#if 0
  // debug
  glReadBuffer(GL_BACK);
  //imdebugPixelsf(RTRF_WORKSPACE_SX,0,128,512,GL_RGBA);
  imdebugPixelsf(RTRF_WORKSPACE_SX,0,64,64,GL_RGBA);
  //exit(1);
#endif

  TracePassNV40();

#if 0
  // debug
  glReadBuffer(GL_BACK);
  imdebugPixelsf(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGBA);
  //exit(1);
#endif

  /////////////////////////////////
  // switch to SHADING pbuffer

  SwitchToPBuffer(RTRF_PBUFFER_SHADING);

  glDrawBuffer(GL_FRONT);
  SetupColorControl(true);
  SetupDepthControl(false, false);
  Shade();

#if 0
  // debug
  glReadBuffer(GL_FRONT);
  imdebugPixels(0,0,RTRF_VIEWPORT_SX,RTRF_VIEWPORT_SY,GL_RGB);
  exit(1);
#endif

  /////////////////////////////////
  // switch to MAIN context

  SwitchToPBuffer(RTRF_PBUFFER_NONE);

  SetupColorControl(true);
  SetupDepthControl(false, false);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  RenderFinal();
  glDisable(GL_BLEND);

  glViewport(0, 0, m_uiScreenWidth, m_uiScreenHeight);
  if (m_kOptions.m_bBoundingBoxesVisible)
  {
    // display bounding box
    SetupColorControl(true);
    SetupDepthControl(true, false, GL_LEQUAL);
    DisplayBoundingBoxes();
  }

  SetupColorControl(true);
  SetupDepthControl(false, false);
  PrepareSceneProjection();
  DisplayGizmos();

  CHECK_OPENGL_ERROR;

  return true;
}

/*! 
Early depth test. I hope all is set correctly to work on NV40.
I have only NV38 here so I don't see any early stencil improvement.
This is method as described by M.Harris at http://www.gpgpu.org/forums/viewtopic.php?t=256
*/
bool 
RaytracerRForce::RenderEarlyZRejection(bool bEnable)
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);

  // do not mess with stencil stuff - safety net
  glDisable(GL_STENCIL_TEST);
  glStencilMask(0);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // driver safety
  glStencilFunc(GL_ALWAYS, 0, 0); // driver safety

  // clear buffer
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(0, 0, 0, 0);
  glDepthMask(GL_TRUE);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  SetActiveKernel(RTRF_KERNEL_DEBUG);

  // prepare computation mask
  if (bEnable) // if optimization is enabled
  {
    // prepare masking kernel
    ActiveKernel()->SetMode(KERNEL_DEBUG_MODE_TEST1);
    ActiveKernel()->Activate();

    // setup depth writing without depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);

    // init transformation & projection
    glViewport(0,0,512,512);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw quad
    glBegin(GL_QUADS);
    glVertex3f(-1,-1, HARDDEPTH(0.0));
    glVertex3f( 1,-1, HARDDEPTH(0.0));
    glVertex3f( 1, 1, HARDDEPTH(0.0));
    glVertex3f(-1, 1, HARDDEPTH(0.0));
    glEnd();

    ActiveKernel()->Deactivate();

#if 0
    // debug
    imdebugPixelsf(0,0,512,512,GL_DEPTH_COMPONENT);
    exit(1);
#endif
  }

  // compute main (heavyweight) kernel
  ActiveKernel()->SetMode(KERNEL_DEBUG_MODE_WORK1);
  ActiveKernel()->Activate();

  // setup depth testing compatible with early-z
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // init transformation & projection
  glViewport(0,0,512,512);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // draw quad
  glBegin(GL_QUADS);
  glVertex3f(-1,-1, HARDDEPTH(0.5));
  glVertex3f( 1,-1, HARDDEPTH(0.5));
  glVertex3f( 1, 1, HARDDEPTH(0.5));
  glVertex3f(-1, 1, HARDDEPTH(0.5));
  glEnd();

  ActiveKernel()->Deactivate();

  // state cleanup
  glDisable(GL_DEPTH_TEST);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(~0);

  CHECK_OPENGL_ERROR;

  return true;
}


/*! 
Early stencil test. I hope all is set correctly to work on NV40.
I have only NV38 here so I don't see any early stencil improvement.
*/
bool 
RaytracerRForce::RenderEarlySRejection(bool bEnable)
{
  /////////////////////////////////
  // main context

  SetActivePBuffer(RTRF_PBUFFER_NONE);

  // do not mess with depth stuff - safety net
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // clear buffer
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(0, 0, 0, 0);
  glStencilMask(~0);
  glClearStencil(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  SetActiveKernel(RTRF_KERNEL_DEBUG);

  // prepare computation mask
  if (bEnable)
  {
    ActiveKernel()->SetMode(KERNEL_DEBUG_MODE_TEST1);
    ActiveKernel()->Activate();

    // enable stencil writes
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilMask(~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // init transformation & projection
    glViewport(0,0,512,512);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw quad
    glBegin(GL_QUADS);
    glVertex3f(-1,-1, HARDDEPTH(0.5));
    glVertex3f( 1,-1, HARDDEPTH(0.5));
    glVertex3f( 1, 1, HARDDEPTH(0.5));
    glVertex3f(-1, 1, HARDDEPTH(0.5));
    glEnd();

    ActiveKernel()->Deactivate();

#if 0
    // debug
    imdebugPixels(0,0,512,512,GL_STENCIL_INDEX);
    exit(1);
#endif
  }

  // compute main (heavyweight) kernel
  ActiveKernel()->SetMode(KERNEL_DEBUG_MODE_WORK1);
  ActiveKernel()->Activate();

  // setup stencil testing, disable stencil writes
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_EQUAL, 1, ~0);
  glStencilMask(0);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  // init transformation & projection
  glViewport(0,0,512,512);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // draw quad
  glBegin(GL_QUADS);
  glVertex3f(-1,-1, HARDDEPTH(0.5));
  glVertex3f( 1,-1, HARDDEPTH(0.5));
  glVertex3f( 1, 1, HARDDEPTH(0.5));
  glVertex3f(-1, 1, HARDDEPTH(0.5));
  glEnd();

  ActiveKernel()->Deactivate();

  // state cleanup
  glDisable(GL_STENCIL_TEST);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(~0);

  CHECK_OPENGL_ERROR;

  return true;
}

/*! 
Dispatching of different schemes. 
*/
bool 
RaytracerRForce::Execute()
{
  switch (m_kOptions.m_iRenderScheme){
    case SCHEME_A: return RenderA();
    case SCHEME_B: return RenderB();
    case SCHEME_C: return RenderC();
    case SCHEME_D: return RenderD();
    case SCHEME_E: return RenderE();
    case SCHEME_F: return RenderEarlyZRejection(false);
    case SCHEME_G: return RenderEarlyZRejection(true);
    case SCHEME_H: return RenderEarlySRejection(false);
    case SCHEME_I: return RenderEarlySRejection(true);
  }
  return false;
}

//! @}      //doxygen group
