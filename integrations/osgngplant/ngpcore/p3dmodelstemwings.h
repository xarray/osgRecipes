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

#ifndef __P3DMODELSTEMWINGS_H__
#define __P3DMODELSTEMWINGS_H__

#include <ngpcore/p3dmodel.h>

#include <ngpcore/p3dmathspline.h>

#include <ngpcore/p3dmodelstemtube.h>

class P3DStemModelWings : public P3DStemModel
 {
  public           :

                   P3DStemModelWings  (const P3DStemModelTube
                                                          *ParentStemModel);

  virtual P3DStemModelInstance
                  *CreateInstance     (P3DMathRNG         *RNG,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       float               Offset,
                                       const P3DQuaternionf
                                                          *Orientation) const;

  virtual void     ReleaseInstance    (P3DStemModelInstance
                                                          *Instance) const;

  virtual P3DStemModel
                  *CreateCopy         () const;

  void             SetParent          (const P3DStemModelTube
                                                          *ParentStemModel)
   {
    this->ParentStemModel = ParentStemModel;
   }

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

  void             SetWingsAngle      (float               Angle);
  float            GetWingsAngle      () const;

  void             SetWidth           (float               Width);
  float            GetWidth           () const;

  void             SetSectionCount    (unsigned int        SectionCount);
  unsigned int     GetSectionCount    () const;

  void             SetCurvature       (const P3DMathNaturalCubicSpline
                                                          *Curve);

  const P3DMathNaturalCubicSpline
                  *GetCurvature       () const;

  void             SetThickness       (float               Thickness);
  float            GetThickness       () const;

  static void      MakeDefaultCurvatureCurve
                                     (P3DMathNaturalCubicSpline
                                                          &Curve);

  private          :

  unsigned int     GetParentAxisResolution
                                      () const
   {
    return(ParentStemModel->GetAxisResolution());
   }

  const P3DStemModelTube              *ParentStemModel;

  float                                WingsAngle; /* Currently unused */
  float                                Width;

  unsigned int                         SectionCount;
  P3DMathNaturalCubicSpline            Curvature;
  float                                Thickness;
 };

#endif

