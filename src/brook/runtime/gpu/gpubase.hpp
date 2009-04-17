// gpubase.hpp
#ifndef GPU_BASE_HPP
#define GPU_BASE_HPP

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <brt.hpp>
#include <kerneldesc.hpp>
#include "../runtime.hpp"

#include "../profiler.hpp"
#include "../logger.hpp"

#define GPUPROFILE( __name ) BROOK_PROFILE( __name )

#define GPULOG( __level ) \
  BROOK_LOG( __level ) << "DX9 - "

#define GPULOGPRINT( __level ) \
  BROOK_LOG_PRINT( __level )

#define GPUWARN \
  std::cerr << "Brook Runtime (gpu) - "

inline void GPUAssertImpl( const char* fileName, 
                           int lineNumber, 
                           const char* comment )
{
  GPUWARN << fileName << "(" << lineNumber << "): " << comment << std::endl;
  assert(false);
  exit(1);
}

#define GPUError( _message ) \
  GPUAssertImpl( __FILE__, __LINE__, _message )

#ifndef BROOK_NDEBUG

#define GPUAssert( _condition, _message ) \
  if(_condition) {} else GPUAssertImpl( __FILE__, __LINE__, _message )

#else

#define GPUAssert( _condition, _message ) \
  do {} while(false)

#endif

namespace brook
{

  class GPURuntime;
  class GPUStream;
  class GPUIterator;
  class GPUKernel;
  class GPUContext;

  struct GPURect
  {
    GPURect() {}
    GPURect( float inLeft, float inTop, float inRight, float inBottom )
      : left(inLeft), top(inTop), right(inRight), bottom(inBottom) {}
    
    operator float*() { return (float*)this; }
      
    operator const float*() const { return (const float*)this; }

    float left, top, right, bottom;
  };

  // A 'fattened' version of the GPURect structure
  // that allows us to specify arbitrary float4
  // values at the corners
  struct GPUFatRect
  {
    GPUFatRect() {}
    GPUFatRect( const GPURect& inRect ) {
      *this = inRect;
    }
    
    const GPUFatRect& operator=( const GPURect& inRect ) {
      for( int i = 0; i < 4; i++ ) {
        int xIndex = (i&0x01) ? 0 : 2;
        int yIndex = (i&0x02) ? 3 : 1;

        vertices[i].x = inRect[xIndex];
        vertices[i].y = inRect[yIndex];
        vertices[i].z = 0.0f;
        vertices[i].w = 1.0f;
      }
      return *this;
    }

    float4 vertices[4];
  };

  struct GPUInterpolant {
    float4 vertices[3];
  };

  struct GPURegion {
    float4 vertices[3];
  };

}



#endif
