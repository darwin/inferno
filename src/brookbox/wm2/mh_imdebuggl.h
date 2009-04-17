#ifndef __IMDEBUGGL_H__
#define __IMDEBUGGL_H__

//#include <imdebug.h>
//#include <gl/gl.h>

inline int format2components(GLenum format);
inline string format2string(GLenum format);


//-----------------------------------------------------------------------------
// Function     	  : imdebugTexImage
// Description	    : 
//-----------------------------------------------------------------------------
/**
 * @fn imdebugTexImage(GLenum target, GLuint texture, GLint level, GLenum format, char *argstring = NULL)
 * @brief Gets a texture level from OpenGL, and displays it using imdebug.
 * 
 * If argstring is non-NULL, then it is prepended to the string " w=%d h=%d %p"
 * and passed as the imdebug format string.  Otherwise, the correct format
 * ("rgb", "lum", etc.) is determined from the @a format argument, and 
 * prepended to the same string and used as the imdebug format string.
 */ 
inline void imdebugTexImage(GLenum target, GLuint texture, GLenum format = GL_RGB, 
                     GLint level = 0, char *argstring = NULL)
{  
  int w, h;
  int prevTexBind;
  glGetIntegerv(target, &prevTexBind);

  glBindTexture(target, texture);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &h);
  GLubyte *p = new GLubyte[w * h * format2components(format)];
  
  glGetTexImage(target, level, format, GL_UNSIGNED_BYTE, p);
 
  string s = (NULL == argstring) ? format2string(format) : argstring;
  imdebug(s.append(" w=%d h=%d %p").c_str(), w, h, p);
  
  delete [] p;

  glBindTexture(target, prevTexBind);
}

//-----------------------------------------------------------------------------
// Function     	  : imdebugTexImagef
// Description	    : 
//-----------------------------------------------------------------------------
/**
 * @fn imdebugTexImage(GLenum target, GLuint texture, GLint level, GLenum format, char *argstring = NULL)
 * @brief Gets a texture level from OpenGL, and displays it as floats using imdebug.
 * 
 * If argstring is non-NULL, then it is prepended to the string " w=%d h=%d %p"
 * and passed as the imdebug format string.  Otherwise, the correct format
 * ("rgb", "lum", etc.) is determined from the @a format argument, and 
 * prepended to the same string and used as the imdebug format string.
 */ 
inline void imdebugTexImagef(GLenum target, GLuint texture, GLenum format = GL_RGB, 
                      GLint level = 0, char *argstring = NULL)
{  
  int w, h;
  int prevTexBind;
  glGetIntegerv(target, &prevTexBind);

  glBindTexture(target, texture);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &h);
  float *p = new float[w * h * format2components(format)];
  
  glGetTexImage(target, level, format, GL_FLOAT, p);
 
  string s = (NULL == argstring) ? format2string(format) : argstring;
  imdebug(s.append(" b=32f w=%d h=%d %p").c_str(), w, h, p);
  
  delete [] p;

  glBindTexture(target, prevTexBind);
}

inline void imdebugTexImageposf(GLenum target, GLuint texture, GLenum format = GL_RGB, 
                             GLint level = 0, int x=0, int y=0, int w=64, int h=64, char *argstring = NULL)
{  
  int tw, th;
  int prevTexBind;
  glGetIntegerv(target, &prevTexBind);

  glBindTexture(target, texture);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &tw);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &th);
  float *p = new float[w * h * format2components(format)];

//  glGetTexImage(target, level, format, GL_FLOAT, p);
//  glGetTexSubImage2D(target, level, x, y, w, h, format, GL_FLOAT, p);

  string s = (NULL == argstring) ? format2string(format) : argstring;
  imdebug(s.append(" b=32f w=%d h=%d %p").c_str(), w, h, p);

  delete [] p;

  glBindTexture(target, prevTexBind);
}


//-----------------------------------------------------------------------------
// Function     	  : imdebugPixels
// Description	    : 
//-----------------------------------------------------------------------------
/**
 * @fn imdebugPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, char *argstring = NULL)
 * @brief performs a glReadPixels(), and then displays the results in imdebug.
 * 
 * If argstring is non-NULL, then it is prepended to the string " w=%d h=%d %p"
 * and passed as the imdebug format string.  Otherwise, the correct format
 * ("rgb", "lum", etc.) is determined from the @a format argument, and 
 * prepended to the same string and used as the imdebug format string.
 */ 
inline void imdebugPixels(GLint x, GLint y, GLsizei width, GLsizei height, 
                   GLenum format = GL_RGB, char *argstring = NULL)
{
  GLubyte *p = new GLubyte[width * height * format2components(format)];
  glReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, p);
 
  string s = (NULL == argstring) ? format2string(format) : argstring;
  imdebug(s.append(" w=%d h=%d %p").c_str(), width, height, p);

    
  delete [] p;
}

//-----------------------------------------------------------------------------
// Function     	  : imdebugPixels
// Description	    : 
//-----------------------------------------------------------------------------
/**
 * @fn imdebugPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, char *argstring = NULL)
 * @brief performs a glReadPixels(), and then displays the results in imdebug.
 * 
 * If argstring is non-NULL, then it is prepended to the string " w=%d h=%d %p"
 * and passed as the imdebug format string.  Otherwise, the correct format
 * ("rgb", "lum", etc.) is determined from the @a format argument, and 
 * prepended to the same string and used as the imdebug format string.
 */ 
inline void imdebugPixelsf(GLint x, GLint y, GLsizei width, GLsizei height, 
                    GLenum format = GL_RGB, char *argstring = NULL)
{
  float *p = new float[width * height * format2components(format)];
  glReadPixels(x, y, width, height, format, GL_FLOAT, p);
 
  string s = (NULL == argstring) ? format2string(format) : argstring;
  imdebug(s.append(" b=32f w=%d h=%d %p").c_str(), width, height, p);
    
  delete [] p;
}

inline int format2components(GLenum format)
{
  switch(format) 
  {
  case GL_LUMINANCE:
  case GL_INTENSITY:
  case GL_ALPHA:
    return 1;
  	break;
  case GL_LUMINANCE_ALPHA:
    return 2;
  	break;
  case GL_RGB:
  case GL_BGR:
    return 3;
    break;
  case GL_RGBA:
  case GL_BGRA:
  case GL_ABGR_EXT:
    return 4;
    break;
  default:
    return 4;
    break;
  }
}

inline string format2string(GLenum format)
{
  switch(format) 
  {
  case GL_LUMINANCE:
  case GL_INTENSITY:
  case GL_ALPHA:
    return "lum";
  	break;
  case GL_LUMINANCE_ALPHA:
    return "luma";
  	break;
  case GL_RGB:
  case GL_BGR:
    return "rgb";
    break;
  case GL_RGBA:
  case GL_BGRA:
  case GL_ABGR_EXT:
    return "rgba";
    break;
  default:
    return "rgb";
    break;
  }
}
#endif //__IMDEBUGGL_H__