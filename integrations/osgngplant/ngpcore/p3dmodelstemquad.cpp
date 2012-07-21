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

#include <ngpcore/p3dmodelstemquad.h>

class P3DStemModelQuadInstance : public P3DStemModelInstance
 {
  public           :

                   P3DStemModelQuadInstance
                                      (float               Length,
                                       float               Width,
                                       unsigned int        BillboardMode,
                                       unsigned int        SectionCount,
                                       const P3DMathNaturalCubicSpline
                                                          *Curvature,
                                       float               Thickness,
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

  virtual void     GetBoundBox        (float              *Min,
                                       float              *Max) const;

  virtual float    GetLength          () const;
  virtual float    GetMinRadiusAt     (float               Offset) const;

  /* fill Transform with 4x4 Stem To World transformation matrix */
  virtual void     GetWorldTransform  (float              *Transform) const;
  /* get axis point coordinates (x,y,z - vector) in stem coordinate space */
  /* Offset must be in range [0.0 .. 1.0 ]                                */
  virtual void     GetAxisPointAt     (float              *Pos,
                                       float               Offset) const;
  /* get axis orientation (quaternion) in stem coordinate space */
  /* Offset must be in range [0.0 .. 1.0 ]                      */
  virtual void     GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const;

  private          :

  void             CalcVertexNormalAt (float              *Normal,
                                       unsigned int        Index) const;
  void             CalcVertexBiNormalAt
                                      (float              *BiNormal,
                                       unsigned int        Index) const;
  void             CalcVertexTangentAt(float              *Tangent,
                                       unsigned int        Index) const;

  float                                Length;
  float                                Width;
  unsigned int                         BillboardMode;
  P3DMatrix4x4f                        WorldTransform;

  unsigned int                         SectionCount;
  const P3DMathNaturalCubicSpline     *Curvature;
  float                                Thickness;
 };

                   P3DStemModelQuadInstance::P3DStemModelQuadInstance
                                      (float               Length,
                                       float               Width,
                                       unsigned int        BillboardMode,
                                       unsigned int        SectionCount,
                                       const P3DMathNaturalCubicSpline
                                                          *Curvature,
                                       float               Thickness,
                                       const P3DMatrix4x4f*Transform)
 {
  this->Length        = Length;
  this->Width         = Width;
  this->BillboardMode = BillboardMode;

  this->SectionCount = SectionCount;
  this->Curvature    = Curvature;
  this->Thickness    = Thickness;

  if (SectionCount > 1)
   {
    this->BillboardMode = P3D_BILLBOARD_MODE_NONE;
   }

  if (Transform == 0)
   {
    P3DMatrix4x4f::MakeIdentity(WorldTransform.m);
   }
  else
   {
    WorldTransform = *Transform;
   }
 }

unsigned int       P3DStemModelQuadInstance::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if (SectionCount == 1)
   {
    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      return(1);
     }
    else
     {
      return(4);
     }
   }
  else
   {
    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      return(SectionCount + 1);
     }
    else
     {
      return((SectionCount + 1) * 2);
     }
   }
 }

void               P3DStemModelQuadInstance::CalcVertexNormalAt
                                      (float              *Normal,
                                       unsigned int        Index) const
 {
  P3DVector3f                        VertexNormal(0.0f,0.0f,1.0f);
  P3DMatrix4x4f                      Rotation;

  if (SectionCount > 1)
   {
    float                            YOffset;
    float                            Tangent;

    YOffset = ((float)(Index >> 1)) / SectionCount;
    Tangent = Curvature->GetTangent(YOffset);

    VertexNormal.Y() = -Tangent;
    VertexNormal.Z() = 1.0f;

    VertexNormal.Normalize();
   }

  P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

  VertexNormal.MultMatrix(&Rotation);
  VertexNormal.Normalize();

  Normal[0] = VertexNormal.X();
  Normal[1] = VertexNormal.Y();
  Normal[2] = VertexNormal.Z();
 }

