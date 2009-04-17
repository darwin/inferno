// NV38Box (http://inferno.hildebrand.cz)
// Copyright (c) 2004 Antonin Hildebrand
// licensed under zlib/libpng license (see license.txt)
// released in Shader Triathlon – Summer 2004 ("Contest")

/*! \addtogroup NV38Box
@{
*/
/*! \file nv38box.h
\brief Declaration of main application routines.
*/

void ApplyCameraTransform();
void ApplyCameraProjection();

void SetActiveScene(int iIndex);

void GetSceneTransform(Matrix4f& kM);
int GetActiveCamera();
int GetActiveLight();
int GetActiveElement();

//! @}      //doxygen group
