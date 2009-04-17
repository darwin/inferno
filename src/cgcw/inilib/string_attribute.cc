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
// $Id: string_attribute.cc,v 1.8 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#include <string>

#include <stdlib.h>

#include "string_attribute.h"
#include "ini_functions.h"

// Note: we could have used "using namespace std; using namespace
// INI;" in here, but this file used to be part of a .h file, where
// such things would be a distinct no-no.  Hence, just for consistency
// (and because we're lazy and have 6 billion other details to attend
// to), we kept the std:: and INI:: everywhere.

INI::string_attribute::string_attribute(const std::string& _value)
  : value(_value)
{
  // Don't need nuttin' here, but the default constructor is necessary
}


// 4 functions for assigning a value where the
// value is supposed to be stored internally as a strings.
INI::attribute&
INI::string_attribute::operator=(int new_value)
{
  debug << "*** Entering string_attribute::operator=(int) ***" << std::endl;

  value = int2str(new_value);

  return *this;
}


INI::attribute&
INI::string_attribute::operator=(double new_value)
{
  debug << "*** Entering string_attribute::operator=(double) ***" << std::endl;

  value = double2str(new_value);

  return *this;
}


INI::attribute&
INI::string_attribute::operator=(const std::string& new_value)
{
  debug << "*** Entering string_attribute::operator=(string) ***" << std::endl;

  value = new_value;

  return *this;
}


INI::attribute&
INI::string_attribute::operator=(bool new_value)
{
  debug << "*** Entering string_attribute::operator=(bool) ***" << std::endl;

  if (new_value)
    value = "true";
  else
    value = "";
  return *this;
}


INI::attribute&
INI::string_attribute::operator=(const attribute& new_value)
{
  debug << "*** Entering string_attribute::operator=(attribute&) ***" 
	<< std::endl;

  value = (std::string) new_value;

  return (*this);
}


// 4 functions to cast to the correct type for the user...
INI::string_attribute::operator int() const
{
  debug << "*** Entering string_attribute::operator int() ***" << std::endl;

  return atoi(value.c_str());
}


INI::string_attribute::operator double() const
{
  debug << "*** Entering string_attribute::operator double() ***" 
	<< std::endl;

  return atof(value.c_str());
}


INI::string_attribute::operator std::string() const
{
  debug << "*** Entering string_attribute::operator string() ***"
	<< std::endl;

  return value;
}


INI::string_attribute::operator bool() const
{
  debug << "*** Entering string_attribute::operator bool() ***"
	<< std::endl;

  if (value.size() == 0)
    return false;

  return true;
}