void               P3DStemModelQuadInstance::CalcVertexBiNormalAt
                                      (float              *BiNormal,
                                       unsigned int        Index) const
 {
  P3DVector3f                        VertexBiNormal(0.0f,1.0f,0.0f);
  P3DMatrix4x4f                      Rotation;

  if (SectionCount > 1)
   {
    float                            YOffset;
    float                            Tangent;

    YOffset = ((float)(Index >> 1)) / SectionCount;
    Tangent = Curvature->GetTangent(YOffset);

    VertexBiNormal.Y() = 1.0f;
    VertexBiNormal.Z() = Tangent;

    VertexBiNormal.Normalize();
   }

  P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

  VertexBiNormal.MultMatrix(&Rotation);
  VertexBiNormal.Normalize();

  BiNormal[0] = VertexBiNormal.X();
  BiNormal[1] = VertexBiNormal.Y();
  BiNormal[2] = VertexBiNormal.Z();
 }

/*FIXME: not very optimal implementation */
void               P3DStemModelQuadInstance::CalcVertexTangentAt
                                      (float              *Tangent,
                                       unsigned int        Index) const
 {
  float                                Normal[3];
  float                                BiNormal[3];

  CalcVertexNormalAt(Normal,Index);
  CalcVertexBiNormalAt(BiNormal,Index);

  P3DVector3f::CrossProduct(Tangent,BiNormal,Normal);
 }

void               P3DStemModelQuadInstance::GetVAttrValue
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if      (Attr == P3D_ATTR_VERTEX)
   {
    P3DVector3f                        VertexPos;
    float                              YOffset;

    if (Index & 0x01)
     {
      VertexPos.X() = Width / 2.0f;
     }
    else
     {
      VertexPos.X() = -Width / 2.0f;
     }

    YOffset = ((float)(Index >> 1)) / SectionCount;

    VertexPos.Y() = Length * YOffset;

    if (SectionCount > 1)
     {
      VertexPos.Z() = (Curvature->GetValue(YOffset) - 0.5f) * Thickness;
     }
    else
     {
      VertexPos.Z() = 0.0f;
     }

    P3DVector3f::MultMatrix(Value,&WorldTransform,VertexPos.v);
   }
  else if (Attr == P3D_ATTR_NORMAL)
   {
    CalcVertexNormalAt(Value,Index);
   }
  else if (Attr == P3D_ATTR_BINORMAL)
   {
    CalcVertexBiNormalAt(Value,Index);
   }
  else if (Attr == P3D_ATTR_TANGENT)
   {
    CalcVertexTangentAt(Value,Index);
   }
  else if (Attr == P3D_ATTR_TEXCOORD0)
   {
    if (Index & 0x01)
     {
      Value[0] = 1.0f;
     }
    else
     {
      Value[0] = 0.0f;
     }

    Value[1] = ((float)(Index >> 1)) / SectionCount;
   }
  else
   {
    /*FIXME: throw error here */
   }
 }

unsigned int       P3DStemModelQuadInstance::GetPrimitiveCount
                                      () const
 {
  return(SectionCount);
 }

unsigned int       P3DStemModelQuadInstance::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

unsigned int       P3DStemModelQuadInstance::GetVAttrCountI
                                      () const
 {
  return(2 + SectionCount * 2);
 }

void               P3DStemModelQuadInstance::GetVAttrValueI
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if (Attr == P3D_ATTR_BILLBOARD_POS)
   {
    if (BillboardMode == P3D_BILLBOARD_MODE_NONE)
     {
      throw P3DExceptionGeneric("trying to get biilboard info from non-billboard branch");
     }

    P3DVector3f                        CenterPos(0.0f,Length * 0.5f,0.0f);

    CenterPos.MultMatrix(&WorldTransform);

    Value[0] = CenterPos.X();
    Value[1] = CenterPos.Y();
    Value[2] = CenterPos.Z();
   }
  else
   {
    if (Index >= GetVAttrCountI())
     {
      throw P3DExceptionGeneric("invalid vertex index");
     }

    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      GetVAttrValue(Value,Attr,Index / 2);
     }
    else
     {
      GetVAttrValue(Value,Attr,Index);
     }
   }
 }

