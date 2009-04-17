// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

// originally based on code by Bruce "Sinner" Barrera (sinner@opengl.com.br)

/*! \defgroup NV38Box
Application shell.
@{
*/
/*! \file nv38box.cpp
\brief Implementation of main application. Application entry point.
*/

#include "base.h"

#include "Options.h"
#include "RaytracerRForce.h"

// custom defines
#define ONE_SECOND 	 1000	// one second
#define SCENES_DIR "Scenes"
#define MANIPULATOR_ROTATION_SPEED (1)

//! Enum type for mouse buttons.
enum E_BUTTON {
  BUTTON_LEFT = 0,
  BUTTON_RIGHT,
  BUTTON_LEFT_TRANSLATE,
};

//! Method IDs.
enum E_METHOD {
  METHOD_RAYTRACERRFORCE = 0,
  METHOD_COUNT
};

//! Enum for manipulator types.
enum E_MANIPULATOR {
  MANIPULATOR_CAMERA = 0,
  MANIPULATOR_SCENE,
  MANIPULATOR_LIGHT,
  MANIPULATOR_OBJECT,
  MANIPULATOR_COUNT
};

//! String table for manipulator types.
const char* g_acManipulatorTable[MANIPULATOR_COUNT] = {
  "Camera",
  "Scene",
  "Light",
  "Object"
};

//! String table for scheme types.
const char* g_acSchemeNames[SCHEME_COUNT] = {
  "A: classic OpenGL rendering with custom shading",
  "B: scheme A + building acceleration data structure on GPU (AABB)",
  "C: scheme B + brute force raytracing (not using bounding boxes)",
  "D: scheme B + bounding boxes optimization (NV38 path)",
  "E: scheme B + bounding boxes optimization (NV40 path)",
  "F: debug scheme - data dependent early-z test (full)",
  "G: debug scheme - data dependent early-z test (half)",
  "H: debug scheme - data dependent early-stencil test (full)",
  "I: debug scheme - data dependent early-stencil test (half)"
};

typedef vector<SimpleScene> TScenesContainer;
typedef vector<Raytracer*> TRaytracersContainer;

// global variables
TScenesContainer g_akScenes;         //!< Container of available scenes.
TRaytracersContainer g_apRaytracers; //!< Container of available raytracers (methods).
DWORD g_dwStartTime;                 //!< Application start time.
SimpleScene* g_pkActiveScene = NULL; //!< Pointer to active scene.
int g_iActiveManipulator;            //!< Index of active manipulator (see E_MANIPULATOR).
int g_iActiveSubManipulator;         //!< Index of active sub-manipulator (object#, light# etc.).
int g_iActiveCamera;                 //!< Index of active camera.
int g_iActiveMethod;                 //!< Index of active raytracer (or method).

static char g_acFrameRateText[50]    = {0}; //!< Framerate display string. Computed in CalculateFrameRate().

//////////////////////////////////////////////////////////////////////////
/*! \name Window Constants
Defines intial GLUT window placement.
\ingroup NV38Box
@{
*/
// windows size and position constants
const int GL_WIN_WIDTH = 512;
const int GL_WIN_HEIGHT = 512;
const int GL_WIN_INITIAL_X = 0;
const int GL_WIN_INITIAL_Y = 0;
/*! @} */

//////////////////////////////////////////////////////////////////////////
/*! \name Helper functions
These functions can be used outside from nv38box.cpp
\ingroup NV38Box
@{
*/

//! If rotation angle is greater of 360 or lesser than -360, resets it back to zero.
void ClampAngle(float *v)
{
  int i;

  for (i = 0; i < 3; i ++)
    if (v[i] > 360 || v[i] < -360)
      v[i] = 0;
}


//! Returns active method (that means active raytracer instance).
inline Raytracer* GetActiveMethod()
{
  return g_apRaytracers[g_iActiveMethod];
}

