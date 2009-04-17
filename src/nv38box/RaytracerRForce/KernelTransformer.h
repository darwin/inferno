// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelTransformer.h
\brief Declaration of KernelTransformer class.
*/

#ifndef _KERNELTRANSFORMER_H_
#define _KERNELTRANSFORMER_H_

#include "Kernel.h"

//! Enum for Transformer kernel modes
enum E_KERNEL_TRANSFORMER_MODE {
  KERNEL_TRANSFORMER_MODE_POSITIONS = 0,
  KERNEL_TRANSFORMER_MODE_NORMALS,
  KERNEL_TRANSFORMER_MODE_COUNT
};

//! Class managing Transformer kernel.
/*!
    Transformer kernel is responsible for transforming geometry from object space to world space.
    Results are stored in texture.

    <b>Mode KERNEL_TRANSFORMER_MODE_POSITIONS</b><br>
    Program transforms positions.
    <b>Mode KERNEL_TRANSFORMER_MODE_NORMALS</b><br>
    Program Transforms normals.
*/
class KernelTransformer : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelTransformer ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