INI::attribute& 
INI::string_attribute::operator*=(bool a)
{
  if (!a)
    value = "";

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator*=(double a)
{
  value = double2str(atof(value.c_str()) * a);

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator*=(int a)
{
  value = int2str(atoi(value.c_str()) * a);

  return (*this);
}


// This really makes no sense, so its a noop...
INI::attribute& 
INI::string_attribute::operator*=(const std::string& a)
{
  return (*this);
}


// argh... look at type... perform right thing...
INI::attribute& 
INI::string_attribute::operator*=(const attribute& a)
{
  switch (a.get_type()) {
  case INI::DOUBLE: 
    value = double2str(atof(value.c_str()) * (double) a);
    break;
  case INI::INT:
    value = int2str(atoi(value.c_str()) * (int) a);
    break;
  case INI::BOOL:
    if (!(bool) a)
      value = "";
    break;
  case INI::STRING:
    // same as operator*=(std::string);
    break;
  default:
    break;
  }

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator/=(bool a)
{
  return (*this);
}


INI::attribute& 
INI::string_attribute::operator/=(double a)
{
  value = double2str(atof(value.c_str()) / a);

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator/=(int a)
{
  value = int2str(atoi(value.c_str()) / a);

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator/=(const std::string& a)
{
  return (*this);
}


INI::attribute& 
INI::string_attribute::operator/=(const attribute& a)
{
  switch (a.get_type()) {
  case INI::DOUBLE: 
    value = double2str(atof(value.c_str()) / (double) a);
    break;
  case INI::INT:
    value = int2str(atoi(value.c_str()) / (int) a);
    break;
  case INI::BOOL:
    break;
  case INI::STRING:
    // same as operator*=(std::string);
    break;
  default:
    break;
  }

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator%=(bool a)
{
  if (!a)
    value = "";

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator%=(double a)
{
  value = double2str((double) ((int) atof(value.c_str()) % (int) a));

  return (*this);
}


INI::attribute& 
INI::string_attribute::operator%=(int a)
{
  value = int2str(atoi(value.c_str()) % a);

  return (*this);
}


// This really makes no sense, so its a noop...
INI::attribute& 
INI::string_attribute::operator%=(const std::string& a)
{
  return (*this);
}


// argh... look at type... perform right thing...
INI::attribute& 
INI::string_attribute::operator%=(const attribute& a)
{
  switch (a.get_type()) {
  case INI::DOUBLE: 
    value = double2str((double) ((int) atof(value.c_str()) % (int) a));
    break;
  case INI::INT:
    value = int2str(atoi(value.c_str()) % (int) a);
    break;
  case INI::BOOL:
    if (!(bool) a)
      value = "";
    break;
  case INI::STRING:
    // same as operator%=(std::string);
    break;
  default:
    break;
  }

  return (*this);
}


INI::attribute&
INI::string_attribute::operator+=(bool a)
{
  return (*this);
}


INI::attribute&
INI::string_attribute::operator+=(double a)
{
  value = double2str(atof(value.c_str()) + a);

  return (*this);
}


INI::attribute&
INI::string_attribute::operator+=(int a)
{
  value = int2str(atoi(value.c_str()) + a);

  return (*this);
}


INI::attribute&
INI::string_attribute::operator+=(const std::string& a)
{
  value = value + a;
  return (*this);
}


INI::attribute&
INI::string_attribute::operator+=(const attribute& a)
{
  switch (a.get_type()) {
  case INI::DOUBLE: 
    value = double2str(atof(value.c_str()) + (double) a);
    break;
  case INI::INT:
    value = int2str(atoi(value.c_str()) + (int) a);
    break;
  case INI::BOOL:
    break;
  case INI::STRING:
    value = value + (std::string) a;
    break;
  default:
    break;
  }

  return (*this);
}


INI::attribute&
INI::string_attribute::operator-=(bool a)
{
  if (!a)
    value = "";

  return (*this);
}


INI::attribute&
INI::string_attribute::operator-=(double a)
{
  value = double2str(atof(value.c_str()) - a);

  return (*this);
}


INI::attribute&
INI::string_attribute::operator-=(int a)
{
  value = int2str(atoi(value.c_str()) - a);

  return (*this);
}


INI::attribute&
INI::string_attribute::operator-=(const std::string& a)
{
  return (*this);
}


INI::attribute&
INI::string_attribute::operator-=(const attribute& a)
{
  switch (a.get_type()) {
  case INI::DOUBLE: 
    value = double2str(atof(value.c_str()) - (double) a);
    break;
  case INI::INT:
    value = int2str(atoi(value.c_str()) - (int) a);
    break;
  case INI::BOOL:
    if (! (bool) a)
      value = "";
    break;
  case INI::STRING:
    // same as operator%=(std::string);
    break;
  default:
    break;
  }

  return (*this);
}


// Prefix operators
INI::attribute&
INI::string_attribute::operator++()
{
  if (is_int(value))
    value = int2str(atoi(value.c_str()) + 1);
  else if (is_double(value))
    value = double2str(atof(value.c_str()) + 1);
  
  return (*this);
}


INI::attribute&
INI::string_attribute::operator--()
{
  if (is_int(value))
    value = int2str(atoi(value.c_str()) - 1);
  else if (is_double(value))
    value = double2str(atof(value.c_str()) - 1);
  
  return (*this);
}


INI::attr_type
INI::string_attribute::get_type() const
{
  return STRING;
}


INI::attribute* 
INI::string_attribute::make_copy() const
{
  return new string_attribute(value);
}
