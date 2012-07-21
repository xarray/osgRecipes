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

#include <string.h>

#include <ngpcore/p3dgmeshdata.h>

                   P3DGMeshData::P3DGMeshData
                                      (const unsigned int *VAttrCount,
                                       unsigned int        PrimitiveCount,
                                       unsigned int        IndexCount,
                                       unsigned int        VAttrCountI,
                                       unsigned int        IndexCountI)
 {
  unsigned int     Index;

  for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
   {
    VAttrValues[Index]           = 0;
    VAttrValueCounts[Index]      = 0;
    VAttrValueIndices[Index]     = 0;
    VAttrBuffersI[Index]         = 0;
   }

  VAttrValueIndexCount = 0;
  PrimitiveTypes = 0;
  IndexBufferI   = 0;

  try
   {
    for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
     {
      if (Index == P3D_ATTR_TEXCOORD0)
       {
        VAttrValues[Index] = new float[2 * VAttrCount[Index]];
       }
      else
       {
        VAttrValues[Index] = new float[3 * VAttrCount[Index]];
       }

      VAttrValueCounts[Index]      = VAttrCount[Index];
      VAttrValueIndices[Index]     = new unsigned int[IndexCount];
     }

    VAttrValueIndexCount = IndexCount;

    PrimitiveTypes = new unsigned int[PrimitiveCount];

    this->PrimitiveCount = PrimitiveCount;

    for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
     {
      if (Index == P3D_ATTR_TEXCOORD0)
       {
        VAttrBuffersI[Index] = new float[2 * VAttrCountI];
       }
      else
       {
        VAttrBuffersI[Index] = new float[3 * VAttrCountI];
       }
     }

    this->VAttrCountI = VAttrCountI;

    IndexBufferI = new unsigned int[IndexCountI];

    this->IndexCountI = IndexCountI;
   }
  catch (...)
   {
    for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
     {
      delete VAttrValues[Index];
      delete VAttrValueIndices[Index];
      delete VAttrBuffersI[Index];
     }

    delete PrimitiveTypes;
    delete IndexBufferI;

    throw;
   }
 }

                   P3DGMeshData::~P3DGMeshData
                                      ()
 {
  unsigned int     Index;

  for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
   {
    delete VAttrValues[Index];
    delete VAttrValueIndices[Index];
    delete VAttrBuffersI[Index];
   }

  delete PrimitiveTypes;
  delete IndexBufferI;
 }

unsigned int       P3DGMeshData::GetVAttrCount
                                      (unsigned int        Attr) const
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrValueCounts[Attr]);
   }
  else
   {
    return(0);
   }
 }

float             *P3DGMeshData::GetVAttrBuffer
                                      (unsigned int        Attr)
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrValues[Attr]);
   }
  else
   {
    return(0);
   }
 }

const float       *P3DGMeshData::GetVAttrBuffer
                                      (unsigned int        Attr) const
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrValues[Attr]);
   }
  else
   {
    return(0);
   }
 }

unsigned int       P3DGMeshData::GetPrimitiveCount
                                      () const
 {
  return(PrimitiveCount);
 }

unsigned int      *P3DGMeshData::GetPrimitiveBuffer
                                      ()
 {
  return(PrimitiveTypes);
 }

const
unsigned int      *P3DGMeshData::GetPrimitiveBuffer
                                      () const
 {
  return(PrimitiveTypes);
 }


unsigned int       P3DGMeshData::GetIndexCount
                                      () const
 {
  return(VAttrValueIndexCount);
 }

unsigned int      *P3DGMeshData::GetIndexBuffer
                                      (unsigned int        Attr) const
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrValueIndices[Attr]);
   }
  else
   {
    return(0);
   }
 }

unsigned int       P3DGMeshData::GetVAttrCountI
                                      () const
 {
  return(VAttrCountI);
 }

float             *P3DGMeshData::GetVAttrBufferI
                                      (unsigned int        Attr)
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrBuffersI[Attr]);
   }
  else
   {
    return(0);
   }
 }

const float       *P3DGMeshData::GetVAttrBufferI
                                      (unsigned int        Attr) const
 {
  if (Attr < P3D_GMESH_MAX_ATTRS)
   {
    return(VAttrBuffersI[Attr]);
   }
  else
   {
    return(0);
   }
 }


unsigned int       P3DGMeshData::GetIndexCountI
                                      () const
 {
  return(IndexCountI);
 }

unsigned int      *P3DGMeshData::GetIndexBufferI
                                      () const
 {
  return(IndexBufferI);
 }

P3DGMeshData      *P3DGMeshData::CreateCopy
                                      () const
 {
  P3DGMeshData    *Result;
  unsigned int     Index;

  Result = new P3DGMeshData(VAttrValueCounts,
                            PrimitiveCount,
                            VAttrValueIndexCount,
                            VAttrCountI,
                            IndexCountI);

  for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
   {
    memcpy(Result->VAttrValues[Index],
           VAttrValues[Index],
           sizeof(float) * (Index == P3D_ATTR_TEXCOORD0 ? 2 : 3) * VAttrValueCounts[Index]);

   }

  for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
   {
    memcpy(Result->VAttrValueIndices[Index],
           VAttrValueIndices[Index],
           sizeof(unsigned int) * VAttrValueIndexCount);
   }

  memcpy(Result->PrimitiveTypes,PrimitiveTypes,sizeof(unsigned int) * PrimitiveCount);

  for (Index = 0; Index < P3D_GMESH_MAX_ATTRS; Index++)
   {
    memcpy(Result->VAttrBuffersI[Index],
           VAttrBuffersI[Index],
           sizeof(float) * (Index == P3D_ATTR_TEXCOORD0 ? 2 : 3) * VAttrCountI);
   }

  memcpy(Result->IndexBufferI,IndexBufferI,sizeof(unsigned int) * IndexCountI);

  return(Result);
 }

