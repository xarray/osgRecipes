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

#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemwings.h>

class P3DStemModelWingsInstance : public P3DStemModelInstance
 {
  public           :

                   P3DStemModelWingsInstance
                                      (const P3DStemModelTube
                                                          *ParentStemModel,
                                       const P3DStemModelTubeInstance
                                                          *ParentInstance,
                                       unsigned int        SectionCount,
                                       float               Width,
                                       const P3DMathNaturalCubicSpline
                                                          *Curvature,
                                       float               Thickness,
                                       const P3DMatrix4x4f*Transform,
                                       const P3DQuaternionf
                                                          *Rotation);

  /* Per-attribute information */

  virtual
  unsigned int     GetVAttrCount      (unsigned int        Attr) const;
  virtual void     GetVAttrValue      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const;

  virtual
  unsigned int     GetPrimitiveCount  () const;
  virtual
  unsigned int     GetPrimitiveType   (unsigned int        PrimitiveIndex) const;

  /* Per-index information */

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

  private          :

  void             CalcVertexPosAt    (float              *Pos,
                                       int                 XSect,
                                       int                 YSect) const;

  void             CalcVertexNormalAt (float              *Normal,
                                       int                 XSect,
                                       int                 YSect,
                                       bool                Opposite) const;

  void             CalcVertexBiNormalAt
                                      (float              *BiNormal,
                                       int                 YSect) const;

  void             CalcVertexTangentAt(float              *Tangent,
                                       int                 XSect,
                                       int                 YSect,
                                       bool                Opposite) const;

  void             CalcVertexTexCoord0At
                                      (float              *TexCoord,
                                       int                 XSect,
                                       int                 YSect) const;

  const P3DStemModelTube              *ParentStemModel;
  const P3DStemModelTubeInstance      *ParentInstance;
  unsigned int                         SectionCount;
  float                                Width;
  const P3DMathNaturalCubicSpline     *Curvature;
  float                                Thickness;
  P3DMatrix4x4f                        WorldTransform;
  P3DQuaternionf                       Rotation;
 };

                   P3DStemModelWingsInstance::P3DStemModelWingsInstance
                                      (const P3DStemModelTube
                                                          *ParentStemModel,
                                       const P3DStemModelTubeInstance
                                                          *ParentInstance,
                                       unsigned int        SectionCount,
                                       float               Width,
                                       const P3DMathNaturalCubicSpline
                                                          *Curvature,
                                       float               Thickness,
                                       const P3DMatrix4x4f*Transform,
                                       const P3DQuaternionf
                                                          *Rotation)
 {
  this->ParentStemModel = ParentStemModel;
  this->ParentInstance  = ParentInstance;
  this->SectionCount    = SectionCount;
  this->Width           = Width;
  this->Curvature       = Curvature;
  this->Thickness       = Thickness;
  this->Rotation.q[0]   = Rotation->q[0];
  this->Rotation.q[1]   = Rotation->q[1];
  this->Rotation.q[2]   = Rotation->q[2];
  this->Rotation.q[3]   = Rotation->q[3];

  if (Transform == 0)
   {
    P3DMatrix4x4f::MakeIdentity(WorldTransform.m);
   }
  else
   {
    WorldTransform = *Transform;
   }
 }

unsigned int       P3DStemModelWingsInstance::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if ((Attr == P3D_ATTR_NORMAL)   ||
      (Attr == P3D_ATTR_BINORMAL) ||
      (Attr == P3D_ATTR_TANGENT))
   {
    return((ParentStemModel->GetAxisResolution() + 1) * ((SectionCount + 1) * 2));
   }
  else
   {
    return((ParentStemModel->GetAxisResolution() + 1) * (SectionCount * 2 + 1));
   }
 }

