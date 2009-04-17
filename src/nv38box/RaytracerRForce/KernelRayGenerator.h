// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelRayGenerator.h
\brief Declaration of KernelRayGenerator class.
*/

#ifndef _KERNELRAYGENERATOR_H_
#define _KERNELRAYGENERATOR_H_

#include "Kernel.h"

//! Enum for RayGenerator kernel modes
enum E_KERNEL_RAYGENERATOR_MODE { 
  KERNEL_RAYGENERATOR_MODE_RAYSTARTS = 0,
  KERNEL_RAYGENERATOR_MODE_RAYENDS,
  KERNEL_RAYGENERATOR_MODE_COUNT
};

//! Class managing RayGenerator kernel.
/*!
    RayGenerator kernel is responsible for generating data for new rays.

    <b>Mode KERNEL_RAYGENERATOR_MODE_RAYSTARTS</b><br>
    Program returns ray start in world space coordinates.

    <b>Mode KERNEL_RAYGENERATOR_MODE_RAYENDS</b><br>
    Program returns ray end in world space coordinates.
*/
class KernelRayGenerator : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelRayGenerator ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
