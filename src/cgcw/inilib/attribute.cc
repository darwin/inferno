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
// $Id: attribute.cc,v 1.11 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#include <string>
#include <iostream>

#include "attribute.h"
#include "int_attribute.h"
#include "double_attribute.h"
#include "string_attribute.h"
#include "bool_attribute.h"

// Note: we could have used "using namespace std; using namespace
// INI;" in here, but this file used to be part of a .h file, where
// such things would be a distinct no-no.  Hence, just for consistency
// (and because we're lazy and have 6 billion other details to attend
// to), we kept the std:: and INI:: everywhere.

// instantiate the static variable
int INI::attribute::precision = 3;

INI::attribute::attribute() : data(0), debug(true)
{
  // Don't need nuttin' here, but the default constructor is necessary
}


INI::attribute::attribute(attribute* _data) : debug(true)
{
  data = _data->make_copy();
}


INI::attribute::attribute(const attribute& _attr) : debug(true)
{
  if (_attr.data)
    data = _attr.data->make_copy();
  else
    data = 0;
}


INI::attribute::~attribute() 
{
  debug << "*** Entering attribute::~attribute() ***" << std::endl;
  if (data)
    delete data;
}


INI::attribute&
INI::attribute::operator=(int new_value)
{
  debug << "*** Entering attribute::operator=(int) ***" << std::endl;

  if (data)
    (*data) = new_value;
  else
    data = new int_attribute(new_value);

  return *this;
}


INI::attribute&
INI::attribute::operator=(double new_value)
{
  debug << "*** Entering attribute::operator=(double) ***" << std::endl;

  if (data)
    (*data) = new_value;
  else
    data = new double_attribute(new_value);

  return *this;
}


INI::attribute&
INI::attribute::operator=(const std::string& new_value)
{
  debug << "*** Entering attribute::operator=(std::string&) ***" << std::endl;

  if (data)
    (*data) = new_value;
  else
    data = new string_attribute(new_value);

  return *this;
}


INI::attribute&
INI::attribute::operator=(bool new_value)
{
  debug << "*** Entering attribute::operator=(bool) ***" << std::endl;

  if (data)
    (*data) = new_value;
  else
    data = new bool_attribute(new_value);
    
  return *this;
}


INI::attribute&
INI::attribute::operator=(const INI::attribute& new_value)
{
  debug << "*** Entering attribute::operator=(attribute&) ***"
	<< std::endl;
  if (data)
    delete data;

  switch (new_value.get_type()) {
  case INI::BOOL:
    data = new bool_attribute((bool) new_value);
    break;
  case INI::DOUBLE:
    data = new double_attribute((double) new_value);
    break;
  case INI::INT:
    data = new int_attribute((int) new_value);
    break;
  case INI::STRING:
    data = new string_attribute((std::string) new_value);
    break;
  case INI::NONE:
    break;
  }

  return *this;
}
  


INI::attribute::operator int() const
{
  debug << "*** Entering attribute::operator int() ***" << std::endl;
  if (!data)
    data = new int_attribute(0);

  return (int) (*data);
}


INI::attribute::operator double() const
{
  debug << "*** Entering attribute::operator double() ***" << std::endl;

  if (!data)
    data = new double_attribute(0.0);
  
  return (double) (*data);
}


INI::attribute::operator std::string() const
{
  debug << "*** Entering attribute::operator std::string() ***" 
	<< std::endl;

  if (!data)
    data = new string_attribute("");

  return (std::string) (*data);
}


INI::attribute::operator bool() const
{
  debug << "*** Entering attribute::operator bool() ***" << std::endl;

  if (!data)
    data = new bool_attribute(false);

  return (bool) (*data);
}