void               P3DStemModelWingsInstance::GetVAttrValue
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  int                                  XSect;
  int                                  YSect;
  unsigned int                         RowSize;

  if (Index >= GetVAttrCount(Attr))
   {
    throw P3DExceptionGeneric("invalid attribute index");
   }

  if      (Attr == P3D_ATTR_NORMAL)
   {
    RowSize = (SectionCount + 1) * 2;

    YSect = Index / RowSize;
    XSect = Index % RowSize;

    if (XSect < (int)(RowSize / 2))
     {
      CalcVertexNormalAt(Value,RowSize / 2 - XSect - 1,YSect,false);
     }
    else
     {
      CalcVertexNormalAt(Value,RowSize / 2 - XSect,YSect,true);
     }
   }
  else if (Attr == P3D_ATTR_BINORMAL)
   {
    RowSize = (SectionCount + 1) * 2;

    CalcVertexBiNormalAt(Value,Index / RowSize);
   }
  else if (Attr == P3D_ATTR_TANGENT)
   {
    RowSize = (SectionCount + 1) * 2;

    YSect = Index / RowSize;
    XSect = Index % RowSize;

    if (XSect < (int)(RowSize / 2))
     {
      CalcVertexTangentAt(Value,RowSize / 2 - XSect - 1,YSect,false);
     }
    else
     {
      CalcVertexTangentAt(Value,RowSize / 2 - XSect,YSect,true);
     }
   }
  else
   {
    RowSize = SectionCount * 2 + 1;

    YSect = Index / RowSize;
    XSect = (int)(RowSize / 2) - (Index % RowSize);

    if      (Attr == P3D_ATTR_VERTEX)
     {
      CalcVertexPosAt(Value,XSect,YSect);
     }
    else if (Attr == P3D_ATTR_TEXCOORD0)
     {
      CalcVertexTexCoord0At(Value,XSect,YSect);
     }
    else
     {
      throw P3DExceptionGeneric("invalid attribute");
     }
   }
 }

unsigned int       P3DStemModelWingsInstance::GetVAttrCountI
                                      () const
 {
  return(GetVAttrCount(P3D_ATTR_NORMAL));
 }

void               P3DStemModelWingsInstance::GetVAttrValueI
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  unsigned int                         RowSize;
  int                                  XSect;
  int                                  YSect;
  bool                                 Opposite;

  if (Index >= GetVAttrCountI())
   {
    throw P3DExceptionGeneric("invalid attribute index");
   }

  RowSize = (SectionCount + 1) * 2;

  YSect = Index / RowSize;
  XSect = Index % RowSize;

  if (XSect < (int)(RowSize / 2))
   {
    XSect    = RowSize / 2 - XSect - 1;
    Opposite = false;
   }
  else
   {
    XSect    = RowSize / 2 - XSect;
    Opposite = true;
   }

  if      (Attr == P3D_ATTR_VERTEX)
   {
    CalcVertexPosAt(Value,XSect,YSect);
   }
  else if (Attr == P3D_ATTR_NORMAL)
   {
    CalcVertexNormalAt(Value,XSect,YSect,Opposite);
   }
  else if (Attr == P3D_ATTR_BINORMAL)
   {
    CalcVertexBiNormalAt(Value,YSect);
   }
  else if (Attr == P3D_ATTR_TANGENT)
   {
    CalcVertexTangentAt(Value,XSect,YSect,Opposite);
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    CalcVertexTexCoord0At(Value,XSect,YSect);
   }
 }

unsigned int       P3DStemModelWingsInstance::GetPrimitiveCount
                                      () const
 {
  return(SectionCount * 2 * ParentStemModel->GetAxisResolution());
 }

unsigned int       P3DStemModelWingsInstance::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

float              P3DStemModelWingsInstance::GetLength
                                      () const
 {
  return(ParentInstance->GetLength());
 }

float              P3DStemModelWingsInstance::GetMinRadiusAt
                                      (float               Offset P3D_UNUSED_ATTR) const
 {
  return(0.0f);
 }

void               P3DStemModelWingsInstance::GetWorldTransform
                                      (float              *Transform) const
 {
  /*FIXME: need standard way to copy matrix into float array */
  for (unsigned int i = 0; i < 16; i++)
   {
    Transform[i] = WorldTransform.m[i];
   }
 }