void               P3DStemModelQuadInstance::GetBoundBox
                                      (float              *Min,
                                       float              *Max) const
 {
  if (BillboardMode == P3D_BILLBOARD_MODE_NONE)
   {
    P3DStemModelInstance::GetBoundBox(Min,Max);
   }
  else
   {
    float                              HalfWidth;
    float                              HalfHeight;
    float                              Radius;

    HalfWidth  = Width  * 0.5f;
    HalfHeight = Length * 0.5f;
    Radius     = P3DMath::Sqrtf(HalfWidth * HalfWidth + HalfHeight * HalfHeight);

    P3DVector3f                        CenterPos(0.0f,HalfHeight,0.0f);

    CenterPos.MultMatrix(&WorldTransform);

    Min[0] = CenterPos.X() - Radius;
    Min[1] = CenterPos.Y() - Radius;
    Min[2] = CenterPos.Z() - Radius;

    Max[0] = CenterPos.X() + Radius;
    Max[1] = CenterPos.Y() + Radius;
    Max[2] = CenterPos.Z() + Radius;
   }
 }

float              P3DStemModelQuadInstance::GetLength
                                      () const
 {
  return(Length);
 }

float              P3DStemModelQuadInstance::GetMinRadiusAt
                                      (float               Offset P3D_UNUSED_ATTR) const
 {
  return(0.0f); /*FIXME: does not have sense for this stem type - I must */
                /*       refactor P3DStemModelInstance                   */
 }

void               P3DStemModelQuadInstance::GetWorldTransform
                                      (float              *Transform) const
 {
  /*FIXME: need standard way to copy matrix into float array */
  for (unsigned int i = 0; i < 16; i++)
   {
    Transform[i] = WorldTransform.m[i];
   }
 }

void               P3DStemModelQuadInstance::GetAxisPointAt
                                      (float              *Pos,
                                       float               Offset) const
 {
  Pos[0] = 0.0f;
  Pos[1] = P3DMath::Clampf(0.0f,1.0f,Offset) * Length;
  Pos[2] = 0.0f;
 }

void               P3DStemModelQuadInstance::GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset P3D_UNUSED_ATTR) const
 {
  P3DQuaternionf::MakeIdentity(Orientation);
 }

                   P3DStemModelQuad::P3DStemModelQuad
                                      ()
 {
  Length = 0.05;
  Width  = 0.05;

  BillboardMode = P3D_BILLBOARD_MODE_NONE;

  MakeDefaultScalingCurve(ScalingCurve);

  SectionCount = 1;
  MakeDefaultCurvatureCurve(Curvature);
  Thickness    = 0.0f;
 }

void               P3DStemModelQuad::MakeDefaultScalingCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(1.0f);
 }

void               P3DStemModelQuad::MakeDefaultCurvatureCurve
                                      (P3DMathNaturalCubicSpline
                                                          &Curve)
 {
  Curve.SetConstant(0.5f);
 }

P3DStemModel      *P3DStemModelQuad::CreateCopy
                                      () const
 {
  P3DStemModelQuad                    *Result;

  Result = new P3DStemModelQuad();

  Result->Length = Length;
  Result->Width  = Width;

  Result->BillboardMode = BillboardMode;

  Result->ScalingCurve.CopyFrom(ScalingCurve);

  Result->SectionCount = SectionCount;
  Result->Curvature.CopyFrom(Curvature);
  Result->Thickness    = Thickness;

  return(Result);
 }

