// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelHitTest.cpp
\brief Implementation of KernelHitTest class.
*/

#include "base.h"

#include "KernelHitTest.h"

KernelHitTest::KernelHitTest()
{
  m_iMode = KERNEL_HITTEST_MODE_MARK;
}

bool KernelHitTest::Prepare()
{
  ReserveSlots(KERNEL_HITTEST_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_HITTEST_MODE_MARK,         SHADER_DIR "/" "kernel_hittest1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_HITTEST_MODE_MARK,         SHADER_DIR "/" "kernel_hittest1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_HITTEST_MODE_TEST,         SHADER_DIR "/" "kernel_hittest2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_HITTEST_MODE_TEST,         SHADER_DIR "/" "kernel_hittest2_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_HITTEST_MODE_MARKPOINTS,   SHADER_DIR "/" "kernel_hittest3_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_HITTEST_MODE_MARKPOINTS,   SHADER_DIR "/" "kernel_hittest3_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_HITTEST_MODE_TESTASPOINTS, SHADER_DIR "/" "kernel_hittest4_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_HITTEST_MODE_TESTASPOINTS, SHADER_DIR "/" "kernel_hittest4_fp.cg.shd")) return false;

  return true;
}

//! @}      //doxygen group
