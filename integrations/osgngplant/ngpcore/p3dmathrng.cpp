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

#include <ngpcore/p3dmathrng.h>

/* Algorithm and constants are taken from "Numerical recipes in C" ch.7 p.284 */

#define P3DMathRNGSimpleMax (0xFFFFFFFFU)

                   P3DMathRNGSimple::P3DMathRNGSimple
                                      (unsigned int        Seed)
 {
  this->Seed = Seed;

  Rand();
 }

void               P3DMathRNGSimple::SetSeed
                                      (unsigned int        Seed)
 {
  this->Seed = Seed;

  Rand();
 }

int                P3DMathRNGSimple::RandomInt
                                      (int                 Min,
                                       int                 Max)
 {
  return(Min + int((Max - Min + 1.0) * Rand() / (P3DMathRNGSimpleMax + 1.0)));
 }

float              P3DMathRNGSimple::UniformFloat
                                      (float               Min,
                                       float               Max)
 {
  return(Min + Rand() / (P3DMathRNGSimpleMax + 1.0) * (Max - Min));
 }

unsigned int       P3DMathRNGSimple::Rand
                                      ()
 {
  Seed = (1664525U * Seed + 1013904223U) & 0xFFFFFFFFU;

  return(Seed);
 }

