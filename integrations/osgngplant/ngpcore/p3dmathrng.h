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

#ifndef __P3DMATHRNG_H__
#define __P3DMATHRNG_H__

class P3DMathRNG
 {
  public           :

  virtual         ~P3DMathRNG         () {};

  virtual void     SetSeed            (unsigned int        Seed) = 0;

  virtual
  int              RandomInt          (int                 Min,
                                       int                 Max) = 0;

  virtual float    UniformFloat       (float               Min,
                                       float               Max) = 0;
 };

/*FIXME: find more definite name for it*/
class P3DMathRNGSimple : public P3DMathRNG
 {
  public           :

                   P3DMathRNGSimple   (unsigned int        Seed);

  virtual void     SetSeed            (unsigned int        Seed);

  virtual int      RandomInt          (int                 Min,
                                       int                 Max);

  virtual float    UniformFloat       (float               Min,
                                       float               Max);

  private          :

  unsigned int     Rand               ();

  unsigned int     Seed;
 };

#endif