P3DStemModelInstance
                  *P3DStemModelQuad::CreateInstance
                                      (P3DMathRNG         *RNG P3D_UNUSED_ATTR,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       float               Offset,
                                       const P3DQuaternionf
                                                          *Orientation) const
 {
  P3DStemModelQuadInstance            *Instance;
  float                                Scale;

  Scale = ScalingCurve.GetValue(Offset);

  if (Parent == 0)
   {
    if (Orientation != 0)
     {
      P3DMatrix4x4f                    WorldTransform;

      Orientation->ToMatrix(WorldTransform.m);

      Instance = new P3DStemModelQuadInstance
                      ( Length * Scale,
                        Width * Scale,
                        BillboardMode,
                        SectionCount,
                       &Curvature,
                        Thickness * Scale,
                       &WorldTransform);
     }
    else
     {
      Instance = new P3DStemModelQuadInstance
                      ( Length * Scale,
                        Width * Scale,
                        BillboardMode,
                        SectionCount,
                       &Curvature,
                        Thickness * Scale,
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

    Parent->GetAxisOrientationAt(ParentOrientation.q,Offset);
    Parent->GetAxisPointAt(ParentAxisPos.v,Offset);
    Parent->GetWorldTransform(ParentTransform.m);

    P3DQuaternionf::CrossProduct(InstanceOrientation.q,
                                 ParentOrientation.q,
                                 Orientation->q);

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

    Instance = new P3DStemModelQuadInstance
                    ( Length * Scale,
                      Width * Scale,
                      BillboardMode,
                      SectionCount,
                     &Curvature,
                      Thickness * Scale,
                     &WorldTransform);
   }

  return(Instance);
 }

void               P3DStemModelQuad::ReleaseInstance
                                      (P3DStemModelInstance
                                                          *Instance) const
 {
  delete Instance;
 }

unsigned int       P3DStemModelQuad::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if (SectionCount == 1)
   {
    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      return(1);
     }
    else
     {
      return(4);
     }
   }
  else
   {
    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      return(SectionCount + 1);
     }
    else
     {
      return((SectionCount + 1) * 2);
     }
   }
 }

unsigned int       P3DStemModelQuad::GetPrimitiveCount
                                      () const
 {
  return(SectionCount);
 }

unsigned int       P3DStemModelQuad::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex P3D_UNUSED_ATTR) const
 {
  return(P3D_QUAD);
 }

