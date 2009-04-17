/* src/ini_config.h.  Generated automatically by configure.  */
/* src/ini_config.h.in.  Generated automatically from configure.in by autoheader.  */
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
// $Id: acconfig.h,v 1.3 2000/12/19 18:21:57 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_CONFIG_H_
#define INI_CONFIG_H_


//
// Do we want debugging?
//
#define INI_DEBUG 0

//
// What architecture are we on?
//
#define INI_ARCH "i686-pc-linux-gnu"

//
// Since this #define name is actually created by a variable name (and
// is not hard coded in the .m4 script that set it), autoheader is not
// smart enough to know its real name.  Hence, we have to put it
// outside of the normal autoheader block.
//

//
// Do we have STL or not?
// We don't use this anywhere, but the configure test AC_DEFINE's it
//
#define INILIB_CXX_STL 1

#define INILIB_GETLINE getline

#endif
