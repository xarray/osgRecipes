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

#include <ngpcore/p3dmodelstemgmesh.h>

class P3DStemModelGMeshInstance : public P3DStemModelInstance
 {
  public           :

                   P3DStemModelGMeshInstance
                                      (const P3DGMeshData *MeshData,
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

  const P3DGMeshData                  *MeshData;
  P3DMatrix4x4f                        WorldTransform;
 };

                   P3DStemModelGMeshInstance::P3DStemModelGMeshInstance
                                      (const P3DGMeshData *MeshData,
                                       const P3DMatrix4x4f*Transform)
 {
  this->MeshData = MeshData;

  if (Transform == 0)
   {
    P3DMatrix4x4f::MakeIdentity(WorldTransform.m);
   }
  else
   {
    WorldTransform = *Transform;
   }
 }

unsigned int       P3DStemModelGMeshInstance::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if      (Attr >= P3D_GMESH_MAX_ATTRS)
   {
    throw P3DExceptionGeneric("invalid vertex attribute");
   }
  else if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetVAttrCount(Attr));
   }
 }

void               P3DStemModelGMeshInstance::GetVAttrValue
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if (Index >= GetVAttrCount(Attr))
   {
    throw P3DExceptionGeneric("invalid attribyte index");
   }

  if (Attr >= P3D_GMESH_MAX_ATTRS)
   {
    throw P3DExceptionGeneric("invalid vertex attribute");
   }

  const float     *SrcValue;

  SrcValue = MeshData->GetVAttrBuffer(Attr);

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    SrcValue += 2 * Index;
    *Value++ = *SrcValue++;
    *Value   = *SrcValue;
   }
  else
   {
    SrcValue += 3 * Index;

    if (Attr == P3D_ATTR_VERTEX)
     {
      P3DVector3f::MultMatrix(Value,&WorldTransform,SrcValue);
     }
    else if ((Attr == P3D_ATTR_NORMAL)   ||
             (Attr == P3D_ATTR_BINORMAL) ||
             (Attr == P3D_ATTR_TANGENT))
     {
      P3DMatrix4x4f                    Rotation;
      P3DVector3f                      V;

      P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

      P3DVector3f::MultMatrix(V.v,&Rotation,SrcValue);
      V.Normalize();

      Value[0] = V.X();
      Value[1] = V.Y();
      Value[2] = V.Z();
     }
    else
     {
      Value[0] = SrcValue[0];
      Value[1] = SrcValue[1];
      Value[2] = SrcValue[2];
     }
   }
 }

unsigned int       P3DStemModelGMeshInstance::GetPrimitiveCount
                                      () const
 {
  if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetPrimitiveCount());
   }
 }

unsigned int       P3DStemModelGMeshInstance::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex) const
 {
  if (PrimitiveIndex >= GetPrimitiveCount())
   {
    throw P3DExceptionGeneric("invalid primitive index");
   }

  return(*(MeshData->GetPrimitiveBuffer() + PrimitiveIndex));
 }

unsigned int       P3DStemModelGMeshInstance::GetVAttrCountI
                                      () const
 {
  if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetVAttrCountI());
   }
 }

void               P3DStemModelGMeshInstance::GetVAttrValueI
                                      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const
 {
  if (Index >= GetVAttrCountI())
   {
    throw P3DExceptionGeneric("invalid vertex index");
   }

  if (Attr >= P3D_GMESH_MAX_ATTRS)
   {
    throw P3DExceptionGeneric("invalid vertex attribute");
   }

  const float     *SrcValue;

  SrcValue = MeshData->GetVAttrBufferI(Attr);

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    SrcValue += 2 * Index;
    *Value++ = *SrcValue++;
    *Value   = *SrcValue;
   }
  else
   {
    SrcValue += 3 * Index;

    if (Attr == P3D_ATTR_VERTEX)
     {
      P3DVector3f::MultMatrix(Value,&WorldTransform,SrcValue);
     }
    else if ((Attr == P3D_ATTR_NORMAL)   ||
             (Attr == P3D_ATTR_BINORMAL) ||
             (Attr == P3D_ATTR_TANGENT))
     {
      P3DMatrix4x4f                    Rotation;
      P3DVector3f                      V;

      P3DMatrix4x4f::GetRotationOnly(Rotation.m,WorldTransform.m);

      P3DVector3f::MultMatrix(V.v,&Rotation,SrcValue);
      V.Normalize();

      Value[0] = V.X();
      Value[1] = V.Y();
      Value[2] = V.Z();
     }
    else
     {
      Value[0] = SrcValue[0];
      Value[1] = SrcValue[1];
      Value[2] = SrcValue[2];
     }
   }
 }

