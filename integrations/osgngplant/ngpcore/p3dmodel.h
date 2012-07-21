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

#ifndef __P3DMODEL_H__
#define __P3DMODEL_H__

#include <ngpcore/p3ddefs.h>

#include <ngpcore/p3dmath.h>
#include <ngpcore/p3dmathspline.h>
#include <ngpcore/p3dmathrng.h>

#include <ngpcore/p3dplant.h>

#include <ngpcore/p3diostream.h>

typedef struct
 {
  unsigned int     Major;
  unsigned int     Minor;
 } P3DFileVersion;

typedef struct
 {
  float            Min;
  float            Max;
 } P3DVisRange;

class P3DVisRangeState
 {
  public           :

                   P3DVisRangeState   ();
                   P3DVisRangeState   (float               Min,
                                       float               Max);

  bool             IsEnabled          () const;
  void             SetState           (bool                Enable);

  void             GetRange           (float              *Min,
                                       float              *Max) const;

  void             SetRange           (float               Min,
                                       float               Max);

  void             Save               (P3DOutputStringStream
                                                          *TargetStream) const;

  void             Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version);

  private          :

  bool             Enabled;
  P3DVisRange      Range;
 };

class P3D_DLL_ENTRY P3DMaterialDef
 {
  public           :

                   P3DMaterialDef     ();
                   P3DMaterialDef     (const P3DMaterialDef
                                                          &Source);
                  ~P3DMaterialDef     ();

  void             operator =         (const P3DMaterialDef
                                                          &Source);

  void             Save               (P3DOutputStringStream
                                                          *TargetStream) const;

  void             Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version);

  void             GetColor           (float              *R,
                                       float              *G,
                                       float              *B) const;

  void             SetColor           (float               R,
                                       float               G,
                                       float               B);

  const char      *GetTexName         (unsigned int        Layer) const;
  void             SetTexName         (unsigned int        Layer,
                                       const char         *TexName);

  bool             IsDoubleSided      () const;
  void             SetDoubleSided     (bool                DoubleSided);

  bool             IsTransparent      () const;
  void             SetTransparent     (bool                Transparent);

  bool             IsBillboard        () const;
  unsigned int     GetBillboardMode   () const;
  void             SetBillboardMode   (unsigned int        Mode);

  bool             IsAlphaCtrlEnabled () const;
  void             SetAlphaCtrlState  (bool                Enable);

  float            GetAlphaFadeIn     () const;
  float            GetAlphaFadeOut    () const;

  void             SetAlphaFadeIn     (float               FadeIn);
  void             SetAlphaFadeOut    (float               FadeOut);

  void             CopyFrom           (const P3DMaterialDef
                                                          *Source);

  private          :

  float            R,G,B;
  char            *TexNames[P3D_MAX_TEX_LAYERS];
  bool             DoubleSided;
  bool             Transparent;
  unsigned int     BillboardMode;

  bool             AlphaCtrlEnabled;
  float            AlphaFadeIn;
  float            AlphaFadeOut;
 };

class P3DMaterialInstance
 {
  public           :

  virtual         ~P3DMaterialInstance() {};

  virtual
  const
  P3DMaterialDef  *GetMaterialDef     () const = 0;

  virtual
  P3DMaterialInstance
                  *CreateCopy         () const = 0;
 };

class P3DMaterialFactory
 {
  public           :

  virtual         ~P3DMaterialFactory () {};

  virtual P3DMaterialInstance
                  *CreateMaterial     (const P3DMaterialDef
                                                          &MaterialDef) const = 0;
 };

class P3DMaterialSaver
 {
  public           :

  virtual         ~P3DMaterialSaver   () {};

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream,
                                       const P3DMaterialInstance
                                                          *Material) const = 0;
 };

