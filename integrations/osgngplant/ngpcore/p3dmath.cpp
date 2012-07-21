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

#include <math.h>
/*
#include <stdio.h>
*/

#include <ngpcore/p3dmath.h>

void                P3DMath::SinCosf  (float              *sina,
                                       float              *cosa,
                                       float               a)
 {
  #ifdef HAVE_SINCOSF
  sincosf(a,sina,cosa);
  #else
  *sina = sinf(a);
  *cosa = cosf(a);
  #endif
 }

float              P3DMath::Sinf      (float               a)
 {
  return(sinf(a));
 }

float              P3DMath::Cosf      (float               a)
 {
  return(cosf(a));
 }

float              P3DMath::ACosf     (float               a)
 {
  return(acosf(a));
 }

float              P3DMath::Sqrtf     (float               a)
 {
  return(sqrtf(a));
 }

float              P3DMath::Roundf    (float               a)
 {
  #ifdef HAVE_ROUNDF
  return(roundf(a));
  #else
  return((float)((int)(a + 0.5f)));
  #endif
 }

void               P3DVector3f::MultMatrix
                                      (float              *V,
                                       const P3DMatrix4x4f*M,
                                       const float        *V0)
 {
  V[0] = M->m[0] * V0[0] + M->m[4] * V0[1] + M->m[8]  * V0[2] + M->m[12];
  V[1] = M->m[1] * V0[0] + M->m[5] * V0[1] + M->m[9]  * V0[2] + M->m[13];
  V[2] = M->m[2] * V0[0] + M->m[6] * V0[1] + M->m[10] * V0[2] + M->m[14];
 }

void               P3DVector3f::MultMatrix
                                      (const P3DMatrix4x4f
                                                          *M)
 {
  float                                x,y,z;

  x = M->m[0] * v[0] + M->m[4] * v[1] + M->m[8]  * v[2] + M->m[12];
  y = M->m[1] * v[0] + M->m[5] * v[1] + M->m[9]  * v[2] + M->m[13];
  z = M->m[2] * v[0] + M->m[6] * v[1] + M->m[10] * v[2] + M->m[14];

  v[0] = x;
  v[1] = y;
  v[2] = z;
 }

void               P3DVector3f::MultMatrixTranspose
                                      (const P3DMatrix4x4f*M)
 {
  float                                x,y,z;

  x = M->m[0] * v[0] + M->m[1] * v[1] + M->m[2] * v[2]  + M->m[3];
  y = M->m[4] * v[0] + M->m[5] * v[1] + M->m[6] * v[2]  + M->m[7];
  z = M->m[8] * v[0] + M->m[9] * v[1] + M->m[10] * v[2] + M->m[11];

  v[0] = x;
  v[1] = y;
  v[2] = z;
 }

                   P3DMatrix4x4f::P3DMatrix4x4f
                                      (bool                identity)
 {
  for (unsigned int i = 0; i < 16; i++)
   {
    m[i] = 0.0f;
   }

  if (identity)
   {
    m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
   }
 }

void               P3DMatrix4x4f::MakeTranslation
                                      (float              *m,
                                       float               x,
                                       float               y,
                                       float               z)
 {
  for (unsigned int i = 0; i < 12; i++)
   {
    m[i] = 0.0f;
   }

  m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
  m[12] = x; m[13] = y; m[14] = z;
 }

