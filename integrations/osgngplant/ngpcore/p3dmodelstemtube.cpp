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

#include <stdio.h>
#include <string.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dtypes.h>

#include <ngpcore/p3dmath.h>
#include <ngpcore/p3dmathspline.h>

#include <ngpcore/p3dplant.h>

#include <ngpcore/p3dmodel.h>
#include <ngpcore/p3dmodelstemtube.h>

enum /* These constants are needed for pre-0.9.3 compatibility only */
 {
  P3DPhototropismModePositive,
  P3DPhototropismModeNegative
 };

                   P3DStemModelTubeInstance::P3DStemModelTubeInstance
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
                                       const P3DMatrix4x4f*Transform)
                   : Axis(Length,AxisResolution),
                     Profile(ProfileResolution),
                     ProfileScale(0.0f,ProfileScaleBase,ScaleProfileCurve)
 {
  if (Transform == 0)
   {
    P3DMatrix4x4f::MakeIdentity(WorldTransform.m);
   }
  else
   {
    WorldTransform = *Transform;
   }

  this->UMode  = UMode;
  this->UScale = UScale;
  this->VMode  = VMode;
  this->VScale = VScale;
 }

unsigned int       P3DStemModelTubeInstance::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    return((Axis.GetResolution() + 1) * (Profile.GetResolution() + 1));
   }
  else
   {
    return((Axis.GetResolution() + 1) * Profile.GetResolution());
   }
 }

void               P3DStemModelTubeInstance::GetVAttrValue
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if      (Attr == P3D_ATTR_VERTEX)
   {
    CalcVertexPos(Value,Index);
   }
  else if (Attr == P3D_ATTR_NORMAL)
   {
    CalcVertexNormal(Value,Index);
   }
  else if (Attr == P3D_ATTR_BINORMAL)
   {
    CalcVertexBiNormal(Value,Index);
   }
  else if (Attr == P3D_ATTR_TANGENT)
   {
    CalcVertexTangent(Value,Index);
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    CalcVertexTexCoord(Value,Index);
   }
  else
   {
    /*FIXME: throw something here */
   }
 }

void               P3DStemModelTubeInstance::CalcVertexPos
                                      (float              *Pos,
                                       unsigned int        VertexIndex) const
 {
  unsigned int                         SegIndex;
  float                                HeightFraction;
  float                                PScale;
  P3DQuaternionf                       SegOrient;
  P3DVector3f                          VertexPoint;
  P3DVector3f                          AxisPoint;

  SegIndex = VertexIndex / Profile.GetResolution();

  if (SegIndex > Axis.GetResolution())
   {
    /*FIXME: it's an error condition, must I throw something here? */

    Pos[0] = Pos[1] = Pos[2] = 0.0f;

    return;
   }

  Profile.GetPoint(VertexPoint.X(),VertexPoint.Z(),VertexIndex % Profile.GetResolution());

  HeightFraction = ((float)(Axis.GetResolution() - SegIndex)) / Axis.GetResolution();
  PScale         = ProfileScale.GetScale(HeightFraction);

  VertexPoint.X() *= PScale;
  VertexPoint.Y()  = 0.0f;
  VertexPoint.Z() *= PScale;

  Axis.GetOrientationAt(SegOrient.q,Axis.GetResolution() - SegIndex);

  P3DQuaternionf::RotateVector(VertexPoint.v,SegOrient.q);

  Axis.GetPointAt(AxisPoint.v,HeightFraction);
  VertexPoint.Add(AxisPoint.v);

  P3DVector3f::MultMatrix(Pos,&WorldTransform,VertexPoint.v);
 }

void               P3DStemModelTubeInstance::CalcVertexNormal
                                      (float              *Normal,
                                       unsigned int        VertexIndex) const
 {
  unsigned int                         SegIndex;
  float                                HeightFraction;
  P3DQuaternionf                       SegOrient;
  P3DVector3f                          VertexNormal;
  P3DMatrix4x4f                        Rotation;

  P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

  SegIndex = VertexIndex / Profile.GetResolution();

  if (SegIndex > Axis.GetResolution())
   {
    /*FIXME: it's an error condition, must I throw something here? */

    Normal[0] = Normal[2] = 0.0f; Normal[1] = 1.0f;

    return;
   }

  HeightFraction = ((float)(Axis.GetResolution() - SegIndex)) / Axis.GetResolution();

  Axis.GetOrientationAt(SegOrient.q,Axis.GetResolution() - SegIndex);

  Profile.GetNormal(VertexNormal.X(),VertexNormal.Z(),VertexIndex % Profile.GetResolution());

  VertexNormal.Y() = -ProfileScale.GetTangent(HeightFraction);
  VertexNormal.Normalize();
  P3DQuaternionf::RotateVector(VertexNormal.v,SegOrient.q);
  VertexNormal.MultMatrix(&Rotation);
  VertexNormal.Normalize();

  Normal[0] = VertexNormal.X();
  Normal[1] = VertexNormal.Y();
  Normal[2] = VertexNormal.Z();
 }