class P3DStemModelInstance
 {
  public           :

  virtual         ~P3DStemModelInstance    () {};

  /* Per-attribute information */

  virtual
  unsigned int     GetVAttrCount      (unsigned int        Attr) const = 0;
  /* Value must a pointer to attribute buffer. Size of buffer: */
  /* 2 - for texture coordinates                               */
  /* 3 - for vertex and normal                                 */
  virtual void     GetVAttrValue      (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const = 0;

  virtual
  unsigned int     GetPrimitiveCount  () const = 0;
  virtual
  unsigned int     GetPrimitiveType   (unsigned int        PrimitiveIndex) const = 0;

  /* Per-index information */

  virtual
  unsigned int     GetVAttrCountI     () const = 0;

  virtual void     GetVAttrValueI     (float              *Value,
                                       unsigned int        Attr,
                                       unsigned int        Index) const = 0;

  /* Bound-box information */

  /* generic implementation - do not take into account billboard mode, */
  /* so must be overrided for Quad, but can be used for Tube and Wings */
  virtual void     GetBoundBox        (float              *Min,
                                       float              *Max) const;

  virtual float    GetLength          () const = 0;
  virtual float    GetMinRadiusAt     (float               Offset) const = 0;

  /* fill Transform with 4x4 Stem To World transformation matrix */
  virtual void     GetWorldTransform  (float              *Transform) const = 0;
  /* get axis point coordinates (x,y,z - vector) in stem coordinate space */
  /* Offset must be in range [0.0 .. 1.0 ]                                */
  virtual void     GetAxisPointAt     (float              *Pos,
                                       float               Offset) const = 0;
  /* get axis orientation (quaternion) in stem coordinate space */
  /* Offset must be in range [0.0 .. 1.0 ]                      */
  virtual void     GetAxisOrientationAt
                                      (float              *Orientation,
                                       float               Offset) const = 0;
 };

class P3DStemModel
 {
  public           :

  virtual         ~P3DStemModel       () {};

  virtual P3DStemModelInstance
                  *CreateInstance     (P3DMathRNG         *rng,
                                       const P3DStemModelInstance
                                                          *parent,
                                       float               offset,
                                       const P3DQuaternionf
                                                          *orientation) const = 0;

  virtual void     ReleaseInstance    (P3DStemModelInstance
                                                          *instance) const = 0;

  virtual P3DStemModel
                  *CreateCopy         () const = 0;

  /* Per-attribute information */

  virtual
  unsigned int     GetVAttrCount      (unsigned int        Attr) const = 0;

  virtual
  unsigned int     GetPrimitiveCount  () const = 0;
  virtual
  unsigned int     GetPrimitiveType   (unsigned int        PrimitiveIndex) const = 0;

  virtual void     FillVAttrIndexBuffer
                                      (void               *IndexBuffer,
                                       unsigned int        Attr,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase = 0) const = 0;

  /* Per-index information */

  virtual
  unsigned int     GetVAttrCountI     () const = 0;

  virtual
  unsigned int     GetIndexCount      (unsigned int        PrimitiveType) const = 0;

  virtual void     FillIndexBuffer    (void               *IndexBuffer,
                                       unsigned int        PrimitiveType,
                                       unsigned int        ElementType,
                                       unsigned int        IndexBase = 0) const = 0;

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream) const = 0;

  virtual void     Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version) = 0;
 };

class P3DBranchingFactory
 {
  public           :

  virtual         ~P3DBranchingFactory() {};

  virtual void     GenerateBranch     (float               offset,
                                       const P3DQuaternionf
                                                          *orientation) = 0;
 };

class P3DBranchingAlg
 {
  public           :

  virtual         ~P3DBranchingAlg    () {};

  virtual void     CreateBranches     (P3DBranchingFactory          *factory,
                                       const P3DStemModelInstance   *parent,
                                       P3DMathRNG                   *rng) = 0;

  virtual P3DBranchingAlg
                  *CreateCopy         () const = 0;

  virtual void     Save               (P3DOutputStringStream
                                                          *TargetStream) const = 0;

  virtual void     Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       const P3DFileVersion
                                                          *Version) = 0;
 };

#define P3DBranchModelSubBranchMaxCount (8)

