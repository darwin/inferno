// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Kernels
@{
\ingroup Resource
*/
/*! \file Kernel.h
\brief Declaration of Kernel class.
*/
#ifndef _KERNEL_H_
#define _KERNEL_H_

typedef vector<CGprogram> TProgramContainer; //!< Holds array of Cg program handles.
typedef vector<CGparameter> TParamsArray;    //!< Holds array of Cg parameter handles.

//! Class used for caching set of common Cg parameters in programs.
class ParamsCache
{
public:
  //! Fills cache with parameters from given vertex program.
  void CacheVertexProgramParams(CGprogram hVP);
  //! Fills cache with parameters from given fragment program.
  void CacheFragmentProgramParams(CGprogram hFP);
  
  //! Releases cache.
  void Release();

  // cached parameters
  CGparameter m_kModelParam;
  CGparameter m_kModelIParam; 
  CGparameter m_kModelITParam;
  CGparameter m_kModelTParam;

  CGparameter m_kModelViewParam;
  CGparameter m_kModelViewIParam;
  CGparameter m_kModelViewITParam;
  CGparameter m_kModelViewTParam;

  CGparameter m_kModelViewProjParam;
  CGparameter m_kModelViewProjIParam;
  CGparameter m_kModelViewProjITParam;
  CGparameter m_kModelViewProjTParam;

  CGparameter m_kProjParam;
  CGparameter m_kProjIParam; 
  CGparameter m_kProjITParam;
  CGparameter m_kProjTParam;

  CGparameter m_kWLightPosParam;
  TParamsArray m_kWLightPosParams;

  CGparameter m_kWEyePosParam;
  CGparameter m_kWFocusPosParam;
  CGparameter m_kWEyeVecParam;
};

typedef vector<ParamsCache> TParamsCacheContainer; //!< Container of parameter caches.

//! Base class representing general program kernel.
/*!
    Kernel class represents one or more pairs of vertex+fragment program.
    If kernel contains more than one pair, actual pair can be switched calling SetMode().
    One kernel usually represents one operation of GPU algorithm and contains
    relative programs.

    Class provides these functionalities for all derived classes:
    - managing vertex and fragment programs (loading, switching)
    - managing parameter caches for common parameters

    \note It is possible to override Activate() method in derived classes. 
    Here can be implemented another scheme of program activation. 
    For example it is possible to enable only one program from pair and let fixed pipeline do the rest.
*/
class Kernel 
{
public:
  //! Kernel constructor.
  Kernel ();
  //! Kerne destructor.
  ~Kernel ();

  //! Returns vertex program active for given mode.
  virtual CGprogram GetVertexProgram(int iIndex) { return m_hVP.at(iIndex); };
  //! Returns fragment program active for given mode.
  virtual CGprogram GetFragmentProgram(int iIndex) { return m_hFP.at(iIndex); };

  //! Reserves slots for programs of given count.
  virtual void ReserveSlots(int iCount) { m_hFP.resize(iCount); m_hVP.resize(iCount); }

  //! Loads vertex program from dist into given slot.
  /*! \note Vertex program is loaded using Cg runtime, but is specified as CG_OBJECT in assembly.
  */
  virtual bool LoadVertexProgram(int iIndex, const char* acFilename);
  //! Loads fragment program from dist into given slot.
  /*! \note Fragment program is loaded using Cg runtime, but is specified as CG_OBJECT in assembly.
  */
  virtual bool LoadFragmentProgram(int iIndex, const char* acFilename);

  //! Sets active mode.
  virtual void SetMode(int iMode) { m_iMode = iMode; }
  //! Gets active mode.
  virtual int GetMode() { return m_iMode; }

  //! Gets active vertex program cache.
  virtual ParamsCache* GetVertexProgramParamsCache() { return &m_kParamsCacheVP[m_iMode]; }
  //! Gets active fragment program cache.
  virtual ParamsCache* GetFragmentProgramParamsCache() { return &m_kParamsCacheFP[m_iMode]; }

  //! Prepares kernel.
  virtual bool Prepare()=0;
  //! Activates kernel in OpenGL.
  virtual void Activate();
  //! Deactivates kernel in OpenGL.
  virtual void Deactivate();

  //! Builds parameter caches for all available programs.
  virtual void CacheParams();
  //! Releases parameter caches of all available programs.
  virtual void ReleaseParams();
  //! Releases all program handles using Cg runtime.
  virtual void DestroyPrograms();
  //! Releases whole kernel.
  virtual void Release();

  int m_iMode;                             //!< Active mode.

  TProgramContainer m_hVP;                 //!< Vertex programs handles.
  TProgramContainer m_hFP;                 //!< Fragment programs handles.

  TParamsCacheContainer m_kParamsCacheVP;  //!< Vertex program caches.
  TParamsCacheContainer m_kParamsCacheFP;  //!< Fragment programs caches.
};

#endif
//! @}      //doxygen group
