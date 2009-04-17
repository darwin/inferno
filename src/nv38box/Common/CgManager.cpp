// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Common Common code
@{
\ingroup NV38Box
*/
/*! \file CgManager.cpp
\brief Implementation of CgManager class.
*/
#include "base.h"

CgManager g_kCgManager; //!< global instance of CgManager (singleton).

//! Returns pointer to global Cg manager
CgManager* GetCgManager()
{
  return &g_kCgManager;
}

//! Rutine for handling various Cg errors. Registered in CgManager::Init.
static void HandleCgError() 
{
  const char* err = cgGetErrorString(cgGetError());
  if (err)
    log_write(LOG_LEVEL_ERROR, "Cg error: %s\n", err);
  else
    log_write(LOG_LEVEL_ERROR, "Cg error: unknown error\n");

  exit(1);
}

//! CgManager constructor.
CgManager::CgManager()
{

}

//! Initializes CgManager.
bool 
CgManager::Init()
{
  // Basic Cg setup; register a callback function for any errors
  // and create an initial context
  cgSetErrorCallback(HandleCgError);
  m_hContext = cgCreateContext();

  ChooseProfiles();

  return true;
}

//! Releases CgManager
bool
CgManager::Done()
{
  cgSetErrorCallback(NULL);
  cgDestroyContext(m_hContext);
  return true;
}

//! Selects Cg profiles
void 
CgManager::ChooseProfiles()
{
  if (cgGLIsProfileSupported(CG_PROFILE_VP30))
    m_eVertexProfile = CG_PROFILE_VP30;
  else 
  {
    log_write(LOG_LEVEL_ERROR, "vp30 vertex profile NOT supported on this system.\n");
    exit(1);
  }

  if (cgGLIsProfileSupported(CG_PROFILE_FP30))
    m_eFragmentProfile = CG_PROFILE_FP30;
  else {
    log_write(LOG_LEVEL_ERROR, "fp30 fragment profile NOT supported on this system.\n");
    exit(1);
  }
}

//! @}      //doxygen group
