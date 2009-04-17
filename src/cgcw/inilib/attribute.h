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
// $Id: attribute.h,v 1.15 2000/09/03 19:47:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_ATTRIBUTE_H_
#define INI_ATTRIBUTE_H_

#include <string>
#include <stdio.h>
#include <iostream>

#include "Debug.h"

namespace INI {
  // need to pre-declare section.  It's messy, but the only real way
  // to do it, as section needs attribute defined in a bunch of
  // places.
  class section;

  // type of an attribute note that NONE means the attribute has not
  // been assigned a type yet.  Otherwise, it will return one of the
  // other four types.
  enum attr_type {BOOL, DOUBLE, INT, STRING, NONE};  

  class attribute {
    friend class section;

  public:
    attribute();
    attribute(attribute*);
    attribute(const attribute&);
     
    virtual ~attribute();

    virtual attribute& operator=(int);
    virtual attribute& operator=(double);
    virtual attribute& operator=(const std::string&);
    virtual attribute& operator=(bool);
    virtual attribute& operator=(const attribute&);

    virtual operator int() const;
    virtual operator double() const;
    virtual operator std::string() const;
    virtual operator bool() const;

    virtual attribute& operator*=(bool);
    virtual attribute& operator*=(double);
    virtual attribute& operator*=(int);
    virtual attribute& operator*=(const std::string&);
    virtual attribute& operator*=(const attribute&);

    virtual attribute& operator/=(bool);
    virtual attribute& operator/=(double);
    virtual attribute& operator/=(int);
    virtual attribute& operator/=(const std::string&);
    virtual attribute& operator/=(const attribute&);

    virtual attribute& operator%=(bool);
    virtual attribute& operator%=(double);
    virtual attribute& operator%=(int);
    virtual attribute& operator%=(const std::string&);
    virtual attribute& operator%=(const attribute&);

    virtual attribute& operator+=(bool);
    virtual attribute& operator+=(double);
    virtual attribute& operator+=(int);
    virtual attribute& operator+=(const std::string&);
    virtual attribute& operator+=(const attribute&);

    virtual attribute& operator-=(bool);
    virtual attribute& operator-=(double);
    virtual attribute& operator-=(int);
    virtual attribute& operator-=(const std::string&);
    virtual attribute& operator-=(const attribute&);

    virtual attribute& operator++(); // prefix
    virtual attribute& operator--(); 

    virtual attr_type get_type() const;

    inline static void set_precision(const int);
    inline static  int get_precision();


  protected:
    virtual attribute* make_copy() const;

    // We love mutable.  :-) Go see the C++ standard to find out what
    // it does.  If your C++ compiler doesn't support it, go get one
    // that does (while we don't typically recomend g++, the latest
    // versions of g++ [2.95 and above] seem to support it fine).  :-)
    mutable attribute *data;
    Debug debug;
    static int precision;

  private:
  };

  inline void
  attribute::set_precision(const int prec)
  {
    if (prec > 99999) 
      INI::attribute::precision = 99999;
    else
      INI::attribute::precision = prec;
  }


  inline int
  attribute::get_precision()
  {
    return precision;
  }

  // probably there are better places for this, but I don't know
  // where that would be...
  std::ostream& operator<<(std::ostream&, const attribute&);

}

#endif // INI_ATTRIBUTE_ATTRIBUTE_H_
