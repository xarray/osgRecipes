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

#include <stdlib.h>
#include <math.h>

#include <ngpcore/p3ddefs.h>

#include <ngpcore/p3dmath.h>

#include <ngpcore/p3dplant.h>

                   P3DTubeAxisLine::P3DTubeAxisLine
                                      (float               Length,
                                       unsigned int        Resolution)
 {
  this->Length     = Length;
  this->Resolution = Resolution;
 }

unsigned int       P3DTubeAxisLine::GetResolution
                                      () const
 {
  return(Resolution);
 }

float              P3DTubeAxisLine::GetLength
                                      () const
 {
  return(Length);
 }

void               P3DTubeAxisLine::GetPointAt
                                      (float              *Pos,
                                       float               Offset) const
 {
  Pos[0] = Pos[2] = 0.0f;
  Pos[1] = Length * Offset;
 }

void               P3DTubeAxisLine::GetOrientationAt
                                      (float              *Orientation,
                                       float               Offset P3D_UNUSED_ATTR) const
 {
  P3DQuaternionf::MakeIdentity(Orientation);
 }

void               P3DTubeAxisLine::GetOrientationAt
                                      (float              *Orientation,
                                       unsigned int        SegIndex P3D_UNUSED_ATTR) const
 {
  P3DQuaternionf::MakeIdentity(Orientation);
 }

                   P3DTubeAxisSegLine::P3DTubeAxisSegLine
                                      (float               Length,
                                       unsigned int        Resolution)
 {
  this->Length     = Length;
  this->Resolution = Resolution;

  if (Resolution > 1)
   {
    SegOrientations = new float[4 * (Resolution - 1)];

    for (unsigned int SegIndex = 0; SegIndex < (Resolution - 1); SegIndex++)
     {
      P3DQuaternionf::MakeIdentity(&(SegOrientations[4 * SegIndex]));
     }
   }
  else
   {
    Resolution      = 1;
    SegOrientations = 0;
   }
 }

                   P3DTubeAxisSegLine::~P3DTubeAxisSegLine
                                      ()
 {
  delete[] SegOrientations;
 }

unsigned int       P3DTubeAxisSegLine::GetResolution
                                      () const
 {
  return(Resolution);
 }

float              P3DTubeAxisSegLine::GetLength
                                      () const
 {
  return(Length);
 }

void               P3DTubeAxisSegLine::GetPointAt
                                      (float              *Pos,
                                       float               Offset) const
 {
  unsigned int                         SegIndex;
  float                                SegLength;

  Offset = P3DMath::Clampf(0.0f,1.0f,Offset);

  SegIndex = (unsigned int)(Offset * Resolution);

  if (SegIndex >= Resolution)
   {
    SegIndex--;
   }

  SegLength = Length / Resolution;

  Pos[0] = Pos[2] = 0.0f;
  Pos[1] = Length * Offset - SegIndex * SegLength;

  while (SegIndex > 0)
   {
    P3DQuaternionf::RotateVector(Pos,&(SegOrientations[(SegIndex - 1) * 4]));

    Pos[1] += SegLength;

    SegIndex--;
   }
 }

void               P3DTubeAxisSegLine::GetOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const
 {
  unsigned int                         SegIndex;
  unsigned int                         SegCount;
  float                                SegFraction;
  P3DQuaternionf                       Next;
  P3DQuaternionf                       Prev;
  P3DQuaternionf                       Temp;

  Offset = P3DMath::Clampf(0.0f,1.0f,Offset);

  SegCount = Resolution - 1;

  P3DQuaternionf::MakeIdentity(Orientation);

  SegIndex  = (unsigned int)(Offset * Resolution);

  if (SegIndex >= Resolution)
   {
    SegIndex = SegCount;
   }

  SegFraction = (Offset - ((float)SegIndex * 1.0f /  Resolution)) * Resolution;

  if (SegIndex == SegCount)
   {
    Next.MakeIdentity();
   }
  else
   {
    Next.q[0] = SegOrientations[SegIndex * 4];
    Next.q[1] = SegOrientations[SegIndex * 4 + 1];
    Next.q[2] = SegOrientations[SegIndex * 4 + 2];
    Next.q[3] = SegOrientations[SegIndex * 4 + 3];

    P3DQuaternionf::Power(Next.q,0.5f);

    Next.Normalize();
   }

  if (SegIndex == 0)
   {
    Prev.MakeIdentity();
   }
  else
   {
    Prev.q[0] = -SegOrientations[(SegIndex - 1) * 4];
    Prev.q[1] = -SegOrientations[(SegIndex - 1) * 4 + 1];
    Prev.q[2] = -SegOrientations[(SegIndex - 1) * 4 + 2];
    Prev.q[3] =  SegOrientations[(SegIndex - 1) * 4 + 3];

    P3DQuaternionf::Power(Prev.q,0.5f);

    Prev.Normalize();
   }

  P3DQuaternionf::Slerp(Orientation,Prev.q,Next.q,SegFraction);

  P3DQuaternionf::Normalize(Orientation);

  while (SegIndex > 0)
   {
    P3DQuaternionf::CrossProduct( Temp.q,
                                 &(SegOrientations[(SegIndex - 1) * 4]),
                                  Orientation);

    Orientation[0] = Temp.q[0];
    Orientation[1] = Temp.q[1];
    Orientation[2] = Temp.q[2];
    Orientation[3] = Temp.q[3];

    SegIndex--;
   }
 }

