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
// $Id: registry.h,v 1.11 2000/09/03 19:46:13 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_REGISTRY_H_
#define INI_REGISTRY_H_

#include <string>
#include <map>
#include <fstream>

#include "ini_functions.h"
#include "section.h"
#include "Debug.h"


namespace INI {
  // registry is the container for the data.  There should be one
  // registry per file you intend to create
  class registry {
  public:
    typedef std::map<std::string, section> regMap;
    typedef regMap::iterator iterator;
    typedef regMap::const_iterator const_iterator;

    inline registry();
    inline registry(const registry&);
    inline registry(const std::string& file, bool read=true, bool write=true);
    inline ~registry();

    inline registry& operator=(const registry& r);
    inline registry& operator+=(const registry& r);
    inline section& operator[](const std::string&);

    inline void insert(const std::string&, const section&);
    inline void insert(const registry&);

    inline void clear();
    inline iterator find(const std::string&);
    inline bool empty();

    // Access to underlying iterator
    inline iterator begin();
    inline iterator end();

    // file access commands.  The second two will call the first two
    bool file_read(const std::string&);
    bool file_write(const std::string&);
    inline bool file_read();
    inline bool file_write();

    inline const std::string get_filename();
    inline void set_filename(const std::string&);

    inline bool get_write_on_destruct();
    inline void set_write_on_destruct(bool);

  protected:
    inline void copy(const registry& r);

    regMap sections;
    bool write_on_destruct;
    std::string filename;

  private:
    Debug debug;

    // "helper" functions, to make the file reading and
    // writing code a little cleaner.  These are responsible for
    // writing or reading a particular section.  They are not
    // protected, because they really, really should not be
    // called from anything but the file_read & file_write
    void section_read(std::fstream&, section&);
    void section_write(std::fstream&, section&);
  };
}


inline
INI::registry::registry() 
  : write_on_destruct(false), debug(true)
{
  debug << "*** Entering registry::registry() ***" << std::endl;

  // doesn't need to do anything....
}


inline 
INI::registry::registry(const registry& r)
  : write_on_destruct(r.write_on_destruct), debug(true)
{
  debug << "*** Entering registry::registry(const registry&) ***" << std::endl;
  copy(r);
}


inline
INI::registry::registry(const std::string& file, 
			bool read, bool write ) 
  : write_on_destruct(write), filename(file), debug(true)
{
  debug << "*** Entering registry::registry(const std::string, bool, bool ***"
	<< std::endl;

  if (read)
    file_read();
}  


inline
INI::registry::~registry()
{
  debug << "*** Entering registry::~registry ***" << std::endl;

  if (write_on_destruct)
    file_write();

  clear();
}


inline INI::registry& 
INI::registry::operator=(const registry& r)
{
  debug << "*** Entering registry::operator= ***" << std::endl;

  clear();
  copy(r);
  write_on_destruct = r.write_on_destruct;
  return (*this);
}


inline INI::registry&
INI::registry::operator+=(const registry& r)
{
  debug << "*** Entering registry::operator+= ***" << std::endl;

  copy(r);
  return (*this);
}


inline INI::section& 
INI::registry::operator[](const std::string& key)
{
  debug << "*** Entering registry::operator[] ***" << std::endl;

  // we don't have to check to see if the section exists, because 
  // if it doesn't, an empty one will be created when we try to 
  // find it...
  return sections[make_lowercase(key)];
}


inline void 
INI::registry::insert(const std::string& key, const section& s)
{
  debug << "*** Entering registry::insert(string&, section&) ***" << std::endl;

  sections[make_lowercase(key)] += s;
}


inline void
INI::registry::insert(const registry& r)
{
  debug << "*** Entering registry::insert(registry&) ***" << std::endl;

  copy(r);
}


inline void
INI::registry::clear()
{
  debug << "*** Entering registry::clear() ***" << std::endl;

  sections.clear();
}


inline INI::registry::iterator
INI::registry::find(const std::string& key)
{
  debug << "*** Entering registry::find(const std::string&) *** " 
	<< std::endl;

  return sections.find(make_lowercase(key));
}


inline bool
INI::registry::empty()
{
  debug << "*** Entering registry::empty() ***" << std::endl;
  
  return sections.empty();
}


inline INI::registry::iterator 
INI::registry::begin() 
{ 
  debug << "*** Entering registry::begin() ***" << std::endl;

  return sections.begin(); 
}


inline INI::registry::iterator 
INI::registry::end() 
{ 
  debug << "*** Entering registry::end() ***" << std::endl;

  return sections.end(); 
}


inline bool
INI::registry::file_write()
{
  debug << "*** Entering registry::file_write() ***" << std::endl;

  if (!filename.empty())
    return file_write(filename);
  else
    return false;
}


inline bool
INI::registry::file_read()
{
  debug << "*** Entering registry::file_read() ***" << std::endl;

  if (!filename.empty())
    return file_read(filename);
  else
    return false;
}


inline const std::string
INI::registry::get_filename()
{
  debug << "*** Entering registry::get_filename() ***" << std::endl;

  return filename;
}


inline void
INI::registry::set_filename(const std::string &file)
{
  debug << "*** Entering registry::set_filename(const std::string&) ***" 
	<< std::endl;

  filename = file;
}


inline bool
INI::registry::get_write_on_destruct()
{
  debug << "*** Entering registry::get_write_on_destruct() ***" << std::endl;

  return write_on_destruct;
}


inline void
INI::registry::set_write_on_destruct(bool value)
{
  debug << "*** Entering registry::get_write_on_destruct(bool) ***" 
	<< std::endl;

  write_on_destruct = value;
}


inline void
INI::registry::copy(const registry &r)
{
  debug << "*** Entering registry::copy(const registry&) ***" 
	<< std::endl;

  const_iterator i;
  for (i = r.sections.begin() ; i != r.sections.end() ; ++i)
    sections[make_lowercase((*i).first)] += (*i).second;
}

#endif // INI_REGISTRY_H_
