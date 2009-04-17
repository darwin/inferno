// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelRayGenerator.cpp
\brief Implementation of KernelRayGenerator class.
*/

#include "base.h"

#include "KernelRayGenerator.h"

KernelRayGenerator::KernelRayGenerator()
{
  m_iMode = KERNEL_RAYGENERATOR_MODE_RAYSTARTS;
}

bool KernelRayGenerator::Prepare()
{
  ReserveSlots(KERNEL_RAYGENERATOR_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_RAYGENERATOR_MODE_RAYSTARTS, SHADER_DIR "/" "kernel_raygenerator1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_RAYGENERATOR_MODE_RAYSTARTS, SHADER_DIR "/" "kernel_raygenerator1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_RAYGENERATOR_MODE_RAYENDS,   SHADER_DIR "/" "kernel_raygenerator2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_RAYGENERATOR_MODE_RAYENDS,   SHADER_DIR "/" "kernel_raygenerator2_fp.cg.shd")) return false;

 return true;
}

//! @}      //doxygen group
