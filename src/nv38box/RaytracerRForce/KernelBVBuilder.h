// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelBVBuilder.h
\brief Declaration of KernelBVBuilder class.
*/

#ifndef _KERNELBVBUILDER_H_
#define _KERNELBVBUILDER_H_

#include "Kernel.h"

//! Enum for BVBuilder kernel modes
enum E_KERNEL_BVBUILDER_MODE {
  KERNEL_BVBUILDER_MODE_MINS = 0,
  KERNEL_BVBUILDER_MODE_MAXS,       
  KERNEL_BVBUILDER_MODE_COUNT
};

//! Class managing BVBuilder kernel.
/*!
    BVBuilder kernel is responsible for building bounding volumes structure in GPU.
    Acceleration structure is AABB computed using reduction from transformed geometry 
    with application MIN and MAX functions per components.
    
    <b>Mode KERNEL_BVBUILDER_MODE_MINS</b><br>
    Reduction program for points in world space. Result is minimum in all three axes.

    <b>Mode KERNEL_BVBUILDER_MODE_MAXS</b><br>
    Reduction program for points in world space. Result is maximum in all three axes.
*/
class KernelBVBuilder : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelBVBuilder ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