void               P3DStemModelWingsInstance::GetAxisPointAt
                                      (float              *Pos,
                                       float               Offset) const
 {
  ParentInstance->GetAxisPointAt(Pos,Offset);
 }

void               P3DStemModelWingsInstance::GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const
 {
  ParentInstance->GetAxisOrientationAt(Orientation,Offset);
 }

void               P3DStemModelWingsInstance::CalcVertexPosAt
                                      (float              *Pos,
                                       int                 XSect,
                                       int                 YSect) const
 {
  P3DVector3f                          AxisPoint;
  P3DQuaternionf                       AxisOrientation;
  P3DVector3f                          TempPos;
  float                                XFraction;
  float                                YFraction;

  XFraction = (float)XSect / SectionCount;
  YFraction = (float)YSect / ParentStemModel->GetAxisResolution();

  TempPos.X() = Width * XFraction;
  TempPos.Y() = 0.0f;

  if (XFraction < 0.0f)
   {
    TempPos.Z() = (Curvature->GetValue(-XFraction) - 0.5f) * Thickness;
   }
  else
   {
    TempPos.Z() = (Curvature->GetValue(XFraction) - 0.5f) * Thickness;
   }

  ParentInstance->GetAxisOrientationAt(AxisOrientation.q,YFraction);
  ParentInstance->GetAxisPointAt(AxisPoint.v,YFraction);

  P3DQuaternionf::RotateVector(TempPos.v,Rotation.q);
  P3DQuaternionf::RotateVector(TempPos.v,AxisOrientation.q);

  TempPos += AxisPoint;

  P3DVector3f::MultMatrix(Pos,&WorldTransform,TempPos.v);
 }

void               P3DStemModelWingsInstance::CalcVertexNormalAt
                                      (float              *Normal,
                                       int                 XSect,
                                       int                 YSect,
                                       bool                Opposite) const
 {
  P3DQuaternionf                       AxisOrientation;
  float                                XFraction;
  float                                YFraction;
  P3DVector3f                          VertexNormal(0.0f,0.0f,1.0f);
  P3DMatrix4x4f                        WorldRotation;

  if (XSect < 0)
   {
    XSect = -XSect;
   }

  XFraction = (float)XSect / SectionCount;
  YFraction = (float)YSect / ParentStemModel->GetAxisResolution();

  VertexNormal.X() = Curvature->GetTangent(XFraction);

  if (!Opposite)
   {
    VertexNormal.X() = -VertexNormal.X();
   }

  VertexNormal.Normalize();

  ParentInstance->GetAxisOrientationAt(AxisOrientation.q,YFraction);

  P3DQuaternionf::RotateVector(VertexNormal.v,Rotation.q);
  P3DQuaternionf::RotateVector(VertexNormal.v,AxisOrientation.q);

  P3DMatrix4x4f::GetRotationOnly(WorldRotation.m,WorldTransform.m);

  VertexNormal.MultMatrix(&WorldRotation);
  VertexNormal.Normalize();

  Normal[0] = VertexNormal.X();
  Normal[1] = VertexNormal.Y();
  Normal[2] = VertexNormal.Z();
 }

void               P3DStemModelWingsInstance::CalcVertexBiNormalAt
                                      (float              *BiNormal,
                                       int                 YSect) const
 {
  P3DQuaternionf                       AxisOrientation;
  float                                YFraction;
  P3DVector3f                          VertexBiNormal(0.0f,1.0f,0.0f);
  P3DMatrix4x4f                        WorldRotation;

  YFraction = (float)YSect / ParentStemModel->GetAxisResolution();

  ParentInstance->GetAxisOrientationAt(AxisOrientation.q,YFraction);

  P3DQuaternionf::RotateVector(VertexBiNormal.v,AxisOrientation.q);

  P3DMatrix4x4f::GetRotationOnly(WorldRotation.m,WorldTransform.m);

  VertexBiNormal.MultMatrix(&WorldRotation);
  VertexBiNormal.Normalize();

  BiNormal[0] = VertexBiNormal.X();
  BiNormal[1] = VertexBiNormal.Y();
  BiNormal[2] = VertexBiNormal.Z();
 }

