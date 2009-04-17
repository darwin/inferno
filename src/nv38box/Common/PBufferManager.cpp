// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup PBuffers
@{
\ingroup Resource
*/
/*! \file PBufferManager.cpp
\brief Implementation of PBufferManager class.
*/
#include "base.h"

#include "PBufferManager.h"

PBufferManager::PBufferManager()
{

}

PBufferManager::~PBufferManager()
{
  Release();
}

void 
PBufferManager::Release()
{
  iterator i = begin();
  while (i!=end())
  {
    if (*i) delete *i;
    i++;
  }
  clear();
}
//! @}      //doxygen group
