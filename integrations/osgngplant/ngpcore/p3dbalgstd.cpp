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

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dbalgstd.h>

                   P3DBranchingAlgStd::P3DBranchingAlgStd
                                      ()
 {
  Density      = 1.0f;
  DensityV     = 0.0f;
  Multiplicity = 1;
  RevAngle     = 0.0f;
  RevAngleV    = 0.0f;
  MinOffset    = 0.0f;
  MaxOffset    = 1.0f;

  MakeDefaultDeclinationCurve(DeclinationCurve);

  DeclinationV = 0.0f;

  Rotation = 0.0f;
 }

void               P3DBranchingAlgStd::MakeDefaultDeclinationCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(0.5f);
 }

P3DBranchingAlg   *P3DBranchingAlgStd::CreateCopy
                                      () const
 {
  P3DBranchingAlgStd                  *Result;

  Result = new P3DBranchingAlgStd();

  Result->Density      = Density;
  Result->DensityV     = DensityV;
  Result->Multiplicity = Multiplicity;
  Result->RevAngle     = RevAngle;
  Result->RevAngleV    = RevAngleV;
  Result->MinOffset    = MinOffset;
  Result->MaxOffset    = MaxOffset;

  Result->DeclinationCurve.CopyFrom(DeclinationCurve);

  Result->DeclinationV = DeclinationV;

  Result->Rotation = Rotation;

  return(Result);
 }

float              P3DBranchingAlgStd::GetDensity
                                      () const
 {
  return(Density);
 }

void               P3DBranchingAlgStd::SetDensity
                                      (float                         Density)
 {
  if (Density < 0.0f)
   {
   }
  else
   {
    this->Density = Density;
   }
 }

float              P3DBranchingAlgStd::GetDensityV
                                      () const
 {
  return(DensityV);
 }

void               P3DBranchingAlgStd::SetDensityV
                                      (float                         DensityV)
 {
  this->DensityV = P3DMath::Clampf(0.0f,1.0f,DensityV);
 }

unsigned int       P3DBranchingAlgStd::GetMultiplicity
                                      () const
 {
  return(Multiplicity);
 }

void               P3DBranchingAlgStd::SetMultiplicity
                                      (unsigned int                  Multiplicity)
 {
  if (Multiplicity > 0)
   {
    this->Multiplicity = Multiplicity;
   }
  else
   {
    this->Multiplicity = 1;
   }
 }

float              P3DBranchingAlgStd::GetRevAngle
                                      () const
 {
  return(RevAngle);
 }

void               P3DBranchingAlgStd::SetRevAngle
                                      (float                         RevAngle)
 {
  this->RevAngle = RevAngle;
 }

float              P3DBranchingAlgStd::GetRevAngleV
                                      () const
 {
  return(RevAngleV);
 }

void               P3DBranchingAlgStd::SetRevAngleV
                                      (float                         RevAngleV)
 {
  this->RevAngleV = P3DMath::Clampf(0.0f,1.0f,RevAngleV);
 }

float              P3DBranchingAlgStd::GetMinOffset
                                      () const
 {
  return(MinOffset);
 }

void               P3DBranchingAlgStd::SetMinOffset
                                      (float                         MinOffset)
 {
  this->MinOffset = P3DMath::Clampf(0.0f,1.0f,MinOffset);
 }

float              P3DBranchingAlgStd::GetMaxOffset
                                      () const
 {
  return(MaxOffset);
 }

void               P3DBranchingAlgStd::SetMaxOffset
                                      (float                         MaxOffset)
 {
  this->MaxOffset = P3DMath::Clampf(0.0f,1.0f,MaxOffset);
 }