void               P3DStemModelTubeInstance::CalcVertexBiNormal
                                      (float              *BiNormal,
                                       unsigned int        VertexIndex) const
 {
  unsigned int                         SegIndex;
  float                                HeightFraction;
  P3DQuaternionf                       SegOrient;
  P3DVector3f                          VertexBiNormal(0.0f,1.0f,0.0f);
  P3DMatrix4x4f                        Rotation;

  P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

  SegIndex = VertexIndex / Profile.GetResolution();

  if (SegIndex <= Axis.GetResolution())
   {
    HeightFraction = ((float)(Axis.GetResolution() - SegIndex)) / Axis.GetResolution();

    Axis.GetOrientationAt(SegOrient.q,Axis.GetResolution() - SegIndex);

    P3DQuaternionf::RotateVector(VertexBiNormal.v,SegOrient.q);
   }
  else
   {
    /*FIXME: it's an error condition, must I throw something here? */
   }

  VertexBiNormal.MultMatrix(&Rotation);
  VertexBiNormal.Normalize();

  BiNormal[0] = VertexBiNormal.X();
  BiNormal[1] = VertexBiNormal.Y();
  BiNormal[2] = VertexBiNormal.Z();
 }

/*FIXME: not very optimal implementation */
void               P3DStemModelTubeInstance::CalcVertexTangent
                                      (float              *Tangent,
                                       unsigned int        VertexIndex) const
 {
  float                                Normal[3];
  float                                BiNormal[3];

  CalcVertexNormal(Normal,VertexIndex);
  CalcVertexBiNormal(BiNormal,VertexIndex);

  P3DVector3f::CrossProduct(Tangent,BiNormal,Normal);
 }

void               P3DStemModelTubeInstance::CalcVertexTexCoord
                                      (float              *TexCoord,
                                       unsigned int        VertexIndex) const
 {
  unsigned int                         SegIndex;
  float                                HeightFraction;

  SegIndex = VertexIndex / (Profile.GetResolution() + 1);

  if (SegIndex > Axis.GetResolution())
   {
    /*FIXME: it's an error condition, must I throw something here? */

    TexCoord[0] = TexCoord[1] = 0.0f;

    return;
   }

  HeightFraction = ((float)(Axis.GetResolution() - SegIndex)) / Axis.GetResolution();

  if (VMode == P3DTexCoordModeRelative)
   {
    TexCoord[1] = HeightFraction * VScale;
   }
  else
   {
    TexCoord[1] = HeightFraction * Axis.GetLength() / Axis.GetResolution() * VScale;
   }

  TexCoord[0] = ((float)(VertexIndex % (Profile.GetResolution() + 1))) / (Profile.GetResolution()) * UScale;
 }

unsigned int       P3DStemModelTubeInstance::GetVAttrCountI
                                      () const
 {
  return((Profile.GetResolution() + 1) * (Axis.GetResolution() + 1));
 }

void               P3DStemModelTubeInstance::GetVAttrValueI
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if (Index >= GetVAttrCountI())
   {
    throw P3DExceptionGeneric("vertex index out of range");
   }

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    CalcVertexTexCoord(Value,Index);
   }
  else
   {
    unsigned int   AttrIndex;
    unsigned int   ProfileResolution;

    ProfileResolution = Profile.GetResolution();

    AttrIndex = ((Index / (ProfileResolution + 1)) * ProfileResolution) +
                 ((Index % (ProfileResolution + 1)) % ProfileResolution);

    if      (Attr == P3D_ATTR_VERTEX)
     {
      CalcVertexPos(Value,AttrIndex);
     }
    else if (Attr == P3D_ATTR_NORMAL)
     {
      CalcVertexNormal(Value,AttrIndex);
     }
    else if (Attr == P3D_ATTR_BINORMAL)
     {
      CalcVertexBiNormal(Value,AttrIndex);
     }
    else if (Attr == P3D_ATTR_TANGENT)
     {
      CalcVertexTangent(Value,AttrIndex);
     }
   }
 }

unsigned int       P3DStemModelTubeInstance::GetPrimitiveCount
                                      () const
 {
  return(Profile.GetResolution() * Axis.GetResolution());
 }

unsigned int       P3DStemModelTubeInstance::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