INI::attribute&
INI::attribute::operator*=(bool a)
{
  if (!data)
    data = new bool_attribute(false);

  (*data) *= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator*=(double a)
{
  if (!data)
    data = new double_attribute(0.0);

  (*data) *= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator*=(int a)
{
  if (!data)
    data = new int_attribute(0);
  
  (*data) *= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator*=(const std::string& a)
{
  if (!data)
    data = new string_attribute("");
  
  (*data) *= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator*=(const attribute& a)
{
  if (!data)
    switch (a.get_type()) {
    case INI::BOOL:
      data = new bool_attribute(false);
      break;
    case INI::DOUBLE:
      data = new double_attribute(0.0);
      break;
    case INI::INT:
      data = new int_attribute(0);
      break;
    case INI::STRING:
      data = new string_attribute("");
      break;
    case INI::NONE:
      // This is kind of interesting...  Arbitrary decision time
      data = new string_attribute("");
      break;
    }

  (*data) *= a;

  return *this;
}


INI::attribute&
INI::attribute::operator/=(bool a)
{
  if (!data)
    data = new bool_attribute(false);

  (*data) /= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator/=(double a)
{
  if (!data)
    data = new double_attribute(0.0);

  (*data) /= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator/=(int a)
{
  if (!data)
    data = new int_attribute(0);
  
  (*data) /= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator/=(const std::string& a)
{
  if (!data)
    data = new string_attribute("");
  
  (*data) /= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator/=(const attribute& a)
{
  if (!data)
    switch (a.get_type()) {
    case INI::BOOL:
      data = new bool_attribute(false);
      break;
    case INI::DOUBLE:
      data = new double_attribute(0.0);
      break;
    case INI::INT:
      data = new int_attribute(0);
      break;
    case INI::STRING:
      data = new string_attribute("");
      break;
    case INI::NONE:
      // This is kind of interesting...  Arbitrary decision time
      data = new string_attribute("");
      break;
    }

  (*data) /= a;

  return *this;
}


INI::attribute&
INI::attribute::operator%=(bool a)
{
  if (!data)
    data = new bool_attribute(false);

  (*data) %= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator%=(double a)
{
  if (!data)
    data = new double_attribute(0.0);

  (*data) %= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator%=(int a)
{
  if (!data)
    data = new int_attribute(0);
  
  (*data) %= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator%=(const std::string& a)
{
  if (!data)
    data = new string_attribute("");
  
  (*data) %= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator%=(const attribute& a)
{
  if (!data)
    switch (a.get_type()) {
    case INI::BOOL:
      data = new bool_attribute(false);
      break;
    case INI::DOUBLE:
      data = new double_attribute(0.0);
      break;
    case INI::INT:
      data = new int_attribute(0);
      break;
    case INI::STRING:
      data = new string_attribute("");
      break;
    case INI::NONE:
      // This is kind of interesting...  Arbitrary decision time
      data = new string_attribute("");
      break;
    }

  (*data) %= a;

  return *this;
}


INI::attribute&
INI::attribute::operator+=(bool a)
{
  if (!data)
    data = new bool_attribute(false);

  (*data) += a;

  return *this;
}


INI::attribute& 
INI::attribute::operator+=(double a)
{
  if (!data)
    data = new double_attribute(0.0);

  (*data) += a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator+=(int a)
{
  if (!data)
    data = new int_attribute(0);
  
  (*data) += a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator+=(const std::string& a)
{
  if (!data)
    data = new string_attribute("");
  
  (*data) += a;

  return *this;
}


INI::attribute& 
INI::attribute::operator+=(const attribute& a)
{
  if (!data)
    switch (a.get_type()) {
    case INI::BOOL:
      data = new bool_attribute(false);
      break;
    case INI::DOUBLE:
      data = new double_attribute(0.0);
      break;
    case INI::INT:
      data = new int_attribute(0);
      break;
    case INI::STRING:
      data = new string_attribute("");
      break;
    case INI::NONE:
      // This is kind of interesting...  Arbitrary decision time
      data = new string_attribute("");
      break;
    }

  (*data) += a;

  return *this;
}


INI::attribute&
INI::attribute::operator-=(bool a)
{
  if (!data)
    data = new bool_attribute(false);

  (*data) -= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator-=(double a)
{
  if (!data)
    data = new double_attribute(0.0);

  (*data) -= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator-=(int a)
{
  if (!data)
    data = new int_attribute(0);
  
  (*data) -= a;
  
  return *this;
}


INI::attribute&
INI::attribute::operator-=(const std::string& a)
{
  if (!data)
    data = new string_attribute("");
  
  (*data) -= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator-=(const attribute& a)
{
  if (!data)
    switch (a.get_type()) {
    case INI::BOOL:
      data = new bool_attribute(false);
      break;
    case INI::DOUBLE:
      data = new double_attribute(0.0);
      break;
    case INI::INT:
      data = new int_attribute(0);
      break;
    case INI::STRING:
      data = new string_attribute("");
      break;
    case INI::NONE:
      // This is kind of interesting...  Arbitrary decision time
      data = new string_attribute("");
      break;
    }

  (*data) -= a;

  return *this;
}


INI::attribute& 
INI::attribute::operator++() // prefix
{
  if (!data)
    data = new string_attribute("");
  
  ++(*data);

  return *this;
}


INI::attribute&
INI::attribute::operator--()
{
  if (!data)
    (*data) = new string_attribute("");
  
  --(*data);
  
  return *this;
}


INI::attr_type
INI::attribute::get_type() const
{
  if (data)
    return data->get_type();
  else
    return NONE;
}


INI::attribute* 
INI::attribute::make_copy() const
{
  if (!data)
    return new attribute();
  else
    return data->make_copy();
}


std::ostream& 
INI::operator<<(std::ostream& out_stream, 
			      const attribute& thingy)
{
  out_stream << (std::string) thingy;
  
  return out_stream;
}
