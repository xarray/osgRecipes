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

#include <ngpcore/p3dmath.h>
#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dbalgbase.h>

                   P3DBranchingAlgBase::P3DBranchingAlgBase
                                      ()
 {
  Rotation  = 0.0f;
 }

P3DBranchingAlg   *P3DBranchingAlgBase::CreateCopy
                                      () const
 {
  P3DBranchingAlgBase                 *Result;

  Result = new P3DBranchingAlgBase();

  Result->Rotation = Rotation;

  return(Result);
 }

float              P3DBranchingAlgBase::GetRotationAngle
                                      () const
 {
  return(Rotation);
 }

void               P3DBranchingAlgBase::SetRotationAngle
                                      (float               Rotation)
 {
  this->Rotation = P3DMath::Clampf(0.0f,P3DMATH_2PI,Rotation);
 }

void               P3DBranchingAlgBase::CreateBranches
                                      (P3DBranchingFactory          *Factory,
                                       const P3DStemModelInstance   *Parent P3D_UNUSED_ATTR,
                                       P3DMathRNG                   *RNG P3D_UNUSED_ATTR)
 {
  P3DQuaternionf                       Orientation;

  Orientation.FromAxisAndAngle(0.0f,1.0f,0.0f,Rotation);

  Factory->GenerateBranch(0.0f,&Orientation);
 }

void               P3DBranchingAlgBase::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","BranchingAlg","Base");

  FmtStream.WriteString("sf","RotAngle",Rotation);
 }

void               P3DBranchingAlgBase::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream P3D_UNUSED_ATTR,
                                       const P3DFileVersion
                                                          *Version P3D_UNUSED_ATTR)
 {
  float                                FloatValue;

  SourceStream->ReadFmtStringTagged("RotAngle","f",&FloatValue);
  SetRotationAngle(FloatValue);
 }