void               P3DMatrix4x4f::MultMatrix
                                      (float              *m,
                                       const float        *m0,
                                       const float        *m1)
 {
  m[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8]  * m1[2] + m0[12] * m1[3];
  m[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9]  * m1[2] + m0[13] * m1[3];
  m[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
  m[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];

  m[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8]  * m1[6] + m0[12] * m1[7];
  m[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9]  * m1[6] + m0[13] * m1[7];
  m[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
  m[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];

  m[8]  = m0[0] * m1[8] + m0[4] * m1[9] + m0[8]  * m1[10] + m0[12] * m1[11];
  m[9]  = m0[1] * m1[8] + m0[5] * m1[9] + m0[9]  * m1[10] + m0[13] * m1[11];
  m[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
  m[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];

  m[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8]  * m1[14] + m0[12] * m1[15];
  m[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9]  * m1[14] + m0[13] * m1[15];
  m[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
  m[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
 }

void               P3DMatrix4x4f::Translate
                                      (float              *m,
                                       const float        *m0,
                                       float               x,
                                       float               y,
                                       float               z)
 {
  m[0] = m0[0];
  m[1] = m0[1];
  m[2] = m0[2];
  m[3] = m0[3];

  m[4] = m0[4];
  m[5] = m0[5];
  m[6] = m0[6];
  m[7] = m0[7];

  m[8]  = m0[8];
  m[9]  = m0[9];
  m[10] = m0[10];
  m[11] = m0[11];

  m[12] = m0[0] * x + m0[4] * y + m0[8]  * z + m0[12];
  m[13] = m0[1] * x + m0[5] * y + m0[9]  * z + m0[13];
  m[14] = m0[2] * x + m0[6] * y + m0[10] * z + m0[14];
  m[15] = m0[3] * x + m0[7] * y + m0[11] * z + m0[15];
 }

void               P3DMatrix4x4f::MakeIdentity
                                      (float              *m)
 {
  for (unsigned int i = 0; i < 16; i++)
   {
    m[i] = 0.0f;
   }

  m[0] = m[5] = m[10] = m[15] = 1.0f;
 }

void               P3DMatrix4x4f::GetRotationOnly
                                      (float              *m,
                                       const float        *m0)
 {
  m[0] = m0[0];
  m[1] = m0[1];
  m[2] = m0[2];
  m[3] = 0.0f;
  m[4] = m0[4];
  m[5] = m0[5];
  m[6] = m0[6];
  m[7] = 0.0f;
  m[8] = m0[8];
  m[9] = m0[9];
  m[10] = m0[10];
  m[11] = 0.0f;

  m[12] = m[13] = m[14] = 0.0f;
  m[15] = 0.0f;
 }

                   P3DQuaternionf::P3DQuaternionf
                                      (float               x,
                                       float               y,
                                       float               z,
                                       float               w)
 {
  q[0] = x; q[1] = y; q[2] = z; q[3] = w;
 }

void               P3DQuaternionf::ToMatrix
                                      (float              *m) const
 {
  m[0]  = 1.0f - 2.0f * q[1] * q[1] - 2.0f * q[2] * q[2];
  m[4]  = 2.0f * q[0] * q[1] - 2.0f * q[3] * q[2];
  m[8]  = 2.0f * q[0] * q[2] + 2.0f * q[3] * q[1];

  m[1]  = 2.0f * q[0] * q[1] + 2.0f * q[3] * q[2];
  m[5]  = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[2] * q[2];
  m[9]  = 2.0f * q[1] * q[2] - 2.0f * q[3] * q[0];

  m[2]  = 2.0f * q[0] * q[2] - 2.0f * q[3] * q[1];
  m[6]  = 2.0f * q[1] * q[2] + 2.0f * q[3] * q[0];
  m[10] = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[1] * q[1];

  m[3]  = 0.0f;
  m[7]  = 0.0f;
  m[11] = 0.0f;
  m[12] = 0.0f;
  m[13] = 0.0f;
  m[14] = 0.0f;

  m[15] = 1.0f;
 }

void               P3DQuaternionf::FromAxisAndAngle
                                      (float               x,
                                       float               y,
                                       float               z,
                                       float               a)
 {
  float                                sa,ca;

  P3DMath::SinCosf(&sa,&ca,a / 2.0);

  q[0] = sa * x;
  q[1] = sa * y;
  q[2] = sa * z;
  q[3] = ca;
 }

void               P3DQuaternionf::CrossProduct
                                      (float              *q,
                                       const float        *q0,
                                       const float        *q1)
 {
  q[3] = (q0[3] * q1[3] - q0[0] * q1[0]) - (q0[1] * q1[1] + q0[2] * q1[2]);
  q[0] = (q0[3] * q1[0] + q0[0] * q1[3]) + (q0[1] * q1[2] - q0[2] * q1[1]);
  q[1] = (q0[3] * q1[1] + q0[1] * q1[3]) + (q0[2] * q1[0] - q0[0] * q1[2]);
  q[2] = (q0[3] * q1[2] + q0[2] * q1[3]) + (q0[0] * q1[1] - q0[1] * q1[0]);
 }

void               P3DQuaternionf::RotateVector
                                      (float              *v,
                                       const float        *q)
 {
  float                                q1[4];

  q1[3] = (-q[0] * v[0]) - (q[1] * v[1] + q[2] * v[2]);
  q1[0] = ( q[3] * v[0]) + (q[1] * v[2] - q[2] * v[1]);
  q1[1] = ( q[3] * v[1]) + (q[2] * v[0] - q[0] * v[2]);
  q1[2] = ( q[3] * v[2]) + (q[0] * v[1] - q[1] * v[0]);

  v[0] = (q1[0] * q[3] - q1[3] * q[0]) + (q1[2] * q[1] - q1[1] * q[2]);
  v[1] = (q1[1] * q[3] - q1[3] * q[1]) + (q1[0] * q[2] - q1[2] * q[0]);
  v[2] = (q1[2] * q[3] - q1[3] * q[2]) + (q1[1] * q[0] - q1[0] * q[1]);
 }

void               P3DQuaternionf::RotateVectorInv
                                      (float              *v,
                                       const float        *q)
 {
  float                                q1[4];

  q1[3] = (q[0] * v[0]) + (q[1] * v[1]) + (q[2] * v[2]);
  q1[0] = (q[3] * v[0]) + (q[2] * v[1]) - (q[1] * v[2]);
  q1[1] = (q[3] * v[1]) + (q[0] * v[2]) - (q[2] * v[0]);
  q1[2] = (q[3] * v[2]) + (q[1] * v[0]) - (q[0] * v[1]);

  v[0] = (q1[3] * q[0] + q1[0] * q[3]) + (q1[1] * q[2] - q1[2] * q[1]);
  v[1] = (q1[3] * q[1] + q1[1] * q[3]) + (q1[2] * q[0] - q1[0] * q[2]);
  v[2] = (q1[3] * q[2] + q1[2] * q[3]) + (q1[0] * q[1] - q1[1] * q[0]);
 }

void               P3DQuaternionf::Power
                                      (float              *q,
                                       float               power)
 {
  if ((q[3] > -1.0f) && (q[3] < 1.0f))
   {
    float                              OldHalfAngle;
    float                              NewHalfAngle;
    float                              NewHalfAngleSin;
    float                              NewHalfAngleCos;
    float                              Temp;

    OldHalfAngle = P3DMath::ACosf(q[3]);
    NewHalfAngle = OldHalfAngle * power;

    P3DMath::SinCosf(&NewHalfAngleSin,&NewHalfAngleCos,NewHalfAngle);

    q[3] = NewHalfAngleCos;

    Temp = NewHalfAngleSin / P3DMath::Sinf(OldHalfAngle);

    q[0] *= Temp;
    q[1] *= Temp;
    q[2] *= Temp;
   }
 }

void               P3DQuaternionf::Slerp
                                      (float              *q,
                                       const float        *q0,
                                       const float        *q1,
                                       float               t)
 {
  P3DQuaternionf                       q0Inv;
  P3DQuaternionf                       Delta;

  q0Inv.q[0] = -q0[0];
  q0Inv.q[1] = -q0[1];
  q0Inv.q[2] = -q0[2];
  q0Inv.q[3] =  q0[3];

  P3DQuaternionf::CrossProduct(Delta.q,q0Inv.q,q1);
  P3DQuaternionf::Power(Delta.q,t);
  P3DQuaternionf::CrossProduct(q,q0,Delta.q);

  /******************************
   {
    float temp[4];

    Slerp2(temp,q0,q1,t);

    if ((fabs(temp[0] - q[0]) > 0.001f) ||
        (fabs(temp[1] - q[1]) > 0.001f) ||
        (fabs(temp[2] - q[2]) > 0.001f) ||
        (fabs(temp[3] - q[3]) > 0.001f))
     {
      printf("Slerp differs:\n");
      printf("q0 = (%.04f %.04f %.04f %.04f)\n",q0[0],q0[1],q0[2],q0[3]);
      printf("q1 = (%.04f %.04f %.04f %.04f)\n",q1[0],q1[1],q1[2],q1[3]);
      printf("t = %.4f\n",t);

      printf("r1 = (%.04f %.04f %.04f %.04f)\n",q[0],q[1],q[2],q[3]);
      printf("r2 = (%.04f %.04f %.04f %.04f)\n",temp[0],temp[1],temp[2],temp[3]);
     }
   }
   ***********************************************************/
 }

void               P3DQuaternionf::Slerp2
                                      (float              *q,
                                       const float        *q0,
                                       const float        *q1,
                                       float               t)
 {
  float                                CosPhi;
  float                                Phi;
  float                                SinPhiInv;
  float                                SinPhiT;
  float                                SinPhiOneMinusT;
  float                                a,b;

  CosPhi = q0[0] * q1[0] + q0[1] * q1[1] + q0[2] * q1[2] +q0[3] * q1[3];

  Phi = P3DMath::ACosf(CosPhi);

  if (Phi != 0.0f)
   {
    SinPhiInv       = 1.0f / P3DMath::Sinf(Phi);
    SinPhiT         = P3DMath::Sinf(Phi * t);
    SinPhiOneMinusT = P3DMath::Sinf(Phi * (1 - t));

    a = SinPhiOneMinusT * SinPhiInv;
    b = SinPhiT         * SinPhiInv;

    q[0] = q0[0] * a + q1[0] * b;
    q[1] = q0[1] * a + q1[1] * b;
    q[2] = q0[2] * a + q1[2] * b;
    q[3] = q0[3] * a + q1[3] * b;
   }
  else
   {
    q[0] = q[1] = q[2] = 0.0f;
    q[3] = 1.0f;
   }
 }