//! Switches to new scene.
void SetActiveScene(int iIndex)
{
  if (iIndex>=(int)g_akScenes.size()) return; // safety net
  if (g_pkActiveScene==&g_akScenes[iIndex]) return;

  if (g_pkActiveScene)
  {
    log_write(LOG_LEVEL_INFO, "Shuting down scene '%s'\n", g_pkActiveScene->m_kSceneName.c_str());
    log_indent_increase();
    GetActiveMethod()->Shutdown();
    log_indent_decrease();
  }

  g_pkActiveScene = &g_akScenes[iIndex];

  log_write(LOG_LEVEL_INFO, "Switching to scene '%s'\n", g_pkActiveScene->m_kSceneName.c_str());

  // reset all sub-selections
  g_iActiveSubManipulator = 0;
  g_iActiveCamera = 0;

  GetActiveMethod()->m_pkSimpleScene = g_pkActiveScene;
  log_write(LOG_LEVEL_INFO, "Preparing raytracer ...\n"); 
  log_indent_increase();
  if (!GetActiveMethod()->Prepare())
    log_write(LOG_LEVEL_ERROR, "Error: cannot prepare raytracer\n");
  log_indent_decrease();

  log_write(LOG_LEVEL_INFO, "Building scene ... \n");
  log_indent_increase();
  if (!GetActiveMethod()->Build())
    log_write(LOG_LEVEL_ERROR, "Error: cannot build raytracer\n");
  log_indent_decrease();
  log_write(LOG_LEVEL_INFO, "Here we go ... \n");
}

//! Loads actual camera transformation matrix into OpenGL. To be used by raytracers.
void ApplyCameraTransform()
{
  //  camera.apply_inverse_transform(); 
  glLoadMatrixf(g_pkActiveScene->m_kCameras[g_iActiveCamera].m_kManipulator.get_mat().mat_array);
}

//! Applies actual camera settings into OpenGL. To be used by raytracers.
void ApplyCameraProjection()
{
  nv_manip* pkManip = &g_pkActiveScene->m_kCameras[g_iActiveCamera].m_kManipulator;

  //  camera.apply_inverse_transform(); 
  gluPerspective( pkManip->get_fov(), 
    pkManip->get_screen_ratio(), 
    pkManip->get_near_z(), 
    pkManip->get_far_z());
}

//! Returns actual scene transformation.
void GetSceneTransform(Matrix4f& kM)
{
  memcpy(&kM[0][0], g_pkActiveScene->m_kManipulator.get_mat().mat_array, 16*sizeof(float));
}

//! Returns active camera index.
int GetActiveCamera()
{
  return g_iActiveCamera;
}

//! Returns active light index.
int GetActiveLight()
{
  if (g_iActiveManipulator==MANIPULATOR_LIGHT)
    return g_iActiveSubManipulator;
  else
    return -1;
}

//! Returns active element index.
int GetActiveElement()
{
  if (g_iActiveManipulator==MANIPULATOR_OBJECT)
    return g_iActiveSubManipulator;
  else
    return -1;
}

/*! @} */

//! Searches for given manipulator in active scene data structure.
nv_manip* FindManipulator(int iManipulator, int iSubManipulator)
{
  switch (iManipulator) {
  case MANIPULATOR_CAMERA:
    return &g_pkActiveScene->m_kCameras[iSubManipulator].m_kManipulator;
  case MANIPULATOR_SCENE:
    return &g_pkActiveScene->m_kManipulator;
  case MANIPULATOR_LIGHT:
    return &g_pkActiveScene->m_kLights[iSubManipulator].m_kManipulator;
  case MANIPULATOR_OBJECT:
    return &g_pkActiveScene->m_kElements[iSubManipulator].m_kManipulator;
  }
  return NULL;
}

//! Updates given manipulator in active scene data structure.
void UpdateManipulator(int iManipulator, int iSubManipulator)
{
  switch (iManipulator) {
  case MANIPULATOR_CAMERA:
    g_pkActiveScene->m_kCameras[iSubManipulator].Update();
    break;
  case MANIPULATOR_SCENE:
    g_pkActiveScene->Update();
    break;
  case MANIPULATOR_LIGHT:
    g_pkActiveScene->m_kLights[iSubManipulator].Update();
    break;
  case MANIPULATOR_OBJECT:
    g_pkActiveScene->m_kElements[iSubManipulator].Update();
    break;
  }
}

