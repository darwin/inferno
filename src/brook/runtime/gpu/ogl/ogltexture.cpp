

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "glext.h"

#include "oglcontext.hpp"

using namespace brook;

/* Allocates a floating point texture */ 
OGLTexture::OGLTexture (unsigned int width,
                        unsigned int height,
                        GPUContext::TextureFormat format,
                        const unsigned int glFormat[4],
                        const unsigned int glType[4],
                        const unsigned int sizeFactor[4]):
   _width(width), _height(height), _format(format) {

   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
      _components = 1;
      break;
   case GPUContext::kTextureFormat_Float2:
      _components = 2;
      break;
   case GPUContext::kTextureFormat_Float3:
      _components = 3;
      break;
   case GPUContext::kTextureFormat_Float4:
      _components = 4;
      break;
   default: 
      GPUError("Unkown Texture Format");
   }

   _bytesize = _width*_height*sizeFactor[_components-1]*sizeof(float);

   _nativeFormat = glFormat[_components-1];

   glGenTextures(1, &_id);
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glBindTexture (GL_TEXTURE_RECTANGLE_NV, _id);
 
   // Create a texture with NULL data
   glTexImage2D (GL_TEXTURE_RECTANGLE_NV, 0, 
                 glType[_components-1],
                 width, height, 0,
                 glFormat[_components-1],
                 GL_FLOAT, NULL);
   
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   CHECK_GL();
}

OGLTexture::~OGLTexture () {
  glDeleteTextures (1, &_id);
  CHECK_GL();
}


bool
OGLTexture::isFastSetPath( unsigned int inStrideBytes, 
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}

bool
OGLTexture::isFastGetPath( unsigned int inStrideBytes, 
                           unsigned int inElemCount ) const {
   return (inStrideBytes == _components*sizeof(float) &&
           inElemCount   == _width*_height);
}


void
OGLTexture::copyToTextureFormat(const float *src, 
                                unsigned int srcStrideBytes, 
                                unsigned int srcElemCount,
                                float *dst) const {
   unsigned int i;
   
   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float2:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float3:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         *dst++ = *(src+2);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float4:
      for (i=0; i<srcElemCount; i++) {
         *dst++ = *src;
         *dst++ = *(src+1);
         *dst++ = *(src+2);
         *dst++ = *(src+3);
         src = (float *)(((unsigned char *) (src)) + srcStrideBytes);
      }
      break;
   default: 
      GPUError("Unkown Texture Format");
   }
}


void
OGLTexture::copyFromTextureFormat(const float *src, 
                                  unsigned int dstStrideBytes, 
                                  unsigned int dstElemCount,
                                  float *dst) const {
   unsigned int i;
   
   switch (_format) {
   case GPUContext::kTextureFormat_Float1:
      for (i=0; i<dstElemCount; i++) {
         *dst = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float2:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float3:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         *(dst+2) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   case GPUContext::kTextureFormat_Float4:
      for (i=0; i<dstElemCount; i++) {
         *dst     = *src++;
         *(dst+1) = *src++;
         *(dst+2) = *src++;
         *(dst+3) = *src++;
         dst = (float *)(((unsigned char *) (dst)) + dstStrideBytes);
      }
      break;
   default: 
      GPUError("Unknown Texture Format");
   }
}

