// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelTransformer.cpp
\brief Implementation of KernelTransformer class.
*/

#include "base.h"

#include "KernelTransformer.h"

KernelTransformer::KernelTransformer()
{
  m_iMode = KERNEL_TRANSFORMER_MODE_POSITIONS;
}

bool KernelTransformer::Prepare()
{
  ReserveSlots(KERNEL_TRANSFORMER_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_TRANSFORMER_MODE_POSITIONS, SHADER_DIR "/" "kernel_transformer1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_TRANSFORMER_MODE_POSITIONS, SHADER_DIR "/" "kernel_transformer1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_TRANSFORMER_MODE_NORMALS,   SHADER_DIR "/" "kernel_transformer2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_TRANSFORMER_MODE_NORMALS,   SHADER_DIR "/" "kernel_transformer2_fp.cg.shd")) return false;

  return true;
}

//! @}      //doxygen group