//! Calculates the current frame rate
void CalculateFrameRate()
{
  static float framesPerSecond    = 0.0f;        
  static DWORD lastTime           = 0;           
  static long frame = 0;
  static long lastFrame = 0;

  static float framesPerSecond2    = 0.0f;        
  static DWORD lastTime2           = 0;           
  static long frame2 = 0;
  static long lastFrame2 = 0;

  // get actual time
  DWORD time = timeGetTime();

  // increase the frame counter
  frame++;

  if (time - lastTime >= 5*ONE_SECOND)
  {
    framesPerSecond = ((float)(frame - lastFrame)) / (time - lastTime);

    lastTime = time;
    lastFrame = frame;
  }

  if (time - lastTime2 >= ONE_SECOND / 4)
  {
    framesPerSecond2 = ((float)(frame - lastFrame2)) / (time - lastTime2);

    _snprintf(g_acFrameRateText, 50, "FPS: %0.2f, avg %0.2f in last 5sec", framesPerSecond2 * 1000, framesPerSecond * 1000);

    lastTime2 = time;
    lastFrame2 = frame;
  }
}

//! Draws text on screen using OpenGL
void DrawText(char *str, void *font, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y) 
{
  // font: font to use, e.g., GLUT_BITMAP_HELVETICA_10
  // r, g, b: text colour
  // x, y: text position in window: range [0,0] (bottom left of window)
  // to [1,1] (top right of window). 

  char *ch;

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, 1.0, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor3f(r, g, b);
  glRasterPos3f(x, y, 0.0);
  for(ch= str; *ch; ch++) {
    glutBitmapCharacter(font, (int)*ch);
  }
  glColor3f(1, 1, 1);
}

//! Draws NV38Box info panel on screen using OpenGL.
void DrawInfo()
{
  int h = GetActiveMethod()->m_uiScreenHeight;
  int w = GetActiveMethod()->m_uiScreenWidth;
  float row = 1.f / (h / 10.f);
  float col = 1.f / (w / 10.f);

  char buf[256];
  glViewport(0,0,GetActiveMethod()->m_uiScreenWidth, GetActiveMethod()->m_uiScreenHeight); // hack

  // draw background quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendColor(0,0,0,0.8);
  glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

  glColor3f(0.5, 0.5, 0.5);
  glBegin(GL_QUADS);
  glVertex2f(0, 1 - 7*row);
  glVertex2f(1, 1 - 7*row);
  glVertex2f(1, 1);
  glVertex2f(0, 1);
  glEnd();

  glDisable(GL_BLEND);

  // draw info panel texts
  DrawText("Method: RForce", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 1*row );
  DrawText(g_acFrameRateText, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 2*row );
  _snprintf(buf, 256, "Time: %.2f", (timeGetTime()-g_dwStartTime)/1000.0f);
  DrawText(buf, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 3*row );
  _snprintf(buf, 256, "Scene: %s", g_pkActiveScene->m_kSceneName.c_str());
  DrawText(buf, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 4*row );
  if (g_iActiveManipulator==MANIPULATOR_SCENE)
  {
    _snprintf(buf, 256, "Selection: %s", g_acManipulatorTable[g_iActiveManipulator]);
  }
  else
  {
    _snprintf(buf, 256, "Selection: %s[%d]", g_acManipulatorTable[g_iActiveManipulator], g_iActiveSubManipulator+1);
  }
  DrawText(buf, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 5*row );
  _snprintf(buf, 256, "Scheme: %s", g_acSchemeNames[GetActiveOptions()->m_iRenderScheme]);
  DrawText(buf, GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 1*col, 1.0 - 6*row );

  DrawText("i - hide", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, 47*col, 1.0 - 1*row );
}