class P3DBranchModel
 {
  public           :

                   P3DBranchModel     ();
                  ~P3DBranchModel     ();

  const char      *GetName            () const;
  void             SetName            (const char         *Name);

  P3DStemModel    *GetStemModel       ();
  const
  P3DStemModel    *GetStemModel       () const;
  void             SetStemModel       (P3DStemModel       *StemModel);

  P3DBranchingAlg *GetBranchingAlg    ();
  const
  P3DBranchingAlg *GetBranchingAlg    () const;
  void             SetBranchingAlg    (P3DBranchingAlg    *BranchingAlg);

  P3DMaterialInstance
                  *GetMaterialInstance();
  const P3DMaterialInstance
                  *GetMaterialInstance() const;
  void             SetMaterialInstance(P3DMaterialInstance*MaterialInstance);

  P3DVisRangeState*GetVisRangeState   ();
  const
  P3DVisRangeState*GetVisRangeState   () const;

  unsigned int     GetSubBranchCount  () const;
  const
  P3DBranchModel  *GetSubBranchModel  (unsigned int        SubBranchIndex) const;
  P3DBranchModel  *GetSubBranchModel  (unsigned int        SubBranchIndex);

  void             AppendSubBranch    (P3DBranchModel     *SubBranchModel);
  void             InsertSubBranch    (P3DBranchModel     *SubBranchModel,
                                       unsigned int        SubBranchIndex);
  void             RemoveSubBranch    (unsigned int        SubBranchIndex);
  P3DBranchModel  *DetachSubBranch    (unsigned int        SubBranchIndex);

  void             Save               (P3DOutputStringStream
                                                          *TargetStream,
                                       P3DMaterialSaver   *MaterialSaver) const;

  void             Load               (P3DInputStringFmtStream
                                                          *SourceStream,
                                       P3DMaterialFactory *MaterialFactory,
                                       const P3DBranchModel
                                                          *ParentBranchModel,
                                       const P3DFileVersion
                                                          *Version);

  private          :

  char                                *Name;
  P3DStemModel                        *StemModel;
  P3DBranchingAlg                     *BranchingAlg;
  P3DMaterialInstance                 *MaterialInstance;
  P3DVisRangeState                     VisRangeState;
  P3DBranchModel                      *SubBranches[P3DBranchModelSubBranchMaxCount];
  unsigned int                         SubBranchCount;
 };

#define P3D_MODEL_FLAG_NO_RANDOMNESS (0x1)

class P3D_DLL_ENTRY P3DPlantModel
 {
  public           :

                   P3DPlantModel      ();
                  ~P3DPlantModel      ();

  P3DBranchModel  *GetPlantBase       ();
  const
  P3DBranchModel  *GetPlantBase       () const;

  unsigned int     GetBaseSeed        () const;
  void             SetBaseSeed        (unsigned int        BaseSeed);

  unsigned int     GetFlags           () const;
  void             SetFlags           (unsigned int        Flags);

  void             Save               (P3DOutputStringStream
                                                          *TargetStream,
                                       P3DMaterialSaver   *MaterialSaver) const;

  void             Load               (P3DInputStringStream
                                                          *SourceStream,
                                       P3DMaterialFactory *MaterialFactory);

  static const P3DBranchModel
                  *GetBranchModelByIndex
                                      (const P3DPlantModel*Model,
                                       unsigned int        Index);

  static P3DBranchModel
                  *GetBranchModelByIndex
                                      (P3DPlantModel      *Model,
                                       unsigned int        Index);

  static P3DBranchModel
                  *GetBranchModelByName
                                      (P3DPlantModel      *Model,
                                       const char         *BranchName);

  static void      BranchModelSetUniqueName
                                      (P3DPlantModel      *PlantModel,
                                       P3DBranchModel     *BranchModel);

  private          :

  P3DBranchModel                      *PlantBase;
  unsigned int                         BaseSeed;
  unsigned int                         Flags;
 };

/*FIXME: not best header for next prototype (implemented in p3dmodelstemtube.cpp) */

extern void        P3DSaveSplineCurve (P3DOutputStringFmtStream
                                                          *FmtStream,
                                       const P3DMathNaturalCubicSpline
                                                          *Spline);


extern void        P3DLoadSplineCurve (P3DMathNaturalCubicSpline
                                                          *Spline,
                                       P3DInputStringFmtStream
                                                          *SourceStream,
                                       const char         *CurveName);

#endif

