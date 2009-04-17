// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup NV38Box
@{
*/
/*! \file base.h
\brief Support for precompiled headers. Include file for standard system include files.
*/
// base.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(_BASE_H_)
#define _BASE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment(lib, "opengl32.lib" )							// search for openGL32.lib while linking
#pragma comment(lib, "glu32.lib" )								// search for glu32.lib while linking

//#pragma warning(disable: 4786 4355 4512 4267 4312 4311 4267)

// include STD C
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <assert.h>
#include <time.h>   
#include <math.h>   
#include <io.h>

// Windows headers
#include <windows.h>											// header File For Windows

// include Corona library
#include "corona.h"

#ifdef _DEBUG
#  pragma comment(lib, "corona_static.lib" )
//#  pragma comment(lib, "corona_staticd.lib" )
#else
#  pragma comment(lib, "corona_static.lib" )
#endif

/*
//	half -- a 16-bit floating point number class:
#include "half.h"

#ifdef _DEBUG
#  pragma comment(lib, "halfd.lib" )							
#else
#  pragma comment(lib, "half.lib" )							
#endif
*/

// OpenGL includes (using GLEW+GLUT+GLH)
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/glut.h>

//#include <glh/glh_glut.h>
//#ifndef __doxygen // doxygen hack
//using namespace glh;
//#endif

// include nvidia manipulators
#include <nv/nv_manip.h> // use manipulators from nvidia SDK

// include WM2 math library
#define WML_ITEM  
#include "WmlMath.h"
#include "WmlMath.h"
#include "WmlVector2.h"
#include "WmlVector3.h"
#include "WmlVector4.h"
#include "WmlMatrix2.h"
#include "WmlMatrix3.h"
#include "WmlMatrix4.h"
#include "WmlQuaternion.h"
#include "WmlColorRGB.h"
#include "WmlAxisAlignedBox2.h"

#ifndef __doxygen // doxygen hack
using namespace Wml;
#endif

// Cg includes
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include "stl_ext.h"
#ifndef __doxygen // doxygen hack
using namespace std;
#endif

// image debugger includes
#include "mh_imdebuggl.h"

// nv38box global includes
#include "utils.h"
#include "CgManager.h"

// common defines
#ifndef SHADER_DIR
#define SHADER_DIR "Shaders"
#endif

#include "Bin/Shaders/constants.h"
#include "Bin/Shaders/defines.h"

#define SHOW_STRUCTURE 0 // use imdebug to output internal buffers

#endif // !defined(BASE__H)

//! @}      //doxygen group
