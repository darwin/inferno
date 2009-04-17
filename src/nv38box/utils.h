// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup NV38Box
@{
*/
/*! \file utils.h
\brief Declaration of utility functions (logging, miscellaneous, etc.).
*/
#ifndef __UTILS_H__
#define __UTILS_H__

//////////////////////////////////////////////////////////////////////////
/*! \name Logging support
\ingroup NV38Box
@{
*/

#define LOG_INDENT_SPACE      2        //!< Default log indentation in spaces.

//! Log levels.
enum ELOG_LEVEL {
  LOG_LEVEL_INFO = 0,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_COUNT
};

//! Returns actual indentation in spaces.
int& log_get_indent();
//! Increases actual indentation.
void log_indent_increase();
//! Decreases actual indentation.
void log_indent_decrease();

//! Sets log level from which are log messages sent to output. 
void log_setlevel(unsigned char level);
//! Opens log file for logging.
bool log_open(char* filename);
//! Closes log file previously opened by log_open().
bool log_close();
//! Writes log message with given log level.
void log_write(unsigned char level, char* string, ...);
//! Writes log message with given log level without indentation.
void log_write_nd(unsigned char level, char* string, ...);

/*! @} */

//////////////////////////////////////////////////////////////////////////
/*! \name Miscellaneous support
\ingroup NV38Box
@{
*/

#define BUFFER_OFFSET(i) ((char *)NULL + (i)*sizeof(float))
#define BUFFER_STRIDE(i) ((i)*sizeof(float))

#define M_PI       3.14159265358979323846

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>=(b)?(a):(b))

typedef unsigned short GLhalfNV; //!< Data type for float16 (known as half in Cg)

GLhalfNV float_to_half(float val);
float half_to_float(GLhalfNV val);
//! Packs two 16-bit halfs into one 32-bit float. Like nVidia Cg function pack_2half().
float gpu_pack(float a, float b);

//! Macro for checking OpenGL errors easily.
#if (_DEBUG || __doxygen)
#define CHECK_OPENGL_ERROR  \
{ \
  GLenum  error; \
  while ( (error = glGetError()) != GL_NO_ERROR) { \
    log_write(LOG_LEVEL_WARNING, "[%s:%d] failed with error %s\n", \
    __FILE__, __LINE__, \
    gluErrorString(error) ); \
  }\
}
#else
#define CHECK_OPENGL_ERROR  
#endif

/*! @} */

#endif

//! @}      //doxygen group