float              P3DStemModelTubeInstance::GetLength
                                      () const
 {
  return(Axis.GetLength());
 }

float              P3DStemModelTubeInstance::GetMinRadiusAt
                                      (float               Offset) const
 {
  float                                MinRadius;
  float                                X,Z;

  /*FIXME: not exactly correct - only one cross-section point is used */
  /*       for calculation                                            */

  Profile.GetPoint(X,Z,0);

  MinRadius = P3DMath::Sqrtf(X * X + Z * Z) * ProfileScale.GetScale(Offset);

  return(MinRadius);
 }

void               P3DStemModelTubeInstance::GetWorldTransform
                                       (float              *Transform) const
 {
  for (unsigned int i = 0; i < 16; i++)
   {
    Transform[i] = WorldTransform.m[i];
   }
 }

void               P3DStemModelTubeInstance::GetAxisPointAt
                                      (float              *Pos,
                                       float               Offset) const
 {
  Axis.GetPointAt(Pos,Offset);
 }

void               P3DStemModelTubeInstance::GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const
 {
  Axis.GetOrientationAt(Orientation,Offset);
 }

void               P3DStemModelTubeInstance::SetSegOrientation
                                      (unsigned int        SegIndex,
                                       float              *Orientation)
 {
  Axis.SetSegOrientation(SegIndex,Orientation);
 }

                   P3DStemModelTube::P3DStemModelTube
                                      ()
 {
  Length            = 1.0f;
  LengthV           = 0.0f;
  AxisVariation     = 0.0f;
  ProfileScaleBase  = 1.0f;
  AxisResolution    = 5;
  ProfileResolution = 8;

  MakeDefaultLengthOffsetInfluenceCurve(LengthOffsetInfluenceCurve);
  MakeDefaultProfileScaleCurve(ProfileScaleCurve);
  MakeDefaultPhototropismCurve(PhototropismCurve);

  UMode  = P3DTexCoordModeRelative;
  UScale = 1.0f;
  VMode  = P3DTexCoordModeRelative;
  VScale = 1.0f;
 }

void               P3DStemModelTube::MakeDefaultLengthOffsetInfluenceCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(1.0f);
 }

void               P3DStemModelTube::MakeDefaultProfileScaleCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetLinear(0.0f,1.0f,1.0f,0.0f);
 }

void               P3DStemModelTube::MakeDefaultPhototropismCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(0.5f);
 }

P3DStemModel      *P3DStemModelTube::CreateCopy
                                      () const
 {
  P3DStemModelTube                    *Result;

  Result = new P3DStemModelTube();

  Result->Length            = Length;
  Result->LengthV           = LengthV;
  Result->AxisVariation     = AxisVariation;
  Result->ProfileScaleBase  = ProfileScaleBase;
  Result->AxisResolution    = AxisResolution;
  Result->ProfileResolution = ProfileResolution;

  Result->LengthOffsetInfluenceCurve.CopyFrom(LengthOffsetInfluenceCurve);
  Result->ProfileScaleCurve.CopyFrom(ProfileScaleCurve);
  Result->PhototropismCurve.CopyFrom(PhototropismCurve);

  Result->UMode  = UMode;
  Result->UScale = UScale;
  Result->VMode  = VMode;
  Result->VScale = VScale;

  return(Result);
 }

                   P3DStemModelTube::~P3DStemModelTube
                                      ()
 {
 }

/*
 * CurrOrientation is in parent seg space
 * DestVector is in parent seg space
 */
static void        ApplyPhototropismToSegment
                                      (P3DQuaternionf       *NewOrientation,
                                       const float          *CurrOrientation,
                                       const float          *DestVector,
                                       float                 Factor)
 {
  // Find  branch direction after applying phototopism effect

  P3DVector3f SegmentY;

  SegmentY.Set(0.0f,1.0f,0.0);

  P3DQuaternionf::RotateVector(SegmentY.v,CurrOrientation);

  float CosA = P3DVector3f::ScalarProduct(DestVector,SegmentY.v);

  if (CosA >= 1.0f)
   {
    NewOrientation->Set(CurrOrientation);

    return; // nothing to do, SegmentY already points in DestVector direction
   }

  float Angle = P3DMath::ACosf(CosA);

  P3DVector3f Axis;

  if (Angle >= P3DMATH_PI)
   {
    Axis.Set(0.0f,0.0f,1.0f);
   }
  else
   {
    P3DVector3f::CrossProduct(Axis.v,SegmentY.v,DestVector);
    Axis.Normalize();
   }

  P3DQuaternionf Rotation;

  Rotation.FromAxisAndAngle(Axis.X(),Axis.Y(),Axis.Z(),Angle * Factor);

  P3DQuaternionf::RotateVector(SegmentY.v,Rotation.q);

  // Now find transformation from parent branch to SegmentY
  // CosA = P3DVector3f::ScalarProduct(SegmentY,(0,1,0));
  CosA = SegmentY.Y();

  if (CosA >= 1.0f)
   {
    NewOrientation->MakeIdentity();
   }
  else if (CosA <= -1.0f)
   {
    // P3Quaternion::FromAxisAndAngle((0,0,1),PI);
    NewOrientation->Set(0.0f,0.0f,1.0f,0.0f);
   }
  else
   {
    //Axis = P3DVector3f::CrossProduct(Axis.v,(0,1,0),SegmentY);

    Axis.Set(SegmentY.Z(),0.0f,-SegmentY.X());
    Axis.Normalize();

    NewOrientation->FromAxisAndAngle(Axis.X(),Axis.Y(),Axis.Z(),P3DMath::ACosf(CosA));
   }
 }

