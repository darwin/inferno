// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Raytracers
@{
\ingroup NV38Box
*/

/*! \file Raytracer.h
\brief Declaration of Raytracer class.
*/
#ifndef _RAYTRACER_H_
#define _RAYTRACER_H_

#include "Options.h"
#include "Scene.h"
#include "GPUAlgorithm.h"

#include "KernelManager.h"
#include "PBufferManager.h"
#include "TextureManager.h"

#include "nv38box.h"

//! Base class for NV38Box raytracers.
/*! 
    Raytracer = scene + textures + p-buffers + kernels.
    Kernels operate on data in textures and p-buffers.

    Class provides services to support this schema:
    - scene reference
    - management of p-buffers
    - management of textures
    - management of kernels
    - simple hard coded ad-hoc scene animation
    - access to options
    - other properties like eye position or output resolution
*/
class Raytracer : public GPUAlgorithm
{
public:
  //! Raytracer constructor.
  Raytracer();
  //! Raytracer destructor.
  ~Raytracer();

  //! Selects active texture.
  bool SetActiveTexture(int iSlot);
  //! Returns active texture.
  Texture* ActiveTexture();

  //! Selects active kernel.
  bool SetActiveKernel(int iSlot);
  //! Returns active kernel.
  Kernel* ActiveKernel();
  
  //! Selects active PBuffer.
  bool SetActivePBuffer(int iSlot);
  //! Returns active PBuffer.
  PBuffer* ActivePBuffer();

  //! Performs effective switch from previsou PBuffer or main context to new PBuffer.
  /*! \note Use -1 to switch to main context.
  */
  void SwitchToPBuffer(int iSlot);

  //! Sets default OpenGL state in given PBuffer.
  bool SetupPBuffer(PBuffer* pPBuffer);
  //! Sets lighting properties of the OpenGL machine.
  void SetupSceneLighting();
  //! Disables lighting properties of the OpenGL machine.
  void DisableSceneLighting();

  //! Sets z-buffer control of the OpenGL machine.
  void SetupDepthControl(bool bTest, bool bWrite, int iFunction = GL_ALWAYS);

  //! Sets color buffer control of the OpenGL machine.
  void SetupColorControl(bool bWrite);

  //! Sets alpha test control of the OpenGL machine.
  void SetupAlphaControl(bool bTest, int iFunction = GL_ALWAYS, float fValue = 0.0);

  //! Simple hard coded ad-hoc scene animation.
  void Animate(DWORD time);

  //! Displays light gizmo.
  void DisplayLightGizmo();
  //! Displays element pivot point marker.
  void DisplayElementPivot();

  // scene
  SimpleScene* m_pkSimpleScene;            //!< Scene pointer.

  // object containers
  KernelManager m_kKernelManager;          //!< Embeded kernel manager.
  PBufferManager m_kPBufferManager;        //!< Embeded PBuffer manager.
  TextureManager m_kTextureManager;        //!< Embeded texture manager.

  // active objects
  int m_iActiveKernel;                     //!< Active kernel index. 
  int m_iActivePBuffer;                    //!< Active PBuffer index.
  int m_iActiveTexture;                    //!< Active texture index.

  // properties
  GLuint m_uiScreenWidth;                  //!< Output scene X resolution.
  GLuint m_uiScreenHeight;                 //!< Output scene Y resolution.
  Vector3f m_kEyePos;                      //!< Eye position in world space.
  Vector3f m_kFocusPos;                    //!< Focus position in world space.

  // options
  Options m_kOptions;                      //!< Raytracer options.

  bool m_bInitialized;                     //!< Initialized flag.
};

#endif //_RAYTRACER_H_

//! @}      //doxygen group
