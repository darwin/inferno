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
// $Id: ini_getline.h,v 1.3 2001/04/18 22:02:09 jsquyres Exp $
//
// FUNCTION: Replacement for std::getline(std::istream&,
// std::string&), which is broken in some releases of the Sun Workshop
// 5.0 compilers.  This is only used when the getline in std::string
// is broken.
//


#ifndef INI_GETLINE_H_
#define INI_GETLINE_H_

// Short-circuit if not needed...

#if INILIB_HAVE_BROKEN_GETLINE
#define INILIB_GETLINE INILIB_getline

#include <iostream>
#include <fstream>
#include <string>

namespace INI {

inline std::iostream&
INILIB_getline(std::iostream &is, std::string &str, char delim)
{
  // clear the string and all...
  str.resize(0);
  int ch;
  int count = 0;

  while(1) {
    ch = is.get();

    if (ch == EOF) {
      if (count == 0)
	is.setstate(std::ios::failbit|std::ios::eofbit);
      else
	is.setstate(std::ios::eofbit);
      
      break;
    }

    ++count;

    if (ch == delim)
      break;

    str += ch;

    // do we have space for another char?  if not, we need to fail...
    if (str.length() == str.npos - 1) {
      is.setstate(std::ios::failbit);
      break;
    }
  }

  return is;
}

}

#else // #if INILIB_HAVE_BROKEN_GETLINE

// Otherwise, the compiler-provided std::getline is ok

#define INILIB_GETLINE getline

#endif // #if INILIB_HAVE_BROKEN_GETLINE

#endif // #ifndef INI_GETLINE_H_
