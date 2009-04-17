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
// $Id: attr_attr_comp.h,v 1.5 2000/08/31 17:19:28 bwbarrett Exp $
//
// Operator overloading for attribut-attribute situations, for
// comparison operators.
//

#ifndef INI_ATTR_ATTR_COMP_H_
#define INI_ATTR_ATTR_COMP_H_

#include <string>

#include "Debug.h"
#include "attribute.h"
#include "bool_attribute.h"
#include "double_attribute.h"
#include "int_attribute.h"
#include "string_attribute.h"

namespace INI {
  inline bool
  operator!(const INI::attribute& a)
  {
    return (!(bool) a);
  }


  inline bool
  operator==(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a == (double)b);
    case INI::INT:
      return ((int)a == (int)b);
    case INI::BOOL:
      return ((bool)a == (bool)b);
    default:
      return ((std::string)a == (std::string)b);
    }
  }


  inline bool
  operator<(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a < (double)b);
    case INI::INT:
      return ((int)a < (int)b);
    case INI::BOOL:
      return ((bool)a < (bool)b);
    default:
      return ((std::string)a < (std::string)b);
    }
  }


  inline bool
  operator!=(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a != (double)b);
    case INI::INT:
      return ((int)a != (int)b);
    case INI::BOOL:
      return ((bool)a == (bool)b);
    default:
      return ((std::string)a != (std::string)b);
    }
  }


  inline bool
  operator<=(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a <= (double)b);
    case INI::INT:
      return ((int)a <= (int)b);
    case INI::BOOL:
      return ((bool)a <= (bool)b);
    default:
      return ((std::string)a <= (std::string)b);
    }
  }


  inline bool
  operator>(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a > (double)b);
    case INI::INT:
      return ((int)a > (int)b);
    case INI::BOOL:
      return ((bool)a > (bool)b);
    default:
      return ((std::string)a > (std::string)b);
    }
  }


  inline bool
  operator>=(const INI::attribute& a, const INI::attribute& b)
  {
    switch (promote_to(a, b)) {
    case INI::DOUBLE: 
      return ((double)a >= (double)b);
    case INI::INT:
      return ((int)a >= (int)b);
    case INI::BOOL:
      return ((bool)a >= (bool)b);
    default:
      return ((std::string)a >= (std::string)b);
    }
  }
}

#endif // INI_ATTR_ATTR_COMP_H_
