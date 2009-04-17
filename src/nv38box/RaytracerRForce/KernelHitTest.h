// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup RForce
@{
\ingroup Raytracers
*/

/*! \file KernelHitTest.h
\brief Declaration of KernelHitTest class.
*/

#ifndef _KERNELHITTEST_H_
#define _KERNELHITTEST_H_

#include "Kernel.h"

//! Enum for HitTest kernel modes
enum E_KERNEL_HITTEST_MODE {
  KERNEL_HITTEST_MODE_MARK = 0,
  KERNEL_HITTEST_MODE_TEST,
  KERNEL_HITTEST_MODE_MARKPOINTS,
  KERNEL_HITTEST_MODE_TESTASPOINTS,
  KERNEL_HITTEST_MODE_COUNT
};

//! Class managing HitTest kernel.
/*!
    HitTest kernel is responsible for testing triangle against ray.

    <b>Mode KERNEL_HITTEST_MODE_MARK</b><br>
    TODO
    <b>Mode KERNEL_HITTEST_MODE_TEST</b><br>
    TODO
    <b>Mode KERNEL_HITTEST_MODE_MARKPOINTS</b><br>
    TODO
    <b>Mode KERNEL_HITTEST_MODE_TESTASPOINTS</b><br>
    TODO
*/
class KernelHitTest : public Kernel
{
public:
  //! KernelBVBuilder constructor.
  KernelHitTest ();

  //! Loads programs from disk and prepares kernel.
  virtual bool Prepare();
};

#endif

//! @}      //doxygen group
