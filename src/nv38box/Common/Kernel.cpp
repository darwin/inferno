// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Kernels
@{
\ingroup Resource
*/
/*! \file Kernel.cpp
\brief Implementation of Kernel class.
*/
#include "base.h"

#include "Kernel.h"

Kernel::Kernel()
{
  m_iMode = 0;
}

Kernel::~Kernel()
{
  Release();
}

bool 
Kernel::LoadFragmentProgram(int iIndex, const char* acFilename)
{
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return false;

  if ((unsigned int)iIndex>=m_hFP.size())
    m_hFP.resize(iIndex+1);

  m_hFP[iIndex] = cgCreateProgramFromFile(pCgManager->GetContext(), CG_OBJECT, acFilename,  pCgManager->GetFragmentProfile(), NULL, NULL);
  if (!cgIsProgramCompiled(m_hFP[iIndex]))
    cgCompileProgram(m_hFP[iIndex]);

  cgGLLoadProgram(m_hFP[iIndex]);

  return true;
}

bool 
Kernel::LoadVertexProgram(int iIndex, const char* acFilename)
{
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return false;

  if ((unsigned int)iIndex>=m_hVP.size())
    m_hVP.resize(iIndex+1);

  m_hVP[iIndex] = cgCreateProgramFromFile(pCgManager->GetContext(), CG_OBJECT, acFilename,  pCgManager->GetVertexProfile(), NULL, NULL);
  if (!cgIsProgramCompiled(m_hVP[iIndex]))
    cgCompileProgram(m_hVP[iIndex]);

  cgGLLoadProgram(m_hVP[iIndex]);

  return true;
}

void Kernel::Activate()
{
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return;

  // set active vertex program
  cgGLEnableProfile(pCgManager->GetVertexProfile());
  cgGLBindProgram(m_hVP[m_iMode]);
 
  // set active fragment program
  cgGLEnableProfile(pCgManager->GetFragmentProfile());
  cgGLBindProgram(m_hFP[m_iMode]);
}

void Kernel::Deactivate()
{
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return;

  // deactivate vertex program
  cgGLDisableProfile(pCgManager->GetVertexProfile());

  // deactivate fragment program
  cgGLDisableProfile(pCgManager->GetFragmentProfile());
}

void Kernel::DestroyPrograms()
{
  TProgramContainer::iterator iVP = m_hVP.begin();
  while (iVP!=m_hVP.end())
  {
    if (cgIsProgram(*iVP))
      cgDestroyProgram(*iVP);
    iVP++;
  }

  TProgramContainer::iterator iFP = m_hFP.begin();
  while (iFP!=m_hFP.end())
  {
    if (cgIsProgram(*iFP))
      cgDestroyProgram(*iFP);
    iFP++;
  }
}

void Kernel::Release()
{
  ReleaseParams();
  DestroyPrograms();
}

//////////////////////////////////////////////////////////////////////////

void ParamsCache::CacheVertexProgramParams(CGprogram hVP)
{
  if (hVP)
  {
    m_kModelParam =   cgGetNamedParameter(hVP, "Model");
    m_kModelIParam =  cgGetNamedParameter(hVP, "ModelI");
    m_kModelITParam = cgGetNamedParameter(hVP, "ModelIT");
    m_kModelTParam =  cgGetNamedParameter(hVP, "ModelT");

    m_kModelViewParam =   cgGetNamedParameter(hVP, "ModelView");
    m_kModelViewIParam =  cgGetNamedParameter(hVP, "ModelViewI");
    m_kModelViewITParam = cgGetNamedParameter(hVP, "ModelViewIT");
    m_kModelViewTParam =  cgGetNamedParameter(hVP, "ModelViewT");

    m_kModelViewProjParam =   cgGetNamedParameter(hVP, "ModelViewProj");
    m_kModelViewProjIParam =  cgGetNamedParameter(hVP, "ModelViewProjI");
    m_kModelViewProjITParam = cgGetNamedParameter(hVP, "ModelViewProjIT");
    m_kModelViewProjTParam =  cgGetNamedParameter(hVP, "ModelViewProjT");

    m_kProjParam =   cgGetNamedParameter(hVP, "Proj");
    m_kProjIParam =  cgGetNamedParameter(hVP, "ProjI");
    m_kProjITParam = cgGetNamedParameter(hVP, "ProjIT");
    m_kProjTParam =  cgGetNamedParameter(hVP, "ProjT");

    m_kWLightPosParam = cgGetNamedParameter(hVP, "WLightPos");

    m_kWLightPosParams.clear();
    if (m_kWLightPosParam)
    {
      unsigned int iArraySize = cgGetArraySize(m_kWLightPosParam, 0); // 1D array
      for(unsigned int i=0; i < iArraySize; ++i)
      {
        CGparameter kElement = cgGetArrayParameter(m_kWLightPosParam, i);
        m_kWLightPosParams.push_back(kElement);
      }
    }

    m_kWEyePosParam = cgGetNamedParameter(hVP, "WEyePos");
    m_kWFocusPosParam = cgGetNamedParameter(hVP, "WFocusPos");
    m_kWEyeVecParam = cgGetNamedParameter(hVP, "WEyeVec");
  }
}

void ParamsCache::CacheFragmentProgramParams(CGprogram hFP)
{
  if (hFP)
  {
    m_kModelParam =   cgGetNamedParameter(hFP, "Model");
    m_kModelIParam =  cgGetNamedParameter(hFP, "ModelI");
    m_kModelITParam = cgGetNamedParameter(hFP, "ModelIT");
    m_kModelTParam =  cgGetNamedParameter(hFP, "ModelT");

    m_kModelViewParam =   cgGetNamedParameter(hFP, "ModelView");
    m_kModelViewIParam =  cgGetNamedParameter(hFP, "ModelViewI");
    m_kModelViewITParam = cgGetNamedParameter(hFP, "ModelViewIT");
    m_kModelViewTParam =  cgGetNamedParameter(hFP, "ModelViewT");

    m_kModelViewProjParam =   cgGetNamedParameter(hFP, "ModelViewProj");
    m_kModelViewProjIParam =  cgGetNamedParameter(hFP, "ModelViewProjI");
    m_kModelViewProjITParam = cgGetNamedParameter(hFP, "ModelViewProjIT");
    m_kModelViewProjTParam =  cgGetNamedParameter(hFP, "ModelViewProjT");

    m_kProjParam =   cgGetNamedParameter(hFP, "Proj");
    m_kProjIParam =  cgGetNamedParameter(hFP, "ProjI");
    m_kProjITParam = cgGetNamedParameter(hFP, "ProjIT");
    m_kProjTParam =  cgGetNamedParameter(hFP, "ProjT");

    m_kWLightPosParam = cgGetNamedParameter(hFP, "WLightPos");

    m_kWLightPosParams.clear();
    if (m_kWLightPosParam)
    {
      unsigned int iArraySize = cgGetArraySize(m_kWLightPosParam, 0); // 1D array
      for(unsigned int i=0; i < iArraySize; ++i)
      {
        CGparameter kElement = cgGetArrayParameter(m_kWLightPosParam, i);
        m_kWLightPosParams.push_back(kElement);
      }
    }

    m_kWEyePosParam = cgGetNamedParameter(hFP, "WEyePos");
    m_kWFocusPosParam = cgGetNamedParameter(hFP, "WFocusPos");
    m_kWEyeVecParam = cgGetNamedParameter(hFP, "WEyeVec");
  }
}

void ParamsCache::Release()
{
}

void 
Kernel::CacheParams()
{
  TProgramContainer::iterator iVP = m_hVP.begin();
  m_kParamsCacheVP.clear();
  while (iVP!=m_hVP.end())
  {
    ParamsCache kCache;
    kCache.CacheVertexProgramParams(*iVP);
    m_kParamsCacheVP.push_back(kCache);
    iVP++;
  }

  TProgramContainer::iterator iFP = m_hFP.begin();
  m_kParamsCacheFP.clear();
  while (iFP!=m_hFP.end())
  {
    ParamsCache kCache;
    kCache.CacheFragmentProgramParams(*iFP);
    m_kParamsCacheFP.push_back(kCache);
    iFP++;
  }
}

void 
Kernel::ReleaseParams()
{
  TParamsCacheContainer::iterator i = m_kParamsCacheVP.begin();
  while (i!=m_kParamsCacheVP.end())
  {
    (*i).Release();
    i++;
  }
  m_kParamsCacheVP.clear();

  i = m_kParamsCacheFP.begin();
  while (i!=m_kParamsCacheFP.end())
  {
    (*i).Release();
    i++;
  }
  m_kParamsCacheFP.clear();
}
//! @}      //doxygen group
