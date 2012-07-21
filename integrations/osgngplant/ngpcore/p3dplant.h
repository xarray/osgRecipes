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

#ifndef __P3DPLANT_H__
#define __P3DPLANT_H__

#include <ngpcore/p3dtypes.h>
#include <ngpcore/p3dmathspline.h>

class P3DTubeAxis
 {
  public           :

  virtual         ~P3DTubeAxis        () {};

  virtual
  unsigned int     GetResolution      () const = 0;

  virtual
  float            GetLength          () const = 0;

  virtual
  void             GetPointAt         (float              *Pos,
                                       float               Offset) const = 0;

  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       float               Offset) const = 0;
  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       unsigned int        SegIndex) const = 0;
 };

class P3DTubeProfile
 {
  public           :

  virtual         ~P3DTubeProfile     () {};

  virtual
  unsigned int     GetResolution      () const = 0;

  virtual void     GetPoint           (float              &x,
                                       float              &y,
                                       unsigned int        t) const = 0;

  virtual void     GetNormal          (float              &x,
                                       float              &y,
                                       unsigned int        t) const = 0;
 };

class P3DTubeProfileScale
 {
  public           :

  virtual         ~P3DTubeProfileScale() {};

  virtual float    GetScale           (float               t) const = 0;
 };

/* some concrete classes */

class P3DTubeAxisLine : public P3DTubeAxis
 {
  public           :

                   P3DTubeAxisLine    (float               length,
                                       unsigned int        resolution);

  virtual
  unsigned int     GetResolution      () const;

  virtual
  float            GetLength          () const;

  virtual
  void             GetPointAt         (float              *Pos,
                                       float               Offset) const;

  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       float               Offset) const;
  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       unsigned int        SegIndex) const;

  private          :

  unsigned int     Resolution;
  float            Length;
 };

class P3DTubeAxisSegLine : public P3DTubeAxis
 {
  public           :

                   P3DTubeAxisSegLine (float               Length,
                                       unsigned int        Resolution);

  virtual         ~P3DTubeAxisSegLine ();

  virtual
  unsigned int     GetResolution      () const;

  virtual
  float            GetLength          () const;

  virtual
  void             GetPointAt         (float              *Pos,
                                       float               Offset) const;

  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       float               Offset) const;
  virtual
  void             GetOrientationAt   (float              *Orientation,
                                       unsigned int        SegIndex) const;

  void             SetSegOrientation  (unsigned int        SegIndex,
                                       float              *Orientation);

  const float     *GetSegOrientation  (unsigned int        SegIndex) const
   {
    return(&(SegOrientations[SegIndex * 4]));
   }

  private          :

  unsigned int     Resolution;
  float            Length;
  float           *SegOrientations;
 };

class P3DTubeProfileCircle : public P3DTubeProfile
 {
  public           :

                   P3DTubeProfileCircle
                                      (unsigned int        resolution);

  virtual
  unsigned int     GetResolution      () const;

  virtual void     GetPoint           (float              &x,
                                       float              &y,
                                       unsigned int        t) const;

  virtual void     GetNormal          (float              &x,
                                       float              &y,
                                       unsigned int        t) const;

  private          :

  unsigned int     Resolution;
 };

class P3DTubeProfileScaleLinear : public P3DTubeProfileScale
 {
  public           :

                   P3DTubeProfileScaleLinear
                                      (float               min,
                                       float               max);

  virtual float    GetScale           (float               t) const;

  private          :

  float            Min;
  float            Max;
 };

class P3DTubeProfileScaleCustomCurve : public P3DTubeProfileScale
 {
  public           :

                   P3DTubeProfileScaleCustomCurve
                                      (float               Min,
                                       float               Max,
                                       const P3DMathNaturalCubicSpline
                                                          *curve);

  virtual float    GetScale           (float               t) const;
  virtual float    GetTangent         (float               t) const;

  void             GetRange           (float              *Min,
                                       float              *Max) const;

  void             SetRange           (float               Min,
                                       float               Max);

  const P3DMathNaturalCubicSpline
                  *GetCurve           () const;

  void             SetCurve           (const P3DMathNaturalCubicSpline
                                                          *Curve);

  private          :

  float                                Min;
  float                                Max;
  P3DMathNaturalCubicSpline            Curve;
 };

#endif

