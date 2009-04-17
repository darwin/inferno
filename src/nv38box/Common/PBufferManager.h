// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup PBuffers
@{
\ingroup Resource
*/
/*! \file PBufferManager.h
\brief Declaration of PBufferManager class.
*/
#ifndef _PBUFFERMANAGER_H_
#define _PBUFFERMANAGER_H_

#include "ManagerBase.h"
#include "pbuffer.h"

typedef vector<PBuffer*> TPBufferContainer; //!< Container for PBuffers.

//! Class responsible for managing PBuffers (singleton)
/*!
\note Use STD container functions to access kernels.
*/
class PBufferManager : public ManagerBase, public TPBufferContainer
{
public:
  //! PBufferManager constructor.
  PBufferManager();
  //! PBufferManager destructors.
  ~PBufferManager();

  //! Releases all PBuffers.
  void Release();

protected:
};

#endif
//! @}      //doxygen group
