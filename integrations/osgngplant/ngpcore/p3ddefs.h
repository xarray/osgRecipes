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

#ifndef __P3DDEFS_H__
#define __P3DDEFS_H__

#if defined(__GNUC__)
 #if defined(__cplusplus)
  #if defined(HAVE_GXX_ARG_ATTR_UNUSED)
   #define P3D_UNUSED_ATTR __attribute__((unused))
  #else
   #define P3D_UNUSED_ATTR
  #endif
 #else
  #define P3D_UNUSED_ATTR __attribute__((unused))
 #endif
#else
 #define P3D_UNUSED_ATTR
#endif

#if defined(_WIN32) && defined(_MSC_VER)
 #if defined(P3D_DLL_BUILD)
  #define P3D_DLL_ENTRY __declspec(dllexport)
 #elif defined(P3D_DLL_USE)
  #define P3D_DLL_ENTRY __declspec(dllimport)
 #else
  #define P3D_DLL_ENTRY
 #endif
#else
 #define P3D_DLL_ENTRY
#endif

#define P3D_BYTE           (0)
#define P3D_FLOAT          (1)
#define P3D_UNSIGNED_SHORT (2)
#define P3D_UNSIGNED_INT   (3)

#define P3D_TRIANGLE        (0)
#define P3D_TRIANGLE_LIST   P3D_TRIANGLE
#define P3D_TRIANGLE_STRIP  (1)
#define P3D_QUAD            (2)

#define P3D_BILLBOARD_MODE_NONE        (0)
#define P3D_BILLBOARD_MODE_SPHERICAL   (1)
#define P3D_BILLBOARD_MODE_CYLINDRICAL (2)

#define P3D_ATTR_VERTEX          (0)
#define P3D_ATTR_NORMAL          (1)
#define P3D_ATTR_TEXCOORD0       (2)
#define P3D_ATTR_TANGENT         (3)
#define P3D_ATTR_BINORMAL        (4)
#define P3D_ATTR_BILLBOARD_POS   (5)

#define P3D_MAX_ATTRS            (P3D_ATTR_BILLBOARD_POS + 1)

#define P3D_TEX_DIFFUSE          (0)
#define P3D_TEX_NORMAL_MAP       (1)
#define P3D_TEX_AUX0             (2)
#define P3D_TEX_AUX1             (3)

#define P3D_MAX_TEX_LAYERS       (P3D_TEX_AUX1 + 1)

#endif