void               P3DStemModelGMeshInstance::GetBoundBox
                                      (float              *Min,
                                       float              *Max) const
 {
  P3DStemModelInstance::GetBoundBox(Min,Max);
 }

float              P3DStemModelGMeshInstance::GetLength
                                      () const
 {
  return(0.0f);
 }

float              P3DStemModelGMeshInstance::GetMinRadiusAt
                                      (float               Offset P3D_UNUSED_ATTR) const
 {
  return(0.0f);
 }

void               P3DStemModelGMeshInstance::GetWorldTransform
                                      (float              *Transform) const
 {
  /*FIXME: need standard way to copy matrix into float array */
  for (unsigned int i = 0; i < 16; i++)
   {
    Transform[i] = WorldTransform.m[i];
   }
 }

void               P3DStemModelGMeshInstance::GetAxisPointAt
                                      (float              *Pos,
                                       float               Offset P3D_UNUSED_ATTR) const
 {
  Pos[0] = Pos[1] = Pos[2] = 0.0f;
 }

void               P3DStemModelGMeshInstance::GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset P3D_UNUSED_ATTR) const
 {
  P3DQuaternionf::MakeIdentity(Orientation);
 }

                   P3DStemModelGMesh::P3DStemModelGMesh
                                      ()
 {
  MeshData = 0;
 }

P3DStemModelInstance
                  *P3DStemModelGMesh::CreateInstance
                                      (P3DMathRNG         *RNG P3D_UNUSED_ATTR,
                                       const P3DStemModelInstance
                                                          *Parent,
                                       float               Offset,
                                       const P3DQuaternionf
                                                          *Orientation) const
 {
  P3DStemModelGMeshInstance           *Instance;

  if (Parent == 0)
   {
    if (Orientation != 0)
     {
      P3DMatrix4x4f                    WorldTransform;

      Orientation->ToMatrix(WorldTransform.m);

      Instance = new P3DStemModelGMeshInstance(MeshData,&WorldTransform);
     }
    else
     {
      Instance = new P3DStemModelGMeshInstance(MeshData,0);
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

    Instance = new P3DStemModelGMeshInstance
                    (MeshData,&WorldTransform);
   }

  return(Instance);
 }

void               P3DStemModelGMesh::ReleaseInstance
                                      (P3DStemModelInstance
                                                          *Instance) const
 {
  delete Instance;
 }

P3DStemModel      *P3DStemModelGMesh::CreateCopy
                                      () const
 {
  P3DStemModelGMesh                  *Result;

  Result = new P3DStemModelGMesh();

  if (MeshData != 0)
   {
    Result->SetMeshData(MeshData->CreateCopy());
   }

  return(Result);
 }

unsigned int       P3DStemModelGMesh::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if      (Attr >= P3D_GMESH_MAX_ATTRS)
   {
    throw P3DExceptionGeneric("invalid vertex attribute");
   }
  else if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetVAttrCount(Attr));
   }
 }

unsigned int       P3DStemModelGMesh::GetPrimitiveCount
                                      () const
 {
  if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetPrimitiveCount());
   }
 }

unsigned int       P3DStemModelGMesh::GetPrimitiveType
                                      (unsigned int        PrimitiveIndex) const
 {
  if (PrimitiveIndex >= GetPrimitiveCount())
   {
    throw P3DExceptionGeneric("invalid primitive index");
   }
  else
   {
    return(MeshData->GetPrimitiveBuffer()[PrimitiveIndex]);
   }
 }

void               P3DStemModelGMesh::FillIndexArray
                                      (unsigned short     *Target,
                                       const unsigned int *Source,
                                       unsigned int        Count,
                                       unsigned int        IndexBase)
 {
  while (Count-- > 0)
   {
    *Target++ = *Source++ + IndexBase;
   }
 }