void               P3DStemModelQuad::FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  unsigned short                      *ShortBuffer;
  unsigned int                        *IntBuffer;

  ShortBuffer = (unsigned short*)IndexBuffer;
  IntBuffer   = (unsigned int*)IndexBuffer;

  if (SectionCount == 1)
   {
    if ((Attr == P3D_ATTR_NORMAL)  ||
        (Attr == P3D_ATTR_TANGENT) ||
        (Attr == P3D_ATTR_BINORMAL))
     {
      if (ElementType == P3D_UNSIGNED_INT)
       {
        IntBuffer[0] = IndexBase;
        IntBuffer[1] = IntBuffer[0];
        IntBuffer[2] = IntBuffer[0];
        IntBuffer[3] = IntBuffer[0];
       }
      else
       {
        ShortBuffer[0] = (unsigned short)IndexBase;
        ShortBuffer[1] = ShortBuffer[0];
        ShortBuffer[2] = ShortBuffer[0];
        ShortBuffer[3] = ShortBuffer[0];
       }
     }
    else
     {
      if (ElementType == P3D_UNSIGNED_INT)
       {
        IntBuffer[0] = IndexBase;
        IntBuffer[1] = IntBuffer[0] + 1;
        IntBuffer[2] = IntBuffer[0] + 3;
        IntBuffer[3] = IntBuffer[0] + 2;
       }
      else
       {
        ShortBuffer[0] = (unsigned short)IndexBase;
        ShortBuffer[1] = ShortBuffer[0] + 1;
        ShortBuffer[2] = ShortBuffer[0] + 3;
        ShortBuffer[3] = ShortBuffer[0] + 2;
       }
     }
   }
  else
   {
    for (unsigned int SectionIndex = 0; SectionIndex < SectionCount; SectionIndex++)
     {
      if ((Attr == P3D_ATTR_NORMAL)  ||
          (Attr == P3D_ATTR_TANGENT) ||
          (Attr == P3D_ATTR_BINORMAL))
       {
        if (ElementType == P3D_UNSIGNED_INT)
         {
          IntBuffer[0] = IndexBase + SectionIndex;
          IntBuffer[1] = IntBuffer[0];
          IntBuffer[2] = IntBuffer[0] + 1;
          IntBuffer[3] = IntBuffer[0] + 1;

          IntBuffer += 4;
         }
        else
         {
          ShortBuffer[0] = (unsigned short)(IndexBase + SectionIndex);
          ShortBuffer[1] = ShortBuffer[0];
          ShortBuffer[2] = ShortBuffer[0] + 1;
          ShortBuffer[3] = ShortBuffer[0] + 1;

          ShortBuffer += 4;
         }
       }
      else
       {
        if (ElementType == P3D_UNSIGNED_INT)
         {
          IntBuffer[0] = IndexBase + SectionIndex * 2;
          IntBuffer[1] = IntBuffer[0] + 1;
          IntBuffer[2] = IntBuffer[1] + 2;
          IntBuffer[3] = IntBuffer[1] + 1;

          IntBuffer += 4;
         }
        else
         {
          ShortBuffer[0] = (unsigned short)(IndexBase + SectionIndex * 2);
          ShortBuffer[1] = ShortBuffer[0] + 1;
          ShortBuffer[2] = ShortBuffer[1] + 2;
          ShortBuffer[3] = ShortBuffer[1] + 1;

          ShortBuffer += 4;
         }
       }
     }
   }
 }

unsigned int       P3DStemModelQuad::GetVAttrCountI
                                      () const
 {
  return(2 + SectionCount * 2);
 }

unsigned int       P3DStemModelQuad::GetIndexCount
                                      (unsigned int        PrimitiveType) const
 {
  if (PrimitiveType == P3D_TRIANGLE_LIST)
   {
    return(SectionCount * 2 * 3);
   }
  else
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }
 }

void               P3DStemModelQuad::FillIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  if (PrimitiveType == P3D_TRIANGLE_LIST)
   {
    unsigned short                    *ShortBuffer;
    unsigned int                      *IntBuffer;
    unsigned int                       SectionIndex;
    unsigned int                       Index;

    Index       = 0;
    ShortBuffer = (unsigned short*)IndexBuffer;
    IntBuffer   = (unsigned int*)IndexBuffer;

    for (SectionIndex = 0; SectionIndex < SectionCount; SectionIndex++)
     {
      if (ElementType == P3D_UNSIGNED_INT)
       {
        IntBuffer[Index]     = IndexBase + SectionIndex * 2;
        IntBuffer[Index + 1] = IntBuffer[Index] + 1;
        IntBuffer[Index + 2] = IntBuffer[Index] + 2;
        IntBuffer[Index + 3] = IntBuffer[Index + 2];
        IntBuffer[Index + 4] = IntBuffer[Index + 1];
        IntBuffer[Index + 5] = IntBuffer[Index] + 3;
       }
      else /* (ElementType == P3D_UNSIGNED_SHORT) */
       {
        ShortBuffer[Index]     = (unsigned short)(IndexBase + SectionIndex * 2);
        ShortBuffer[Index + 1] = (unsigned short)(ShortBuffer[Index] + 1);
        ShortBuffer[Index + 2] = (unsigned short)(ShortBuffer[Index] + 2);
        ShortBuffer[Index + 3] = (unsigned short)(ShortBuffer[Index + 2]);
        ShortBuffer[Index + 4] = (unsigned short)(ShortBuffer[Index + 1]);
        ShortBuffer[Index + 5] = (unsigned short)(ShortBuffer[Index] + 3);
       }

      Index += 6;
     }
   }
  else
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }
 }

