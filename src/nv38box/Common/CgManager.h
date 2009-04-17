// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Common Common code
@{
\ingroup NV38Box
*/
/*! \file CgManager.h
\brief Declaration of CgManager class.
*/
#ifndef _CGMANAGER_H_
#define _CGMANAGER_H_

#include "ManagerBase.h"

//! Class responsible for managing Cg runtime (singleton).
/*!
    - holds Cg context.
    - holds Cg vertex and fragment profiles.
*/
class CgManager : public ManagerBase
{
public:
  //! Constructor
  CgManager();

  //! Initializes CgManager.
  bool Init();
  //! Releases CgManager.
  bool Done();

  //! Returns Cg context. May be called after Init().
  inline CGcontext GetContext() { return m_hContext; }
  //! Returns Cg vertex profile. May be called after Init().
  inline CGprofile GetVertexProfile() { return m_eVertexProfile; }
  //! Returns Cg fragment profile. May be called after Init().
  inline CGprofile GetFragmentProfile() { return m_eFragmentProfile; }

protected:
  //! Internal function for choosing proper Cg profiles.
  void ChooseProfiles();

  CGcontext m_hContext;               //!< Cg context.
  CGprofile m_eVertexProfile;         //!< Cg vertex profile.
  CGprofile m_eFragmentProfile;       //!< Cg fragment profile.
};

CgManager* GetCgManager();

#endif
//! @}      //doxygen group
