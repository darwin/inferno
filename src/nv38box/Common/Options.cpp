// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Common Common code
@{
\ingroup NV38Box
*/
/*! \file Options.cpp
\brief Implementation of Options class.
*/
#include "base.h"

#include "Options.h"

Options* g_pkActiveOptions = NULL;

void SetActiveOptions(Options* pkOptions)
{
  g_pkActiveOptions = pkOptions;
}

Options* GetActiveOptions()
{
  return g_pkActiveOptions;
}

void OptionsKeyboardHandler(unsigned char key, int x, int y)
{
  Options* pkOptions = GetActiveOptions();
  if (!pkOptions) return; // safety net

  switch (key)  {
  case 'a':
    pkOptions->m_bAnimate = !pkOptions->m_bAnimate;
    if (!pkOptions->m_bAnimate)
    {
      pkOptions->m_dwStopTime = timeGetTime();
    }
    else
    {
      pkOptions->m_dwStopTimeDelta += timeGetTime() - pkOptions->m_dwStopTime;
    }
    break;
  case 'b':
    pkOptions->m_bBoundingBoxesVisible = !pkOptions->m_bBoundingBoxesVisible;
    break;
  case 'i':
    pkOptions->m_bShowInfo = !pkOptions->m_bShowInfo;
    break;
  case 'h':
    pkOptions->m_bShowHelp = !pkOptions->m_bShowHelp;
    break;
  case 'k':
    pkOptions->m_bTakeScreenshot = true;
    break;
  case 'l':
    pkOptions->m_bTakeStepsShots = true;
    break;
  case 'z':
    if (pkOptions->m_iRenderScheme==0)
      pkOptions->m_iRenderScheme=SCHEME_COUNT;
    pkOptions->m_iRenderScheme--;
    break;
  case 'x':
    pkOptions->m_iRenderScheme++;
    if (pkOptions->m_iRenderScheme==SCHEME_COUNT)
      pkOptions->m_iRenderScheme = 0;
    break;
  }
}

void
ImageFilterMenuHandler(int iCommand)
{
  if (g_pkActiveOptions==NULL) return; // safety net

  g_pkActiveOptions->m_iImageFilter = iCommand;
}

void
BoundingBoxesMenuHandler(int iCommand)
{
  if (g_pkActiveOptions==NULL) return; // safety net

  if (iCommand==BOUNDING_BOXES_TOGGLE)
  {
    g_pkActiveOptions->m_bBoundingBoxesVisible = !g_pkActiveOptions->m_bBoundingBoxesVisible;
  }
  else
    g_pkActiveOptions->m_iBoundingBoxesColor = iCommand;
}

void
RenderModeMenuHandler(int iCommand)
{
  if (g_pkActiveOptions==NULL) return; // safety net

  g_pkActiveOptions->m_iRenderMode = iCommand;
}

void
RenderSchemeMenuHandler(int iCommand)
{
  if (g_pkActiveOptions==NULL) return; // safety net

  g_pkActiveOptions->m_iRenderScheme = iCommand;
}

Options::Options()
{
  LoadDefaults();
}

Options::~Options()
{
}

void 
Options::LoadDefaults()
{
  m_iImageFilter = IMAGE_FILTER_LINEAR;
  m_bBoundingBoxesVisible = false;
  m_iBoundingBoxesColor = BOUNDING_BOXES_COLOR_RED;
  m_iRenderMode = MODE_PP_TEXTURING;
  m_iRenderScheme = SCHEME_A;
  m_bAnimate = true;
  m_bShowInfo = true;
  m_bShowHelp = true;
  m_bTakeScreenshot = false;
  m_bTakeStepsShots = false;
  m_dwStopTime = 0;
  m_dwStopTimeDelta = 0;
}

void 
Options::ApplyImageFilter()
{
  switch(m_iImageFilter) {
  case IMAGE_FILTER_NEAREST:
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    break;
  case IMAGE_FILTER_LINEAR:
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    break;
  default:
    assert(0); // unknown value
  }
}

void 
Options::ApplyBoundingBoxesColor()
{
  switch(m_iBoundingBoxesColor) {
  case BOUNDING_BOXES_COLOR_BLACK:
    glColor3f(0.0f, 0.0f, 0.0f);
    break;
  case BOUNDING_BOXES_COLOR_WHITE:
    glColor3f(1.0f, 1.0f, 1.0f);
    break;
  case BOUNDING_BOXES_COLOR_RED:
    glColor3f(1.0f, 0.0f, 0.0f);
    break;
  case BOUNDING_BOXES_COLOR_GREEN:
    glColor3f(0.0f, 1.0f, 0.0f);
    break;
  case BOUNDING_BOXES_COLOR_BLUE:
    glColor3f(0.0f, 0.0f, 1.0f);
    break;
  default:
    assert(0); // unknown value
  }
}

void 
Options::BuildMenu(int iOptionsMenu)
{
  int iImageFilterMenu = glutCreateMenu(ImageFilterMenuHandler);
  glutAddMenuEntry("    Nearest", IMAGE_FILTER_NEAREST);
  glutAddMenuEntry("    Linear", IMAGE_FILTER_LINEAR);

  int iBoundingBoxesMenu = glutCreateMenu(BoundingBoxesMenuHandler);
  glutAddMenuEntry("[b] Toggle On/Off", BOUNDING_BOXES_TOGGLE);
  glutAddMenuEntry("    Black Color", BOUNDING_BOXES_COLOR_BLACK);
  glutAddMenuEntry("    White Color", BOUNDING_BOXES_COLOR_WHITE);
  glutAddMenuEntry("    Red Color", BOUNDING_BOXES_COLOR_RED);
  glutAddMenuEntry("    Green Color", BOUNDING_BOXES_COLOR_GREEN);
  glutAddMenuEntry("    Blue Color", BOUNDING_BOXES_COLOR_BLUE);

  int iRenderModeMenu = glutCreateMenu(RenderModeMenuHandler);
  glutAddMenuEntry("    Fixed + Flat", MODE_FIXED_FLAT);
  glutAddMenuEntry("    Fixed + Texturing", MODE_FIXED_TEXTURING);
  glutAddMenuEntry("    Per Vertex + Flat", MODE_PV_FLAT);
  glutAddMenuEntry("    Per Vertex + Texturing", MODE_PV_TEXTURING);
  glutAddMenuEntry("    Per Pixel + Flat", MODE_PP_FLAT);
  glutAddMenuEntry("    Per Pixel + Texturing", MODE_PP_TEXTURING);

  int iRenderSchemeMenu = glutCreateMenu(RenderSchemeMenuHandler);
  glutAddMenuEntry("    A", SCHEME_A);
  glutAddMenuEntry("    B", SCHEME_B);
  glutAddMenuEntry("    C", SCHEME_C);
  glutAddMenuEntry("    D", SCHEME_D);
  glutAddMenuEntry("    E", SCHEME_E);
  glutAddMenuEntry("    F", SCHEME_F);
  glutAddMenuEntry("    G", SCHEME_G);
  glutAddMenuEntry("    H", SCHEME_H);
  glutAddMenuEntry("    I", SCHEME_I);

  glutSetMenu(iOptionsMenu);
  glutAddSubMenu("    Image Filter", iImageFilterMenu);
  glutAddSubMenu("    Bounding Boxes", iBoundingBoxesMenu);
  glutAddSubMenu("    Render Mode", iRenderModeMenu);
  glutAddSubMenu("    Render Scheme", iRenderSchemeMenu);
}
//! @}      //doxygen group
