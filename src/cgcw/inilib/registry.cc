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
// $Id: registry.cc,v 1.5 2000/12/19 18:21:17 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program


#include <iostream>
#include <fstream>
#include <string>

#include "Debug.h"
#include "ini_getline.h"
#include "ini_functions.h"
#include "registry.h"

using namespace std;
using namespace INI;


bool
registry::file_read(const string& file)
{
  debug << "*** Entering registry::file_read(const string&) : " 
	<< file << " ***" << endl;

  fstream input_file;
  input_file.open(file.c_str(), ios::in);
  if (!input_file)
    return false;

  // a temporary string that will be shreaded to bits
  // throughout this function call.
  string section_name;
  string tmp_parse_string;
  section tmp_section;

  while (input_file.good() &&  input_file.get() != '[')
     debug << "try to find [" << endl;

  while (input_file.good()) {
    INILIB_GETLINE(input_file, section_name, ']');
    crop(section_name);
    if (input_file.good()) {
      section_read(input_file, tmp_section);
      (*this)[section_name] += tmp_section;
      tmp_section.clear();
    }
  }

  return true;
}

bool
registry::file_write(const string& file)
{
  debug << "*** Entering registry::file_write(const string&) ***" << endl;

  fstream output_file(file.c_str(), ios::out);

  for (regMap::iterator i = sections.begin();
       i != sections.end();
       ++i) {
    debug << "Writing section: " << (*i).first << endl;
    output_file << "[" << (*i).first << "]" << endl;

    section_write(output_file, (*i).second);
    output_file << endl;
  }

  return true;
}

void
registry::section_read(fstream &input_file, section& curr_section)
{
  debug << "*** Entering section::file_read(fstream&, section&) ***" 
	<< endl;

  string key_string;
  string value_string;
  char last_char;

  while ( input_file.good() ) {
    // find the next entry.  begin by stripping whitespace
    while (isspace((last_char = input_file.get())))
      debug << "ate whitespace" << endl;

    // See if we hit EOF

    if (!input_file.good())
      break;
    
    // If we didn't hit EOF, check and see what we got

    if (last_char == '[') {
      debug << "Grabbed a [." << endl;
      break;
    } else {
      input_file.putback(last_char);

      INILIB_GETLINE(input_file, key_string, '=');
      crop(key_string);

      INILIB_GETLINE(input_file, value_string, '\n');
      debug << "grabbed: " << value_string << endl;
      crop(value_string);

      debug << "Adding key: -" << key_string << "- value: -" 
	    << value_string << "-" << endl;

      if (is_int(value_string))
	curr_section.insert(key_string, atoi(value_string.c_str()));
      else if (is_double(value_string))
	curr_section.insert(key_string, atof(value_string.c_str()));
      else
	curr_section.insert(key_string, value_string);
    }
  }
}


void
registry::section_write(fstream &output_file, section& curr_section)
{
  debug << "*** Entering section::file_wrtie ***" << endl;

  section::iterator s;

  for (s = curr_section.begin(); s != curr_section.end(); ++s) {
    debug << "Writing attribute: " << (*s).first << endl;
    output_file << (*s).first << " = " 
		<< (string) (*s).second
		<< endl;
  }
}
