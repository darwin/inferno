// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelShader.cpp
\brief Implementation of KernelShader class.
*/

#include "base.h"

#include "KernelShader.h"

KernelShader::KernelShader()
{
  m_iMode = KERNEL_SHADER_MODE_BASIC_PV_FLAT;
}

bool KernelShader::Prepare()
{
  ReserveSlots(KERNEL_SHADER_MODE_COUNT);

  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY,   SHADER_DIR "/" "kernel_shader1_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY,   SHADER_DIR "/" "kernel_shader1_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_COMPOSITION_COLOREDSECONDARY,  SHADER_DIR "/" "kernel_shader2_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_COMPOSITION_COLOREDSECONDARY,  SHADER_DIR "/" "kernel_shader2_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BOUNDINGBOXES,         SHADER_DIR "/" "kernel_shader3_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BOUNDINGBOXES,         SHADER_DIR "/" "kernel_shader3_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_FIXED_FLAT,      SHADER_DIR "/" "kernel_shader4_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_FIXED_FLAT,      SHADER_DIR "/" "kernel_shader4_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING, SHADER_DIR "/" "kernel_shader5_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING, SHADER_DIR "/" "kernel_shader5_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_PV_FLAT,         SHADER_DIR "/" "kernel_shader6_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_PV_FLAT,         SHADER_DIR "/" "kernel_shader6_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_PV_TEXTURING,    SHADER_DIR "/" "kernel_shader7_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_PV_TEXTURING,    SHADER_DIR "/" "kernel_shader7_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_PP_FLAT,         SHADER_DIR "/" "kernel_shader8_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_PP_FLAT,         SHADER_DIR "/" "kernel_shader8_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_BASIC_PP_TEXTURING,    SHADER_DIR "/" "kernel_shader9_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_BASIC_PP_TEXTURING,    SHADER_DIR "/" "kernel_shader9_fp.cg.shd")) return false;
  if (!LoadVertexProgram  (KERNEL_SHADER_MODE_COMPOSITION_TEXTUREDSECONDARY,   SHADER_DIR "/" "kernel_shader10_vp.cg.shd")) return false;
  if (!LoadFragmentProgram(KERNEL_SHADER_MODE_COMPOSITION_TEXTUREDSECONDARY,   SHADER_DIR "/" "kernel_shader10_fp.cg.shd")) return false;

  return true;
}

void KernelShader::Activate()
{
  CgManager* pCgManager = GetCgManager();
  if (!pCgManager) return;
  if (m_iMode!=KERNEL_SHADER_MODE_BASIC_FIXED_FLAT && m_iMode!=KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING)
  {
    // in fixed mode, use fixed-function vertex processing
    cgGLEnableProfile(pCgManager->GetVertexProfile());
    cgGLBindProgram(m_hVP[m_iMode]);
  }

  cgGLEnableProfile(pCgManager->GetFragmentProfile());
  cgGLBindProgram(m_hFP[m_iMode]);
}

//! @}      //doxygen group
