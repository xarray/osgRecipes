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

#include <stdlib.h>
#include <string.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dcompat.h> /* for snprintf definition in MSVC environment */
#include <ngpcore/p3dexcept.h>
#include <ngpcore/p3diostream.h>
#include <ngpcore/p3dplant.h>

/* I need these includes for load functionality, if I'll use factories for */
/* balgs and models, I wouldn't need them                                  */
#include <ngpcore/p3dbalgbase.h>
#include <ngpcore/p3dbalgstd.h>
#include <ngpcore/p3dbalgwings.h>
#include <ngpcore/p3dmodelstemtube.h>
#include <ngpcore/p3dmodelstemquad.h>
#include <ngpcore/p3dmodelstemwings.h>
#include <ngpcore/p3dmodelstemgmesh.h>

#include <ngpcore/p3dmodel.h>

                   P3DVisRangeState::P3DVisRangeState
                                      ()
 {
  Enabled   = false;
  Range.Min = 0.0f;
  Range.Max = 1.0f;
 }

                   P3DVisRangeState::P3DVisRangeState
                                      (float               Min,
                                       float               Max)
 {
  SetRange(Min,Max);

  Enabled = true;
 }

bool               P3DVisRangeState::IsEnabled
                                      () const
 {
  return(Enabled);
 }

void               P3DVisRangeState::SetState
                                      (bool                Enable)
 {
  Enabled = Enable;
 }

void               P3DVisRangeState::GetRange
                                      (float              *Min,
                                       float              *Max) const
 {
  *Min = Range.Min;
  *Max = Range.Max;
 }

void               P3DVisRangeState::SetRange
                                      (float               Min,
                                       float               Max)
 {
  Range.Max = P3DMath::Clampf(0.0f,1.0f,Max);
  Range.Min = P3DMath::Clampf(0.0f,Range.Max,Min);
 }

void               P3DVisRangeState::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("sb","VisRangeEnabled",Enabled);
  FmtStream.WriteString("sff","VisRange",Range.Min,Range.Max);
 }

