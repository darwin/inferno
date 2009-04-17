// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup PBuffers
@{
\ingroup Resource
*/
/*! \file PBuffer.h
\brief Declaration of PBuffer class.
*/
#ifndef PBUFFER_H
#define PBUFFER_H

#include "RenderTexture.h"

//! Class representing PBuffer resource.
/*! \note My previous class was replaced by excellent RenderTexture2 implementation by M.Harris.
*/
class PBuffer : public RenderTexture
{
public:
  //! PBufer contructor.
  PBuffer(const char *strMode);
  //! PBufer destructor.
  ~PBuffer();
};

#endif
//! @}      //doxygen group