//! Draws NV38Box help panel on screen using OpenGL.
void DrawHelp()
{
  int h = GetActiveMethod()->m_uiScreenHeight;
  int w = GetActiveMethod()->m_uiScreenWidth;
  float row = 1.f / (h / 10.f);
  float col = 1.f / (w / 10.f);
  float colbase = 25*col;
  float rowbase = 37*row;

  glViewport(0,0,GetActiveMethod()->m_uiScreenWidth, GetActiveMethod()->m_uiScreenHeight); // hack

  // draw background quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendColor(0,0,0,0.8);
  glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

  glColor3f(0.5, 0.5, 0.5);
  glBegin(GL_QUADS);
  glVertex2f(colbase - 1* col, 0);
  glVertex2f(1, 0);
  glVertex2f(1, 1 - (rowbase - 1*row));
  glVertex2f(colbase  - 1* col, 1 - (rowbase - 1*row));
  glEnd();

  glDisable(GL_BLEND);

  // draw help panel texts
  DrawText("h - hide", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 22*col, 1.0 - (rowbase + 0*row) );
  DrawText("Use mouse to manipulate object", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 1*row) );
  DrawText("LEFT Mouse = rotation", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 2*row) );
  DrawText("CTRL + LEFT Mouse = move in Z axis (or zoom)", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 3*row) );
  DrawText("SHIFT + LEFT Mouse = move in XY plane", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 4*row) );

  DrawText("Main controls", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 6*row) );
  DrawText("Use C to switch / manipulate camera", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 7*row) );
  DrawText("Use S to manipulate scene", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 8*row) );
  DrawText("Use L to switch / manipulate lights", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 9*row) );
  DrawText("Use O to switch / manipulate objects", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 10*row) );
  
  DrawText("Use 1..9 to change scene", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 1.0f, colbase + 0*col, 1.0 - (rowbase + 12*row) );
  DrawText("Use Z / X to change scheme (raytracing method)", GLUT_BITMAP_HELVETICA_10, 1.0f, 1.0f, 0.0f, colbase + 0*col, 1.0 - (rowbase + 13*row) );
}


//////////////////////////////////////////////////////////////////////////
/*! \name Application specific functions
Initialization and shutdowning of main application.
\ingroup NV38Box
@{
*/

//! Performs shutdown of application and all sub-components.
void Shutdown()
{
  log_write(LOG_LEVEL_INFO, "Shuting down scene '%s'\n", g_pkActiveScene->m_kSceneName.c_str());
  log_indent_increase();

  TRaytracersContainer::iterator i = g_apRaytracers.begin();
  while (i != g_apRaytracers.end())
  {
    (*i)->Shutdown();
    i++;
  }

  log_indent_decrease();

  log_write(LOG_LEVEL_INFO, "Exiting ...\n");
  exit(0);
}

//! Searches for scene files on disk.
void ScanForScenes(const char* acDirectory)
{
  struct _finddata_t c_file;
  long hFile;

  char buf[MAX_PATH];
  _snprintf(buf, MAX_PATH, "%s/*.csv", acDirectory);

  if ((hFile = _findfirst(buf, &c_file)) == -1L)
  {
    log_write(LOG_LEVEL_ERROR, "Error: No scenes found in %s\n", buf);
  }
  else
  {
    do 
    {
      SimpleScene SS;
      _snprintf(buf, MAX_PATH, "%s/%s", acDirectory, c_file.name);
      log_write(LOG_LEVEL_INFO, "Loading scene '%s' ...\n", buf);
      log_indent_increase();
      if (!SS.Load(buf)) 
      {
        log_write(LOG_LEVEL_ERROR, "Error: couldn't load file %s\n", buf);
      }
      log_indent_decrease();
      SS.m_kSceneName = c_file.name;
      SS.m_kSceneName.resize(SS.m_kSceneName.size()-4);
      SS.InitManipulators();
      g_akScenes.push_back(SS);
    }
    while (_findnext( hFile, &c_file ) == 0);

    _findclose(hFile);
  }
}

//! Initializes basic OpenGL machine settings.
void InitializeOGL()
{
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
}