void               P3DTubeAxisSegLine::GetOrientationAt
                                      (float              *Orientation,
                                       unsigned int        SegIndex) const
 {
  P3DQuaternionf::MakeIdentity(Orientation);

  if ((SegIndex == 0) || (SegIndex > Resolution))
   {
   }
  else
   {
    unsigned int   Index;
    P3DQuaternionf Temp1;

    for (Index = 0; Index < (SegIndex - 1); Index++)
     {
      P3DQuaternionf::CrossProduct( Temp1.q,
                                    Orientation,
                                   &(SegOrientations[Index * 4]));

      Orientation[0] = Temp1.q[0];
      Orientation[1] = Temp1.q[1];
      Orientation[2] = Temp1.q[2];
      Orientation[3] = Temp1.q[3];
     }

    if (SegIndex < Resolution)
     {
      P3DQuaternionf Temp2;

      SegIndex--;

      Temp2.q[0] = SegOrientations[SegIndex * 4];
      Temp2.q[1] = SegOrientations[SegIndex * 4 + 1];
      Temp2.q[2] = SegOrientations[SegIndex * 4 + 2];
      Temp2.q[3] = SegOrientations[SegIndex * 4 + 3];

      P3DQuaternionf::Power(Temp2.q,0.5f);

      Temp2.Normalize();

      P3DQuaternionf::CrossProduct(Temp1.q,Orientation,Temp2.q);

      Orientation[0] = Temp1.q[0];
      Orientation[1] = Temp1.q[1];
      Orientation[2] = Temp1.q[2];
      Orientation[3] = Temp1.q[3];
     }
   }
 }

void               P3DTubeAxisSegLine::SetSegOrientation
                                      (unsigned int        SegIndex,
                                       float              *Orientation)
 {
  if (SegIndex < (Resolution - 1))
   {
    SegOrientations[SegIndex * 4]     = Orientation[0];
    SegOrientations[SegIndex * 4 + 1] = Orientation[1];
    SegOrientations[SegIndex * 4 + 2] = Orientation[2];
    SegOrientations[SegIndex * 4 + 3] = Orientation[3];
   }
  else
   {
    /*FIXME: throw something here */
   }
 }

                   P3DTubeProfileCircle::P3DTubeProfileCircle
                                      (unsigned int        Resolution)
 {
  this->Resolution = Resolution;
 }

unsigned int       P3DTubeProfileCircle::GetResolution
                                      () const
 {
  return(Resolution);
 }

void               P3DTubeProfileCircle::GetPoint
                                      (float              &x,
                                       float              &y,
                                       unsigned int        t) const
 {
  float                              a;

  a = ((float)t / Resolution) * 2.0f * P3DMATH_PI;

  P3DMath::SinCosf(&x,&y,a);
 }

void               P3DTubeProfileCircle::GetNormal
                                      (float              &x,
                                       float              &y,
                                       unsigned int        t) const
 {
  GetPoint(x,y,t);
 }

                   P3DTubeProfileScaleLinear::P3DTubeProfileScaleLinear
                                      (float               min,
                                       float               max)
 {
  this->Min = min; this->Max = max;
 }

float              P3DTubeProfileScaleLinear::GetScale
                                      (float               t) const
 {
  return(Min + (Max - Min) * t);
 }

                   P3DTubeProfileScaleCustomCurve::P3DTubeProfileScaleCustomCurve
                                      (float               Min,
                                       float               Max,
                                       const P3DMathNaturalCubicSpline
                                                          *curve)
 {
  this->Min = Min; this->Max = Max;

  this->Curve.CopyFrom(*curve);
 }

float              P3DTubeProfileScaleCustomCurve::GetScale
                                      (float               t) const
 {
  return(Min + (Max - Min) * Curve.GetValue(t));
 }

float              P3DTubeProfileScaleCustomCurve::GetTangent
                                      (float               t) const
 {
  return(Curve.GetTangent(t));
 }

void               P3DTubeProfileScaleCustomCurve::GetRange
                                      (float              *Min,
                                       float              *Max) const
 {
  *Min = this->Min;
  *Max = this->Max;
 }

void               P3DTubeProfileScaleCustomCurve::SetRange
                                      (float               Min,
                                       float               Max)
 {
  this->Min = Min;
  this->Max = Max;
 }

const P3DMathNaturalCubicSpline
                  *P3DTubeProfileScaleCustomCurve::GetCurve
                                      () const
 {
  return(&Curve);
 }

void               P3DTubeProfileScaleCustomCurve::SetCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  this->Curve.CopyFrom(*Curve);
 }

