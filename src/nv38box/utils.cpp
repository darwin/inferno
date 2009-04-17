// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup NV38Box
@{
*/
/*! \file utils.cpp
\brief Implementation of utility functions (logging, miscellaneous, etc.).
*/
#include "base.h"

FILE* log_file = NULL; //!< Logging file handle.
unsigned char log_level = 0; //!< Actual log level for output.
static int log_indent = 0; //!< Actual log indentation level.

int& log_get_indent()
{
  return log_indent;
}

void log_indent_increase()
{
  log_indent+=LOG_INDENT_SPACE;
}

void log_indent_decrease()
{
  log_indent-=LOG_INDENT_SPACE;
}

void log_setlevel(unsigned char level)
{
  log_level = level;
}

bool log_open(char* filename)
{
  log_file = fopen(filename, "wt");
  return log_file!=NULL;
}

bool log_close()
{
  fclose(log_file);
  return true;
}

void log_write(unsigned char level, char* string, ...)
{
  if (level>=log_level)
  {
    char	      buffer[10*1024];
    int         buffer_size = 10*1024;
    va_list	    args;
    char* p = buffer;

    size_t len = strlen(string);
    //if (string[len-1]=='\n')
    {
      for (int i=0; i<log_indent; i++)
      {
        buffer[i] = ' ';
      }
      p = buffer+log_indent;
      buffer_size-=log_indent;
    }

    va_start(args, string);
    _vsnprintf(p, buffer_size, string, args);
    va_end(args);

    fputs(buffer, log_file);
    fflush(log_file);
    printf(buffer);
    OutputDebugString(buffer);

    if (level==LOG_LEVEL_ERROR) 
    {
//      assert(0);
      exit(2);
    }
  }
}

void log_write_nd(unsigned char level, char* string, ...)
{
  if (level>=log_level)
  {
    char	      buffer[10*1024];
    int         buffer_size = 10*1024;
    va_list	    args;
    char* p = buffer;

    size_t len = strlen(string);

    va_start(args, string);
    _vsnprintf(p, buffer_size, string, args);
    va_end(args);

    fputs(buffer, log_file);
    fflush(log_file);
    printf(buffer);
    OutputDebugString(buffer);

    if (level==LOG_LEVEL_ERROR) 
    {
      //      assert(0);
      exit(2);
    }
  }
}

/**
* Convert a 4-byte float to a 2-byte half float.
* Based on code from:
* http://www.opengl.org/discussion_boards/ubb/Forum3/HTML/008786.html
*/
GLhalfNV
float_to_half(float val)
{
  const int flt = *((int *) &val);
  const int flt_m = flt & 0x7fffff;
  const int flt_e = (flt >> 23) & 0xff;
  const int flt_s = (flt >> 31) & 0x1;
  int s, e, m = 0;
  GLhalfNV result;

  /* sign bit */
  s = flt_s;

  /* handle special cases */
  if ((flt_e == 0) && (flt_m == 0)) {
    /* zero */
    /* m = 0; - already set */
    e = 0;
  }
  else if ((flt_e == 0) && (flt_m != 0)) {
    /* denorm -- denorm float maps to 0 half */
    /* m = 0; - already set */
    e = 0;
  }
  else if ((flt_e == 0xff) && (flt_m == 0)) {
    /* infinity */
    /* m = 0; - already set */
    e = 31;
  }
  else if ((flt_e == 0xff) && (flt_m != 0)) {
    /* NaN */
    m = 1;
    e = 31;
  }
  else {
    /* regular number */
    const int new_exp = flt_e - 127;
    if (new_exp < -24) {
      /* this maps to 0 */
      /* m = 0; - already set */
      e = 0;
    }
    else if (new_exp < -14) {
      /* this maps to a denorm */
      unsigned int exp_val = (unsigned int) (-14 - new_exp); /* 2^-exp_val*/
      e = 0;
      switch (exp_val) {
            case 0:
              log_write(LOG_LEVEL_ERROR, "float_to_half: logical error in denorm creation!\n");
              /* m = 0; - already set */
              break;
            case 1: m = 512 + (flt_m >> 14); break;
            case 2: m = 256 + (flt_m >> 15); break;
            case 3: m = 128 + (flt_m >> 16); break;
            case 4: m = 64 + (flt_m >> 17); break;
            case 5: m = 32 + (flt_m >> 18); break;
            case 6: m = 16 + (flt_m >> 19); break;
            case 7: m = 8 + (flt_m >> 20); break;
            case 8: m = 4 + (flt_m >> 21); break;
            case 9: m = 2 + (flt_m >> 22); break;
            case 10: m = 1; break;
      }
    }
    else if (new_exp > 15) {
      /* map this value to infinity */
      /* m = 0; - already set */
      e = 31;
    }
    else {
      /* regular */
      e = new_exp + 15;
      m = flt_m >> 13;
    }
  }

  result = (s << 15) | (e << 10) | m;
  return result;
}


/**
* Convert a 2-byte half float to a 4-byte float.
* Based on code from:
* http://www.opengl.org/discussion_boards/ubb/Forum3/HTML/008786.html
*/
float
half_to_float(GLhalfNV val)
{
  /* XXX could also use a 64K-entry lookup table */
  const int m = val & 0x3ff;
  const int e = (val >> 10) & 0x1f;
  const int s = (val >> 15) & 0x1;
  int flt_m, flt_e, flt_s, flt;
  float result;

  /* sign bit */
  flt_s = s;

  /* handle special cases */
  if ((e == 0) && (m == 0)) {
    /* zero */
    flt_m = 0;
    flt_e = 0;
  }
  else if ((e == 0) && (m != 0)) {
    /* denorm -- denorm half will fit in non-denorm single */
    const float half_denorm = 1.0f / 16384.0f; /* 2^-14 */
    float mantissa = ((float) (m)) / 1024.0f;
    float sign = s ? -1.0f : 1.0f;
    return sign * mantissa * half_denorm;
  }
  else if ((e == 31) && (m == 0)) {
    /* infinity */
    flt_e = 0xff;
    flt_m = 0;
  }
  else if ((e == 31) && (m != 0)) {
    /* NaN */
    flt_e = 0xff;
    flt_m = 1;
  }
  else {
    /* regular */
    flt_e = e + 112;
    flt_m = m << 13;
  }

  flt = (flt_s << 31) | (flt_e << 23) | flt_m;
  result = *((float *) &flt);
  return result;
}

float 
gpu_pack(float a, float b)
{
  // I'm stuck here ! This code seems to be right, 
  // but my tests shown differences between GPU conversion and this CPU conversion.
  // Better to do pack_2half on GPU

  unsigned int r;
  GLhalfNV pa = float_to_half(a);
  GLhalfNV pb = float_to_half(b);
  unsigned int ua = pa;
  unsigned int ub = pb;

  r = ua | (ub << 16);
  return *((float *) &r);

/*
  half ha(a);
  half hb(b);
  unsigned int r;
  r = ha.bits() | (hb.bits() << 16);
  return *((float *) &r);
  */
}

//! @}      //doxygen group

