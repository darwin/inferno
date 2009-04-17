// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelBVBuilder.cpp
\brief Implementation of KernelBVBuilder class.
*/

#include "base.h"

#include "KernelBVBuilder.h"

KernelBVBuilder::KernelBVBuilder()
{
  m_iMode = KERNEL_BVBUILDER_MODE_MINS;
}

bool KernelBVBuilder::Prepare()
{
  ReserveSlots(KERNEL_BVBUILDER_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_BVBUILDER_MODE_MINS, SHADER_DIR "/" "kernel_bvbuilder1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_BVBUILDER_MODE_MINS, SHADER_DIR "/" "kernel_bvbuilder1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_BVBUILDER_MODE_MAXS, SHADER_DIR "/" "kernel_bvbuilder2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_BVBUILDER_MODE_MAXS, SHADER_DIR "/" "kernel_bvbuilder2_fp.cg.shd")) return false;

  return true;
}

//! @}      //doxygen group
