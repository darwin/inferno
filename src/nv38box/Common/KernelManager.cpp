// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Kernels
@{
\ingroup Resource
*/
/*! \file KernelManager.cpp
\brief Implementation of KernelManager class.
*/
#include "base.h"

#include "KernelManager.h"

KernelManager::KernelManager()
{

}

KernelManager::~KernelManager()
{
  Release();
}

bool
KernelManager::Prepare()
{
  iterator ki = begin();
  while (ki!=end())
  {
    if (*ki)
    {
      (*ki)->Prepare();
      (*ki)->CacheParams();
    }
    ki++;
  }

  return true;
}

bool
KernelManager::Release()
{
  iterator ki = begin();
  while (ki!=end())
  {
    if (*ki)
    {
      (*ki)->Release();
      delete (*ki);
    }
    ki++;
  }
  clear();

  return true;
}

//! @}      //doxygen group
