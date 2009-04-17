// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelReduceHits.cpp
\brief Implementation of KernelReduceHits class.
*/

#include "base.h"

#include "KernelReduceHits.h"

KernelReduceHits::KernelReduceHits()
{
  m_iMode = KERNEL_REDUCEHITS_MODE_REDUCE;
}

bool KernelReduceHits::Prepare()
{
  ReserveSlots(KERNEL_REDUCEHITS_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_REDUCEHITS_MODE_REDUCE,         SHADER_DIR "/" "kernel_reducehits1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_REDUCEHITS_MODE_REDUCE,         SHADER_DIR "/" "kernel_reducehits1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_REDUCEHITS_MODE_REDUCEASPOINTS, SHADER_DIR "/" "kernel_reducehits2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_REDUCEHITS_MODE_REDUCEASPOINTS, SHADER_DIR "/" "kernel_reducehits2_fp.cg.shd")) return false;

  return true;
}

void KernelReduceHits::Activate()
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

//! @}      //doxygen group