void               P3DStemModelGMesh::FillIndexArray
                                      (unsigned int       *Target,
                                       const unsigned int *Source,
                                       unsigned int        Count,
                                       unsigned int        IndexBase)
 {
  while (Count-- > 0)
   {
    *Target++ = *Source++ + IndexBase;
   }
 }

void               P3DStemModelGMesh::FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  if (MeshData == 0)
   {
    return;
   }

  if (Attr >= P3D_GMESH_MAX_ATTRS)
   {
    throw P3DExceptionGeneric("invalid vertex attribute");
   }

  if (ElementType == P3D_UNSIGNED_INT)
   {
    FillIndexArray((unsigned int*)IndexBuffer,
                   MeshData->GetIndexBuffer(Attr),
                   MeshData->GetIndexCount(),
                   IndexBase);
   }
  else
   {
    FillIndexArray((unsigned short*)IndexBuffer,
                   MeshData->GetIndexBuffer(Attr),
                   MeshData->GetIndexCount(),
                   IndexBase);

   }
 }

unsigned int       P3DStemModelGMesh::GetVAttrCountI
                                      () const
 {
  if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetVAttrCountI());
   }
 }

unsigned int       P3DStemModelGMesh::GetIndexCount
                                      (unsigned int        PrimitiveType) const
 {
  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    throw P3DExceptionGeneric("invalid primitive type");
   }

  if (MeshData == 0)
   {
    return(0);
   }
  else
   {
    return(MeshData->GetIndexCountI());
   }
 }

void               P3DStemModelGMesh::FillIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase) const
 {
  if (PrimitiveType != P3D_TRIANGLE_LIST)
   {
    throw P3DExceptionGeneric("unsupported primitive type");
   }

  if (MeshData == 0)
   {
    return;
   }

  if (ElementType == P3D_UNSIGNED_INT)
   {
    FillIndexArray((unsigned int*)IndexBuffer,
                   MeshData->GetIndexBufferI(),
                   MeshData->GetIndexCountI(),
                   IndexBase);
   }
  else
   {
    FillIndexArray((unsigned short*)IndexBuffer,
                   MeshData->GetIndexBufferI(),
                   MeshData->GetIndexCountI(),
                   IndexBase);
   }
 }

static void        SaveVAttr          (P3DOutputStringFmtStream
                                                          *FmtStream,
                                       const P3DGMeshData *MeshData,
                                       unsigned int        Attr,
                                       const char         *AttrName)
 {
  unsigned int                         AttrIndex;
  unsigned int                         AttrCount;
  const float                         *Buffer;

  Buffer    = MeshData->GetVAttrBuffer(Attr);
  AttrCount = MeshData->GetVAttrCount(Attr);

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->WriteString("sff",AttrName,Buffer[0],Buffer[1]);

      Buffer += 2;
     }
   }
  else
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->WriteString("sfff",AttrName,Buffer[0],Buffer[1],Buffer[2]);

      Buffer += 3;
     }
   }
 }

static void        LoadVAttr          (P3DInputStringFmtStream
                                                          *FmtStream,
                                       P3DGMeshData       *MeshData,
                                       unsigned int        Attr,
                                       const char         *AttrName)
 {
  unsigned int                         AttrIndex;
  unsigned int                         AttrCount;
  float                               *Buffer;

  Buffer    = MeshData->GetVAttrBuffer(Attr);
  AttrCount = MeshData->GetVAttrCount(Attr);

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->ReadFmtStringTagged(AttrName,"ff",&Buffer[0],&Buffer[1]);

      Buffer += 2;
     }
   }
  else
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->ReadFmtStringTagged(AttrName,"fff",&Buffer[0],&Buffer[1],&Buffer[2]);

      Buffer += 3;
     }
   }
 }

static void        SaveVAttrI         (P3DOutputStringFmtStream
                                                          *FmtStream,
                                       const P3DGMeshData *MeshData,
                                       unsigned int        Attr,
                                       const char         *AttrName)
 {
  unsigned int                         AttrIndex;
  unsigned int                         AttrCount;
  const float                         *Buffer;

  Buffer    = MeshData->GetVAttrBufferI(Attr);
  AttrCount = MeshData->GetVAttrCountI();

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->WriteString("sff",AttrName,Buffer[0],Buffer[1]);

      Buffer += 2;
     }
   }
  else
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->WriteString("sfff",AttrName,Buffer[0],Buffer[1],Buffer[2]);

      Buffer += 3;
     }
   }
 }