void               P3DStemModelTube::ApplyPhototropism
                                      (P3DStemModelTubeInstance
                                                          *Instance) const
 {
  unsigned int                         SegIndex;
  P3DVector3f                          YVector;
  P3DQuaternionf                       SegOrientation;
  P3DMatrix4x4f                        WorldTransform;
  P3DMatrix4x4f                        WorldRotation;
  float                                Factor;

  Instance->GetWorldTransform(WorldTransform.m);

  P3DMatrix4x4f::GetRotationOnly(WorldRotation.m,WorldTransform.m);

  YVector.Set(0.0f,1.0f,0.0f);

  YVector.MultMatrixTranspose(&WorldRotation);

  for (SegIndex = 0; SegIndex < (AxisResolution - 1); SegIndex++)
   {
    if (AxisResolution > 2)
     {
      Factor = PhototropismCurve.GetValue((float)SegIndex / (AxisResolution - 2));
     }
    else
     {
      Factor = P3DMath::Clampf(0.0f,1.0f,PhototropismCurve.GetValue(0.5f));
     }

    Factor = (Factor * 2.0f) - 1.0f;

    if (Factor < 0.0f)
     {
      P3DVector3f YVectorNeg;

      YVectorNeg.Set(-YVector.X(),-YVector.Y(),-YVector.Z());

      ApplyPhototropismToSegment
       (&SegOrientation,Instance->GetSegOrientation(SegIndex),YVectorNeg.v,-Factor);
     }
    else
     {
      ApplyPhototropismToSegment
       (&SegOrientation,Instance->GetSegOrientation(SegIndex),YVector.v,Factor);
     }

    Instance->SetSegOrientation(SegIndex,SegOrientation.q);

    P3DQuaternionf::RotateVectorInv(YVector.v,SegOrientation.q);
   }
 }

void               P3DStemModelTube::ApplyAxisVariation
                                      (P3DMathRNG         *RNG,
                                       P3DStemModelTubeInstance
                                                          *Instance) const
 {
  float                                Angle1;
  float                                Angle2;
  float                                SinAngle1;
  float                                CosAngle1;
  P3DQuaternionf                       Q;
  unsigned int                         SegIndex;

  if (RNG == 0)
   {
    return;
   }

  for (SegIndex = 0; SegIndex < (AxisResolution - 1); SegIndex++)
   {
    Angle1 = RNG->UniformFloat(0,P3DMATH_PI * 2.0f);
    Angle2 = RNG->UniformFloat(-AxisVariation,AxisVariation) * P3DMATH_PI;

    P3DMath::SinCosf(&SinAngle1,&CosAngle1,Angle1);

    Q.FromAxisAndAngle(CosAngle1,0.0f,SinAngle1,Angle2);

    Instance->SetSegOrientation(SegIndex,Q.q);
   }
 }

