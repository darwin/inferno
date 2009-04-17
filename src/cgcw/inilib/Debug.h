// -*- c++ -*-
//
// This software may only be used by you under license from the
// University of Notre Dame.  A copy of the University of Notre Dame's
// Source Code Agreement is available at the inilib Internet website
// having the URL: <http://inilib.sourceforge.net/license/> If you
// received this software without first entering into a license with the
// University of Notre Dame, you have an infringing copy of this software
// and cannot use it without violating the University of Notre Dame's
// intellectual property rights.
//
// INILIB debugging Code.
//
// WRITTEN BY: Jeff Squyres (Thanks Jeff!)
// 

#ifndef INI_DEBUG_H_
#define INI_DEBUG_H_

#include <iostream>
#include "ini_config.h"

namespace INI {

  //
  // Brings printf debugging to a new low
  //
  class Debug {
  public:
    inline Debug(bool initial = false) : on(initial) {}
    
    inline void On() { on = true; }
    inline void Off() { on = false; }
    inline bool isOn() const { return on; }
    inline Debug& flush();
    
  protected:
    bool on;
  };


  //
  // This is icky, but necessary to remove some compiler warnings when
  // we're compiling !INI_DEBUG.  Other than having two prototypes for
  // each definition, there's really no better way to do this... :-)
  //

#if INI_DEBUG
#define ARG1 arg1
#else
#define ARG1
#endif

  
  //
  // A sick and wrong templated debug function
  //
  template <class T>
    inline Debug&
    operator<<(Debug& d, const T& ARG1) {
#if INI_DEBUG
    if (d.isOn())
      std::cerr << ARG1;
#endif
    return d;
  }
  
  
  template <class T>
    inline const Debug&
    operator<<(const Debug& d, const T& ARG1) {
#if INI_DEBUG
    if (d.isOn())
      std::cerr << ARG1;
#endif
    return d;
  }
  
  
  //
  // These need to be here for the endl on some compilers
  //
  inline Debug&
    operator<< (Debug& d, std::ostream& (*)(std::ostream&))
    {
#if INI_DEBUG
      if (d.isOn())
	std::cerr << std::endl;
#endif
      return d;
    }
  
  
  inline const Debug&
    operator<< (const Debug& d, std::ostream& (*)(std::ostream&))
    {
#if INI_DEBUG
      if (d.isOn())
	std::cerr << std::endl;
#endif
      return d;
    }
  
  
  inline Debug& 
    operator<< (Debug& d, std::ios& (*)(std::ios&))
    {
#if INI_DEBUG
      if (d.isOn())
	std::cerr << std::endl;
#endif
      return d;
    }
  
  
  inline const Debug& 
    operator<< (const Debug& d, std::ios& (*)(std::ios&))
    {
#if INI_DEBUG
      if (d.isOn())
	std::cerr << std::endl;
#endif
      return d;
    }
  
  
  inline Debug&
    Debug::flush()
    {
#if INI_DEBUG
      if (isOn())
	std::cerr.flush();
#endif
      return (*this);
    }
  
} // namespace INI

#endif // INI_DEBUG_H_
