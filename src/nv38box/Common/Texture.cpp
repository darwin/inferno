// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Textures
@{
\ingroup Resource
*/
/*! \file Texture.cpp
\brief Implementation of Texture class.
*/
#include "base.h"

#include "Texture.h"

Texture::Texture()
{
  m_uiId = 0;
  m_eTarget = GL_TEXTURE_RECTANGLE_NV;
}

Texture::~Texture()
{
}

bool Texture::Generate()
{
  glGenTextures(1, &m_uiId);
  return true;
}

bool Texture::Delete()
{
  glDeleteTextures(1, &m_uiId);
  return true;
}

bool Texture::Bind()
{
  //glGetIntegerv(m_eTarget, &m_iPrevBind);
  //glGetBooleanv(m_eTarget, &m_bPrevBindEnabled);

  glEnable(m_eTarget);
  glBindTexture(m_eTarget, m_uiId);
  return true;
}

bool Texture::Release()
{
  //glBindTexture(m_eTarget, m_iPrevBind);
  //if (!m_bPrevBindEnabled)
  glDisable(m_eTarget);
  return true;
}

bool Texture::SetSafeFPParameters()
{
  glTexParameteri(m_eTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(m_eTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(m_eTarget, GL_TEXTURE_PRIORITY, 1); // TODO: expose
  return true;
}

bool Texture::SetSimpleParameters()
{
  glTexParameteri(m_eTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(m_eTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(m_eTarget, GL_TEXTURE_PRIORITY, 1); // TODO: expose
  return true;
}

bool Texture::SetImageParameters()
{
  glTexParameteri(m_eTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(m_eTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(m_eTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(m_eTarget, GL_TEXTURE_PRIORITY, 1); // TODO: expose
  return true;
}

//! @}      //doxygen group