static void        LoadVAttrI         (P3DInputStringFmtStream
                                                          *FmtStream,
                                       P3DGMeshData       *MeshData,
                                       unsigned int        Attr,
                                       const char         *AttrName)
 {
  unsigned int                         AttrIndex;
  unsigned int                         AttrCount;
  float                               *Buffer;

  Buffer    = MeshData->GetVAttrBufferI(Attr);
  AttrCount = MeshData->GetVAttrCountI();

  if (Attr == P3D_ATTR_TEXCOORD0)
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->ReadFmtStringTagged(AttrName,"ff",&Buffer[0],&Buffer[1]);

      Buffer += 2;
     }
   }
  else
   {
    for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
     {
      FmtStream->ReadFmtStringTagged(AttrName,"fff",&Buffer[0],&Buffer[1],&Buffer[2]);

      Buffer += 3;
     }
   }
 }

void               P3DStemModelGMesh::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);
  unsigned int                         PrimitiveIndex;
  unsigned int                         PrimitiveCount;
  const unsigned int                  *PrimitiveBuffer;
  const unsigned int                  *IndexBuffer[P3D_GMESH_MAX_ATTRS];

  FmtStream.WriteString("ss","StemModel","GMesh");

  FmtStream.WriteString("su","VAttrVertexCount",MeshData->GetVAttrCount(P3D_ATTR_VERTEX));
  FmtStream.WriteString("su","VAttrNormalCount",MeshData->GetVAttrCount(P3D_ATTR_NORMAL));
  FmtStream.WriteString("su","VAttrTexCoord0Count",MeshData->GetVAttrCount(P3D_ATTR_TEXCOORD0));
  FmtStream.WriteString("su","VAttrTangentCount",MeshData->GetVAttrCount(P3D_ATTR_TANGENT));
  FmtStream.WriteString("su","VAttrBinormalCount",MeshData->GetVAttrCount(P3D_ATTR_BINORMAL));

  PrimitiveCount = MeshData->GetPrimitiveCount();

  FmtStream.WriteString("su","PrimitiveCount",PrimitiveCount);
  FmtStream.WriteString("su","IndexCount",MeshData->GetIndexCount());

  FmtStream.WriteString("su","VAttrCountI",MeshData->GetVAttrCountI());
  FmtStream.WriteString("su","IndexCountI",MeshData->GetIndexCountI());

  SaveVAttr(&FmtStream,MeshData,P3D_ATTR_VERTEX,"Vertex");
  SaveVAttr(&FmtStream,MeshData,P3D_ATTR_NORMAL,"Normal");
  SaveVAttr(&FmtStream,MeshData,P3D_ATTR_TEXCOORD0,"TexCoord0");
  SaveVAttr(&FmtStream,MeshData,P3D_ATTR_TANGENT,"Tangent");
  SaveVAttr(&FmtStream,MeshData,P3D_ATTR_BINORMAL,"Binormal");

  PrimitiveBuffer = MeshData->GetPrimitiveBuffer();

  IndexBuffer[P3D_ATTR_VERTEX]    = MeshData->GetIndexBuffer(P3D_ATTR_VERTEX);
  IndexBuffer[P3D_ATTR_NORMAL]    = MeshData->GetIndexBuffer(P3D_ATTR_NORMAL);
  IndexBuffer[P3D_ATTR_TEXCOORD0] = MeshData->GetIndexBuffer(P3D_ATTR_TEXCOORD0);
  IndexBuffer[P3D_ATTR_TANGENT]   = MeshData->GetIndexBuffer(P3D_ATTR_TANGENT);
  IndexBuffer[P3D_ATTR_BINORMAL]  = MeshData->GetIndexBuffer(P3D_ATTR_BINORMAL);

  for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
   {
    FmtStream.WriteString("su","PrimType",PrimitiveBuffer[PrimitiveIndex]);

    FmtStream.WriteString("suuuuu","PrimVert",IndexBuffer[P3D_ATTR_VERTEX][0],
                                              IndexBuffer[P3D_ATTR_NORMAL][0],
                                              IndexBuffer[P3D_ATTR_TEXCOORD0][0],
                                              IndexBuffer[P3D_ATTR_TANGENT][0],
                                              IndexBuffer[P3D_ATTR_BINORMAL][0]);

    FmtStream.WriteString("suuuuu","PrimVert",IndexBuffer[P3D_ATTR_VERTEX][1],
                                              IndexBuffer[P3D_ATTR_NORMAL][1],
                                              IndexBuffer[P3D_ATTR_TEXCOORD0][1],
                                              IndexBuffer[P3D_ATTR_TANGENT][1],
                                              IndexBuffer[P3D_ATTR_BINORMAL][1]);

    FmtStream.WriteString("suuuuu","PrimVert",IndexBuffer[P3D_ATTR_VERTEX][2],
                                              IndexBuffer[P3D_ATTR_NORMAL][2],
                                              IndexBuffer[P3D_ATTR_TEXCOORD0][2],
                                              IndexBuffer[P3D_ATTR_TANGENT][2],
                                              IndexBuffer[P3D_ATTR_BINORMAL][2]);

    if (PrimitiveBuffer[PrimitiveIndex] == P3D_QUAD)
     {
      FmtStream.WriteString("suuuuu","PrimVert",IndexBuffer[P3D_ATTR_VERTEX][3],
                                                IndexBuffer[P3D_ATTR_NORMAL][3],
                                                IndexBuffer[P3D_ATTR_TEXCOORD0][3],
                                                IndexBuffer[P3D_ATTR_TANGENT][3],
                                                IndexBuffer[P3D_ATTR_BINORMAL][3]);

      IndexBuffer[P3D_ATTR_VERTEX]    += 4;
      IndexBuffer[P3D_ATTR_NORMAL]    += 4;
      IndexBuffer[P3D_ATTR_TEXCOORD0] += 4;
      IndexBuffer[P3D_ATTR_TANGENT]   += 4;
      IndexBuffer[P3D_ATTR_BINORMAL]  += 4;
     }
    else
     {
      IndexBuffer[P3D_ATTR_VERTEX]    += 3;
      IndexBuffer[P3D_ATTR_NORMAL]    += 3;
      IndexBuffer[P3D_ATTR_TEXCOORD0] += 3;
      IndexBuffer[P3D_ATTR_TANGENT]   += 3;
      IndexBuffer[P3D_ATTR_BINORMAL]  += 3;
     }
   }

  SaveVAttrI(&FmtStream,MeshData,P3D_ATTR_VERTEX,"Vertex");
  SaveVAttrI(&FmtStream,MeshData,P3D_ATTR_NORMAL,"Normal");
  SaveVAttrI(&FmtStream,MeshData,P3D_ATTR_TEXCOORD0,"TexCoord0");
  SaveVAttrI(&FmtStream,MeshData,P3D_ATTR_TANGENT,"Tangent");
  SaveVAttrI(&FmtStream,MeshData,P3D_ATTR_BINORMAL,"Binormal");

  PrimitiveCount = MeshData->GetIndexCountI() / 3;
  IndexBuffer[0] = MeshData->GetIndexBufferI();

  for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
   {
    FmtStream.WriteString("suuu","PrimVert",IndexBuffer[0][0],
                                            IndexBuffer[0][1],
                                            IndexBuffer[0][2]);

    IndexBuffer[0] += 3;
   }
 }