/*FIXME: not very optimal implementation */
void               P3DStemModelWingsInstance::CalcVertexTangentAt
                                      (float              *Tangent,
                                       int                 XSect,
                                       int                 YSect,
                                       bool                Opposite) const
 {
  float                                Normal[3];
  float                                BiNormal[3];

  CalcVertexNormalAt(Normal,XSect,YSect,Opposite);
  CalcVertexBiNormalAt(BiNormal,YSect);

  P3DVector3f::CrossProduct(Tangent,BiNormal,Normal);
 }

void               P3DStemModelWingsInstance::CalcVertexTexCoord0At
                                      (float              *TexCoord,
                                       int                 XSect,
                                       int                 YSect) const
 {
  TexCoord[0] = ((float)(XSect + SectionCount)) / (SectionCount * 2);
  TexCoord[1] = ((float)(YSect)) / ParentStemModel->GetAxisResolution();
 }

                   P3DStemModelWings::P3DStemModelWings
                                      (const P3DStemModelTube
                                                          *ParentStemModel)
 {
  this->ParentStemModel = ParentStemModel;

  WingsAngle   = 0.0f;
  Width        = 0.5f;
  SectionCount = 1;
  MakeDefaultCurvatureCurve(Curvature);
  Thickness    = 0.0f;
 }

void               P3DStemModelWings::MakeDefaultCurvatureCurve
                                     (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(0.5f);
 }

P3DStemModel      *P3DStemModelWings::CreateCopy
                                      () const
 {
  P3DStemModelWings                   *Result;

  Result = new P3DStemModelWings(0);

  Result->WingsAngle   = WingsAngle;
  Result->Width        = Width;
  Result->SectionCount = SectionCount;
  Result->Curvature.CopyFrom(Curvature);
  Result->Thickness    = Thickness;

  return(Result);
 }

P3DStemModelInstance
                  *P3DStemModelWings::CreateInstance
                                      (P3DMathRNG         *RNG P3D_UNUSED_ATTR,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       float               Offset P3D_UNUSED_ATTR,
                                       const P3DQuaternionf
                                                          *Orientation P3D_UNUSED_ATTR) const
 {
  const P3DStemModelTubeInstance      *ParentInstance;
  P3DMatrix4x4f                        ParentTransform;

  ParentInstance = dynamic_cast<const P3DStemModelTubeInstance*>(Parent);

  if (ParentInstance == 0)
   {
    throw P3DExceptionGeneric("invalid parent instance for Wings model");
   }

  Parent->GetWorldTransform(ParentTransform.m);

  return(new P3DStemModelWingsInstance( ParentStemModel,
                                        ParentInstance,
                                        SectionCount,
                                        Width,
                                       &Curvature,
                                        Thickness,
                                       &ParentTransform,
                                        Orientation));
 }

void               P3DStemModelWings::ReleaseInstance
                                      (P3DStemModelInstance
                                                          *Instance) const
 {
  delete Instance;
 }

unsigned int       P3DStemModelWings::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if ((Attr == P3D_ATTR_NORMAL)   ||
      (Attr == P3D_ATTR_BINORMAL) ||
      (Attr == P3D_ATTR_TANGENT))
   {
    return((ParentStemModel->GetAxisResolution() + 1) * ((SectionCount + 1) * 2));
   }
  else
   {
    return((ParentStemModel->GetAxisResolution() + 1) * (SectionCount * 2 + 1));
   }
 }

unsigned int       P3DStemModelWings::GetPrimitiveCount
                                      () const
 {
  return(SectionCount * 2 * ParentStemModel->GetAxisResolution());
 }

unsigned int       P3DStemModelWings::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