P3DStemModelInstance
                  *P3DStemModelTube::CreateInstance
                                      (P3DMathRNG         *rng,
                                       const P3DStemModelInstance
                                                          *parent,
                                       float               offset,
                                       const P3DQuaternionf
                                                          *orientation) const
 {
  P3DStemModelTubeInstance            *Instance;

  if (parent == 0)
   {
    float                              InstanceLength;

    InstanceLength = Length;

    if (rng != 0)
     {
      InstanceLength += rng->UniformFloat(-LengthV,LengthV) * InstanceLength;
     }

    if (orientation != 0)
     {
      P3DMatrix4x4f                    WorldTransform;

      orientation->ToMatrix(WorldTransform.m);

      Instance = new P3DStemModelTubeInstance
                      ( InstanceLength,
                        AxisResolution,
                        ProfileScaleBase,
                       &ProfileScaleCurve,
                        ProfileResolution,
                        UMode,
                        UScale,
                        VMode,
                        VScale,
                       &WorldTransform);
     }
    else
     {
      Instance = new P3DStemModelTubeInstance
                      ( InstanceLength,
                        AxisResolution,
                        ProfileScaleBase,
                       &ProfileScaleCurve,
                        ProfileResolution,
                        UMode,
                        UScale,
                        VMode,
                        VScale,
                        0);
     }
   }
  else
   {
    P3DMatrix4x4f                      ParentTransform;
    P3DMatrix4x4f                      WorldTransform;
    P3DQuaternionf                     ParentOrientation;
    P3DQuaternionf                     InstanceOrientation;
    P3DVector3f                        ParentAxisPos;
    P3DMatrix4x4f                      TempTransform;
    P3DMatrix4x4f                      TempTransform2;
    P3DMatrix4x4f                      TranslateTransform;
    float                              InstanceLength;

    parent->GetAxisOrientationAt(ParentOrientation.q,offset);
    parent->GetAxisPointAt(ParentAxisPos.v,offset);
    parent->GetWorldTransform(ParentTransform.m);

    P3DQuaternionf::CrossProduct(InstanceOrientation.q,
                                 ParentOrientation.q,
                                 orientation->q);

    InstanceOrientation.ToMatrix(TempTransform2.m);

    P3DMatrix4x4f::MakeTranslation(TranslateTransform.m,
                                   ParentAxisPos.X(),
                                   ParentAxisPos.Y(),
                                   ParentAxisPos.Z());
    P3DMatrix4x4f::MultMatrix(TempTransform.m,
                              TranslateTransform.m,
                              TempTransform2.m);

    P3DMatrix4x4f::MultMatrix(WorldTransform.m,
                              ParentTransform.m,
                              TempTransform.m);

    InstanceLength =
     parent->GetLength() * Length * LengthOffsetInfluenceCurve.GetValue(offset);

    if (rng != 0)
     {
      InstanceLength += rng->UniformFloat(-LengthV,LengthV) * InstanceLength;
     }

    Instance = new P3DStemModelTubeInstance
                    ( InstanceLength,
                      AxisResolution,
                      parent->GetMinRadiusAt(offset) * ProfileScaleBase,
                     &ProfileScaleCurve,
                      ProfileResolution,
                      UMode,
                      UScale,
                      VMode,
                      VScale,
                     &WorldTransform);
   }

  ApplyAxisVariation(rng,Instance);
  ApplyPhototropism(Instance);

  return(Instance);
 }

void               P3DStemModelTube::ReleaseInstance
                                      (P3DStemModelInstance
                                                          *Instance) const
 {
  delete Instance;
 }

unsigned int       P3DStemModelTube::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    return((AxisResolution + 1) * (ProfileResolution + 1));
   }
  else
   {
    return((AxisResolution + 1) * ProfileResolution);
   }
 }

unsigned int       P3DStemModelTube::GetPrimitiveCount
                                      () const
 {
  return(ProfileResolution * AxisResolution);
 }

unsigned int       P3DStemModelTube::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

void               P3DStemModelTube::FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  unsigned int                         PrimitiveIndex;
  unsigned int                         PrimitiveCount;
  unsigned short                      *ShortBuffer;
  unsigned int                        *IntBuffer;

  PrimitiveCount = GetPrimitiveCount();
  ShortBuffer    = (unsigned short*)IndexBuffer;
  IntBuffer      = (unsigned int*)IndexBuffer;

  if      ((Attr == P3D_ATTR_VERTEX)   ||
           (Attr == P3D_ATTR_NORMAL)   ||
           (Attr == P3D_ATTR_BINORMAL) ||
           (Attr == P3D_ATTR_TANGENT))
   {
    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      if (ElementType == P3D_UNSIGNED_INT) 
       {
        IntBuffer[0] = IndexBase + PrimitiveIndex;
        IntBuffer[1] = IndexBase + PrimitiveIndex + ProfileResolution;

        IntBuffer[2] = IntBuffer[1] + 1;
        IntBuffer[3] = IntBuffer[0] + 1;

        if (((PrimitiveIndex + 1) % ProfileResolution) == 0)
         {
          IntBuffer[2] -= ProfileResolution;
          IntBuffer[3] -= ProfileResolution;
         }

        IntBuffer += 4;
       }
      else
       {
        ShortBuffer[0] = (unsigned short)(IndexBase + PrimitiveIndex);
        ShortBuffer[1] = (unsigned short)(IndexBase + PrimitiveIndex + ProfileResolution);

        ShortBuffer[2] = ShortBuffer[1] + 1;
        ShortBuffer[3] = ShortBuffer[0] + 1;

        if (((PrimitiveIndex + 1) % ProfileResolution) == 0)
         {
          ShortBuffer[2] -= ProfileResolution;
          ShortBuffer[3] -= ProfileResolution;
         }

        ShortBuffer += 4;
       }
     }
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    unsigned int   Level;

    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      Level = PrimitiveIndex / ProfileResolution;

      if (ElementType == P3D_UNSIGNED_INT) 
       {
        IntBuffer[0] = IndexBase + Level * (ProfileResolution + 1) +
                        (PrimitiveIndex % ProfileResolution);

        IntBuffer[1] = IntBuffer[0] + ProfileResolution + 1;
        IntBuffer[2] = IntBuffer[1] + 1;
        IntBuffer[3] = IntBuffer[0] + 1;

        IntBuffer += 4;
       }
      else
       {
        ShortBuffer[0] = (unsigned short)(IndexBase + Level * (ProfileResolution + 1) +
                          (PrimitiveIndex % ProfileResolution));

        ShortBuffer[1] = ShortBuffer[0] + ProfileResolution + 1;
        ShortBuffer[2] = ShortBuffer[1] + 1;
        ShortBuffer[3] = ShortBuffer[0] + 1;

        ShortBuffer += 4;
       }
     }
   }
  else
   {
    throw P3DExceptionGeneric("invalid attribute");
   }
 }

