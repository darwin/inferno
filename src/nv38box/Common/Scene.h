// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Scene Scene management
@{
\ingroup Common
*/
/*! \file Scene.h
\brief Declaration of SimpleObject, SimpleScene, SimpleElement, SimpleTriangle, SimpleCamera and SimpleLight classes.
*/
#ifndef _SCENE_H_
#define _SCENE_H_

//! Base class for all scene objects.
class SimpleObject
{
public:
};

//! Class representing simple camera.
class SimpleCamera : public SimpleObject 
{
public:
  //! Initializes manipulator.
  bool InitManipulator();
  //! Updates manipulator with screen resize event.
  bool ReshapeManipulator(int w, int h);
  //! Updates data members according to manipulator state.
  bool Update();

  Vector3f m_kPosition;               //!< Camera position.
  Vector3f m_kTarget;                 //!< Camera target position.
  //unused Vector3f m_kDirection;     
  Vector2f m_kFOV;                    //!< Camera field of view.
  //unused Quaternionf m_kRotation;

  // manipulator
  nv_manip m_kManipulator;            //!< Camera manipulator.
};

//! Class representing simple light.
class SimpleLight : public SimpleObject
{
public:
  //! Initializes manipulator.
  bool InitManipulator();
  //! Updates manipulator with screen resize event.
  bool ReshapeManipulator(int w, int h);
  //! Updates data members according to manipulator state.
  bool Update();

  Vector3f m_kPosition;               //!< Light position.
  ColorRGB m_kColor;                  //!< Light color (unused).

  // manipulator
  nv_manip m_kManipulator;            //!< Light manipulator.
};

//! Class representing triangle data.
class SimpleTriangle : public SimpleObject
{
public:
  Vector3f m_akVertices[3];           //!< Vertex positions.
  Vector3f m_akNormals[3];            //!< Vertex normals.
  Vector3f m_akTexCoords[3];          //!< Vertex texturing coordinates.
  ColorRGB m_kColor;                  //!< Triangle color.
  Vector4f m_kMaterial;               //!< Triangle material: [base_k, reflection_k, refraction_k, eta].
};

typedef std::vector<SimpleTriangle> TSimpleTriangleContainer; //!< Container for triangles.

//! Class representing simple element.
/*! Element in Inferno's jargon is object in classic OpenGL aplications. 
    Element = cluster of triangles. Element has own transformation matrix.
    I'm building bounding volumes for elements.
*/
class SimpleElement : public SimpleObject, public TSimpleTriangleContainer
{
public:
  //! Initializes manipulator.
  bool InitManipulator();
  //! Updates manipulator with screen resize event.
  bool ReshapeManipulator(int w, int h);
  //! Updates data members according to manipulator state.
  bool Update();

  Vector3f m_kPosition;             //!< Element position transformation.
  Quaternionf m_kRotation;          //!< Element rotation transformation.
  Vector3f m_kScale;                //!< Element scale transformation.

  // manipulator
  nv_manip m_kManipulator;          //!< Element manipulator.
};

typedef std::vector<SimpleElement> TSimpleElementContainer;  //!< Container for elements.
typedef std::vector<SimpleCamera> TSimpleCameraContainer;    //!< Container for cameras.
typedef std::vector<SimpleLight> TSimpleLightContainer;      //!< Container for ligths.

//! Class representing simple scene.
/*! This is minimalist definition of scene suitable just for my needs.
    Every scene file (.csv) has also associated texture (.png). 
    Textures are loaded separately by raytracer code.
*/
class SimpleScene : public SimpleObject 
{
public:
  //! SimpleScene constructor.
  SimpleScene ();

  //! Loads scene from disk.
  bool Load(const char* filename);

  //! Initializes main manipulator.
  bool InitManipulator();
  //! Updates main manipulator with screen resize event.
  bool ReshapeManipulator(int w, int h);
  //! Updates data members according to main manipulator state.
  bool Update();

  //! Initializes all nested manipulators.
  bool InitManipulators();
  //! Updates all nested manipulators with mouse event.
  bool ReshapeManipulators(int w, int h);
  //! Updates data members according to all nested manipulators states.
  bool UpdateAll();

  TSimpleElementContainer m_kElements;  //!< Scene elements.
  TSimpleCameraContainer m_kCameras;    //!< Scene cameras.
  TSimpleLightContainer m_kLights;      //!< Scene lights.
  string m_kFileName;                   //!< Cached original file name.
  string m_kSceneName;                  //!< Cached scene name.

  // manipulator
  nv_manip m_kManipulator;              //!< Scene manipulator (main manipulator).

protected:
  // custom parsers

  //! Camera parser.
  bool LoadCamera(const char* acSource, SimpleCamera& akCamera);
  //! Light parser.
  bool LoadLight(const char* acSource, SimpleLight& akLight);
  //! Transformation info parser.
  int  LoadTransform(const char* acSource, SimpleElement& akElement);
  //! Triangle data parser.
  int  LoadTriangle(const char* acSource, SimpleTriangle& akLight);
  
  //! Main parsing function. Loads and parses scene from file.
  bool ParseSource(FILE* f);
};

#endif
//! @}      //doxygen group
