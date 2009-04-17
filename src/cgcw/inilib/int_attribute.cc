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
// $Id: int_attribute.cc,v 1.6 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#include <string>

#include "int_attribute.h"
#include "ini_functions.h"

// Note: we could have used "using namespace std; using namespace
// INI;" in here, but this file used to be part of a .h file, where
// such things would be a distinct no-no.  Hence, just for consistency
// (and because we're lazy and have 6 billion other details to attend
// to), we kept the std:: and INI:: everywhere.

INI::int_attribute::int_attribute(int _value) : value(_value)
{
  // Don't need nuttin' here, but the default constructor is necessary
}


// assignment operators for a value that should be stored
// internally as an integer.  4 functions, based on the 
// type of the value we are assigning from
INI::attribute&
INI::int_attribute::operator=(int new_value) 
{
  debug << "*** Entering int_attribute::operator=(int) ***" << std::endl;

  value = new_value;

  return *this;
}


INI::attribute&
INI::int_attribute::operator=(double new_value)
{
  debug << "*** Entering int_attribute::operator=(double) ***"
	<< std::endl;

  value = (int) new_value;

  return *this;
}


INI::attribute&
INI::int_attribute::operator=(const std::string& new_value)
{
  debug << "*** Entering int_attribute::operator=(string) ***"
	<< std::endl;

  value = atoi(new_value.c_str());

  return *this;
}


INI::attribute&
INI::int_attribute::operator=(bool new_value)
{
  debug << "*** Entering int_attribute::operator=(bool) ***"
	<< std::endl;

  value = (int) new_value;

  return *this;
}


INI::attribute&
INI::int_attribute::operator=(const attribute& new_value)
{
  debug << "*** Entering int_attribute::operator=(attribute&) ***"
	<< std::endl;

  value = (int) new_value;
  
  return *this;
}


// overload the casting operator, so that we can assign the 
// int_attribute to an int, float, string, etc.  4 functions,
// based on the type casting to.
INI::int_attribute::operator int() const
{
  debug << "*** Entering int_attribute::operator int() ***" 
	<< std::endl;

  return value;
}


INI::int_attribute::operator double() const
{
  debug << "*** Entering int_attribute::operator double() ***"
	<< std::endl;

  return ((double) value);
}


INI::int_attribute::operator std::string() const
{
  debug << "*** Entering int_attribute::operator string() ***" 
	<< std::endl;

  return int2str(value);
}


INI::int_attribute::operator bool() const
{
  debug << "*** Entering int_attribute::operator bool() ***" 
	<< std::endl;

  return ((bool) (value != 0));
}


INI::attribute& 
INI::int_attribute::operator*=(bool a)
{
  value = value * (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator*=(double a)
{
  value = value * (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator*=(int a)
{
  value = value * a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator*=(const std::string& a)
{
  value = value * atoi(a.c_str());

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator*=(const attribute& a)
{
  value = value * (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator/=(bool a)
{
  value = value * (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator/=(double a)
{
  value = value / (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator/=(int a)
{
  value = value / a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator/=(const std::string& a)
{
  value = value * atoi(a.c_str());

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator/=(const attribute& a)
{
  value = value * (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator%=(bool a)
{
  value = value % (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator%=(double a)
{
  value = value % (int) a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator%=(int a)
{
  value = value % a;

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator%=(const std::string& a)
{
  value = value % atoi(a.c_str());

  return (*this);
}


INI::attribute& 
INI::int_attribute::operator%=(const attribute& a)
{
  value = value % (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator+=(bool a)
{
  value = value + (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator+=(double a)
{
  value = value + (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator+=(int a)
{
  value = value + a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator+=(const std::string& a)
{
  value = value + atoi(a.c_str());

  return (*this);
}


INI::attribute&
INI::int_attribute::operator+=(const attribute& a)
{
  value = value + (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator-=(bool a)
{
  value = value - (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator-=(double a)
{
  value = value - (int) a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator-=(int a)
{
  value = value - a;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator-=(const std::string& a)
{
  value = value - atoi(a.c_str());

  return (*this);
}


INI::attribute&
INI::int_attribute::operator-=(const attribute& a)
{
  value = value - (int) a;

  return (*this);
}


// ++, -- use (double) 1 

// Prefix operators
INI::attribute&
INI::int_attribute::operator++()
{
  ++value;

  return (*this);
}


INI::attribute&
INI::int_attribute::operator--()
{
  --value;

  return (*this);
}


INI::attr_type
INI::int_attribute::get_type() const
{
  return INT;
}


INI::attribute*
INI::int_attribute::make_copy() const
{
  return new int_attribute(value);
}
