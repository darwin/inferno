// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelShader.h
\brief Declaration of KernelShader class.
*/

#ifndef _KERNELSHADER_H_
#define _KERNELSHADER_H_

#include "Kernel.h"

//! Enum for Shader kernel modes
enum E_KERNEL_SHADER_MODE {
  KERNEL_SHADER_MODE_BASIC_FIXED_FLAT= 0,
  KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING,
  KERNEL_SHADER_MODE_BASIC_PV_FLAT,
  KERNEL_SHADER_MODE_BASIC_PV_TEXTURING,
  KERNEL_SHADER_MODE_BASIC_PP_FLAT,
  KERNEL_SHADER_MODE_BASIC_PP_TEXTURING,
  KERNEL_SHADER_MODE_COMPOSITION_COLOREDSECONDARY,
  KERNEL_SHADER_MODE_COMPOSITION_TEXTUREDSECONDARY,
  KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY,
  KERNEL_SHADER_MODE_BOUNDINGBOXES,
  KERNEL_SHADER_MODE_COUNT
};

//! Class managing Shader kernel.
/*!
    Shader kernel is responsible for shading result hits and other needed shading functionality.

    <b>Mode KERNEL_SHADER_MODE_BASIC_FIXED_FLAT</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BASIC_FIXED_TEXTURING</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BASIC_PV_FLAT</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BASIC_PV_TEXTURING</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BASIC_PP_FLAT</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BASIC_PP_TEXTURING</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_COMPOSITION_COLOREDSECONDARY</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_COMPOSITION_TEXTUREDSECONDARY</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_COMPOSITION_SHADEDSECONDARY</b><br>
    TODO
    <b>Mode KERNEL_SHADER_MODE_BOUNDINGBOXES</b><br>
    TODO
*/
class KernelShader : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelShader ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
  //! Activates programs in custom way.
  virtual void Activate();
};

#endif

//! @}      //doxygen group
