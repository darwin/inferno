// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Raytracers
@{
\ingroup NV38Box
*/

/*! \file Raytracer.cpp
\brief Implementation of Raytracer class.
*/
#include "base.h"

#include "Raytracer.h"

Raytracer::Raytracer()
{
  m_iActiveKernel = -1;
  m_iActivePBuffer = -1;
  m_iActiveTexture = -1;
  m_bInitialized = false;
}

Raytracer::~Raytracer()
{
}

bool Raytracer::SetActiveTexture(int iSlot)
{
//  assert(iSlot<RT_TEXTURE_COUNT);
  m_iActiveTexture = iSlot;
  return true;
}

Texture* Raytracer::ActiveTexture()
{
  return m_kTextureManager[m_iActiveTexture];
}

bool Raytracer::SetActiveKernel(int iSlot)
{
//  assert(iSlot<RT_KERNEL_COUNT);
  m_iActiveKernel = iSlot;
  return true;
}

Kernel* Raytracer::ActiveKernel()
{
  return m_kKernelManager[m_iActiveKernel];
}

bool Raytracer::SetActivePBuffer(int iSlot)
{
//  assert(iSlot<RT_PBUFFER_COUNT);
  m_iActivePBuffer = iSlot;
  return true;
}

PBuffer* Raytracer::ActivePBuffer()
{
  if (m_iActivePBuffer==-1) return NULL;
  return m_kPBufferManager[m_iActivePBuffer];
}

void Raytracer::SwitchToPBuffer(int iSlot)
{
  PBuffer* pPreviousBuffer = ActivePBuffer();
  SetActivePBuffer(iSlot);
  
  PBuffer* pNewBuffer = ActivePBuffer();
  if (pNewBuffer)
  {
    pNewBuffer->BeginCapture(pPreviousBuffer);
  }
  else
  {
    pPreviousBuffer->EndCapture();
  }
}


bool Raytracer::SetupPBuffer(PBuffer* pPBuffer)
{
  pPBuffer->BeginCapture();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_AUTO_NORMAL);
  glDisable(GL_BLEND);

  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  glDisable(GL_CLIP_PLANE2);
  glDisable(GL_CLIP_PLANE3);
  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);

  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); 
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  glDisable(GL_INDEX_LOGIC_OP);

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  glDisable(GL_LIGHT3);
  glDisable(GL_LIGHT4);
  glDisable(GL_LIGHT5);
  glDisable(GL_LIGHT6);
  glDisable(GL_LIGHT7);

  glDisable(GL_LIGHTING);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_LINE_STIPPLE);
  glDisable(GL_LOGIC_OP);
  glDisable(GL_MAP1_COLOR_4);
  glDisable(GL_MAP1_INDEX);
  glDisable(GL_MAP1_NORMAL);
  glDisable(GL_MAP1_TEXTURE_COORD_1);
  glDisable(GL_MAP1_TEXTURE_COORD_2);
  glDisable(GL_MAP1_TEXTURE_COORD_3);
  glDisable(GL_MAP1_TEXTURE_COORD_4);
  glDisable(GL_MAP1_VERTEX_3);
  glDisable(GL_MAP1_VERTEX_4);
  glDisable(GL_MAP2_COLOR_4);
  glDisable(GL_MAP2_INDEX);
  glDisable(GL_MAP2_NORMAL);
  glDisable(GL_MAP2_TEXTURE_COORD_1);
  glDisable(GL_MAP2_TEXTURE_COORD_2);
  glDisable(GL_MAP2_TEXTURE_COORD_3);
  glDisable(GL_MAP2_TEXTURE_COORD_4);
  glDisable(GL_MAP2_VERTEX_3);
  glDisable(GL_MAP2_VERTEX_4);
  glDisable(GL_NORMALIZE);
  glDisable(GL_POINT_SMOOTH);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glDisable(GL_POLYGON_OFFSET_POINT);
  glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_POLYGON_STIPPLE);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_Q);
  glDisable(GL_TEXTURE_GEN_R);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);

  pPBuffer->EndCapture();

  return true;
}

void Raytracer::DisableSceneLighting()
{
  glDisable(GL_LIGHTING);
}