//! Initializes NV38Box application.
/*!
- Initializes GLEW in experimental mode.
- Creates raytracers (currently only RForce is present).
- Scans for scenes.
- Initializes manipulators.
- Other initilizations.
*/
void InitializeBox()
{
  log_open("nv38box.txt");

  log_write(LOG_LEVEL_INFO, "Inferno Project: NV38Box (http://inferno.hildebrand.cz)\n");
  log_write(LOG_LEVEL_INFO, "GL Renderer: %s\n",glGetString(GL_RENDERER));
  log_write(LOG_LEVEL_INFO, "GL Vendor: %s\n",glGetString(GL_VENDOR));
  log_write(LOG_LEVEL_INFO, "GL Version: %s\n",glGetString(GL_VERSION));
  log_write(LOG_LEVEL_INFO, "GLEW Version: %s\n", glewGetString(GLEW_VERSION));
  log_write(LOG_LEVEL_INFO, "Cg Version: %s\n", cgGetString(CG_VERSION));
  log_write(LOG_LEVEL_INFO, "-------------------------------------------------------\n");

  // initialize GLEW
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err!=GLEW_OK)
    // problem: glewInit failed, something is seriously wrong
    log_write(LOG_LEVEL_ERROR, "Error: Couldn't initialize GLEW - %s\n", glewGetErrorString(err));
  else
    log_write(LOG_LEVEL_INFO, "GLEW Initialized\n");

  // initialize raytracers, at the moment we support only RForce
  log_write(LOG_LEVEL_INFO, "Creating raytracers ...\n");
  log_indent_increase();
  g_apRaytracers.resize(METHOD_COUNT);
  g_apRaytracers[METHOD_RAYTRACERRFORCE] = new RaytracerRForce();
  g_iActiveMethod = METHOD_RAYTRACERRFORCE;
  log_indent_decrease();

  // scan for scenes
  log_write(LOG_LEVEL_INFO, "Scaning for scenes ...\n");
  log_indent_increase();
  ScanForScenes(SCENES_DIR);
  log_indent_decrease();

  // initialization of manipulators
  g_iActiveManipulator = MANIPULATOR_CAMERA;
  g_iActiveSubManipulator = 0;
  g_iActiveCamera = 0;

  SetActiveScene(0);
}

/*! @} */

//////////////////////////////////////////////////////////////////////////
/*! \name Screenshot support.
\ingroup NV38Box
@{
*/

