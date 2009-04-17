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
// $Id: double_attribute.cc,v 1.6 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program


#include "double_attribute.h"
#include "ini_functions.h"

// Note: we could have used "using namespace std; using namespace
// INI;" in here, but this file used to be part of a .h file, where
// such things would be a distinct no-no.  Hence, just for consistency
// (and because we're lazy and have 6 billion other details to attend
// to), we kept the std:: and INI:: everywhere.

INI::double_attribute::double_attribute(double _value) : value(_value)
{
  // Don't need nuttin' here, but the default constructor is necessary
}


// 4 functions for assigning a value where the
// value is supposed to be stored internally as a double.
INI::attribute&
INI::double_attribute::operator=(int new_value)
{
  debug << "*** Entering double_attribute::operator=(int) ***" 
	<< std::endl;

  value = (double) new_value;

  return *this;
}


INI::attribute&
INI::double_attribute::operator=(double new_value)
{
  debug << "*** Entering double_attribute::operator=(double) ***" 
	<< std::endl;

  value = new_value;

  return *this;
}


INI::attribute&
INI::double_attribute::operator=(const std::string& new_value)
{
  debug << "*** Entering double_attribute::operator=(string) ***" 
	<< std::endl;

  value = atof(new_value.c_str());

  return *this;
}


INI::attribute&
INI::double_attribute::operator=(bool new_value)
{
  debug << "*** Entering double_attribute::operator=(bool) ***" 
	<< std::endl;

  value = (double) new_value;

  return *this;

}


INI::attribute&
INI::double_attribute::operator=(const attribute& new_value)
{
  debug << "*** Entering double_attribute::operator=(attribute&) ***" 
	<< std::endl;

  value = (double) new_value;

  return *this;

}


INI::double_attribute::operator int() const
{
  debug << "*** Entering double_attribute::operator int() ***" 
	<< std::endl;

  return ((int) value);
}


INI::double_attribute::operator double() const
{
  debug << "*** Entering double_attribute::operator double() ***" 
	<< std::endl;

  return value;
}


INI::double_attribute::operator std::string() const
{
  debug << "*** Entering double_attribute::operator string() ***" 
	<< std::endl;

  return double2str(value);
}


INI::double_attribute::operator bool() const
{
  debug << "*** Entering double_attribute::operator bool() ***" 
	<< std::endl;

  return ((bool) (value != 0.0));
}


INI::attribute& 
INI::double_attribute::operator*=(bool a)
{
  value = value * (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator*=(double a)
{
  value = value * a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator*=(int a)
{
  value = value * (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator*=(const std::string& a)
{
  value = value * atof(a.c_str());

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator*=(const attribute& a)
{
  value = value * (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator/=(bool a)
{
  value = value * (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator/=(double a)
{
  value = value / a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator/=(int a)
{
  value = value / (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator/=(const std::string& a)
{
  value = value * atof(a.c_str());

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator/=(const attribute& a)
{
  value = value * (double) a;

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator%=(bool a)
{
  value = (double) ((int) value % (int) a);

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator%=(double a)
{
  value = (double) ((int) value % (int) a);

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator%=(int a)
{
  value = (double) ((int) value % (int) a);

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator%=(const std::string& a)
{
  value = (double) ((int) value % (int) atof(a.c_str()));

  return (*this);
}


INI::attribute& 
INI::double_attribute::operator%=(const attribute& a)
{
  value = (double) ((int) value % (int) a);

  return (*this);
}


INI::attribute&
INI::double_attribute::operator+=(bool a)
{
  value = value + (double) a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator+=(double a)
{
  value = value + a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator+=(int a)
{
  value = value + (double) a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator+=(const std::string& a)
{
  value = value + atof(a.c_str());

  return (*this);
}


INI::attribute&
INI::double_attribute::operator+=(const attribute& a)
{
  value = value + (double) a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator-=(bool a)
{
  value = value - (double) a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator-=(double a)
{
  value = value - a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator-=(int a)
{
  value = value - (double) a;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator-=(const std::string& a)
{
  value = value - atof(a.c_str());

  return (*this);
}


INI::attribute&
INI::double_attribute::operator-=(const attribute& a)
{
  value = value - (double) a;

  return (*this);
}


// ++, -- use (double) 1 
// Prefix operators
INI::attribute&
INI::double_attribute::operator++()
{
  value = value + (double) 1;

  return (*this);
}


INI::attribute&
INI::double_attribute::operator--()
{
  value = value - (double) 1;

  return (*this);
}


INI::attr_type
INI::double_attribute::get_type() const
{
  return DOUBLE;
}


INI::attribute* 
INI::double_attribute::make_copy() const
{
  return new double_attribute(value); 
}
