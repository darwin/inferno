// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup PBuffers
@{
\ingroup Resource
*/
/*! \file PBuffer.cpp
\brief Implementation of PBuffer class.
*/
#include "base.h"

#include "PBuffer.h"

PBuffer::PBuffer(const char *strMode="rgb tex2D"):
RenderTexture(strMode) 
{
}

PBuffer::~PBuffer()
{
  
}
//! @}      //doxygen group