void               P3DStemModelWings::FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  unsigned int                         PrimitiveIndex;
  unsigned int                         PrimitiveCount;
  unsigned short                      *ShortBuffer;
  unsigned int                        *IntBuffer;

  ShortBuffer = (unsigned short*)IndexBuffer;
  IntBuffer   = (unsigned int*)IndexBuffer;

  PrimitiveCount = GetPrimitiveCount();

  if      ((Attr == P3D_ATTR_VERTEX) || (Attr == P3D_ATTR_TEXCOORD0))
   {
    if (ElementType == P3D_UNSIGNED_INT)
     {
      for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
       {
        IntBuffer[0] = IndexBase + (PrimitiveIndex / (SectionCount * 2)) * (SectionCount * 2 + 1) +
                       (PrimitiveIndex % (SectionCount * 2));
        IntBuffer[1] = IntBuffer[0] + 1;
        IntBuffer[2] = IntBuffer[1] + SectionCount * 2 + 1;
        IntBuffer[3] = IntBuffer[2] - 1;

        IntBuffer += 4;
       }
     }
    else
     {
      for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
       {
        ShortBuffer[0] = (unsigned short)(IndexBase + (PrimitiveIndex / (SectionCount * 2)) * (SectionCount * 2 + 1) +
                          (PrimitiveIndex % (SectionCount * 2)));
        ShortBuffer[1] = ShortBuffer[0] + 1;
        ShortBuffer[2] = ShortBuffer[1] + SectionCount * 2 + 1;
        ShortBuffer[3] = ShortBuffer[2] - 1;

        ShortBuffer += 4;
       }
     }
   }
  else if ((Attr == P3D_ATTR_NORMAL)   ||
           (Attr == P3D_ATTR_BINORMAL) ||
           (Attr == P3D_ATTR_TANGENT))
   {
    if (ElementType == P3D_UNSIGNED_INT)
     {
      for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
       {
        IntBuffer[0] = IndexBase + (PrimitiveIndex / (SectionCount * 2)) * ((SectionCount + 1) * 2) +
                       (PrimitiveIndex % (SectionCount * 2));

        if ((PrimitiveIndex % (SectionCount * 2)) >= SectionCount)
         {
          IntBuffer[0]++;
         }

        IntBuffer[1] = IntBuffer[0] + 1;
        IntBuffer[2] = IntBuffer[1] + (SectionCount + 1) * 2;
        IntBuffer[3] = IntBuffer[2] - 1;

        IntBuffer += 4;
       }
     }
    else
     {
      for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
       {
        ShortBuffer[0] = (unsigned short)(IndexBase + (PrimitiveIndex / (SectionCount * 2)) * ((SectionCount + 1) * 2) +
                          (PrimitiveIndex % (SectionCount * 2)));

        if ((PrimitiveIndex % (SectionCount * 2)) >= SectionCount)
         {
          ShortBuffer[0]++;
         }

        ShortBuffer[1] = ShortBuffer[0] + 1;
        ShortBuffer[2] = ShortBuffer[1] + (SectionCount + 1) * 2;
        ShortBuffer[3] = ShortBuffer[2] - 1;

        ShortBuffer += 4;
       }
     }
   }
  else
   {
    throw P3DExceptionGeneric("invalid attribute");
   }
 }

unsigned int       P3DStemModelWings::GetVAttrCountI
                                      () const
 {
  return(GetVAttrCount(P3D_ATTR_NORMAL));
 }

unsigned int       P3DStemModelWings::GetIndexCount
                                      (unsigned int        PrimitiveType) const
 {
  if (PrimitiveType == P3D_TRIANGLE_LIST)
   {
    return(GetPrimitiveCount() * 2 * 3);
   }
  else
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }
 }