void               P3DStemModelGMesh::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version P3D_UNUSED_ATTR)
 {
  P3DGMeshData                        *NewMeshData;
  unsigned int                         VAttrCounts[P3D_GMESH_MAX_ATTRS];
  unsigned int                         PrimitiveIndex;
  unsigned int                         PrimitiveCount;
  unsigned int                         IndexCount;
  unsigned int                         VAttrCountI;
  unsigned int                         IndexCountI;
  unsigned int                         IndexCounter;
  unsigned int                        *PrimitiveBuffer;
  unsigned int                        *IndexBuffer[P3D_GMESH_MAX_ATTRS];

  NewMeshData = 0;

  try
   {
    SourceStream->ReadFmtStringTagged("VAttrVertexCount","u",&VAttrCounts[P3D_ATTR_VERTEX]);
    SourceStream->ReadFmtStringTagged("VAttrNormalCount","u",&VAttrCounts[P3D_ATTR_NORMAL]);
    SourceStream->ReadFmtStringTagged("VAttrTexCoord0Count","u",&VAttrCounts[P3D_ATTR_TEXCOORD0]);
    SourceStream->ReadFmtStringTagged("VAttrTangentCount","u",&VAttrCounts[P3D_ATTR_TANGENT]);
    SourceStream->ReadFmtStringTagged("VAttrBinormalCount","u",&VAttrCounts[P3D_ATTR_BINORMAL]);

    SourceStream->ReadFmtStringTagged("PrimitiveCount","u",&PrimitiveCount);
    SourceStream->ReadFmtStringTagged("IndexCount","u",&IndexCount);
    SourceStream->ReadFmtStringTagged("VAttrCountI","u",&VAttrCountI);
    SourceStream->ReadFmtStringTagged("IndexCountI","u",&IndexCountI);

    if ((VAttrCounts[P3D_ATTR_VERTEX] == 0)    ||
        (VAttrCounts[P3D_ATTR_NORMAL] == 0)    ||
        (VAttrCounts[P3D_ATTR_TEXCOORD0] == 0) ||
        (VAttrCounts[P3D_ATTR_TANGENT] == 0)   ||
        (VAttrCounts[P3D_ATTR_BINORMAL] == 0)  ||
        (VAttrCountI == 0))
     {
      throw P3DExceptionGeneric("invalid vertex attribute count in g-mesh data");
     }

    if (PrimitiveCount == 0)
     {
      throw P3DExceptionGeneric("invalid primitive count in g-mesh data");
     }

    if ((IndexCount == 0) || (IndexCountI == 0) || ((IndexCountI % 3) != 0))
     {
      throw P3DExceptionGeneric("invalid index count in g-mesh data");
     }

    NewMeshData = new P3DGMeshData(VAttrCounts,PrimitiveCount,IndexCount,VAttrCountI,IndexCountI);

    LoadVAttr(SourceStream,NewMeshData,P3D_ATTR_VERTEX,"Vertex");
    LoadVAttr(SourceStream,NewMeshData,P3D_ATTR_NORMAL,"Normal");
    LoadVAttr(SourceStream,NewMeshData,P3D_ATTR_TEXCOORD0,"TexCoord0");
    LoadVAttr(SourceStream,NewMeshData,P3D_ATTR_TANGENT,"Tangent");
    LoadVAttr(SourceStream,NewMeshData,P3D_ATTR_BINORMAL,"Binormal");

    PrimitiveBuffer = NewMeshData->GetPrimitiveBuffer();

    IndexBuffer[P3D_ATTR_VERTEX]    = NewMeshData->GetIndexBuffer(P3D_ATTR_VERTEX);
    IndexBuffer[P3D_ATTR_NORMAL]    = NewMeshData->GetIndexBuffer(P3D_ATTR_NORMAL);
    IndexBuffer[P3D_ATTR_TEXCOORD0] = NewMeshData->GetIndexBuffer(P3D_ATTR_TEXCOORD0);
    IndexBuffer[P3D_ATTR_TANGENT]   = NewMeshData->GetIndexBuffer(P3D_ATTR_TANGENT);
    IndexBuffer[P3D_ATTR_BINORMAL]  = NewMeshData->GetIndexBuffer(P3D_ATTR_BINORMAL);

    IndexCounter = 0;

    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      SourceStream->ReadFmtStringTagged("PrimType","u",&PrimitiveBuffer[PrimitiveIndex]);

      if      (PrimitiveBuffer[PrimitiveIndex] == P3D_TRIANGLE)
       {
        IndexCounter += 3;
       }
      else if (PrimitiveBuffer[PrimitiveIndex] == P3D_QUAD)
       {
        IndexCounter += 4;
       }
      else
       {
        throw P3DExceptionGeneric("invalid primitive type in g-mesh data");
       }

      if (IndexCounter > IndexCount)
       {
        throw P3DExceptionGeneric("primitive types/index count inconsistency found in g-mesh data");
       }

      SourceStream->ReadFmtStringTagged("PrimVert","uuuuu",
                                        &IndexBuffer[P3D_ATTR_VERTEX][0],
                                        &IndexBuffer[P3D_ATTR_NORMAL][0],
                                        &IndexBuffer[P3D_ATTR_TEXCOORD0][0],
                                        &IndexBuffer[P3D_ATTR_TANGENT][0],
                                        &IndexBuffer[P3D_ATTR_BINORMAL][0]);

      SourceStream->ReadFmtStringTagged("PrimVert","uuuuu",
                                        &IndexBuffer[P3D_ATTR_VERTEX][1],
                                        &IndexBuffer[P3D_ATTR_NORMAL][1],
                                        &IndexBuffer[P3D_ATTR_TEXCOORD0][1],
                                        &IndexBuffer[P3D_ATTR_TANGENT][1],
                                        &IndexBuffer[P3D_ATTR_BINORMAL][1]);

      SourceStream->ReadFmtStringTagged("PrimVert","uuuuu",
                                        &IndexBuffer[P3D_ATTR_VERTEX][2],
                                        &IndexBuffer[P3D_ATTR_NORMAL][2],
                                        &IndexBuffer[P3D_ATTR_TEXCOORD0][2],
                                        &IndexBuffer[P3D_ATTR_TANGENT][2],
                                        &IndexBuffer[P3D_ATTR_BINORMAL][2]);

      if (PrimitiveBuffer[PrimitiveIndex] == P3D_QUAD)
       {
        SourceStream->ReadFmtStringTagged("PrimVert","uuuuu",
                                          &IndexBuffer[P3D_ATTR_VERTEX][3],
                                          &IndexBuffer[P3D_ATTR_NORMAL][3],
                                          &IndexBuffer[P3D_ATTR_TEXCOORD0][3],
                                          &IndexBuffer[P3D_ATTR_TANGENT][3],
                                          &IndexBuffer[P3D_ATTR_BINORMAL][3]);

        IndexBuffer[P3D_ATTR_VERTEX]    += 4;
        IndexBuffer[P3D_ATTR_NORMAL]    += 4;
        IndexBuffer[P3D_ATTR_TEXCOORD0] += 4;
        IndexBuffer[P3D_ATTR_TANGENT]   += 4;
        IndexBuffer[P3D_ATTR_BINORMAL]  += 4;
       }
      else
       {
        IndexBuffer[P3D_ATTR_VERTEX]    += 3;
        IndexBuffer[P3D_ATTR_NORMAL]    += 3;
        IndexBuffer[P3D_ATTR_TEXCOORD0] += 3;
        IndexBuffer[P3D_ATTR_TANGENT]   += 3;
        IndexBuffer[P3D_ATTR_BINORMAL]  += 3;
       }
     }

    LoadVAttrI(SourceStream,NewMeshData,P3D_ATTR_VERTEX,"Vertex");
    LoadVAttrI(SourceStream,NewMeshData,P3D_ATTR_NORMAL,"Normal");
    LoadVAttrI(SourceStream,NewMeshData,P3D_ATTR_TEXCOORD0,"TexCoord0");
    LoadVAttrI(SourceStream,NewMeshData,P3D_ATTR_TANGENT,"Tangent");
    LoadVAttrI(SourceStream,NewMeshData,P3D_ATTR_BINORMAL,"Binormal");

    PrimitiveCount = IndexCountI / 3;
    IndexBuffer[0] = NewMeshData->GetIndexBufferI();

    for (PrimitiveIndex = 0; PrimitiveIndex < PrimitiveCount; PrimitiveIndex++)
     {
      SourceStream->ReadFmtStringTagged("PrimVert","uuu",
                                        &IndexBuffer[0][0],
                                        &IndexBuffer[0][1],
                                        &IndexBuffer[0][2]);

      IndexBuffer[0] += 3;
     }
   }
  catch (...)
   {
    delete NewMeshData;

    throw;
   }

  SetMeshData(NewMeshData);
 }

void               P3DStemModelGMesh::SetMeshData
                                      (P3DGMeshData       *MeshData)
 {
  delete this->MeshData;

  this->MeshData = MeshData;
 }

