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

#ifndef __P3DIOSTREAM_H__
#define __P3DIOSTREAM_H__

#include <stdio.h>

#include <ngpcore/p3ddefs.h>
#include <ngpcore/p3dexcept.h>

class P3DExceptionIO : public P3DException
 {
  public           :

  virtual
  const char      *GetMessage         () const;
 };

class P3DInputStringStream
 {
  public           :

  virtual         ~P3DInputStringStream
                                      () {};

  virtual
  void             ReadString         (char               *Buffer,
                                       unsigned int        BufferSize) = 0;

  virtual bool     Eof                () const = 0;
 };

class P3D_DLL_ENTRY P3DInputStringStreamFile : public P3DInputStringStream
 {
  public           :

                   P3DInputStringStreamFile
                                      ();

  virtual         ~P3DInputStringStreamFile
                                      ();

  void             Open               (const char         *FileName);
  void             Close              ();

  virtual
  void             ReadString         (char               *Buffer,
                                       unsigned int        BufferSize);

  virtual bool     Eof                () const;

  private          :

  FILE            *Source;
 };

class P3DInputStringFmtStream
 {
  public           :

                   P3DInputStringFmtStream
                                      (P3DInputStringStream
                                                          *SourceStream);

  void             ReadFmtStringTagged(const char         *Tag,
                                       const char         *Format,
                                       ...);

  private          :

  void             ReadDataString     (char               *Buffer,
                                       unsigned int        BufferSize);

  P3DInputStringStream                *SourceStream;
 };

class P3DOutputStringStream
 {
  public           :

  virtual         ~P3DOutputStringStream
                                      () {};

  virtual void     WriteString        (const char         *Buffer) = 0;
  virtual void     AutoLnEnable       () = 0;
  virtual void     AutoLnDisable      () = 0;
 };

class P3DOutputStringFmtStream
 {
  public           :

                   P3DOutputStringFmtStream
                                      (P3DOutputStringStream
                                                          *Target);
  virtual         ~P3DOutputStringFmtStream
                                      () {};

  virtual void     WriteString        (const char         *Buffer);

  /*NOTE: Format must be a sequence of one or more characters, which */
  /*      describes type of element                                  */
  /*      Supported types:                                           */
  /*      s - string                                                 */
  /*      u - unsigned int                                           */
  /*      f - float                                                  */
  /*      b - bool                                                   */

  void             WriteString        (const char         *Format,
                                       ...);

  private          :

  P3DOutputStringStream               *Target;
 };

class P3DOutputStringStreamFile : public P3DOutputStringStream
 {
  public           :

                   P3DOutputStringStreamFile
                                      ();
  virtual         ~P3DOutputStringStreamFile
                                      ();

  void             Open               (const char         *FileName);
  void             Close              ();

  virtual void     WriteString        (const char         *Buffer);
  virtual void     AutoLnEnable       ();
  virtual void     AutoLnDisable      ();

  private          :

  FILE            *Target;
  bool             AutoLn;
 };

#endif

