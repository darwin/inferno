// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Textures
@{
\ingroup Resource
*/
/*! \file Texture.h
\brief Declaration of Texture class.
*/
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

//! Class representing OpenGL texture.
/*!
  Class handles:
  - texture generation and deletion
  - texture binding and unbinding (releasing)
  - setting predefined sets of texture properties

  <b>NV3x Implementation Details</b><br>
  NV3x GPUs (GeForce FX, etc.) support hardware acceleration for float
  textures with two or more components only when the repeat mode state
  (S and T) is GL_CLAMP_TO_EDGE. If you use either the GL_CLAMP or
  GL_CLAMP_TO_BORDER repeat modes with a float texture with two or
  more components, the software rasterizer is used.
  However, if you use a single-component float texture (GL_FLOAT_R_NV,
  etc.), all clamping repeat modes (GL_CLAMP, GL_CLAMP_TO_EDGE, and
  GL_CLAMP_TO_BORDER) are available with full hardware acceleration.
  The two-, three-, and four-component texture formats all use the
  same amount of texture memory storage (128 bits per texel for the
  GL_FLOAT_x32 formats, and 64 bits per texel for the GL_FLOAT_x16
  formats). Future GPUs will likely store two and three component
  float textures more efficiently.
  The GL_FLOAT_R32_NV and GL_FLOAT_R16_NV texture formats each use 32
  bits per texel. Future GPUs will likely store GL_FLOAT_R16_NV more
  efficiently.
  NVIDIA treats the unsized internal formats GL_FLOAT_R_NV,
  GL_FLOAT_RGBA_NV, etc. the same as GL_FLOAT_R32_NV,
  GL_FLOAT_RGBA32_NV, etc.  

  \note There are several significant limitations on the use of floating-point
  color buffers. First, floating-point color buffers do not support frame
  buffer blending. Second, floating-point texture maps do not support
  mipmapping or any texture filtering other than NEAREST. Third,
  floating-point texture maps must be 2D, and must use the
  NV_texture_rectangle extension.
*/
class Texture 
{
public:
  //! Texture constructor.
  Texture ();
  //! Texture destructor.
  ~Texture ();

  //! Return OpenGL texture ID.
  inline GLuint GetId() { return m_uiId; }

  //! Returns texture OpenGL target.
  inline GLenum GetTarget() { return m_eTarget; }
  //! Sets texture OpenGL target.
  inline void SetTarget(GLenum eTarget) { m_eTarget = eTarget; }

  //! Generates texture in OpenGL.
  bool Generate();
  //! Deletes texture from OpenGL.
  bool Delete();

  //! Binds texture into OpenGL as actual target.
  bool Bind();
  //! Releases bound texture from OpenGL. Previous texture is bound to target.
  bool Release();

  //! Sets safe parameters for floating point texture according to NV3x Implementation Details.
  bool SetSafeFPParameters();
  //! Sets simple texture parameters for GPU data structures. Filtering=NEAREST.
  bool SetSimpleParameters();
  //! Sets parameters for image maps (applied to scene texture).
  bool SetImageParameters();

protected:
  GLuint    m_uiId;                //!< Texture ID.
  GLenum    m_eTarget;             //!< Texture target.
  GLint     m_iPrevBind;           //!< Previous texture ID in "bound mode".
  GLboolean m_bPrevBindEnabled;    //!< Flag for previous texture ID.
};

#endif
//! @}      //doxygen group