void Raytracer::SetupSceneLighting()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  ApplyCameraTransform();

  GLfloat afColor[4];

  // emissive color does not use transparency
  afColor[3] = 1.0f;

  afColor[0] = 0.0;
  afColor[1] = 0.0;
  afColor[2] = 0.0;
  glMaterialfv(GL_FRONT,GL_EMISSION, afColor);

  afColor[3] = 1;

  afColor[0] = 1;
  afColor[1] = 1;
  afColor[2] = 1;
  glMaterialfv(GL_FRONT,GL_SPECULAR, afColor);

  glMaterialf(GL_FRONT,GL_SHININESS,0);

  glEnable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);

  unsigned int iQuantity = (unsigned int)m_pkSimpleScene->m_kLights.size();

  unsigned int i;
  SimpleLight* pkLight;

  for (i = 0; i < iQuantity; i++)
  {
    pkLight = &m_pkSimpleScene->m_kLights[i];
    GLint iIndex = GL_LIGHT0 + i;
    glEnable((GLenum)iIndex);

    // ambient
    afColor[0] = 0.0f;
    afColor[1] = 0.0f;
    afColor[2] = 0.0f;

    // diffuse
    afColor[0] = pkLight->m_kColor[0];
    afColor[1] = pkLight->m_kColor[1];
    afColor[2] = pkLight->m_kColor[2];
    glLightfv((GLenum)iIndex,GL_AMBIENT,afColor);
    glLightfv((GLenum)iIndex,GL_DIFFUSE,afColor);

    // specular
    afColor[0] = 0.0f;
    afColor[1] = 0.0f;
    afColor[2] = 0.0f;
    glLightfv((GLenum)iIndex,GL_SPECULAR,afColor);

    glLightf((GLenum)iIndex,GL_CONSTANT_ATTENUATION,1.0f);
    glLightf((GLenum)iIndex,GL_LINEAR_ATTENUATION,0.3f);
    glLightf((GLenum)iIndex,GL_QUADRATIC_ATTENUATION,0.0f);

    GLfloat afPosParam[4];
    afPosParam[0] = pkLight->m_kPosition[0];
    afPosParam[1] = pkLight->m_kPosition[1];
    afPosParam[2] = pkLight->m_kPosition[2];
    afPosParam[3] = 1.0f;
    glLightfv((GLenum)iIndex,GL_POSITION,afPosParam);
  }

  GLfloat afAmbient[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,afAmbient);

  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
  
  for (i = iQuantity; i < 8; i++)
    glDisable((GLenum)(GL_LIGHT0 + i));
}

void Raytracer::Animate(DWORD time)
{
  if (m_kOptions.m_bAnimate)
  {
    if (m_pkSimpleScene->m_kSceneName==std::string("test"))
    {
      // simple animation
      SimpleElement& rkE = m_pkSimpleScene->m_kElements[1];
      Quaternionf r,q;
      float angle = (((time - m_kOptions.m_dwStopTimeDelta) % 10000) / 10000.0f) * 2 * 3.14;
      r.FromAngleAxis(angle, Vector3f(0,0,1));
      q.FromAngleAxis(angle, Vector3f(0,1,0));

      rkE.m_kRotation = r * q;
      rkE.InitManipulator(); // reset manipulator
    }
    if (m_pkSimpleScene->m_kSceneName==std::string("rotor"))
    {
      // simple animation
      Quaternionf r,q,w,d;
      float angle = (((time - m_kOptions.m_dwStopTimeDelta) % 10000) / 10000.0f) * 2 * 3.14;
      r.FromAngleAxis(angle, Vector3f(0,1,0));
      q.FromAngleAxis(3.14/2, Vector3f(0,1,0));
      w.FromAngleAxis(3.14/2, Vector3f(1,0,0));

      for (int i=3; i<16; i++)
      {
        d.FromAngleAxis((i-3)*(2*3.14)/8.0f, Vector3f(0,1,0));
        SimpleElement& rkE = m_pkSimpleScene->m_kElements[i];
        rkE.m_kRotation = w * q * d * r;
        rkE.InitManipulator(); // reset manipulator
      }
    }
    if (m_pkSimpleScene->m_kSceneName==std::string("prism"))
    {
      // simple animation
      Quaternionf r,q;
      float angle = (((time - m_kOptions.m_dwStopTimeDelta) % 10000) / 10000.0f) * 2 * 3.14;
      r.FromAngleAxis(angle, Vector3f(0,1,0));
      q.FromAngleAxis(angle, Vector3f(0,0,1));

      SimpleElement& rkE = m_pkSimpleScene->m_kElements[1];
      rkE.m_kRotation = r * q;
      rkE.InitManipulator(); // reset manipulator
    }
  }
}

void Raytracer::DisplayLightGizmo()
{
  float iSize = 1.0;
  glBegin(GL_LINES);
    glVertex3f(0,0,-iSize);
    glVertex3f(0,0,iSize);
    glVertex3f(0,-iSize,0);
    glVertex3f(0,iSize,0);
    glVertex3f(-iSize,0,0);
    glVertex3f(iSize,0,0);
  glEnd();
}

void Raytracer::DisplayElementPivot()
{
  float iSize = 1.0;
  glBegin(GL_LINES);
  glVertex3f(0,0,-iSize);
  glVertex3f(0,0,iSize);
  glVertex3f(0,-iSize,0);
  glVertex3f(0,iSize,0);
  glVertex3f(-iSize,0,0);
  glVertex3f(iSize,0,0);
  glEnd();
}

void Raytracer::SetupDepthControl(bool bTest, bool bWrite, int iFunction)
{
  if (bWrite)
  {
    if (bTest)
    {
      glDepthMask(GL_TRUE);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(iFunction);
    }
    else
    {
      glDepthMask(GL_TRUE);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_ALWAYS);
    }
  }
  else
  {
    if (bTest)
    {
      glDepthMask(GL_FALSE);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(iFunction);
    }
    else
    {
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
    }
  }
}

void Raytracer::SetupColorControl(bool bWrite)
{
  if (bWrite)
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  else
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void Raytracer::SetupAlphaControl(bool bTest, int iFunction, float fValue)
{
  if (bTest)
  {
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(iFunction, fValue);
  }
  else
  {
    glDisable(GL_ALPHA_TEST);
  }
}

//! @}      //doxygen group