void               P3DStemModelWings::FillIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  unsigned int                         AxisResolution;
  unsigned int                         Y;
  unsigned int                         X;
  unsigned int                         BaseIndex;
  unsigned int                         IndexOffset;
  unsigned short                      *ShortBuffer;
  unsigned int                        *IntBuffer;

  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }

  ShortBuffer = (unsigned short*)IndexBuffer;
  IntBuffer   = (unsigned int*)IndexBuffer;

  AxisResolution = ParentStemModel->GetAxisResolution();
  BaseIndex      = 0;
  IndexOffset    = (SectionCount + 1) * 2;

  for (Y = 0; Y < AxisResolution; Y++)
   {
    for (X = 0; X < (SectionCount * 2); X++)
     {
      if (ElementType == P3D_UNSIGNED_INT)
       {
        IntBuffer[0] = IndexBase + BaseIndex + X;

        if (X >= SectionCount)
         {
          IntBuffer[0]++;
         }

        IntBuffer[1] = IntBuffer[0] + IndexOffset;
        IntBuffer[2] = IntBuffer[0] + 1;
        IntBuffer[3] = IntBuffer[2] + IndexOffset;
        IntBuffer[4] = IntBuffer[2];
        IntBuffer[5] = IntBuffer[1];

        IntBuffer += 6;
       }
      else
       {
        ShortBuffer[0] = (unsigned short)IndexBase + BaseIndex + X;

        if (X >= SectionCount)
         {
          ShortBuffer[0]++;
         }

        ShortBuffer[1] = ShortBuffer[0] + IndexOffset;
        ShortBuffer[2] = ShortBuffer[0] + 1;
        ShortBuffer[3] = ShortBuffer[2] + IndexOffset;
        ShortBuffer[4] = ShortBuffer[2];
        ShortBuffer[5] = ShortBuffer[1];

        ShortBuffer += 6;
       }
     }

    BaseIndex += IndexOffset;
   }
 }

void               P3DStemModelWings::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","StemModel","Wings");

  FmtStream.WriteString("sf","WingsAngle",WingsAngle);
  FmtStream.WriteString("sf","Width",Width);
  FmtStream.WriteString("su","SectionCount",SectionCount);
  FmtStream.WriteString("ss","Curvature","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&Curvature);
  FmtStream.WriteString("sf","Thickness",Thickness);
 }

void               P3DStemModelWings::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version P3D_UNUSED_ATTR)
 {
  char                                 StrValue[255 + 1];
  float                                FloatValue;
  unsigned int                         UintValue;

  SourceStream->ReadFmtStringTagged("WingsAngle","f",&FloatValue);
  SetWingsAngle(FloatValue);
  SourceStream->ReadFmtStringTagged("Width","f",&FloatValue);
  SetWidth(FloatValue);
  SourceStream->ReadFmtStringTagged("SectionCount","u",&UintValue);
  SetSectionCount(UintValue);
  SourceStream->ReadFmtStringTagged("Curvature","s",StrValue,sizeof(StrValue));
  P3DLoadSplineCurve(&Curvature,SourceStream,StrValue);
  SourceStream->ReadFmtStringTagged("Thickness","f",&FloatValue);
  SetThickness(FloatValue);
 }

void               P3DStemModelWings::SetWingsAngle
                                      (float               Angle)
 {
  this->WingsAngle = P3DMath::Clampf(-P3DMATH_PI / 2.0f,P3DMATH_PI / 2.0f,Angle);
 }

float              P3DStemModelWings::GetWingsAngle
                                      () const
 {
  return(WingsAngle);
 }

void               P3DStemModelWings::SetWidth
                                      (float               Width)
 {
  this->Width = P3DMath::Clampf(0.0f,100.0f,Width);
 }

float              P3DStemModelWings::GetWidth
                                      () const
 {
  return(Width);
 }

void               P3DStemModelWings::SetSectionCount
                                      (unsigned int        SectionCount)
 {
  if (SectionCount > 1)
   {
    this->SectionCount = SectionCount;
   }
  else
   {
    this->SectionCount = 1;
   }
 }

unsigned int       P3DStemModelWings::GetSectionCount
                                      () const
 {
  return(SectionCount);
 }

void               P3DStemModelWings::SetCurvature
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  Curvature.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelWings::GetCurvature
                                      () const
 {
  return(&Curvature);
 }

void               P3DStemModelWings::SetThickness
                                      (float               Thickness)
 {
  if (Thickness > 0.0f)
   {
    this->Thickness = Thickness;
   }
  else
   {
    this->Thickness = 0.0f;
   }
 }

float              P3DStemModelWings::GetThickness
                                      () const
 {
  return(Thickness);
 }