void               P3DVisRangeState::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version)
 {
  if ((Version->Major == 0) && (Version->Minor < 3))
   {
    Enabled   = false;
    Range.Min = 0.0f;
    Range.Max = 1.0f;
   }
  else
   {
    float                              Min,Max;

    SourceStream->ReadFmtStringTagged("VisRangeEnabled","b",&Enabled);
    SourceStream->ReadFmtStringTagged("VisRange","ff",&Min,&Max);

    SetRange(Min,Max);
   }
 }

                   P3DMaterialDef::P3DMaterialDef
                                      ()
 {
  R = 1.0f;
  G = 1.0f;
  B = 1.0f;

  for (unsigned int Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
   {
    TexNames[Layer] = NULL;
   }

  DoubleSided   = false;
  Transparent   = false;
  BillboardMode = P3D_BILLBOARD_MODE_NONE;

  AlphaCtrlEnabled = false;
  AlphaFadeIn      = 0.0f;
  AlphaFadeOut     = 0.0f;
 }

                   P3DMaterialDef::P3DMaterialDef
                                      (const P3DMaterialDef
                                                          &Source)
 {
  for (unsigned int Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
   {
    TexNames[Layer] = NULL;
   }

  CopyFrom(&Source);
 }

void               P3DMaterialDef::operator =
                                      (const P3DMaterialDef
                                                          &Source)
 {
  CopyFrom(&Source);
 }

                   P3DMaterialDef::~P3DMaterialDef
                                      ()
 {
  for (unsigned int Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
   {
    if (TexNames[Layer] != NULL)
     {
      free(TexNames[Layer]);
     }
   }
 }

void               P3DMaterialDef::CopyFrom
                                      (const P3DMaterialDef
                                                          *Source)
 {
  if (Source != this)
   {
    R             = Source->R;
    G             = Source->G;
    B             = Source->B;
    DoubleSided   = Source->DoubleSided;
    Transparent   = Source->Transparent;
    BillboardMode = Source->BillboardMode;
    AlphaCtrlEnabled = Source->AlphaCtrlEnabled;
    AlphaFadeIn   = Source->AlphaFadeIn;
    AlphaFadeOut  = Source->AlphaFadeOut;

    for (unsigned int Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
     {
      if (TexNames[Layer] != NULL)
       {
        free(TexNames[Layer]);

        TexNames[Layer] = NULL;
       }

      if (Source->TexNames[Layer] != NULL)
       {
        TexNames[Layer] = strdup(Source->TexNames[Layer]);

        if (TexNames[Layer] == NULL)
         {
          /*FIXME: throw something here */
         }
       }
     }
   }
 }

static const char *TexLayersNames[] =
 {
  "DiffuseTexture",
  "NormalMap",
  "AuxTexture0",
  "AuxTexture1"
 };

void               P3DMaterialDef::Save
                                      (P3DOutputStringStream
                                                          *TargetStream) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","Material","Simple");
  FmtStream.WriteString("sfff","BaseColor",R,G,B);

  for (unsigned int Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
   {
    if (TexNames[Layer] != NULL)
     {
      FmtStream.WriteString("ss",TexLayersNames[Layer],TexNames[Layer]);
     }
    else
     {
      FmtStream.WriteString("ss",TexLayersNames[Layer],"__None__");
     }
   }

  FmtStream.WriteString("sb","DoubleSided",DoubleSided);
  FmtStream.WriteString("sb","Transparent",Transparent);

  switch (BillboardMode)
   {
    case (P3D_BILLBOARD_MODE_SPHERICAL)     :
     {
      FmtStream.WriteString("ss","BillboardMode","spherical");
     } break;

    case (P3D_BILLBOARD_MODE_CYLINDRICAL)   :
     {
      FmtStream.WriteString("ss","BillboardMode","cylindrical");
     } break;

    default                                 :
     {
      FmtStream.WriteString("ss","BillboardMode","__None__");
     }
   }

  FmtStream.WriteString("sb","AlphaCtrlEnabled",AlphaCtrlEnabled);
  FmtStream.WriteString("sf","AlphaFadeIn",AlphaFadeIn);
  FmtStream.WriteString("sf","AlphaFadeOut",AlphaFadeOut);
 }

void               P3DMaterialDef::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version)
 {
  float                                R1,G1,B1;
  char                                 StrValue[255 + 1];
  unsigned int                         Layer;

  SourceStream->ReadFmtStringTagged("BaseColor","fff",&R1,&G1,&B1);

  SetColor(R1,G1,B1);

  for (Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
   {
    if (TexNames[Layer] != NULL)
     {
      free(TexNames[Layer]);
     }
   }

  if ((Version->Major == 0) && (Version->Minor < 4))
   {
    SourceStream->ReadFmtStringTagged("BaseTexture","s",StrValue,sizeof(StrValue));

    if (strcmp(StrValue,"__None__") == 0)
     {
     }
    else
     {
      SetTexName(P3D_TEX_DIFFUSE,StrValue);
     }
   }
  else
   {
    for (Layer = 0; Layer < P3D_MAX_TEX_LAYERS; Layer++)
     {
      SourceStream->ReadFmtStringTagged(TexLayersNames[Layer],"s",StrValue,sizeof(StrValue));

      if (strcmp(StrValue,"__None__") == 0)
       {
       }
      else
       {
        SetTexName(Layer,StrValue);
       }
     }
   }

  SourceStream->ReadFmtStringTagged("DoubleSided","b",&DoubleSided);
  SourceStream->ReadFmtStringTagged("Transparent","b",&Transparent);

  if ((Version->Major == 0) && (Version->Minor < 3))
   {
    bool           Billboard;

    SourceStream->ReadFmtStringTagged("Billboard","b",&Billboard);

    if (Billboard)
     {
      BillboardMode = P3D_BILLBOARD_MODE_SPHERICAL;
     }
    else
     {
      BillboardMode = P3D_BILLBOARD_MODE_NONE;
     }
   }
  else
   {
    SourceStream->ReadFmtStringTagged("BillboardMode","s",StrValue,sizeof(StrValue));

    if      (strcmp(StrValue,"spherical") == 0)
     {
      BillboardMode = P3D_BILLBOARD_MODE_SPHERICAL;
     }
    else if (strcmp(StrValue,"cylindrical") == 0)
     {
      BillboardMode = P3D_BILLBOARD_MODE_CYLINDRICAL;
     }
    else
     {
      BillboardMode = P3D_BILLBOARD_MODE_NONE;
     }
   }

  if ((Version->Major == 0) && (Version->Minor > 2))
   {
    SourceStream->ReadFmtStringTagged("AlphaCtrlEnabled","b",&AlphaCtrlEnabled);
    SourceStream->ReadFmtStringTagged("AlphaFadeIn","f",&AlphaFadeIn);
    SourceStream->ReadFmtStringTagged("AlphaFadeOut","f",&AlphaFadeOut);

    AlphaFadeIn  = P3DMath::Clampf(0.0f,1.0f,AlphaFadeIn);
    AlphaFadeOut = P3DMath::Clampf(0.0f,1.0f,AlphaFadeOut);
   }
 }

void               P3DMaterialDef::GetColor
                                      (float              *R,
                                       float              *G,
                                       float              *B) const
 {
  *R = this->R;
  *G = this->G;
  *B = this->B;
 }

void               P3DMaterialDef::SetColor
                                      (float               R,
                                       float               G,
                                       float               B)
 {
  /*FIXME: clamp values */
  this->R = R;
  this->G = G;
  this->B = B;
 }

const char        *P3DMaterialDef::GetTexName
                                      (unsigned int        Layer) const
 {
  if (Layer < P3D_MAX_TEX_LAYERS)
   {
    return(TexNames[Layer]);
   }
  else
   {
    return(NULL);
   }
 }

void               P3DMaterialDef::SetTexName
                                      (unsigned int        Layer,
                                       const char         *TexName)
 {
  if (Layer < P3D_MAX_TEX_LAYERS)
   {
    if (TexNames[Layer] != NULL)
     {
      free(TexNames[Layer]);

      TexNames[Layer] = NULL;
     }

    if (TexName != NULL)
     {
      TexNames[Layer] = strdup(TexName);

      if (TexNames[Layer] == NULL)
       {
        /*FIXME: throw something here */
       }
     }
   }
  else
   {
    /*FIXME: throw something here */
   }
 }

bool               P3DMaterialDef::IsDoubleSided
                                      () const
 {
  return(DoubleSided);
 }

void               P3DMaterialDef::SetDoubleSided
                                      (bool                DoubleSided)
 {
  this->DoubleSided = DoubleSided;
 }

bool               P3DMaterialDef::IsTransparent
                                      () const
 {
  return(Transparent);
 }

void               P3DMaterialDef::SetTransparent
                                      (bool                Transparent)
 {
  this->Transparent = Transparent;
 }

bool               P3DMaterialDef::IsBillboard
                                      () const
 {
  return((BillboardMode != P3D_BILLBOARD_MODE_NONE) ? true : false);
 }

unsigned int       P3DMaterialDef::GetBillboardMode
                                      () const
 {
  return(BillboardMode);
 }

void               P3DMaterialDef::SetBillboardMode
                                      (unsigned int        Mode)
 {
  this->BillboardMode = Mode;
 }

bool               P3DMaterialDef::IsAlphaCtrlEnabled
                                      () const
 {
  return(AlphaCtrlEnabled);
 }

void               P3DMaterialDef::SetAlphaCtrlState
                                      (bool                Enable)
 {
  AlphaCtrlEnabled = Enable;
 }

float              P3DMaterialDef::GetAlphaFadeIn
                                      () const
 {
  return(AlphaFadeIn);
 }

float              P3DMaterialDef::GetAlphaFadeOut
                                      () const
 {
  return(AlphaFadeOut);
 }

void               P3DMaterialDef::SetAlphaFadeIn
                                      (float               FadeIn)
 {
  AlphaFadeIn = P3DMath::Clampf(0.0f,1.0f,FadeIn);
 }

void               P3DMaterialDef::SetAlphaFadeOut
                                      (float               FadeOut)
 {
  AlphaFadeOut = P3DMath::Clampf(0.0f,1.0f,FadeOut);
 }

void               P3DStemModelInstance::GetBoundBox
                                      (float              *Min,
                                       float              *Max) const
 {
  unsigned int     VertexCount;

  VertexCount = GetVAttrCount(P3D_ATTR_VERTEX);

  if (VertexCount > 0)
   {
    GetVAttrValue(Min,P3D_ATTR_VERTEX,0);

    Max[0] = Min[0]; Max[1] = Min[1]; Max[2] = Min[2];

    for (unsigned int VertexIndex = 1; VertexIndex < VertexCount; VertexIndex++)
     {
      float        Temp[3];

      GetVAttrValue(Temp,P3D_ATTR_VERTEX,VertexIndex);

      for (unsigned int Axis = 0; Axis < 3; Axis++)
       {
        if      (Temp[Axis] < Min[Axis])
         {
          Min[Axis] = Temp[Axis];
         }
        else if (Temp[Axis] > Max[Axis])
         {
          Max[Axis] = Temp[Axis];
         }
       }
     }
   }
  else
   {
    Min[0] = Min[1] = Min[2] = 0.0f;
    Max[0] = Max[1] = Max[2] = 0.0f;
   }
 }

                   P3DBranchModel::P3DBranchModel
                                      ()
 {
  StemModel        = 0;
  BranchingAlg     = 0;
  MaterialInstance = 0;
  SubBranchCount   = 0;
  Name             = 0;
 }

                   P3DBranchModel::~P3DBranchModel
                                      ()
 {
  delete StemModel;
  delete BranchingAlg;
  delete MaterialInstance;

  free(Name);

  for (unsigned int Index = 0; Index < SubBranchCount; Index++)
   {
    delete SubBranches[Index];
   }
 }

const char        *P3DBranchModel::GetName
                                      () const
 {
  if (Name != 0)
   {
    return(Name);
   }
  else
   {
    return("NoName");
   }
 }

void               P3DBranchModel::SetName
                                      (const char         *Name)
 {
  if (this->Name != 0)
   {
    free(this->Name);

    this->Name = 0;
   }

  if (Name != 0)
   {
    this->Name = strdup(Name);

    if (this->Name == 0)
     {
      throw P3DExceptionGeneric("out of memory");
     }
   }
 }

P3DStemModel      *P3DBranchModel::GetStemModel
                                      ()
 {
  return(StemModel);
 }

const
P3DStemModel      *P3DBranchModel::GetStemModel
                                      () const
 {
  return(StemModel);
 }

void               P3DBranchModel::SetStemModel
                                      (P3DStemModel       *StemModel)
 {
  if (this->StemModel != 0)
   {
    delete this->StemModel;
   }

  this->StemModel = StemModel;
 }

P3DBranchingAlg   *P3DBranchModel::GetBranchingAlg
                                      ()
 {
  return(BranchingAlg);
 }

const
P3DBranchingAlg   *P3DBranchModel::GetBranchingAlg
                                      () const
 {
  return(BranchingAlg);
 }

void               P3DBranchModel::SetBranchingAlg
                                      (P3DBranchingAlg    *BranchingAlg)
 {
  if (this->BranchingAlg != 0)
   {
    delete this->BranchingAlg;
   }

  this->BranchingAlg = BranchingAlg;
 }

P3DMaterialInstance
                  *P3DBranchModel::GetMaterialInstance()
 {
  return(MaterialInstance);
 }

const P3DMaterialInstance
                  *P3DBranchModel::GetMaterialInstance() const
 {
  return(MaterialInstance);
 }

void               P3DBranchModel::SetMaterialInstance
                                      (P3DMaterialInstance*MaterialInstance)
 {
  if (this->MaterialInstance != 0)
   {
    delete this->MaterialInstance;
   }

  this->MaterialInstance = MaterialInstance;
 }

P3DVisRangeState  *P3DBranchModel::GetVisRangeState
                                      ()
 {
  return(&VisRangeState);
 }

const
P3DVisRangeState  *P3DBranchModel::GetVisRangeState
                                      () const
 {
  return(&VisRangeState);
 }

unsigned int       P3DBranchModel::GetSubBranchCount
                                      () const
 {
  return(SubBranchCount);
 }

P3DBranchModel    *P3DBranchModel::GetSubBranchModel
                                      (unsigned int        SubBranchIndex)
 {
  if (SubBranchIndex < SubBranchCount)
   {
    return(SubBranches[SubBranchIndex]);
   }
  else
   {
    return(0);
   }
 }

const
P3DBranchModel    *P3DBranchModel::GetSubBranchModel
                                      (unsigned int        SubBranchIndex) const
 {
  if (SubBranchIndex < SubBranchCount)
   {
    return(SubBranches[SubBranchIndex]);
   }
  else
   {
    return(0);
   }
 }

void               P3DBranchModel::AppendSubBranch
                                      (P3DBranchModel     *SubBranchModel)
 {
  if (SubBranchCount < P3DBranchModelSubBranchMaxCount)
   {
    SubBranches[SubBranchCount] = SubBranchModel;

    SubBranchCount++;
   }
  else
   {
    /*FIXME: throw something here */
   }
 }

void               P3DBranchModel::InsertSubBranch
                                      (P3DBranchModel     *SubBranchModel,
                                       unsigned int        SubBranchIndex)
 {
  if (SubBranchIndex >= SubBranchCount)
   {
    AppendSubBranch(SubBranchModel);
   }
  else
   {
    if (SubBranchCount < P3DBranchModelSubBranchMaxCount)
     {
      for (unsigned int Index = SubBranchCount; Index > SubBranchIndex; Index--)
       {
        SubBranches[Index] = SubBranches[Index - 1];
       }

      SubBranches[SubBranchIndex] = SubBranchModel;

      SubBranchCount++;
     }
    else
     {
      /*FIXME: throw something here */
     }
   }
 }

void               P3DBranchModel::RemoveSubBranch
                                      (unsigned int        SubBranchIndex)
 {
  if (SubBranchIndex < SubBranchCount)
   {
    delete SubBranches[SubBranchIndex];

    for (unsigned int Index = (SubBranchIndex + 1); Index < SubBranchCount; Index++)
     {
      SubBranches[Index - 1] = SubBranches[Index];
     }

    SubBranchCount--;
   }
  else
   {
    /*FIXME: throw something here */
   }
 }

P3DBranchModel    *P3DBranchModel::DetachSubBranch
                                      (unsigned int        SubBranchIndex)
 {
  if (SubBranchIndex < SubBranchCount)
   {
    P3DBranchModel *Result;

    Result = SubBranches[SubBranchIndex];

    for (unsigned int Index = (SubBranchIndex + 1); Index < SubBranchCount; Index++)
     {
      SubBranches[Index - 1] = SubBranches[Index];
     }

    SubBranchCount--;

    return(Result);
   }
  else
   {
    return(0);
   }
 }

void               P3DBranchModel::Save
                                      (P3DOutputStringStream
                                                          *TargetStream,
                                       P3DMaterialSaver   *MaterialSaver) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("ss","BranchGroupName",GetName());

  if (BranchingAlg != 0)
   {
    BranchingAlg->Save(TargetStream);
   }
  else
   {
    FmtStream.WriteString("ss","BranchingAlg","__None__");
   }

  if (StemModel != 0)
   {
    StemModel->Save(TargetStream);
   }
  else
   {
    FmtStream.WriteString("ss","StemModel","__None__");
   }

  if (MaterialInstance != 0)
   {
    MaterialSaver->Save(TargetStream,MaterialInstance);
   }
  else
   {
    FmtStream.WriteString("ss","Material","__None__");
   }

  VisRangeState.Save(TargetStream);

  FmtStream.WriteString("su","BranchModelCount",SubBranchCount);

  for (unsigned int SubBranchIndex = 0; SubBranchIndex < SubBranchCount; SubBranchIndex++)
   {
    SubBranches[SubBranchIndex]->Save(TargetStream,MaterialSaver);
   }
 }

void               P3DBranchModel::Load
                                      (P3DInputStringFmtStream
                                                          *SourceStream,
                                       P3DMaterialFactory *MaterialFactory,
                                       const P3DBranchModel
                                                          *ParentBranchModel,
                                       const P3DFileVersion
                                                          *Version)
 {
  char                                 NameBuffer[255 + 1];
  P3DStemModelQuad                    *StemModelQuad = 0;
  const P3DStemModelTube              *ParentStemModelTube;
  P3DBranchingAlg                     *TempBranchingAlg;

  if (ParentBranchModel == 0)
   {
    ParentStemModelTube = 0;
   }
  else
   {
    ParentStemModelTube = dynamic_cast<const P3DStemModelTube*>(ParentBranchModel->GetStemModel());
   }

  if ((Version->Major == 0) && (Version->Minor < 4))
   {
   }
  else
   {
    SourceStream->ReadFmtStringTagged("BranchGroupName","s",NameBuffer,sizeof(NameBuffer));

    SetName(NameBuffer);
   }

  SourceStream->ReadFmtStringTagged("BranchingAlg","s",NameBuffer,sizeof(NameBuffer));

  TempBranchingAlg = 0;

  try
   {
    if      (strcmp(NameBuffer,"__None__") == 0)
     {
     }
    else if (strcmp(NameBuffer,"Std") == 0)
     {
      TempBranchingAlg = new P3DBranchingAlgStd();
      TempBranchingAlg->Load(SourceStream,Version);
     }
    else if (strcmp(NameBuffer,"Wings") == 0)
     {
      if (ParentStemModelTube == 0)
       {
        throw P3DExceptionGeneric("'wings' model can be attached to 'tube' only");
       }

      TempBranchingAlg = new P3DBranchingAlgWings();
      TempBranchingAlg->Load(SourceStream,Version);
     }
    else if (strcmp(NameBuffer,"Base") == 0)
     {
      TempBranchingAlg = new P3DBranchingAlgBase();
      TempBranchingAlg->Load(SourceStream,Version);
     }
    else
     {
      throw P3DExceptionGeneric("unsupported branching algorithm");
     }
   }
  catch (...)
   {
    delete TempBranchingAlg;

    throw;
   }

  /* checking for ParentBranchModel == 0 to fix problems with 0.9.0 - 0.9.2 compatibility */

  if ((Version->Minor < 3) && (ParentBranchModel == 0))
   {
    delete TempBranchingAlg;
   }
  else
   {
    BranchingAlg = TempBranchingAlg;
   }

  SourceStream->ReadFmtStringTagged("StemModel","s",NameBuffer,sizeof(NameBuffer));

  if      (strcmp(NameBuffer,"__None__") == 0)
   {
   }
  else if (strcmp(NameBuffer,"Tube") == 0)
   {
    StemModel = new P3DStemModelTube();
    StemModel->Load(SourceStream,Version);
   }
  else if (strcmp(NameBuffer,"Quad") == 0)
   {
    StemModelQuad = new P3DStemModelQuad();
    StemModel     = StemModelQuad;
    StemModel->Load(SourceStream,Version);
   }
  else if (strcmp(NameBuffer,"Wings") == 0)
   {
    if (ParentStemModelTube == 0)
     {
      throw P3DExceptionGeneric("'wings' model can be attached to 'tube' only");
     }

    StemModel = new P3DStemModelWings(ParentStemModelTube);
    StemModel->Load(SourceStream,Version);
   }
  else if (strcmp(NameBuffer,"GMesh") == 0)
   {
    StemModel = new P3DStemModelGMesh();
    StemModel->Load(SourceStream,Version);
   }
  else
   {
    throw P3DExceptionGeneric("unsupported stem model");
   }

  SourceStream->ReadFmtStringTagged("Material","s",NameBuffer,sizeof(NameBuffer));

  if      (strcmp(NameBuffer,"__None__") == 0)
   {
   }
  else
   {
    P3DMaterialDef           MaterialDef;

    MaterialDef.Load(SourceStream,Version);

    if (StemModelQuad != 0)
     {
      StemModelQuad->SetBillboardMode(MaterialDef.GetBillboardMode());
     }
    else
     {
      /* In 0.9.4 and earlier versions it was possible to enable billboarding  */
      /* for "Tube" and "Wings" models. Next line forcely disable billboarding */
      /* for non-"Quad" models                                                 */
      MaterialDef.SetBillboardMode(P3D_BILLBOARD_MODE_NONE);
     }

    MaterialInstance = MaterialFactory->CreateMaterial(MaterialDef);

    if (MaterialInstance == 0)
     {
      throw P3DExceptionGeneric("unable to create material");
     }
   }

  VisRangeState.Load(SourceStream,Version);

  unsigned int TempSubBranchCount;

  SourceStream->ReadFmtStringTagged("BranchModelCount","u",&TempSubBranchCount);

  if (TempSubBranchCount > P3DBranchModelSubBranchMaxCount)
   {
    throw P3DExceptionGeneric("too many branch types");
   }

  SubBranchCount = 0;

  for (unsigned int SubBranchIndex = 0; SubBranchIndex < TempSubBranchCount; SubBranchIndex++)
   {
    SubBranches[SubBranchIndex] = new P3DBranchModel();

    SubBranches[SubBranchIndex]->Load(SourceStream,MaterialFactory,this,Version);

    SubBranchCount++;
   }
 }

                   P3DPlantModel::P3DPlantModel
                                      ()
 {
  PlantBase = new P3DBranchModel();
  BaseSeed  = 0;
  Flags     = 0;
 }

                   P3DPlantModel::~P3DPlantModel
                                      ()
 {
  delete PlantBase;
 }

P3DBranchModel    *P3DPlantModel::GetPlantBase
                                      ()
 {
  return(PlantBase);
 }

const
P3DBranchModel    *P3DPlantModel::GetPlantBase
                                      () const
 {
  return(PlantBase);
 }

unsigned int       P3DPlantModel::GetBaseSeed
                                      () const
 {
  return(BaseSeed);
 }

void               P3DPlantModel::SetBaseSeed
                                      (unsigned int        BaseSeed)
 {
  this->BaseSeed = BaseSeed;
 }

unsigned int       P3DPlantModel::GetFlags
                                      () const
 {
  return Flags;
 }

void               P3DPlantModel::SetFlags
                                      (unsigned int        Flags)
 {
  this->Flags = Flags;
 }

#define P3D_VERSION_MINOR (5)
#define P3D_VERSION_MAJOR (0)

void               P3DPlantModel::Save(P3DOutputStringStream
                                                          *TargetStream,
                                       P3DMaterialSaver   *MaterialSaver) const
 {
  P3DOutputStringFmtStream             FmtStream(TargetStream);

  FmtStream.WriteString("suu","P3D",(unsigned int)P3D_VERSION_MAJOR,(unsigned int)P3D_VERSION_MINOR);
  FmtStream.WriteString("su","BaseSeed",BaseSeed);

  PlantBase->Save(TargetStream,MaterialSaver);
 }

static void        AutoGenerateBGroupNames
                                      (P3DPlantModel      *PlantModel)
 {
  unsigned int                         BranchGroupIndex;
  P3DBranchModel                      *BranchModel;
  char                                 NameBuffer[128];

  PlantModel->GetPlantBase()->SetName("Plant");

  BranchGroupIndex = 0;
  BranchModel      = P3DPlantModel::GetBranchModelByIndex(PlantModel,BranchGroupIndex);

  while (BranchModel != 0)
   {
    snprintf(NameBuffer,sizeof(NameBuffer),"Branch%u",BranchGroupIndex + 1);

    NameBuffer[sizeof(NameBuffer) - 1] = 0;

    BranchModel->SetName(NameBuffer);

    BranchGroupIndex++;

    BranchModel = P3DPlantModel::GetBranchModelByIndex(PlantModel,BranchGroupIndex);
   }
 }

void               P3DPlantModel::Load(P3DInputStringStream
                                                          *SourceStream,
                                       P3DMaterialFactory *MaterialFactory)
 {
  P3DInputStringFmtStream                                  FmtStream(SourceStream);
  P3DFileVersion                                           Version;

  while (PlantBase->GetSubBranchCount() > 0)
   {
    PlantBase->RemoveSubBranch(PlantBase->GetSubBranchCount() - 1);
   }

  FmtStream.ReadFmtStringTagged("P3D","uu",&Version.Major,&Version.Minor);

  if ((Version.Major != P3D_VERSION_MAJOR) ||
      (Version.Minor  > P3D_VERSION_MINOR))
   {
    throw P3DExceptionGeneric("invalid file format version");
   }

  FmtStream.ReadFmtStringTagged("BaseSeed","u",&BaseSeed);

  /*FIXME: not fully correct - we must work with FmtStream */
  if (!SourceStream->Eof())
   {
    if (Version.Minor < 3)
     {
      P3DBranchModel        *TrunkBranchModel;

      TrunkBranchModel = new P3DBranchModel();
      TrunkBranchModel->SetBranchingAlg(new P3DBranchingAlgBase());

      PlantBase->AppendSubBranch(TrunkBranchModel);

      TrunkBranchModel->Load(&FmtStream,MaterialFactory,0,&Version);
     }
    else
     {
      PlantBase->Load(&FmtStream,MaterialFactory,0,&Version);
     }

    if ((Version.Major == 0) && (Version.Minor < 4))
     {
      AutoGenerateBGroupNames(this);
     }
   }
 }

void               P3DPlantModel::BranchModelSetUniqueName
                                      (P3DPlantModel      *PlantModel,
                                       P3DBranchModel     *BranchModel)
 {
  bool                                 Done;
  unsigned int                         Index;
  char                                 NameBuffer[128];

  Done  = false;
  Index = 1;

  while ((!Done) && (Index < 1000))
   {
    snprintf(NameBuffer,sizeof(NameBuffer),"Branch%u",Index);

    NameBuffer[sizeof(NameBuffer) - 1] = 0;

    if (P3DPlantModel::GetBranchModelByName(PlantModel,NameBuffer) == 0)
     {
      BranchModel->SetName(NameBuffer);

      Done = true;
     }

    Index++;
   }

  if (!Done)
   {
    BranchModel->SetName("Branch?");
   }
 }

static P3DBranchModel
                  *GetBranchModelByIndex
                                      (const P3DBranchModel
                                                          *Model,
                                       unsigned int       *Index)
 {
  unsigned int                         SubBranchIndex;
  unsigned int                         SubBranchCount;
  const P3DBranchModel                *Result;

  (*Index)--;

  if ((*Index) == 0)
   {
    return(const_cast<P3DBranchModel*>(Model));
   }

  Result         = 0;
  SubBranchIndex = 0;
  SubBranchCount = Model->GetSubBranchCount();

  while ((Result == 0) && (SubBranchIndex < SubBranchCount))
   {
    Result = GetBranchModelByIndex(Model->GetSubBranchModel(SubBranchIndex),
                                   Index);

    SubBranchIndex++;
   }

  return(const_cast<P3DBranchModel*>(Result));
 }

static P3DBranchModel
                  *GetBranchModelByName
                                      (P3DBranchModel     *Model,
                                       const char         *BranchName)
 {
  unsigned int                         SubBranchIndex;
  unsigned int                         SubBranchCount;
  P3DBranchModel                      *Result;

  if (strcmp(Model->GetName(),BranchName) == 0)
   {
    return(Model);
   }

  Result         = 0;
  SubBranchIndex = 0;
  SubBranchCount = Model->GetSubBranchCount();

  while ((Result == 0) && (SubBranchIndex < SubBranchCount))
   {
    Result = GetBranchModelByName(Model->GetSubBranchModel(SubBranchIndex),
                                  BranchName);

    SubBranchIndex++;
   }

  return(Result);
 }

P3DBranchModel    *P3DPlantModel::GetBranchModelByIndex
                                      (P3DPlantModel      *Model,
                                       unsigned int        Index)
 {
  unsigned int                         CurrIndex;

  CurrIndex = Index + 2; /* skip plant base group */

  return(::GetBranchModelByIndex(Model->GetPlantBase(),&CurrIndex));
 }

const P3DBranchModel
                  *P3DPlantModel::GetBranchModelByIndex
                                      (const P3DPlantModel*Model,
                                       unsigned int        Index)
 {
  unsigned int                         CurrIndex;

  CurrIndex = Index + 2; /* skip plant base group */

  return(::GetBranchModelByIndex(Model->GetPlantBase(),&CurrIndex));
 }

P3DBranchModel    *P3DPlantModel::GetBranchModelByName
                                      (P3DPlantModel      *Model,
                                       const char         *BranchName)
 {
  return(::GetBranchModelByName(Model->GetPlantBase(),BranchName));
 }


