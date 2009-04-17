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
// $Id: ini_functions.h,v 1.13 2000/12/19 18:21:17 bwbarrett Exp $
//
// FUNCTION: Parse initialization files,
//           maintain state of program
//           save state of program

#ifndef INI_FUNCTIONS_H_
#define INI_FUNCTIONS_H_

#include <string>
#include <ctype.h>

#include "Debug.h"
#include "attribute.h"

// Note to any user who actually ends up reading this file:
//
// You should not be using any of these functions.  They are simply
// minor helper functions, and are here for our use only -- they are
// subject to change without notice.

namespace INI {

  // internally, all keys are stored in lowercase, just for the sake
  // of consistency.  A quick helper function to convert a string to 
  // lower case.

  inline std::string make_lowercase(const std::string& input_string) 
  {
    Debug debug(false);

    std::string str = input_string;
    debug << "*** Entering make_lowercase ***" << std::endl;
    
    // iterate through all the characters.  If uppercase,
    // convert to lowercase.
    for (std::string::size_type i = 0 ; i < str.length() ; ++i)
      if (isupper(str[i]) )
	str[i] |= 32;

    return str;
  }

  inline void crop(std::string& parse_string) 
  {
    Debug debug(false);

    debug << "*** Crop Start: " << parse_string << std::endl;

    std::string::size_type start = parse_string.find_first_not_of(" \t\n");
    std::string::size_type npos = 
      parse_string.find_last_not_of(" \t\n") - start + 1;

    if (npos == std::string::npos || start == std::string::npos) {
      parse_string = "";
    } else {
      std::string tmp = parse_string.substr(start, npos);
      
      parse_string = tmp;
    }
  }

  inline int find_length(int) 
  {
    // for now, we will just use a over-done assumption...  Base2
    // always requires more chars to represent than Base10
    return 8 * sizeof(int);
  }

  inline int find_length(double input) 
  {
    int value = 0;

    while (input > 1) {
      input = input / 10;
      value++;
    }
    
    // make room for the decimal part and all that ... and some
    // gratuitious extra space.  It's temporary space, anyway -- it
    // almost always will be deleted soon.
    value += (attribute::get_precision() + 16);
    return value;
  }

  inline attr_type promote_to(const attribute& a, const attribute& b)
  {
    if ((a.get_type() == DOUBLE) || (b.get_type() == DOUBLE))
      return DOUBLE;
    else if ((a.get_type() == INT) || (b.get_type() == INT))
      return INT;
    else if ((a.get_type() == BOOL) || (b.get_type() == BOOL))
      return BOOL;
    else
      return STRING;
  }


  inline bool is_int(std::string value)
  {
    crop(value);
    if (value.size() == 0) 
      return false;

    if (value.find_first_not_of("1234567890") < value.size())
      return false;

    return true;
  }

  inline bool is_double(std::string value)
  {
    std::string::size_type mypos;
    
    crop(value);
    
    if (value.size() == 0)
      return false;

    mypos = value.find_first_not_of("1234567890");

    if (mypos >= value.size())
      return false;

    if (value[mypos] != '.')
      return false;

    if (value.find_first_not_of("1234567890", mypos + 1) < value.size())
      return false;
      
    return true;
  }

  inline std::string double2str(double a)
  {
    std::string value;

    // We hate the use of sprintf but, but some compilers don't have
    // snprintf!!  #$%#@%$#@%#!!!  Not to worry'; we tremendously
    // overestimate the size of the buffer.

    // This evilness is all the fault of g++ for not having string
    // streams yet.  #$%@#%$@#%$@#%$@#$%@#%$@#%$@!!!!
    char *format = new char[16];
    sprintf(format, "%%.%df", attribute::get_precision());
    
    int length = find_length(a);
    char *char_string = new char[length];
    
    sprintf(char_string, format, a);
    value = char_string;
    
    delete[] char_string;
    delete[] format;

    return value;
  }

  inline std::string int2str(int a)
  {
    std::string value;

    int length = find_length(a);

    // See disclaimers in double2str, above.
    char *char_string;
    char_string = new char[length];
    sprintf(char_string, "%d", a);
    value = char_string;
    
    delete[] char_string;
    
    return value;
  }
}

#endif // INI_FUNCTIONS_H_
