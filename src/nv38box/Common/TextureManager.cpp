// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Textures
@{
\ingroup Resource
*/
/*! \file TextureManager.cpp
\brief Implementation of TextureManager class.
*/
#include "base.h"

#include "TextureManager.h"

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
  Release();
}

void 
TextureManager::Release()
{
  iterator i = begin();
  while (i!=end())
  {

    if (*i) 
    {
      (*i)->Delete();
      delete *i;
    }
    i++;
  }
  clear();
}

//! @}      //doxygen group
