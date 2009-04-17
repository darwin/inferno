// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup Scene Scene management
@{
\ingroup Common
*/
/*! \file Scene.cpp
\brief Implementation of SimpleObject, SimpleScene, SimpleElement, SimpleTriangle, SimpleCamera and SimpleLight classes.
*/
#include "base.h"

#include "Raytracer.h"

SimpleScene::SimpleScene()
{
}

bool SimpleScene::LoadCamera(const char* acSource, SimpleCamera& akCamera)
{
  float posx, posy, posz;
  float trgx, trgy, trgz;
  float hfov, vfov;
  int res = 
    sscanf(acSource, "[%f,%f,%f];[%f,%f,%f];%f;%f\n", 
    &posx, &posy, &posz,
    &trgx, &trgy, &trgz,
    &hfov, &vfov
    );

  if (res!=8) return false;

  akCamera.m_kPosition = Vector3f(posx, posy, posz);
  akCamera.m_kTarget = Vector3f(trgx, trgy, trgz); 
  akCamera.m_kFOV = Vector2f(hfov*Wml::Mathf::DEG_TO_RAD, vfov*Wml::Mathf::DEG_TO_RAD);

  return true;
}

bool SimpleScene::LoadLight(const char* acSource, SimpleLight& akLight)
{
  float posx, posy, posz;
  float colr, colg, colb;
  int res = 
    sscanf(acSource, "[%f,%f,%f];[%f,%f,%f]\n", 
    &posx, &posy, &posz,
    &colr, &colg, &colb
    );

  if (res!=6) return false;

  akLight.m_kPosition = Vector3f(posx, posy, posz);
  akLight.m_kColor = ColorRGB(colr/255.f, colg/255.f, colb/255.f);

  return true;
}

int SimpleScene::LoadTriangle(const char* acSource, SimpleTriangle& akTriangle)
{
  float v1x, v1y, v1z;
  float v2x, v2y, v2z;
  float v3x, v3y, v3z;
  float n1x, n1y, n1z;
  float n2x, n2y, n2z;
  float n3x, n3y, n3z;
  float t1x, t1y, t1z;
  float t2x, t2y, t2z;
  float t3x, t3y, t3z;
  float cr, cb, cg;
  float mx, my, mz, mw;
  int e;
  int res = 
    sscanf(acSource, "%d;[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];[%f,%f,%f];%f;%f;%f;%f\n", 
    &e,
    &v1x, &v1y, &v1z,
    &v2x, &v2y, &v2z,
    &v3x, &v3y, &v3z,
    &n1x, &n1y, &n1z,
    &n2x, &n2y, &n2z,
    &n3x, &n3y, &n3z,
    &t1x, &t1y, &t1z,
    &t2x, &t2y, &t2z,
    &t3x, &t3y, &t3z,
    &cr, &cg, &cb,
    &mx, &my, &mz, &mw
    );

  if (res!=1+9+9+9+3+4) return -1;

  akTriangle.m_akVertices[0] = Vector3f(v1x, v1y, v1z);
  akTriangle.m_akVertices[1] = Vector3f(v2x, v2y, v2z);
  akTriangle.m_akVertices[2] = Vector3f(v3x, v3y, v3z);
  akTriangle.m_akNormals[0] = Vector3f(n1x, n1y, n1z);
  akTriangle.m_akNormals[1] = Vector3f(n2x, n2y, n2z);
  akTriangle.m_akNormals[2] = Vector3f(n3x, n3y, n3z);
  akTriangle.m_akTexCoords[0] = Vector3f(t1x, t1y, t1z);
  akTriangle.m_akTexCoords[1] = Vector3f(t2x, t2y, t2z);
  akTriangle.m_akTexCoords[2] = Vector3f(t3x, t3y, t3z);
  akTriangle.m_kColor = ColorRGB(cr/255.0f, cg/255.0f, cb/255.0f);
  akTriangle.m_kMaterial = Vector4f(mx, my, mz, mw);

  return e;
}

int SimpleScene::LoadTransform(const char* acSource, SimpleElement& akElement)
{
  float px, py, pz;
  float rx, ry, rz, rw;
  float sx, sy, sz;
  int e;
  int res = 
    sscanf(acSource, "%d;[%f,%f,%f];[%f,%f,%f,%f];[%f,%f,%f]\n", 
    &e,
    &px, &py, &pz,
    &rx, &ry, &rz, &rw,
    &sx, &sy, &sz
    );

  if (res!=1+3+4+3) return -1;


  akElement.m_kPosition = Vector3f(px, py, pz);
  akElement.m_kRotation = Quaternionf(rw, rx, ry, rz);
  akElement.m_kScale = Vector3f(sx, sy, sz);

  return e;
}

bool SimpleScene::ParseSource(FILE* f)
{
  char buf[64*1024];
  buf[0] = 0;

  if (feof(f)) return false;
  fgets(buf, 64*1024, f);

  if (strlen(buf)<2) return true;

  bool res = true;
  switch (buf[0]) {
    case '#': // skip comments
      {
        const char* token = "# Statistics:";
        size_t len = strlen(token);
        if (strncmp(buf, token, len)==0)
        {
          log_write(LOG_LEVEL_INFO, "%s\n", &buf[len+1]);
        }
      }
      break;
    case 'c':
      {
        SimpleCamera kCamera;
        res = LoadCamera(buf+2, kCamera); 
        m_kCameras.push_back(kCamera);
      }
      break;

    case 'l':
      {
        SimpleLight kLight;
        res = LoadLight(buf+2, kLight); 
        m_kLights.push_back(kLight);
      }
      break;

    case 'x':
      {
        SimpleElement kElement;
        int id = LoadTransform(buf+2, kElement);
        if (id==-1) { res = false; break; }
        m_kElements[id] = kElement;
      }
      break;

    case 't':
      {
        SimpleTriangle kTriangle;
        int id = LoadTriangle(buf+2, kTriangle);
        if (id==-1) { res = false; break; }
        m_kElements[id].push_back(kTriangle);
      }
      break;

    default: res = false;
  }

  return res;
}

bool SimpleScene::Load(const char* filename)
{
  FILE* f;
  int res;

  f = fopen(filename, "rt");
  if (!f) return false;

  m_kElements.resize(RTRF_BVMAP_SX*RTRF_BVMAP_SY);

  while (!feof(f))
  {
    res = ParseSource(f);  
    if (!res) 
      return false;
  }
  fclose(f);
 
  m_kFileName = filename;
  return true;
}

bool SimpleScene::ReshapeManipulators(int w, int h)
{
  if (!ReshapeManipulator(w, h)) return false;

  TSimpleCameraContainer::iterator iC = m_kCameras.begin();
  while (iC!=m_kCameras.end())
  {
    if (!(*iC).ReshapeManipulator(w, h)) return false;
    iC++;
  }

  TSimpleLightContainer::iterator iL = m_kLights.begin();
  while (iL!=m_kLights.end())
  {
    if (!(*iL).ReshapeManipulator(w, h)) return false;
    iL++;
  }

  TSimpleElementContainer::iterator iE = m_kElements.begin();
  while (iE!=m_kElements.end())
  {
    if (!(*iE).ReshapeManipulator(w, h)) return false;
    iE++;
  }

  return true;
}

bool SimpleScene::InitManipulators()
{
  if (!InitManipulator()) return false;

  TSimpleCameraContainer::iterator iC = m_kCameras.begin();
  while (iC!=m_kCameras.end())
  {
    if (!(*iC).InitManipulator()) return false;
    iC++;
  }

  TSimpleLightContainer::iterator iL = m_kLights.begin();
  while (iL!=m_kLights.end())
  {
    if (!(*iL).InitManipulator()) return false;
    iL++;
  }

  TSimpleElementContainer::iterator iE = m_kElements.begin();
  while (iE!=m_kElements.end())
  {
    if (!(*iE).InitManipulator()) return false;
    iE++;
  }

  return true;
}

bool SimpleScene::UpdateAll()
{
  if (!Update()) return false;

  TSimpleCameraContainer::iterator iC = m_kCameras.begin();
  while (iC!=m_kCameras.end())
  {
    if (!(*iC).Update()) return false;
    iC++;
  }

  TSimpleLightContainer::iterator iL = m_kLights.begin();
  while (iL!=m_kLights.end())
  {
    if (!(*iL).Update()) return false;
    iL++;
  }

  TSimpleElementContainer::iterator iE = m_kElements.begin();
  while (iE!=m_kElements.end())
  {
    if (!(*iE).Update()) return false;
    iE++;
  }

  return true;
}

bool SimpleScene::InitManipulator()
{
  m_kManipulator.set_manip_behavior(nv_manip::OBJECT | nv_manip::PAN | nv_manip::TRANSLATE | nv_manip::ROTATE | nv_manip::PAN_RELATIVE);
  m_kManipulator.set_mat(mat4_id);
  m_kManipulator.set_clip_planes(.5, 100);
  m_kManipulator.set_fov(45);
  m_kManipulator.set_screen_size(256, 256);

  return true;
}

bool SimpleScene::ReshapeManipulator(int w, int h)
{
  m_kManipulator.set_screen_size(w, h);
  return true;
}

bool SimpleScene::Update()
{
  return true;
}

bool SimpleCamera::InitManipulator()
{
  m_kManipulator.set_manip_behavior(nv_manip::CAMERA | nv_manip::PAN | nv_manip::TRANSLATE | nv_manip::ROTATE | nv_manip::DONT_TRANSLATE_FOCUS | nv_manip::PAN_RELATIVE);
  mat4 m = mat4_id;
  m_kManipulator.set_mat(m);
  m_kManipulator.set_focus_pos(vec3(m_kTarget[0], m_kTarget[1], m_kTarget[2]));
  m_kManipulator.set_eye_pos(vec3(m_kPosition[0], m_kPosition[1], m_kPosition[2]));
  m_kManipulator.set_clip_planes(.5, 100);
  m_kManipulator.set_fov(m_kFOV[1]*Mathf::RAD_TO_DEG);
  m_kManipulator.set_screen_size(256, 256);

  return true;
}

bool SimpleCamera::ReshapeManipulator(int w, int h)
{
  m_kManipulator.set_screen_size(w, h);
  return true;
}

bool SimpleCamera::Update()
{
  vec3 v = m_kManipulator.get_eye_pos();
  m_kPosition[0] = v[0];
  m_kPosition[1] = v[1];
  m_kPosition[2] = v[2];
  m_kManipulator.set_focus_pos(vec3(0.0f, 0.0f, 0.0f));

  return true;
}

bool SimpleLight::InitManipulator()
{
  m_kManipulator.set_manip_behavior(nv_manip::OBJECT | nv_manip::PAN | nv_manip::TRANSLATE | nv_manip::ROTATE | nv_manip::PAN_RELATIVE);
  mat4 m = mat4_id;
  m.set_translation(vec3(m_kPosition[0], m_kPosition[1], m_kPosition[2]));
  m_kManipulator.set_mat(m);
  m_kManipulator.set_clip_planes(.5, 100);
  m_kManipulator.set_fov(45);
  m_kManipulator.set_screen_size(256, 256);

  return true;
}

bool SimpleLight::ReshapeManipulator(int w, int h)
{
  m_kManipulator.set_screen_size(w, h);
  return true;
}

bool SimpleLight::Update()
{
  mat4 m = m_kManipulator.get_mat();
  m_kPosition[0] = m.x;
  m_kPosition[1] = m.y;
  m_kPosition[2] = m.z;

  return true;
}

bool SimpleElement::InitManipulator()
{
  m_kManipulator.set_manip_behavior(nv_manip::OBJECT | nv_manip::PAN | nv_manip::TRANSLATE | nv_manip::ROTATE | nv_manip::PAN_RELATIVE);
  mat4 m = mat4_id;
  m.set_translation(vec3(m_kPosition[0], m_kPosition[1], m_kPosition[2]));
  // i need convert wild magic quaternion to nvidia rotation matrix
  Matrix3f kRM = -m_kRotation;  mat3 A;
  A.a00 = kRM[0][0]; A.a10 = kRM[0][1]; A.a20 = kRM[0][2];
  A.a01 = kRM[1][0]; A.a11 = kRM[1][1]; A.a21 = kRM[1][2];
  A.a02 = kRM[2][0]; A.a12 = kRM[2][1]; A.a22 = kRM[2][2];
  m.set_rot(A);
  m_kManipulator.set_mat(m);
  m_kManipulator.set_clip_planes(.5, 100);
  m_kManipulator.set_fov(45);
  m_kManipulator.set_screen_size(256, 256);
  return true;
}

bool SimpleElement::ReshapeManipulator(int w, int h)
{
  m_kManipulator.set_screen_size(w, h);
  return true;
}

bool SimpleElement::Update()
{
  // ugly conversion from nvidia rotation matrix to Wild Magic quaternion
  
  mat4 M = m_kManipulator.get_mat();
  vec3 T;
  M.get_translation(T);
  m_kPosition[0] = T[0];
  m_kPosition[1] = T[1];
  m_kPosition[2] = T[2];

  mat3 A;
  M.get_rot(A);
  Matrix3f kRM;
  kRM[0][0] = A.a00; kRM[0][1] = A.a10; kRM[0][2] = A.a20;
  kRM[1][0] = A.a01; kRM[1][1] = A.a11; kRM[1][2] = A.a21;
  kRM[2][0] = A.a02; kRM[2][1] = A.a12; kRM[2][2] = A.a22;

  Vector3f kAxis;
  float fAngle;
  kRM.ToAxisAngle(kAxis, fAngle);

  m_kRotation.FromAngleAxis(-fAngle, kAxis);

  return true;
}
//! @}      //doxygen group
