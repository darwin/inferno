// Magic Software, Inc.
// http://www.magic-software.com
// http://www.wild-magic.com
// Copyright (c) 2003.  All Rights Reserved
//
// The Wild Magic Library (WML) source code is supplied under the terms of
// the license agreement http://www.magic-software.com/License/WildMagic.pdf
// and may not be copied or disclosed except in accordance with the terms of
// that agreement.

#ifndef WMLMATRIX4_H
#define WMLMATRIX4_H

#include "WmlMatrix.h"
#include "WmlVector4.h"

namespace Wml
{
  template<class Real>
  class Quaternion;

  template<class Real>
  class Vector3;

  template<class Real>
  class WML_ITEM Matrix4: public Matrix<4, Real>
  {
  public:
    // construction
                                Matrix4();
                                Matrix4(const Matrix4& rkM);
                                Matrix4(const Matrix<4, Real>& rkM);

    // input Mrc is in row r, column c.
                                Matrix4(Real fM00,
                                        Real fM01,
                                        Real fM02,
                                        Real fM03,
                                        Real fM10,
                                        Real fM11,
                                        Real fM12,
                                        Real fM13,
                                        Real fM20,
                                        Real fM21,
                                        Real fM22,
                                        Real fM23,
                                        Real fM30,
                                        Real fM31,
                                        Real fM32,
                                        Real fM33);

    // Create a matrix from an array of numbers.  The input array is
    // interpreted based on the Boolean input as
    //   true:  entry[0..15]={m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,
    //                        m23,m30,m31,m32,m33} [row major]
    //   false: entry[0..15]={m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,
    //                        m32,m03,m13,m23,m33} [col major]
                                Matrix4(const Real afEntry[16], bool bRowMajor);

    // assignment
    Matrix4&                    operator=(const Matrix4& rkM);
    Matrix4&                    operator=(const Matrix<4, Real>& rkM);

    Matrix4&                    ApplyScale(const Vector3<Real>& s, const Quaternion<Real>& r);

    // matrix operations
    Matrix4                     Inverse() const;
    Matrix4                     Adjoint() const;
    Real                        Determinant() const;

    // special matrices
    inline void                 SetTranslation(const Vector3<Real>& v);
    inline Vector3<Real>        GetTranslation() const;

    // builds a translation matrix
    inline void                 BuildTranslationMatrix(const Vector3<Real>& v);
    inline void                 BuildTranslationMatrix(Real tx, Real ty, Real tz);

    // gets a translation matrix.
    static Matrix4<Real>
    GetTranslationMatrix(const Vector3<Real>& v)
    {
      Matrix4<Real>     kM;

      kM.m_afEntry[0] = 1.0; kM.m_afEntry[1] = 0.0; kM.m_afEntry[2] = 0.0; kM.m_afEntry[3] = v[0];
      kM.m_afEntry[4] = 0.0; kM.m_afEntry[5] = 1.0; kM.m_afEntry[6] = 0.0; kM.m_afEntry[7] = v[1];
      kM.m_afEntry[8] = 0.0; kM.m_afEntry[9] = 0.0; kM.m_afEntry[10] = 1.0; kM.m_afEntry[11] = v[2];
      kM.m_afEntry[12] = 0.0; kM.m_afEntry[13] = 0.0; kM.m_afEntry[14] = 0.0; kM.m_afEntry[15] = 1.0;

      return kM;
    }

    // gets a translation matrix.
    static Matrix4<Real>
    GetTranslationMatrix(Real tx, Real ty, Real tz)
    {
      Matrix4<Real>     kM;

      kM.m_afEntry[0] = 1.0; kM.m_afEntry[1] = 0.0; kM.m_afEntry[2] = 0.0; kM.m_afEntry[3] = tx;
      kM.m_afEntry[4] = 0.0; kM.m_afEntry[5] = 1.0; kM.m_afEntry[6] = 0.0; kM.m_afEntry[7] = ty;
      kM.m_afEntry[8] = 0.0; kM.m_afEntry[9] = 0.0; kM.m_afEntry[10] = 1.0; kM.m_afEntry[11] = tz;
      kM.m_afEntry[12] = 0.0; kM.m_afEntry[13] = 0.0; kM.m_afEntry[14] = 0.0; kM.m_afEntry[15] = 1.0;

      return kM;
    }

    // sets the scale part of the matrix.
    inline void                 SetScale(const Vector3<Real>& v);
    inline Vector3<Real>        GetScale() const;

    // builds a scale matrix
    inline void                 BuildScaleMatrix(const Vector3<Real>& s);

    // gets a scale matrix.
    static Matrix4<Real>
    GetScaleMatrix(const Vector3<Real>& v)
    {
      Matrix4<Real>     kM;

      kM.m_afEntry[0] = v[0]; kM.m_afEntry[1] = 0.0; kM.m_afEntry[2] = 0.0; kM.m_afEntry[3] = 0.0;
      kM.m_afEntry[4] = 0.0; kM.m_afEntry[5] = v[1]; kM.m_afEntry[6] = 0.0; kM.m_afEntry[7] = 0.0;
      kM.m_afEntry[8] = 0.0; kM.m_afEntry[9] = 0.0; kM.m_afEntry[10] = v[2]; kM.m_afEntry[11] = 0.0;
      kM.m_afEntry[12] = 0.0; kM.m_afEntry[13] = 0.0; kM.m_afEntry[14] = 0.0; kM.m_afEntry[15] = 1.0;

      return kM;
    }

    // gets a scale matrix.
    static Matrix4<Real>
    GetScaleMatrix(Real sx, Real sy, Real sz)
    {
      Matrix4<Real>     kM;
      kM.m_afEntry[0] = sx; kM.m_afEntry[1] = 0.0; kM.m_afEntry[2] = 0.0; kM.m_afEntry[3] = 0.0;
      kM.m_afEntry[4] = 0.0; kM.m_afEntry[5] = sy; kM.m_afEntry[6] = 0.0; kM.m_afEntry[7] = 0.0;
      kM.m_afEntry[8] = 0.0; kM.m_afEntry[9] = 0.0; kM.m_afEntry[10] = sz; kM.m_afEntry[11] = 0.0;
      kM.m_afEntry[12] = 0.0; kM.m_afEntry[13] = 0.0; kM.m_afEntry[14] = 0.0; kM.m_afEntry[15] = 1.0;

      return kM;
    }

    static const Matrix4        ZERO;
    static const Matrix4        IDENTITY;
  };

  template<class Real>
  WML_ITEM Vector3<Real> operator*(const Matrix4<Real>& rkM, const Vector3<Real>& rkV);

  typedef Matrix4<float> Matrix4f;
  typedef Matrix4<double> Matrix4d;
}

#endif
