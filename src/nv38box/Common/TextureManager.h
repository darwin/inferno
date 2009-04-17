// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Textures
@{
\ingroup Resource
*/
/*! \file TextureManager.h
\brief Declaration of TextureManager class.
*/
#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include "ManagerBase.h"
#include "Texture.h"

typedef vector<Texture*> TTextureContainer; //!< Container fo textures.

//! Class responsible for managing textures (singleton)
/*!
\note Use STD container functions to access kernels.
*/
class TextureManager : public ManagerBase, public TTextureContainer
{
public:
  //! TextureManager constructor.
  TextureManager();
  //! TextureManager destructor.
  ~TextureManager();

  //! Releases all textures.
  void Release();

protected:

};

#endif
//! @}      //doxygen group