unsigned int       P3DStemModelTube::GetVAttrCountI
                                      () const
 {
  return((ProfileResolution + 1) * (AxisResolution + 1));
 }

unsigned int       P3DStemModelTube::GetIndexCount
                                      (unsigned int        PrimitiveType) const
 {
  if (PrimitiveType == P3D_TRIANGLE_LIST)
   {
    return(ProfileResolution * AxisResolution * 2 * 3);
   }
  else
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }
 }

void               P3DStemModelTube::FillIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  if (PrimitiveType == P3D_TRIANGLE_LIST)
   {
    unsigned int                       AxisSegment;
    unsigned int                       ProfileSegment;
    unsigned short                    *ShortBuffer;
    unsigned int                      *IntBuffer;
    unsigned int                       Index;

    ShortBuffer = (unsigned short*)IndexBuffer;
    IntBuffer   = (unsigned int*)IndexBuffer;
    Index       = 0;

    for (AxisSegment = 0; AxisSegment < AxisResolution; AxisSegment++)
     {
      for (ProfileSegment = 0; ProfileSegment < ProfileResolution; ProfileSegment++)
       {
        if (ElementType == P3D_UNSIGNED_INT)
         {
          IntBuffer[Index]     = IndexBase + AxisSegment * (ProfileResolution + 1) + ProfileSegment;
          IntBuffer[Index + 1] = IntBuffer[Index] + ProfileResolution + 1;
          IntBuffer[Index + 2] = IntBuffer[Index] + 1;
          IntBuffer[Index + 3] = IntBuffer[Index + 2];
          IntBuffer[Index + 4] = IntBuffer[Index + 1];
          IntBuffer[Index + 5] = IntBuffer[Index + 4] + 1;
         }
        else /* (ElementType == P3D_UNSIGNED_SHORT) */
         {
          ShortBuffer[Index]     = (unsigned short)(IndexBase + AxisSegment * (ProfileResolution + 1) + ProfileSegment);
          ShortBuffer[Index + 1] = (unsigned short)(ShortBuffer[Index] + ProfileResolution + 1);
          ShortBuffer[Index + 2] = (unsigned short)(ShortBuffer[Index] + 1);
          ShortBuffer[Index + 3] = (unsigned short)(ShortBuffer[Index + 2]);
          ShortBuffer[Index + 4] = (unsigned short)(ShortBuffer[Index + 1]);
          ShortBuffer[Index + 5] = (unsigned short)(ShortBuffer[Index + 4] + 1);
         }

        Index += 6;
       }
     }
   }
  else
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }
 }

void               P3DSaveSplineCurve (P3DOutputStringFmtStream
                                                          *FmtStream,
                                       const P3DMathNaturalCubicSpline
                                                          *Spline)
 {
  unsigned int                         CPCount;
  unsigned int                         CPIndex;

  CPCount = Spline->GetCPCount();

  FmtStream->WriteString("su","CPCount",CPCount);

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    FmtStream->WriteString("sff","Point",Spline->GetCPX(CPIndex),Spline->GetCPY(CPIndex));
   }
 }

