// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelBVTest.cpp
\brief Implementation of KernelBVTest class.
*/

#include "base.h"

#include "KernelBVTest.h"

KernelBVTest::KernelBVTest()
{
  m_iMode = KERNEL_BVTEST_MODE_TEST;
}

bool KernelBVTest::Prepare()
{
  ReserveSlots(KERNEL_BVTEST_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_BVTEST_MODE_TEST, SHADER_DIR "/" "kernel_bvtest1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_BVTEST_MODE_TEST, SHADER_DIR "/" "kernel_bvtest1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_BVTEST_MODE_COMPUTATIONMASK, SHADER_DIR "/" "kernel_bvtest2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_BVTEST_MODE_COMPUTATIONMASK, SHADER_DIR "/" "kernel_bvtest2_fp.cg.shd")) return false;

  return true;
}

//! @}      //doxygen group
