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
// $Id: section.h,v 1.9 2000/08/27 19:53:23 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_SECTION_H_
#define INI_SECTION_H_

#include <string>
#include <fstream>
#include <map>

#include "Debug.h"
#include "attribute.h"
#include "bool_attribute.h"
#include "double_attribute.h"
#include "ini_functions.h"
#include "int_attribute.h"
#include "string_attribute.h"

namespace INI {
  // There should be one of these per logical division.  Example:
  // different sections for each window, if you have a good class
  // system, perhaps one per class.
  class section {
  public:
    typedef std::map<std::string, attribute> secMap;
    typedef secMap::iterator iterator;
    typedef secMap::const_iterator const_iterator;
    
    inline section();
    inline ~section();
    inline section(const section& s);

    inline section& operator=(const section& s);
    inline section& operator+=(const section& s);
    inline attribute& operator[](const std::string&);

    inline void insert(const std::string&, int);
    inline void insert(const std::string&, double);
    inline void insert(const std::string&, const std::string&);
    inline void insert(const std::string&, bool);
    inline void insert(const section&);

    inline void clear();
    inline iterator find(const std::string& key);
    inline bool empty();

    // Access to underlying iterator
    inline iterator begin();
    inline iterator end();

  protected:
    secMap attributes;

  private:
    Debug debug;

    inline void copy(const section &s);
  };
}


inline
INI::section::section() : debug(true)
{
  debug << "*** Entering section::section() ***" << std::endl;
  // empty, baby, empty
}


// copy constructor
inline
INI::section::section(const INI::section& s)
  : debug(true)
{
  debug << "*** Entering section::section(const INI::section&) ***"
	<< std::endl;
  
  copy(s);
}


inline
INI::section::~section()
{
  debug << "*** Entering section::~section() ***" << std::endl;

  clear();
}


// assignment operator
inline INI::section&
INI::section::operator=(const INI::section& s)
{
  debug << "*** Entering section::operator=(const INI::section&) ***"
	<< std::endl;

  // this should mirror exactly.  Therefore, should clear first.
  clear();
  copy(s);
  return (*this);
}


inline INI::section&
INI::section::operator+=(const INI::section& s)
{
  debug << "*** Entering section::operator+=(const INI::section&) ***"
	<< std::endl;

  copy(s);
  return (*this);
}


// get a reference to a value from a key value pair.
inline INI::attribute& 
INI::section::operator[](const std::string& key)
{
  debug << "*** Entering  section::operator[](const std::string&)  ***" 
	<< std::endl;

  return attributes[make_lowercase(key)];
}


// next four functions add a key - value pair to the
// "database".  Only diff. is the type of the value.
inline void 
INI::section::insert(const std::string& key, int new_value)
{
  debug << "*** Entering  section::insert(const std::string&, int) ***" 
	<< std::endl;

  attributes[make_lowercase(key)] = new_value;
}


inline void
INI::section::insert(const std::string& key, double new_value)
{
  debug << "*** Entering  section::insert(const std::string&, double) ***" 
	<< std::endl;

  attributes[make_lowercase(key)] = new_value;
}


inline void
INI::section::insert(const std::string& key, const std::string& new_value)
{
  debug << "*** Entering  section::insert" 
	<< "(const std::string&, const std::string&) ***" 
	<< std::endl;

  attributes[make_lowercase(key)] = new_value;
}


inline void
INI::section::insert(const std::string& key, bool new_value)
{
  debug << "*** Entering  section::insert"
	<< "(const std::string&, bool) ***" 
	<< std::endl;

  attributes[make_lowercase(key)] = new_value;
}


// sometimes, we would like to add all the data of a section to the
// current section.  We can't do this with =, as that should erase
// everything already in the section.  This just takes all of section
// s's key-value pairs and adds them.  Note that this WILL overwrite
// any data with the same key already in the db.
inline void
INI::section::insert(const section &s) 
{
  debug << "*** Entering section::insert(const section&) ***" 
	<< std::endl;

  copy(s);
}


inline INI::section::iterator 
INI::section::find(const std::string& key)
{
  debug << "*** Entering section::find(const std::string&) ***" 
	<< std::endl;

  return attributes.find(make_lowercase(key));
}


inline bool
INI::section::empty()
{
  debug << "*** Entering section::empty() ***" << std::endl;

  return attributes.empty();
}


// clear out all pairs in the section.  If this is done
// after the section is added to the registry, you can
// re-use the section instance by clearing and adding
// the next section.
inline void 
INI::section::clear()
{
  debug << "*** Entering section::clear() ***" << std::endl;

  attributes.clear();
}


inline INI::section::iterator 
INI::section::begin() 
{ 
  debug << "*** Entering section::begin() ***" << std::endl;

  return attributes.begin(); 
}


inline INI::section::iterator 
INI::section::end() 
{ 
  debug << "*** Entering section::end() ***" << std::endl;

  return attributes.end(); 
}


// do the work of the copy constructor and assignment operator
inline void
INI::section::copy(const INI::section& s) 
{
  debug << "*** section::copy(const INI::section&) ***" << std::endl;

  const_iterator i;
  for (i = s.attributes.begin(); i != s.attributes.end(); i++) {
    debug << "*** Copying attribute: " << (*i).first << std::endl;
    attributes[make_lowercase( (*i).first)] = (*(*i).second.make_copy());
  }
}

#endif // INI_SECTION_H_
