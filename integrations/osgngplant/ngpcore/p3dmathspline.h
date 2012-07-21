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

#ifndef __P3DMATHSPLINE_H__
#define __P3DMATHSPLINE_H__

#define P3DMATH_NATURAL_CUBIC_SPLINE_CP_MAX_COUNT (32)

class P3DMathNaturalCubicSpline
 {
  public           :

                   P3DMathNaturalCubicSpline    ();
                   P3DMathNaturalCubicSpline    (const P3DMathNaturalCubicSpline
                                                                    &src);

  void             operator =                   (const P3DMathNaturalCubicSpline
                                                                    &src);

  unsigned int     GetCPCount                   () const;
  float            GetCPX                       (unsigned int        cp) const;
  float            GetCPY                       (unsigned int        cp) const;

  float            GetValue                     (float               x) const;
  float            GetTangent                   (float               x) const;

  void             AddCP                        (float               x,
                                                 float               y);

  void             UpdateCP                     (float               x,
                                                 float               y,
                                                 unsigned int        cp);

  void             DelCP                        (unsigned int        cp);

  void             SetConstant                  (float               value);
  void             SetLinear                    (float               ax,
                                                 float               ay,
                                                 float               bx,
                                                 float               by);

  void             CopyFrom                     (const P3DMathNaturalCubicSpline
                                                                    &src);

  private          :

  void             RecalcY2                     ();


  unsigned int     cp_count;
  float            cp_x[P3DMATH_NATURAL_CUBIC_SPLINE_CP_MAX_COUNT];
  float            cp_y[P3DMATH_NATURAL_CUBIC_SPLINE_CP_MAX_COUNT];
  float            cp_y2[P3DMATH_NATURAL_CUBIC_SPLINE_CP_MAX_COUNT];
 };

#endif

