// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelBVTest.h
\brief Declaration of KernelBVTest class.
*/

#ifndef _KERNELBVTEST_H_
#define _KERNELBVTEST_H_

#include "Kernel.h"

//! Enum for BVTest kernel modes
enum E_KERNEL_BVTEST_MODE {
  KERNEL_BVTEST_MODE_TEST = 0,
  KERNEL_BVTEST_MODE_COMPUTATIONMASK,
  KERNEL_BVTEST_MODE_COUNT       
};

//! Class managing BVTest kernel.
/*!
    BVTest kernel is responsible for testing bounding volume against ray.

    <b>Mode KERNEL_BVTEST_MODE_TEST</b><br>
    Program returns intersection of ray and AABB.

    <b>Mode KERNEL_BVTEST_MODE_COMPUTATIONMASK</b><br>
    Program for creation of computation mask from KERNEL_BVTEST_MODE_TEST results.
*/
class KernelBVTest : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelBVTest();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