void               P3DBranchingAlgStd::SetDeclinationCurve
                                      (const P3DMathNaturalCubicSpline
                                                                    *Curve)
 {
  DeclinationCurve.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DBranchingAlgStd::GetDeclinationCurve
                                      () const
 {
  return(&DeclinationCurve);
 }

float              P3DBranchingAlgStd::GetDeclinationV
                                      () const
 {
  return(DeclinationV);
 }

void               P3DBranchingAlgStd::SetDeclinationV
                                      (float                         DeclinationV)
 {
  this->DeclinationV = P3DMath::Clampf(0.0f,1.0f,DeclinationV);
 }

float              P3DBranchingAlgStd::GetRotationAngle
                                      () const
 {
  return(Rotation);
 }

void               P3DBranchingAlgStd::SetRotationAngle
                                      (float                         RotAngle)
 {
  this->Rotation = RotAngle;
 }

void               P3DBranchingAlgStd::CreateBranches
                                      (P3DBranchingFactory          *Factory,
                                       const P3DStemModelInstance   *Parent,
                                       P3DMathRNG                   *RNG)
 {
  unsigned int                         BranchCount;
  float                                CurrOffset;
  float                                OffsetStep;
  float                                CurrRevAngle;
  float                                BranchRegionLength;

  CurrRevAngle = 0.0f;

  BranchRegionLength = Parent->GetLength() * (MaxOffset - MinOffset);

  if (BranchRegionLength <= 0.0f)
   {
    return;
   }

  if (RNG != 0)
   {
    BranchCount = (int)(BranchRegionLength * (Density + RNG->UniformFloat(-DensityV,DensityV) * Density));
   }
  else
   {
    BranchCount = (int)(BranchRegionLength * Density);
   }

  BranchCount /= Multiplicity;

  if (BranchCount > 0)
   {
    P3DQuaternionf                     orientation;
    P3DQuaternionf                     decl;
    P3DQuaternionf                     rev;
    P3DQuaternionf                     Rot;
    P3DQuaternionf                     TempRot;
    float                              MultRevAngleStep;

    OffsetStep = (MaxOffset - MinOffset) / (BranchCount + 1);
    CurrOffset = MinOffset + OffsetStep;

    MultRevAngleStep = (2.0f * P3DMATH_PI) / Multiplicity;

    for (unsigned int i = 0; i < BranchCount; i++)
     {
      float        BaseDeclination;

      for (unsigned int MultIndex = 0; MultIndex < Multiplicity; MultIndex++)
       {
        BaseDeclination = DeclinationCurve.GetValue(CurrOffset);

        if (RNG != 0)
         {
          BaseDeclination += RNG->UniformFloat(-DeclinationV,DeclinationV) * BaseDeclination;
         }

        decl.FromAxisAndAngle(0.0f,0.0f,-1.0f,P3DMATH_DEG2RAD(180.0f * BaseDeclination));
        rev.FromAxisAndAngle(0.0f,1.0f,0.0f,CurrRevAngle + MultRevAngleStep * MultIndex);

        Rot.FromAxisAndAngle(0.0f,1.0f,0.0f,Rotation);

        P3DQuaternionf::CrossProduct(TempRot.q,rev.q,decl.q);
        P3DQuaternionf::CrossProduct(orientation.q,TempRot.q,Rot.q);

        Factory->GenerateBranch(CurrOffset,&orientation);
       }

      if (RNG != 0)
       {
        CurrRevAngle += RevAngle + (RevAngle * RNG->UniformFloat(-RevAngleV,RevAngleV));
       }
      else
       {
        CurrRevAngle += RevAngle;
       }

      CurrOffset += OffsetStep;
     }
   }
 }

void               P3DBranchingAlgStd::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","BranchingAlg","Std");

  FmtStream.WriteString("sf","Density",Density);
  FmtStream.WriteString("sf","DensityV",DensityV);

  FmtStream.WriteString("su","Multiplicity",Multiplicity);

  FmtStream.WriteString("sf","RevAngle",RevAngle);
  FmtStream.WriteString("sf","RevAngleV",RevAngleV);

  FmtStream.WriteString("sf","RotAngle",Rotation);

  FmtStream.WriteString("sf","MinOffset",MinOffset);
  FmtStream.WriteString("sf","MaxOffset",MaxOffset);

  FmtStream.WriteString("ss","DeclinationCurve","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&DeclinationCurve);
  FmtStream.WriteString("sf","DeclinationV",DeclinationV);
 }

void               P3DBranchingAlgStd::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version P3D_UNUSED_ATTR)
 {
  char                                 StrValue[255 + 1];
  float                                FloatValue;
  unsigned int                         UintValue;

  SourceStream->ReadFmtStringTagged("Density","f",&FloatValue);
  SetDensity(FloatValue);
  SourceStream->ReadFmtStringTagged("DensityV","f",&FloatValue);
  SetDensityV(FloatValue);

  if (Version->Minor > 1)
   {
    SourceStream->ReadFmtStringTagged("Multiplicity","u",&UintValue);
    SetMultiplicity(UintValue);
   }
  else
   {
    Multiplicity = 1;
   }

  SourceStream->ReadFmtStringTagged("RevAngle","f",&FloatValue);
  SetRevAngle(FloatValue);
  SourceStream->ReadFmtStringTagged("RevAngleV","f",&FloatValue);
  SetRevAngleV(FloatValue);

  SourceStream->ReadFmtStringTagged("RotAngle","f",&FloatValue);
  SetRotationAngle(FloatValue);

  SourceStream->ReadFmtStringTagged("MinOffset","f",&FloatValue);
  SetMinOffset(FloatValue);
  SourceStream->ReadFmtStringTagged("MaxOffset","f",&FloatValue);
  SetMaxOffset(FloatValue);

  SourceStream->ReadFmtStringTagged("DeclinationCurve","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(&DeclinationCurve,SourceStream,StrValue);
  SourceStream->ReadFmtStringTagged("DeclinationV","f",&FloatValue);
  SetDeclinationV(FloatValue);
 }

