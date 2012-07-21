/***************************************************************************

 Copyright (c) 2007 Sergey Prokhorchuk.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. Neither the name of the author nor the names of contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

***************************************************************************/

#ifndef __P3DMATH_H__
#define __P3DMATH_H__

#define P3DMATH_PI  (3.1415926)
#define P3DMATH_2PI (P3DMATH_PI * 2.0)

#define P3DMATH_DEG2RAD(deg) ((deg) * P3DMATH_PI / 180.0f)
#define P3DMATH_RAD2DEG(rad) ((rad) * 180.0f / P3DMATH_PI)

class P3DMath
 {
  public           :

  static void      SinCosf            (float              *sina,
                                       float              *cosa,
                                       float               a);

  static float     Sinf               (float               a);
  static float     Cosf               (float               a);

  static float     Sqrtf              (float               a);

  static float     ACosf              (float               a);

  static float     Clampf             (float               min,
                                       float               max,
                                       float               value)
   {
    return((value < min) ? min : ((value > max) ? max : value));
   };

  static float     Roundf               (float               a);
 };

class P3DMatrix4x4f;

class P3DVector3f
 {
  public           :

                   P3DVector3f        () {};
                   P3DVector3f        (float               x,
                                       float               y,
                                       float               z)
   {
    Set(x,y,z);
   }

                   P3DVector3f        (const P3DVector3f  &Source)
   {
    v[0] = Source.v[0];
    v[1] = Source.v[1];
    v[2] = Source.v[2];
   };

  void             Set                (float               x,
                                       float               y,
                                       float               z)
   {
    v[0] = x; v[1] = y; v[2] = z;
   }

  float           &X                  ()
   {
    return(v[0]);
   }

  float           &Y                  ()
   {
    return(v[1]);
   }

  float           &Z                  ()
   {
    return(v[2]);
   }

  void             Add                (const float        *V)
   {
    v[0] += V[0];
    v[1] += V[1];
    v[2] += V[2];
   }

  void             operator *=        (float               Value)
   {
    v[0] *= Value;
    v[1] *= Value;
    v[2] *= Value;
   }

  void             operator +=        (const P3DVector3f  &Value)
   {
    v[0] += Value.v[0];
    v[1] += Value.v[1];
    v[2] += Value.v[2];
   }

  void             MultMatrix         (const P3DMatrix4x4f*M);
  void             MultMatrixTranspose(const P3DMatrix4x4f*M);

  static void      Add                (float              *V,
                                       float              *V0,
                                       float              *V1)
   {
    V[0] = V0[0] + V1[0];
    V[1] = V0[1] + V1[1];
    V[2] = V0[2] + V1[2];
   }

  static void      MultMatrix         (float              *V,
                                       const P3DMatrix4x4f*M,
                                       const float        *V0);

  static float     ScalarProduct      (const float        *v0,
                                       const float        *v1)
   {
    return(v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2]);
   }

  static void      CrossProduct       (float              *v,
                                       const float        *v0,
                                       const float        *v1)
   {
    v[0] = v0[1] * v1[2] - v0[2] * v1[1];
    v[1] = v0[2] * v1[0] - v0[0] * v1[2];
    v[2] = v0[0] * v1[1] - v0[1] * v1[0];
   }

  void             Normalize          ()
   {
    float                              l;

    l = P3DMath::Sqrtf(ScalarProduct(v,v));

    v[0] /= l; v[1] /= l; v[2] /= l;
   }

  float            v[3];
 };

class P3DMatrix4x4f
 {
  public           :

                   P3DMatrix4x4f      () {};
                   P3DMatrix4x4f      (bool                identity);

  static void      MakeTranslation    (float              *m,
                                       float               x,
                                       float               y,
                                       float               z);

  static void      MultMatrix         (float              *m,
                                       const float        *m0,
                                       const float        *m1);

  static void      Translate          (float              *m,
                                       const float        *m0,
                                       float               x,
                                       float               y,
                                       float               z);

  static void      MakeIdentity       (float              *m);

  static void      GetRotationOnly    (float              *m,
                                       const float        *m0);

  public           :

  float            m[16];
 };

class P3DQuaternionf
 {
  public           :

                   P3DQuaternionf     () {};
                   P3DQuaternionf     (float               x,
                                       float               y,
                                       float               z,
                                       float               w);

  void             MakeIdentity       ()
   {
    q[0] = q[1] = q[2] = 0.0f;
    q[3] = 1.0f;
   }

  void             Set                (float               x,
                                       float               y,
                                       float               z,
                                       float               w)
   {
    q[0] = x; q[1] = y; q[2] = z; q[3] = w;
   }

  void             Set                (const float        *q)
   {
    for (unsigned int i = 0; i < 4; i++)
     {
      this->q[i] = q[i];
     }
   }

  void             Normalize          ()
   {
    float                              l;

    l = P3DMath::Sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);

    q[0] /= l; q[1] /= l; q[2] /= l; q[3] /= l;
   };

  void             ToMatrix           (float              *m) const;

  void             FromAxisAndAngle   (float               x,
                                       float               y,
                                       float               z,
                                       float               a);

  static void      CrossProduct       (float              *q,
                                       const float        *q0,
                                       const float        *q1);

  static void      RotateVector       (float              *v,
                                       const float        *q);

  static void      RotateVectorInv    (float              *v,
                                       const float        *q);

  static void      Power              (float              *q,
                                       float               power);

  static void      Slerp              (float              *q,
                                       const float        *q0,
                                       const float        *q1,
                                       float               t);

  /*FIXME: experimental faster version of Slerp, not well tested*/
  static void      Slerp2             (float              *q,
                                       const float        *q0,
                                       const float        *q1,
                                       float               t);

  static void      Normalize          (float              *q)
   {
    float                              l;

    l = P3DMath::Sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);

    q[0] /= l; q[1] /= l; q[2] /= l; q[3] /= l;
   }

  static void      MakeIdentity       (float              *q)
   {
    q[0] = q[1] = q[2] = 0.0f;
    q[3] = 1.0f;
   };

  public           :

  float            q[4];
 };

#endif