//! Takes screenshot of actual frame and saves it on disk.
void TakeScreenShot()
{
  // init dimensions
  unsigned int uiWidth = GetActiveMethod()->m_uiScreenWidth;
  unsigned int uiHeight = GetActiveMethod()->m_uiScreenHeight;

  unsigned int uiSize = uiWidth * uiHeight * (3); // RGB

  // download image
  glReadBuffer(GL_FRONT);
  GLubyte *p = new GLubyte[uiSize];
  glReadPixels(0, 0, uiWidth , uiHeight, GL_RGB, GL_UNSIGNED_BYTE, p);
  corona::Image* pkImage = corona::CreateImage(uiWidth, uiHeight, corona::PF_R8G8B8, p);
  delete[] p;

  // form filename
  struct tm *t;
  time_t v;
  time(&v);
  t = localtime(&v);
  char acFileName[MAX_PATH];
  _snprintf(acFileName, MAX_PATH, "Inferno %04d-%02d-%02d %02d-%02d-%02d.png", t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
  
  // save picture
  corona::FlipImage(pkImage, corona::CA_X);
  corona::SaveImage(acFileName, corona::FF_PNG, pkImage);
  delete pkImage;
}

/*! @} */

//////////////////////////////////////////////////////////////////////////
/*! \name GLUT handlers
Implementation of GLUT callback functions.
\ingroup NV38Box
@{
*/

//! function that handles window resizing
void GLUTResize(int width, int height)
{
  glViewport(0, 0, width, height);
  
  TRaytracersContainer::iterator i = g_apRaytracers.begin();
  while (i != g_apRaytracers.end())
  {
    (*i)->m_uiScreenWidth = width; 
    (*i)->m_uiScreenHeight = height; 
    i++;
  }

  TScenesContainer::iterator sci = g_akScenes.begin();
  while (sci!=g_akScenes.end())
  {
    (*sci).ReshapeManipulators(width, height);
    sci++;
  }
}

//! Function that handles keyboard inputs
void GLUTKeyboard(unsigned char key, int x, int y)
{
  // convert key to lower case
  key = tolower(key);

  // scene selection
  if (key>='1' && key<='9')
  {
    SetActiveScene(key-'1');
  }
  if (key=='0')
  {
    SetActiveScene(9);
  }

  // custom key handlers
  switch (key) {
  case 'q':
  case 27: // ESC
    Shutdown();
    break;
  case 'c':
    if (g_iActiveManipulator==MANIPULATOR_CAMERA)
    {
      g_iActiveSubManipulator++;
      if (g_iActiveSubManipulator>=(int)g_pkActiveScene->m_kCameras.size())
        g_iActiveSubManipulator = 0;
    }
    else
    {
      g_iActiveManipulator = MANIPULATOR_CAMERA;
      g_iActiveSubManipulator = g_iActiveCamera;
    }
    
    g_iActiveCamera = g_iActiveSubManipulator;
    break;
  case 's':
    g_iActiveManipulator = MANIPULATOR_SCENE;
    g_iActiveSubManipulator = 0;
    break;
  case 'l':
    if (g_iActiveManipulator==MANIPULATOR_LIGHT)
    {
      g_iActiveSubManipulator++;
      if (g_iActiveSubManipulator>=(int)g_pkActiveScene->m_kLights.size())
        g_iActiveSubManipulator = 0;
    }
    else
    {
      g_iActiveManipulator = MANIPULATOR_LIGHT;
      g_iActiveSubManipulator = 0;
    }
    break;
  case 'o':
    if (g_iActiveManipulator==MANIPULATOR_OBJECT)
    {
      g_iActiveSubManipulator++;
      if (g_iActiveSubManipulator>=(int)g_pkActiveScene->m_kElements.size())
        g_iActiveSubManipulator = 0;
      if (!g_pkActiveScene->m_kElements[g_iActiveSubManipulator].size())
        g_iActiveSubManipulator = 0;
    }
    else
    {
      g_iActiveManipulator = MANIPULATOR_OBJECT;
      g_iActiveSubManipulator = 0;
    }
    break;
  }

  // call keyboard handler for shared options
  OptionsKeyboardHandler(key, x, y);

  glutPostRedisplay();
}

//! Function that handles mouse movements
void GLUTMotion(int x, int y)
{
  nv_manip* pkActiveManipulator = FindManipulator(g_iActiveManipulator, g_iActiveSubManipulator);
  if (pkActiveManipulator)
  {
    pkActiveManipulator->mouse_move(vec2(x,y),0);
    UpdateManipulator(g_iActiveManipulator, g_iActiveSubManipulator);
  }

  glutPostRedisplay();
}

//! Function that handles mouse clicks
void GLUTMouse(int button, int state, int x, int y)
{
  nv_manip* pkActiveManipulator = FindManipulator(g_iActiveManipulator, g_iActiveSubManipulator);
  if (pkActiveManipulator)
  {
    int input_state = 0;
    // mouse keyboard state mask
    input_state |= (button == GLUT_LEFT_BUTTON) ? 
      ((state == GLUT_DOWN) ? nv_manip::LMOUSE_DN : nv_manip::LMOUSE_UP) : 0;
    input_state |= (button == GLUT_MIDDLE_BUTTON) ? 
      ((state == GLUT_DOWN) ? nv_manip::MMOUSE_DN : nv_manip::MMOUSE_UP) : 0;
    input_state |= (button == GLUT_RIGHT_BUTTON) ? 
      ((state == GLUT_DOWN) ? nv_manip::RMOUSE_DN : nv_manip::RMOUSE_UP) : 0;
    // build keyboard state mask
    int key_state = glutGetModifiers();
    input_state |= (key_state & GLUT_ACTIVE_CTRL) ? nv_manip::CTRL_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_ALT) ? nv_manip::ALT_DN : 0;
    input_state |= (key_state & GLUT_ACTIVE_SHIFT) ? nv_manip::SHIFT_DN : 0;

    // dispatch appropriately
    if (input_state & nv_manip::MOUSE_DN)
      pkActiveManipulator->mouse_down(vec2(x,y),input_state);
    if (input_state & nv_manip::MOUSE_UP)
      pkActiveManipulator->mouse_up(vec2(x,y),input_state);

    UpdateManipulator(g_iActiveManipulator, g_iActiveSubManipulator);
  }

  glutPostRedisplay();
}

