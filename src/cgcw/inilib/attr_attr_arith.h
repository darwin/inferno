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
// $Id: attr_attr_arith.h,v 1.6 2000/08/31 17:19:28 bwbarrett Exp $
//
// Operator overloading for attribut-attribute situations, for
// arithmetic operators.
//

#ifndef INI_ATTR_ATTR_ARITH_H_
#define INI_ATTR_ATTR_ARITH_H_

#include <string>

#include "Debug.h"
#include "attribute.h"
#include "bool_attribute.h"
#include "double_attribute.h"
#include "int_attribute.h"
#include "string_attribute.h"

namespace INI {
  inline INI::attribute
  operator-(const INI::attribute& a)
  {
    INI::attribute tmp;
    
    switch (a.get_type()) {
    case INI::DOUBLE:
      tmp = -((double) a);
      break;
    case INI::INT:
      tmp = -((int) a);
      break;
    case INI::BOOL:
      if ((bool) a)
	tmp = false;
      else
	tmp = true;
      break;
    case INI::STRING:
      if (is_int((std::string) a))
	tmp = -((int) a);
      else if (is_double((std::string) a))
	tmp = -((double) a);
      else
	tmp = (std::string) a;
      break;
    default:
      break;
    }

    return tmp;
  }


  inline INI::attribute
  operator*(const INI::attribute& a, const INI::attribute& b)
  {
    INI::attribute tmp;
    switch (promote_to(a, b)) {
    case INI::DOUBLE:
      tmp = (double) a;
      tmp *= (double) b;
      break;
    case INI::INT:
      tmp = (int) a;
      tmp *= (int) b;
      break;
    case INI::BOOL:
      tmp = (bool) a;
      tmp *= (bool) b;
      break;
    case INI::STRING:
      tmp = (std::string) a;
      tmp *= (std::string) b;
      break;
    default:
      break;
    }
    
    return tmp;
  }


  inline INI::attribute
  operator/(const INI::attribute& a, const INI::attribute& b)
  {
    INI::attribute tmp;
    switch (promote_to(a, b)) {
    case INI::DOUBLE:
      tmp = (double) a;
      tmp /= (double) b;
      break;
    case INI::INT:
      tmp = (int) a;
      tmp /= (int) b;
      break;
    case INI::BOOL:
      tmp = (bool) a;
      tmp /= (bool) b;
      break;
    case INI::STRING:
      tmp = (std::string) a;
      tmp /= (std::string) b;
      break;
    default:
      break;
    }
    
    return tmp;
  }


  inline INI::attribute
  operator%(const INI::attribute& a, const INI::attribute& b)
  {
    INI::attribute tmp;
    switch (promote_to(a, b)) {
    case INI::DOUBLE:
      tmp = (double) a;
      tmp %= (double) b;
      break;
    case INI::INT:
      tmp = (int) a;
      tmp %= (int) b;
      break;
    case INI::BOOL:
      tmp = (bool) a;
      tmp %= (bool) b;
      break;
    case INI::STRING:
      tmp = (std::string) a;
      tmp %= (std::string) b;
      break;
    default:
      break;
    }

    return tmp;
  }


  inline INI::attribute
  operator+(const INI::attribute& a, const INI::attribute& b)
  {
    INI::attribute tmp;
    switch (promote_to(a, b)) {
    case INI::DOUBLE:
      tmp = (double) a;
      tmp += (double) b;
      break;
    case INI::INT:
      tmp = (int) a;
      tmp += (int) b;
      break;
    case INI::BOOL:
      tmp = (bool) a;
      tmp += (bool) b;
      break;
    case INI::STRING:
      tmp = (std::string) a;
      tmp += (std::string) b;
      break;
    default:
      break;
    }

    return tmp;
  }


  inline INI::attribute
  operator-(const INI::attribute& a, const INI::attribute& b)
  {
    INI::attribute tmp;
    switch (promote_to(a, b)) {
    case INI::DOUBLE:
      tmp = (double) a;
      tmp -= (double) b;
      break;
    case INI::INT:
      tmp = (int) a;
      tmp -= (int) b;
      break;
    case INI::BOOL:
      tmp = (bool) a;
      tmp -= (bool) b;
      break;
    case INI::STRING:
      tmp = (std::string) a;
      tmp -= (std::string) b;
      break;
    default:
      break;
    }
    
    return tmp;
  }
}

#endif // INI_ATTR_ATTR_ARITH_H_
