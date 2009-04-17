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
// $Id: bool_attribute.cc,v 1.6 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#include <string>
#include "Debug.h"

#include "bool_attribute.h"

// Note: we could have used "using namespace std; using namespace
// INI;" in here, but this file used to be part of a .h file, where
// such things would be a distinct no-no.  Hence, just for consistency
// (and because we're lazy and have 6 billion other details to attend
// to), we kept the std:: and INI:: everywhere.

INI::bool_attribute::bool_attribute(bool _value) : value(_value)
{
  // Don't need nuttin' here, but the default constructor is necessary
}


// 4 functions for assigning a value where the
// value is supposed to be stored internally as a bools.
INI::attribute&
INI::bool_attribute::operator=(int new_value)
{
  debug << "*** Entering boolattribute::operator=(int) ***" << std::endl;

  value = (bool) (new_value != 0);

  return *this;
}


INI::attribute&
INI::bool_attribute::operator=(double new_value)
{
  debug << "*** Entering boolattribute::operator=(double) ***" << std::endl;

  value = (bool) (new_value != 0.0);

  return *this;
}


INI::attribute&
INI::bool_attribute::operator=(const std::string& new_value)
{
  debug << "*** Entering boolattribute::operator=(string) ***" << std::endl;

  value = false;

  if (make_lowercase(new_value) == "true")
    value = true;

  return *this;
}


INI::attribute&
INI::bool_attribute::operator=(bool new_value)
{
  debug << "*** Entering boolattribute::operator=(bool) ***" << std::endl;

  value = new_value;

  return *this;
}


INI::attribute&
INI::bool_attribute::operator=(const attribute& new_value)
{
  debug << "*** Entering bool_attribute::operator=(attribute&) ***"
	<< std::endl;

  value = (bool) new_value;

  return *this;
}


INI::bool_attribute::operator int() const
{
  debug << "*** Entering boolattribute::operator int() ***" << std::endl;

  return (int) value;
}


INI::bool_attribute::operator double() const
{
  debug << "*** Entering boolattribute::operator double() ***" << std::endl;

  return (double) value;
}


INI::bool_attribute::operator std::string() const
{
  debug << "*** Entering boolattribute::operator string() ***" << std::endl;

  std::string tmp = "false";
  if (value)
    tmp = "true";

  return tmp;
}


INI::bool_attribute::operator bool() const
{
  debug << "*** Entering boolattribute::operator bool() ***" << std::endl;

  return value;
}


// * == && because * was AND in my Logic Design class
INI::attribute& 
INI::bool_attribute::operator*=(bool a)
{
  value = value && a;
  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator*=(double a)
{
  if (a == 0.0) 
    value = false;
  
  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator*=(int a)
{
  value = value && (bool) a;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator*=(const std::string& a)
{
  if (a.size() == 0)
    value = false;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator*=(const attribute& a)
{
  value = value && (bool)a;

  return (*this);
}


// / is || because it is the oposit of *
INI::attribute& 
INI::bool_attribute::operator/=(bool a)
{
  value = value || a;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator/=(double a)
{
  if (a != 0.0)
    value = true;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator/=(int a)
{
  value = value || (bool)a;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator/=(const std::string& a)
{
  if (a.size() != 0)
    value = true;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator/=(const attribute& a)
{
  value = value || (bool)a;

  return (*this);
}


// % is && because I said so.
INI::attribute& 
INI::bool_attribute::operator%=(bool a)
{
  value = value && a;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator%=(double a)
{
  if (a == 0.0)
    value = false;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator%=(int a)
{
  value = value && (bool)a;
  
  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator%=(const std::string& a)
{
  if (a.size() == 0)
    value = false;

  return (*this);
}


INI::attribute& 
INI::bool_attribute::operator%=(const attribute& a)
{
  value = value && (bool)a;

  return (*this);
}


// + is || because + was OR in my Logic Design class
INI::attribute&
INI::bool_attribute::operator+=(bool a)
{
  value = value || a;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator+=(double a)
{
  if (a != 0.0)
    value = true;
  
  return (*this);
}


INI::attribute&
INI::bool_attribute::operator+=(int a)
{
  value = value || (bool)a;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator+=(const std::string& a)
{
  if (a.size() != 0)
    value = true;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator+=(const attribute& a)
{
  value = value || (bool)a;

  return (*this);
}


// - is && because its the oposite of +, which is ||
INI::attribute&
INI::bool_attribute::operator-=(bool a)
{
  value = value && a;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator-=(double a)
{
  if (a == 0.0)
    value = false;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator-=(int a)
{
  value = value && (bool)a;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator-=(const std::string& a)
{
  if (a.size() == 0)
    value = false;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator-=(const attribute& a)
{
  value = value && (bool)a;

  return (*this);
}


// ++ -> true.  Always.
// -- -> false.  Always.
// Prefix operators
INI::attribute&
INI::bool_attribute::operator++()
{
  value = true;

  return (*this);
}


INI::attribute&
INI::bool_attribute::operator--()
{
  value = false;

  return (*this);
}


INI::attr_type
INI::bool_attribute::get_type() const
{
  return BOOL;
}


INI::attribute* 
INI::bool_attribute::make_copy() const
{
  return new bool_attribute(value);
}
