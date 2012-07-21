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

#ifndef __P3DBALGSTD_H__
#define __P3DBALGSTD_H__

#include <ngpcore/p3dmathspline.h>

#include <ngpcore/p3dmodel.h>

class P3DBranchingAlgStd : public P3DBranchingAlg
 {
  public           :

                   P3DBranchingAlgStd ();

  virtual P3DBranchingAlg
                  *CreateCopy         () const;

  float            GetDensity         () const;
  void             SetDensity         (float                         Density);

  float            GetDensityV        () const;
  void             SetDensityV        (float                         DensityV);

  unsigned int     GetMultiplicity    () const;
  void             SetMultiplicity    (unsigned int                  Multiplicity);

  float            GetRevAngle        () const;
  void             SetRevAngle        (float                         RevAngle);

  float            GetRevAngleV       () const;
  void             SetRevAngleV       (float                         RevAngleV);

  float            GetMinOffset       () const;
  void             SetMinOffset       (float                         MinOffset);

  float            GetMaxOffset       () const;
  void             SetMaxOffset       (float                         MaxOffset);

  void             SetDeclinationCurve(const P3DMathNaturalCubicSpline
                                                                    *Curve);

  const P3DMathNaturalCubicSpline
                  *GetDeclinationCurve() const;

  float            GetDeclinationV    () const;
  void             SetDeclinationV    (float                         DeclinationV);

  float            GetRotationAngle   () const;
  void             SetRotationAngle   (float                         RotAngle);

  virtual void     CreateBranches     (P3DBranchingFactory          *Factory,
                                       const P3DStemModelInstance   *Parent,
                                       P3DMathRNG                   *RNG);

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream) const;

  virtual void     Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version);

  static void      MakeDefaultDeclinationCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve);

  private          :

  float                                Density; /* branches per meter */
  float                                DensityV;
  unsigned int                         Multiplicity;
  float                                RevAngle; /* rotation around stem step */
  float                                RevAngleV;

  float                                MinOffset;
  float                                MaxOffset;
  P3DMathNaturalCubicSpline            DeclinationCurve;
  float                                DeclinationV;

  float                                Rotation; /* branch start rotation around its Y axis */
 };

#endif

