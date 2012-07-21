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

#ifndef __P3DGMESHDATA_H__
#define __P3DGMESHDATA_H__

#include <ngpcore/p3ddefs.h>

#define P3D_GMESH_MAX_ATTRS   (P3D_MAX_ATTRS - 1) // do not take into account P3D_ATTR_BILLBOARD_POS

class P3DGMeshData
 {
  public           :

                   P3DGMeshData       (const unsigned int *VAttrCount,
                                       unsigned int        PrimitiveCount,
                                       unsigned int        IndexCount,
                                       unsigned int        VAttrCountI,
                                       unsigned int        IndexCountI);

                  ~P3DGMeshData       ();

  unsigned int     GetVAttrCount      (unsigned int        Attr) const;
  float           *GetVAttrBuffer     (unsigned int        Attr);
  const float     *GetVAttrBuffer     (unsigned int        Attr) const;
  unsigned int     GetPrimitiveCount  () const;
  unsigned int    *GetPrimitiveBuffer ();
  const
  unsigned int    *GetPrimitiveBuffer () const;
  unsigned int     GetIndexCount      () const;
  unsigned int    *GetIndexBuffer     (unsigned int        Attr) const;

  unsigned int     GetVAttrCountI     () const;
  float           *GetVAttrBufferI    (unsigned int        Attr);
  const float     *GetVAttrBufferI    (unsigned int        Attr) const;
  unsigned int     GetIndexCountI     () const;
  unsigned int    *GetIndexBufferI    () const;

  P3DGMeshData    *CreateCopy         () const;

  private          :

  float           *VAttrValues[P3D_GMESH_MAX_ATTRS];
  unsigned int     VAttrValueCounts[P3D_GMESH_MAX_ATTRS];
  unsigned int    *VAttrValueIndices[P3D_GMESH_MAX_ATTRS];
  unsigned int     VAttrValueIndexCount;
  unsigned int     PrimitiveCount;
  unsigned int    *PrimitiveTypes;

  unsigned int     VAttrCountI;
  float           *VAttrBuffersI[P3D_GMESH_MAX_ATTRS];
  unsigned int     IndexCountI;
  unsigned int    *IndexBufferI;
 };

#endif

