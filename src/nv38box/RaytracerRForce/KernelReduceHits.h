// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelReduceHits.h
\brief Declaration of KernelReduceHits class.
*/

#ifndef _KERNELREDUCEHITS_H_
#define _KERNELREDUCEHITS_H_

#include "Kernel.h"

//! Enum for ReduceHits kernel modes
enum E_KERNEL_REDUCEHITS_MODE {
  KERNEL_REDUCEHITS_MODE_REDUCE = 0,
  KERNEL_REDUCEHITS_MODE_REDUCEASPOINTS,
  KERNEL_REDUCEHITS_MODE_COUNT
};

//! Class managing ReduceHits kernel.
/*!
    ReduceHits kernel is responsible for reducing ray-triangle hits across pixel.

    <b>Mode KERNEL_REDUCEHITS_MODE_REDUCE</b><br>
    TODO
    <b>Mode KERNEL_REDUCEHITS_MODE_REDUCEASPOINTS</b><br>
    TODO
*/
class KernelReduceHits : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelReduceHits ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
  //! Activates programs in custom way.
  virtual void Activate();
};

#endif

//! @}      //doxygen group
