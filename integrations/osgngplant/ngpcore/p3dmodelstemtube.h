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

#ifndef __P3DMODELSTEMTUBE_H__
#define __P3DMODELSTEMTUBE_H__

#include <ngpcore/p3dmodel.h>

#include <ngpcore/p3dmathspline.h>

enum
 {
  P3DTexCoordModeRelative,
  P3DTexCoordModeAbsolute
 };

class P3DStemModelTubeInstance : public P3DStemModelInstance
 {
  public           :

                   P3DStemModelTubeInstance
                                      (float               Length,
                                       unsigned int        AxisResolution,
                                       float               ProfileScaleBase,
                                       const P3DMathNaturalCubicSpline
                                                          *ScaleProfileCurve,
                                       unsigned int        ProfileResolution,
                                       unsigned int        UMode,
                                       float               UScale,
                                       unsigned int        VMode,
                                       float               VScale,
                                       const P3DMatrix4x4f*Transform);

  virtual
  unsigned int     GetVAttrCount      (unsigned int        Attr) const;
  virtual void     GetVAttrValue      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const;

  virtual
  unsigned int     GetPrimitiveCount  () const;
  virtual
  unsigned int     GetPrimitiveType   (unsigned int        PrimitiveIndex) const;

  virtual
  unsigned int     GetVAttrCountI     () const;

  virtual void     GetVAttrValueI     (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const;

  virtual float    GetLength          () const;
  virtual float    GetMinRadiusAt     (float               Offset) const;

  virtual void     GetWorldTransform  (float              *Transform) const;
  virtual void     GetAxisPointAt     (float              *Pos,
                                       float               Offset) const;
  virtual void     GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const;

  void             SetSegOrientation  (unsigned int        SegIndex,
                                       float              *Orientation);

  const float     *GetSegOrientation  (unsigned int        SegIndex) const
   {
    return(Axis.GetSegOrientation(SegIndex));
   }

  private          :

  void             CalcVertexPos      (float              *Pos,
                                       unsigned int        VertexIndex) const;


  void             CalcVertexNormal   (float              *Normal,
                                       unsigned int        VertexIndex) const;

  void             CalcVertexBiNormal (float              *BiNormal,
                                       unsigned int        VertexIndex) const;

  void             CalcVertexTangent  (float              *Tangent,
                                       unsigned int        VertexIndex) const;

  void             CalcVertexTexCoord (float              *TexCoord,
                                       unsigned int        VertexIndex) const;

  P3DMatrix4x4f                        WorldTransform;
  P3DTubeAxisSegLine                   Axis;
  P3DTubeProfileCircle                 Profile;
  P3DTubeProfileScaleCustomCurve       ProfileScale;
  unsigned int                         UMode;
  float                                UScale;
  unsigned int                         VMode;
  float                                VScale;
 };

class P3DStemModelTube : public P3DStemModel
 {
  public           :

                   P3DStemModelTube   ();
  virtual         ~P3DStemModelTube   ();

  virtual P3DStemModelInstance
                  *CreateInstance     (P3DMathRNG         *rng,
                                       const P3DStemModelInstance
                                                          *parent,
                                       float               offset,
                                       const P3DQuaternionf
                                                          *orientation) const;

  virtual void     ReleaseInstance    (P3DStemModelInstance
                                                          *instance) const;

  virtual P3DStemModel
                  *CreateCopy         () const;

  /* Per-attribute information */

  virtual
  unsigned int     GetVAttrCount      (unsigned int        Attr) const;

  virtual
  unsigned int     GetPrimitiveCount  () const;
  virtual
  unsigned int     GetPrimitiveType   (unsigned int        PrimitiveIndex) const;

  virtual void     FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase = 0) const;

  /* Per-index information */

  virtual
  unsigned int     GetVAttrCountI     () const;

  virtual
  unsigned int     GetIndexCount      (unsigned int        PrimitiveType) const;

  virtual void     FillIndexBuffer    (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase = 0) const;

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream) const;

  virtual void     Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version);

  void             SetLength          (float               Length);
  float            GetLength          () const;

  void             SetLengthV         (float               LengthV);
  float            GetLengthV         () const;

  void             SetLengthOffsetInfluenceCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve);

  const P3DMathNaturalCubicSpline
                  *GetLengthOffsetInfluenceCurve
                                      () const;

  void             SetAxisVariation   (float               AxisVariation);
  float            GetAxisVariation   () const;

  void             SetAxisResolution  (unsigned int        Resolution);
  unsigned int     GetAxisResolution  () const;

  void             SetProfileResolution
                                      (unsigned int        Resolution);
  unsigned int     GetProfileResolution
                                      () const;

  void             SetProfileScaleBase(float               Scale);
  float            GetProfileScaleBase() const;

  void             SetProfileScaleCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve);

  const P3DMathNaturalCubicSpline
                  *GetProfileScaleCurve
                                      () const;

  void             SetPhototropismCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve);

  const P3DMathNaturalCubicSpline
                  *GetPhototropismCurve
                                      () const;

  void             SetTexCoordUMode   (unsigned int        Mode);
  unsigned int     GetTexCoordUMode   () const;
  void             SetTexCoordUScale  (float               Scale);
  float            GetTexCoordUScale  () const;
  void             SetTexCoordVMode   (unsigned int        Mode);
  unsigned int     GetTexCoordVMode   () const;
  void             SetTexCoordVScale  (float               Scale);
  float            GetTexCoordVScale  () const;

  static void      MakeDefaultLengthOffsetInfluenceCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve);
  static void      MakeDefaultProfileScaleCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve);

  static void      MakeDefaultPhototropismCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve);

  private          :

  void             ApplyPhototropism  (P3DStemModelTubeInstance
                                                          *Instance) const;

  void             ApplyAxisVariation (P3DMathRNG         *RNG,
                                       P3DStemModelTubeInstance
                                                          *Instance) const;

  float                                Length;
  float                                LengthV;
  P3DMathNaturalCubicSpline            LengthOffsetInfluenceCurve;
  float                                AxisVariation;
  unsigned int                         AxisResolution;
  float                                ProfileScaleBase;
  P3DMathNaturalCubicSpline            ProfileScaleCurve;
  unsigned int                         ProfileResolution;

  P3DMathNaturalCubicSpline            PhototropismCurve;

  /* Texture coordinate generation parameters */

  unsigned int                         UMode;
  float                                UScale;
  unsigned int                         VMode;
  float                                VScale;
 };

#endif