void               P3DLoadSplineCurve (P3DMathNaturalCubicSpline
                                                          *Spline,
                                       P3DInputStringFmtStream
                                                          *SourceStream,
                                       const char         *CurveName)
 {
  unsigned int                         CPCount;
  unsigned int                         CPIndex;
  float                                X,Y;

  if (strcmp(CurveName,"CubicSpline") != 0)
   {
    throw P3DExceptionGeneric("Unsupported curve type");
   }

  CPCount = Spline->GetCPCount();

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    Spline->DelCP(0);
   }

  SourceStream->ReadFmtStringTagged("CPCount","u",&CPCount);

  for (CPIndex = 0; CPIndex < CPCount; CPIndex++)
   {
    SourceStream->ReadFmtStringTagged("Point","ff",&X,&Y);

    Spline->AddCP(X,Y);
   }
 }

void               P3DStemModelTube::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","StemModel","Tube");

  FmtStream.WriteString("sf","Length",Length);
  FmtStream.WriteString("sf","LengthV",LengthV);
  FmtStream.WriteString("ss","LengthOffsetDep","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&LengthOffsetInfluenceCurve);

  FmtStream.WriteString("sf","AxisVariation",AxisVariation);
  FmtStream.WriteString("su","AxisResolution",AxisResolution);

  FmtStream.WriteString("sf","ProfileScaleBase",ProfileScaleBase);
  FmtStream.WriteString("ss","ProfileScaleCurve","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&ProfileScaleCurve);
  FmtStream.WriteString("su","ProfileResolution",ProfileResolution);

  FmtStream.WriteString("ss","PhototropismCurve","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&PhototropismCurve);

  FmtStream.WriteString("su","BaseTexUMode",UMode);
  FmtStream.WriteString("sf","BaseTexUScale",UScale);
  FmtStream.WriteString("su","BaseTexVMode",VMode);
  FmtStream.WriteString("sf","BaseTexVScale",VScale);
 }

void               P3DStemModelTube::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version)
 {
  char                                 StrValue[255 + 1];
  float                                FloatValue;
  unsigned int                         UintValue;
  unsigned int                         PhototropismMode;

  PhototropismMode = P3DPhototropismModePositive;

  SourceStream->ReadFmtStringTagged("Length","f",&FloatValue);
  SetLength(FloatValue);
  SourceStream->ReadFmtStringTagged("LengthV","f",&FloatValue);
  SetLengthV(FloatValue);
  SourceStream->ReadFmtStringTagged("LengthOffsetDep","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(&LengthOffsetInfluenceCurve,SourceStream,StrValue);

  SourceStream->ReadFmtStringTagged("AxisVariation","f",&FloatValue);
  SetAxisVariation(FloatValue);
  SourceStream->ReadFmtStringTagged("AxisResolution","u",&UintValue);
  SetAxisResolution(UintValue);

  SourceStream->ReadFmtStringTagged("ProfileScaleBase","f",&FloatValue);
  SetProfileScaleBase(FloatValue);
  SourceStream->ReadFmtStringTagged("ProfileScaleCurve","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(&ProfileScaleCurve,SourceStream,StrValue);
  SourceStream->ReadFmtStringTagged("ProfileResolution","u",&UintValue);
  SetProfileResolution(UintValue);

  if ((Version->Major == 0) && (Version->Minor < 3))
   {
    SourceStream->ReadFmtStringTagged("PhototropismMode","u",&UintValue);

    if ((UintValue == P3DPhototropismModePositive) ||
        (UintValue == P3DPhototropismModeNegative))
     {
      PhototropismMode = UintValue;
     }
    else
     {
      throw P3DExceptionGeneric("Invalid phototropism mode");
     }
   }

  SourceStream->ReadFmtStringTagged("PhototropismCurve","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(&PhototropismCurve,SourceStream,StrValue);

  if ((Version->Major == 0) && (Version->Minor < 3))
   {
    for (unsigned int Index = 0; Index < PhototropismCurve.GetCPCount(); Index++)
     {
      if (PhototropismMode == P3DPhototropismModePositive)
       {
        PhototropismCurve.UpdateCP
         (PhototropismCurve.GetCPX(Index),
          P3DMath::Clampf(0.0f,1.0f,PhototropismCurve.GetCPY(Index) * 0.5f + 0.5f),
          Index);
       }
      else
       {
        PhototropismCurve.UpdateCP
         (PhototropismCurve.GetCPX(Index),
          P3DMath::Clampf(0.0f,1.0f,0.5f - PhototropismCurve.GetCPY(Index) * 0.5f),
          Index);
       }
     }
   }

  SourceStream->ReadFmtStringTagged("BaseTexUMode","u",&UintValue);

  if ((UintValue == P3DTexCoordModeRelative) ||
      (UintValue == P3DTexCoordModeAbsolute))
   {
    SetTexCoordUMode(UintValue);
   }
  else
   {
    throw P3DExceptionGeneric("Invalid texcoord mode");
   }

  SourceStream->ReadFmtStringTagged("BaseTexUScale","f",&FloatValue);
  SetTexCoordUScale(FloatValue);

  SourceStream->ReadFmtStringTagged("BaseTexVMode","u",&UintValue);

  if ((UintValue == P3DTexCoordModeRelative) ||
      (UintValue == P3DTexCoordModeAbsolute))
   {
    SetTexCoordVMode(UintValue);
   }
  else
   {
    throw P3DExceptionGeneric("Invalid texcoord mode");
   }

  SourceStream->ReadFmtStringTagged("BaseTexVScale","f",&FloatValue);
  SetTexCoordVScale(FloatValue);
 }

void               P3DStemModelTube::SetLength
                                      (float               Length)
 {
  if (Length > 0.0f)
   {
    this->Length = Length;
   }
  else
   {
    this->Length = 0.1f;
   }
 }

float              P3DStemModelTube::GetLength
                                      () const
 {
  return(Length);
 }

void               P3DStemModelTube::SetLengthV
                                      (float               LengthV)
 {
  this->LengthV = P3DMath::Clampf(0.0f,1.0f,LengthV);
 }

float              P3DStemModelTube::GetLengthV
                                      () const
 {
  return(LengthV);
 }

void               P3DStemModelTube::SetAxisVariation
                                      (float               AxisVariation)
 {
  this->AxisVariation = P3DMath::Clampf(0.0f,1.0f,AxisVariation);
 }

float              P3DStemModelTube::GetAxisVariation
                                      () const
 {
  return(AxisVariation);
 }

void               P3DStemModelTube::SetAxisResolution
                                      (unsigned int        Resolution)
 {
  if (Resolution > 0)
   {
    AxisResolution = Resolution;
   }
  else
   {
    AxisResolution = 1;
   }
 }

unsigned int       P3DStemModelTube::GetAxisResolution
                                      () const
 {
  return(AxisResolution);
 }

void               P3DStemModelTube::SetProfileResolution
                                      (unsigned int        Resolution)
 {
  if (Resolution > 2)
   {
    ProfileResolution = Resolution;
   }
  else
   {
    ProfileResolution = 3;
   }
 }

unsigned int       P3DStemModelTube::GetProfileResolution
                                      () const
 {
  return(ProfileResolution);
 }

void               P3DStemModelTube::SetProfileScaleBase
                                      (float               Scale)
 {
  this->ProfileScaleBase = Scale;
 }

float              P3DStemModelTube::GetProfileScaleBase
                                      () const
 {
  return(ProfileScaleBase);
 }

void               P3DStemModelTube::SetProfileScaleCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  ProfileScaleCurve.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelTube::GetProfileScaleCurve
                                      () const
 {
  return(&ProfileScaleCurve);
 }

void               P3DStemModelTube::SetLengthOffsetInfluenceCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  LengthOffsetInfluenceCurve.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelTube::GetLengthOffsetInfluenceCurve
                                      () const
 {
  return(&LengthOffsetInfluenceCurve);
 }

void               P3DStemModelTube::SetPhototropismCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  PhototropismCurve.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelTube::GetPhototropismCurve
                                      () const
 {
  return(&PhototropismCurve);
 }

void               P3DStemModelTube::SetTexCoordUMode
                                      (unsigned int        Mode)
 {
  if (Mode == P3DTexCoordModeAbsolute)
   {
    UMode = Mode;
   }
  else
   {
    UMode = P3DTexCoordModeRelative;
   }
 }

unsigned int       P3DStemModelTube::GetTexCoordUMode
                                      () const
 {
  return(UMode);
 }

void               P3DStemModelTube::SetTexCoordUScale
                                      (float               Scale)
 {
  UScale = Scale;
 }

float              P3DStemModelTube::GetTexCoordUScale
                                      () const
 {
  return(UScale);
 }

void               P3DStemModelTube::SetTexCoordVMode
                                      (unsigned int        Mode)
 {
  if (Mode == P3DTexCoordModeAbsolute)
   {
    VMode = Mode;
   }
  else
   {
    VMode = P3DTexCoordModeRelative;
   }
 }

unsigned int       P3DStemModelTube::GetTexCoordVMode
                                      () const
 {
  return(VMode);
 }

void               P3DStemModelTube::SetTexCoordVScale
                                      (float               Scale)
 {
  VScale = Scale;
 }

float              P3DStemModelTube::GetTexCoordVScale
                                      () const
 {
  return(VScale);
 }