//! Function that handles scene display
void GLUTDisplay(void)
{
  bool bDoingScreenShot = GetActiveOptions()->m_bTakeScreenshot;
  GetActiveOptions()->m_bTakeScreenshot = false;
  bool bDoingStepsShots = GetActiveOptions()->m_bTakeStepsShots;
  GetActiveOptions()->m_bTakeStepsShots = false;

  // animate scene
  GetActiveMethod()->Animate(timeGetTime());

  // execute active raytracer
  glPushMatrix();
  GetActiveMethod()->Execute();
  glPopMatrix();

  // calculate frame rate
  CalculateFrameRate();

  // draw info screen if enabled
  if (GetActiveOptions()->m_bShowInfo && !bDoingScreenShot) DrawInfo();
  if (GetActiveOptions()->m_bShowHelp && !bDoingScreenShot) DrawHelp();

  // swap planes
  glutSwapBuffers();

  // take screenshot
  if (bDoingScreenShot)
  {
    TakeScreenShot();
  }
}

//! Function that handles menu
/*!
    The GLUT menu callback function. Called every time an option is selected
*/
void GLUTMenu(int value)
{
  GLUTKeyboard(value, 0, 0);
}

//! Scene submenu handler
void GLUTSceneMenu(int value)
{
  // switch scene
  SetActiveScene(value);
}

/*! @} */

//! Application's entry point.
/*!
It's the main application function. Note the clean code you can
obtain using he GLUT library. No calls to dark windows API
functions with many obscure parameters list. =)
*/
int main(int argc, char** argv)
{
  // little trick to move console window to be visible
  HWND hWnd = GetForegroundWindow();
  SetWindowPos(hWnd, HWND_TOP, GL_WIN_WIDTH+10, 0, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);

  // init GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);
  glutInitWindowPosition(GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y);
  glutInitWindowSize(GL_WIN_WIDTH, GL_WIN_HEIGHT);
  glutInit(&argc, argv);

  glutCreateWindow("Inferno: NV38 Box (http://inferno.hildebrand.cz)");

  // initialize application
  InitializeOGL();
  InitializeBox();

  // create main menu
  int iMainMenu = glutCreateMenu(GLUTMenu);

  // feed scenes into sub-menu
  int iScenesMenu = glutCreateMenu(GLUTSceneMenu);
  int index = 0;
  TScenesContainer::iterator sci = g_akScenes.begin();
  while (sci!=g_akScenes.end())
  {
    char buf[256];
    _snprintf(buf, 256, "[%d] %s", index+1, (*sci).m_kSceneName.c_str());
    glutAddMenuEntry(buf, index);
    sci++;
    index++;
  }
 
  // build options sub-menu
  int iOptionsMenu = glutCreateMenu(GLUTMenu);
  GetActiveMethod()->m_kOptions.BuildMenu(iOptionsMenu);

  // compose main menu items
  glutSetMenu(iMainMenu);
  glutAddSubMenu("    Scenes", iScenesMenu);
  glutAddSubMenu("    Options", iOptionsMenu);

  glutAddMenuEntry("[h] Show/Hide Help", 'h');
  glutAddMenuEntry("[i] Show/Hide Info", 'i');
  glutAddMenuEntry("[k] Take Screenshot", 'k');
// NOT IMPLEMENTED  glutAddMenuEntry("[l] Take Shots of Steps", 'l');
  glutAddMenuEntry("[a] Animate", 'a');
  glutAddMenuEntry("[c] Manipulate Camera", 'c');
  glutAddMenuEntry("[s] Manipulate Scene", 's');
  glutAddMenuEntry("[l] Manipulate Light", 'l');
  glutAddMenuEntry("[o] Manipulate Object", 'o');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  SetActiveOptions(&GetActiveMethod()->m_kOptions);

  // register GLUT handlers
  glutReshapeFunc(GLUTResize);       // called every time  the screen is resized
  glutDisplayFunc(GLUTDisplay);      // called when window needs to be redisplayed
  glutIdleFunc(GLUTDisplay);         // called whenever the application is idle
  glutMouseFunc(GLUTMouse);          // called when the application receives a input from the mouse
  glutMotionFunc(GLUTMotion);        // called when the mouse moves over the screen with one of this button pressed
  glutKeyboardFunc(GLUTKeyboard);    // called when the mouse moves over the screen with one of this button pressed

  g_dwStartTime = timeGetTime();

  // enter main loop
  glutMainLoop();

  return 0;
}
//! @}      //doxygen group
