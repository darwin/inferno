// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelDebug.cpp
\brief Implementation of KernelDebug class.
*/

#include "base.h"

#include "KernelDebug.h"

KernelDebug::KernelDebug()
{
  m_iMode = KERNEL_DEBUG_MODE_TEST1;
}

bool KernelDebug::Prepare()
{
  ReserveSlots(KERNEL_DEBUG_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_DEBUG_MODE_TEST1, SHADER_DIR "/" "kernel_debug1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_DEBUG_MODE_TEST1, SHADER_DIR "/" "kernel_debug1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_DEBUG_MODE_WORK1, SHADER_DIR "/" "kernel_debug2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_DEBUG_MODE_WORK1, SHADER_DIR "/" "kernel_debug2_fp.cg.shd")) return false;

  return true;
}

//! @}      //doxygen group
