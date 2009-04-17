// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>
#include <stdio.h>

#include "stl_ext.h"

#define WML_USE_WGL 1
#define WIN32 1
#include "WmlGraphics.h"
#include "WmlCommand.h"

using namespace Wml;

// TODO: reference additional headers your program requires here
#include "imdebug.h"
#include <brook.hpp>
#include "lib.h"

extern "C"
{
  int triBoxOverlap(float boxcenter[3], float boxhalfsize[3], float triverts[3][3]);
}

