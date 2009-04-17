// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Kernels
@{
\ingroup Resource
*/
/*! \file KernelManager.h
\brief Declaration of KernelManager class.
*/
#ifndef _KERNELMANAGER_H_
#define _KERNELMANAGER_H_

#include "ManagerBase.h"
#include "Kernel.h"

typedef vector<Kernel*> TKernelContainer;  //!< Container for kernels.

//! Class responsible for kernel management (singleton)
/*!
    \note Use STD container functions to access kernels.
*/
class KernelManager : public ManagerBase, public TKernelContainer
{
public:
  //! KernelManager constructor.
  KernelManager();
  //! KernelManager destructor.
  ~KernelManager();

  //! Prepares all kernels.
  bool Prepare();
  //! Releases all kernels.
  bool Release();

protected:
};

#endif
//! @}      //doxygen group