void               P3DStemModelQuad::SetLength
                                      (float               Length)
 {
  this->Length = P3DMath::Clampf(0.0f,100.0f,Length);
 }

float              P3DStemModelQuad::GetLength
                                      () const
 {
  return(Length);
 }

void               P3DStemModelQuad::SetWidth
                                      (float               Width)
 {
  this->Width = P3DMath::Clampf(0.0f,100.0f,Width);
 }

float              P3DStemModelQuad::GetWidth
                                      () const
 {
  return(Width);
 }

unsigned int       P3DStemModelQuad::GetBillboardMode
                                      () const
 {
  return(BillboardMode);
 }

void               P3DStemModelQuad::SetBillboardMode
                                      (unsigned int        Mode)
 {
  BillboardMode = Mode;
 }

bool               P3DStemModelQuad::IsBillboard
                                      () const
 {
  return((BillboardMode != P3D_BILLBOARD_MODE_NONE) ? true : false);
 }

void               P3DStemModelQuad::SetScalingCurve
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  ScalingCurve.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelQuad::GetScalingCurve
                                      () const
 {
  return(&ScalingCurve);
 }

void               P3DStemModelQuad::SetSectionCount
                                      (unsigned int        SectionCount)
 {
  if (SectionCount > 0)
   {
    this->SectionCount = SectionCount;
   }
  else
   {
    this->SectionCount = 1;
   }
 }

unsigned int       P3DStemModelQuad::GetSectionCount
                                      () const
 {
  return(SectionCount);
 }

void               P3DStemModelQuad::SetCurvature
                                      (const P3DMathNaturalCubicSpline
                                                          *Curve)
 {
  Curvature.CopyFrom(*Curve);
 }

const P3DMathNaturalCubicSpline
                  *P3DStemModelQuad::GetCurvature
                                      () const
 {
  return(&Curvature);
 }

void               P3DStemModelQuad::SetThickness
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

float              P3DStemModelQuad::GetThickness
                                      () const
 {
  return(Thickness);
 }

void               P3DStemModelQuad::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","StemModel","Quad");

  FmtStream.WriteString("sf","Length",Length);
  FmtStream.WriteString("sf","Width",Width);
  FmtStream.WriteString("ss","Scaling","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&ScalingCurve);
  FmtStream.WriteString("su","SectionCount",SectionCount);
  FmtStream.WriteString("ss","Curvature","CubicSpline");
  P3DSaveSplineCurve(&FmtStream,&Curvature);
  FmtStream.WriteString("sf","Thickness",Thickness);
 }

void               P3DStemModelQuad::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version P3D_UNUSED_ATTR)
 {
  char                                 StrValue[255 + 1];
  float                                FloatValue;
  unsigned int                         UintValue;

  SourceStream->ReadFmtStringTagged("Length","f",&FloatValue);
  SetLength(FloatValue);
  SourceStream->ReadFmtStringTagged("Width","f",&FloatValue);
  SetWidth(FloatValue);

  if (Version->Minor > 1)
   {
    SourceStream->ReadFmtStringTagged("Scaling","s",StrValue,sizeof(StrValue));
    P3DLoadSplineCurve(&ScalingCurve,SourceStream,StrValue);
    SourceStream->ReadFmtStringTagged("SectionCount","u",&UintValue);
    SetSectionCount(UintValue);
    SourceStream->ReadFmtStringTagged("Curvature","s",StrValue,sizeof(StrValue));
    P3DLoadSplineCurve(&Curvature,SourceStream,StrValue);
    SourceStream->ReadFmtStringTagged("Thickness","f",&FloatValue);
    SetThickness(FloatValue);
   }
  else
   {
    ScalingCurve.SetConstant(1.0f);

    SectionCount = 1;
    Curvature.SetConstant(0.5f);
    Thickness    = 0.0f;
   }
 }

