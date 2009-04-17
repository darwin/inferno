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
// $Id: inilib.h,v 1.7 2001/04/18 21:53:22 jsquyres Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_PARSE_H_
#define INI_PARSE_H_

//
// If we aren't building inilib,

#ifndef INI_BUILDING
#define INI_BUILDING 0
#endif

//
// Now get to the good stuff
//

#if INI_BUILDING

#include "ini_config.h"

#include "attribute.h"         // Base class for attributes
#include "bool_attribute.h"    // bool attribute class
#include "double_attribute.h"  // double attribute class
#include "int_attribute.h"     // int attribute class
#include "string_attribute.h"  // string attribute class

#include "attr_attr_arith.h"   // op overload for attr-attr arithmatic
#include "attr_attr_comp.h"    // op overload for attr-attr comparisons
#include "attr_other_arith.h"  // op overload for attr-other arithmatic
#include "attr_other_comp.h"   // op overload for attr-other comparisons

#include "section.h"
#include "registry.h"       

#include "ini_getline.h"

#else

#include "inilib/ini_config.h"

#include "inilib/attribute.h"         // Base class for attributes
#include "inilib/bool_attribute.h"    // bool attribute class
#include "inilib/double_attribute.h"  // double attribute class
#include "inilib/int_attribute.h"     // int attribute class
#include "inilib/string_attribute.h"  // string attribute class

#include "inilib/attr_attr_arith.h"   // op overload for attr-attr arithmatic
#include "inilib/attr_attr_comp.h"    // op overload for attr-attr comparisons
#include "inilib/attr_other_arith.h"  // op overload for attr-other arithmatic
#include "inilib/attr_other_comp.h"   // op overload for attr-other comparisons

#include "inilib/section.h"
#include "inilib/registry.h"       

#include "inilib/ini_getline.h"

#endif // INI_BUILDING

#endif // INI_PARSE_H_
