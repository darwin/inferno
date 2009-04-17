// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelDebug.h
\brief Declaration of KernelDebug class.
*/

#ifndef _KERNELDEBUG_H_
#define _KERNELDEBUG_H_

#include "Kernel.h"

//! Enum for Debug kernel modes
enum E_KERNEL_DEBUG_MODE {
  KERNEL_DEBUG_MODE_TEST1 = 0,
  KERNEL_DEBUG_MODE_WORK1,
  KERNEL_DEBUG_MODE_COUNT       
};

//! Class managing Debug kernel.
/*!
    Debug kernel is dedicated for testing and debug purposes.

    <b>Mode KERNEL_DEBUG_MODE_TEST1 and KERNEL_DEBUG_MODE_WORK1</b><br>
    Debug programs for early-z test.
*/
class KernelDebug : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelDebug ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
