// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Common Common code
@{
\ingroup NV38Box
*/
/*! \file Options.h
\brief Declaration of Options class.
*/
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

//! Options enum for Image Filters.
enum E_IMAGE {
  IMAGE_FILTER_NEAREST = 0, 
  IMAGE_FILTER_LINEAR
};

//! Options enum for Bounding Boxes.
enum E_BOUNDING_BOXES {
  BOUNDING_BOXES_TOGGLE = 0, 
  BOUNDING_BOXES_COLOR_BLACK, 
  BOUNDING_BOXES_COLOR_WHITE, 
  BOUNDING_BOXES_COLOR_RED, 
  BOUNDING_BOXES_COLOR_GREEN, 
  BOUNDING_BOXES_COLOR_BLUE
};

//! Options enum for Modes.
enum E_MODE {
  MODE_FIXED_FLAT = 0, 
  MODE_FIXED_TEXTURING, 
  MODE_PV_FLAT, 
  MODE_PV_TEXTURING, 
  MODE_PP_FLAT, 
  MODE_PP_TEXTURING
};

//! Options enum for Scheme.
enum E_SCHEME {
  SCHEME_A = 0,  //!< scene drawn using classic rasterization OpenGL = primary rays
  SCHEME_B,      //!< SCHEME_A + building scene data structures on GPU 
  SCHEME_C,      //!< SCHEME_B + computing + shading secondary rays without acceleration structure
  SCHEME_D,      //!< SCHEME_B + computing + shading secondary rays with acceleration structure (NV38 path)
  SCHEME_E,      //!< SCHEME_B + computing + shading secondary rays with acceleration structure (NV40 path)
  SCHEME_F,      //!< Test scheme for early-z test (disabled)
  SCHEME_G,      //!< Test scheme for early-z test (enabled)
  SCHEME_H,      //!< Test scheme for early-stencil test (disabled)
  SCHEME_I,      //!< Test scheme for early-stencil test (enabled)
  SCHEME_COUNT
};


//! Class representing data structure for storing options settings.
/*!
    Class provides:
    - holding options data
    - loading default values
    - building GLUT menu and implementing event handling
    - application of selected features into OpenGL state
*/
class Options
{
public:
  //! Options constructor.
  Options();
  //! Options destructor.
  ~Options();

  //! Resets default values.
  void LoadDefaults();

  //! Builds Options sub-menu using GLUT.
  void BuildMenu(int iOptionsMenu);

  //! Applies image filter settings into OpenGL state. 
  void ApplyImageFilter();
  //! Applies bounding boxes color into OpenGL state.
  void ApplyBoundingBoxesColor();

  // options
  int  m_iImageFilter;                   //!< Image Filter selection
  bool m_bBoundingBoxesVisible;          //!< Bounding Boxes Visible On/Off selection
  int  m_iBoundingBoxesColor;            //!< Bounding Boxes Color selection
  int  m_iRenderMode;                    //!< Render Mode selection
  int  m_iRenderScheme;                  //!< Render Scheme selection
  bool m_bAnimate;                       //!< Animation On/Off selection
  bool m_bShowInfo;                      //!< Show Info On/Off selection
  bool m_bTakeScreenshot;                //!< Screenshot selection
  bool m_bTakeStepsShots;                //!< Screenshots of steps selection
  bool m_bShowHelp;                      //!< Show Help On/Off selection

  // state
  DWORD m_dwStopTime;                    //!< Last animation stop event time in ms
  DWORD m_dwStopTimeDelta;               //!< Delta time of frozen animation (to prevent time skip in Raytracer::Animate)
};

//! Sets active global options to given pointer.
void SetActiveOptions(Options* pkOptions);
//! Returns active global options.
Options* GetActiveOptions();

//! Keyboard handler for options handling (called from nv38box.cpp)
void OptionsKeyboardHandler(unsigned char key, int x, int y);

extern Options* g_pkActiveOptions; //!< Active global options pointer.

#endif
//! @}      //doxygen group
