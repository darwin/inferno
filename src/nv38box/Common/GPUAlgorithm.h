// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Common Common code
@{
\ingroup NV38Box
*/
/*! \file GPUAlgorithm.h
\brief Declaration of GPUAlgorithm class.
*/
#ifndef _GPUALGORITHM_H_
#define _GPUALGORITHM_H_

//! Base class for GPU algorithm.
class GPUAlgorithm 
{
public:
  //! Constructor.
  GPUAlgorithm ();

  //! Prepares GPU algorithm.
  /*! This function should be called only once in the initialization step.
  */
  virtual bool Prepare() = 0;

  //! Builds or rebuilds data structures for GPU algorithm.
  /*! This function can be more times after initialization.
  */
  virtual bool Build() = 0;

  //! Executes GPU algorithm.
  /*! This function can be called many times after initialization.
  */
  virtual bool Execute() = 0;

  //! Releases GPU algorithm.
  /*! This function can be called once to free algortihm resources.
  */
  virtual bool Shutdown() = 0;
};

#endif
//! @}      //doxygen group
