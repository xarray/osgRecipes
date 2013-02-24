/*************************************************************************\

  Copyright 2001 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify OR distribute this software and its
  documentation for educational, research and non-profit purposes, without
  fee, and without a written agreement is hereby granted, provided that the
  above copyright notice and the following three paragraphs appear in all
  copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The authors may be contacted via:

  US Mail:             S. Ehmann, M. Lin
                       Department of Computer Science
                       Sitterson Hall, CB #3175
                       University of N. Carolina
                       Chapel Hill, NC 27599-3175

  Phone:               (919) 962-1749

  EMail:               geom@cs.unc.edu
                       ehmann@cs.unc.edu
                       lin@cs.unc.edu

\**************************************************************************/


//////////////////////////////////////////////////////////////////////////////
//
// SWIFT_common.h
//
// Description:
//      Common stuff.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_COMMON_H_
#define _SWIFT_COMMON_H_

///////////////////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////////////////

// If bool is not defined by your compiler uncomment this
//typedef char bool;

#ifndef _SWIFT_H_
typedef enum { MEDIAN, MIDPOINT, MEAN, GAP } SPLIT_TYPE;
#endif

typedef enum { DISJOINT = -3, PENETRATION = -2, LOCAL_MINIMUM = -1,
               CONTINUE_VV = 0, CONTINUE_VE, CONTINUE_EV,
               CONTINUE_VF, CONTINUE_FV, CONTINUE_EE, CONTINUE_EF, CONTINUE_FE
             } RESULT_TYPE;

///////////////////////////////////////////////////////////////////////////////
// Numerical constants
///////////////////////////////////////////////////////////////////////////////

#ifdef SWIFT_USE_FLOAT
typedef float SWIFT_Real;
#else
typedef double SWIFT_Real;
#endif

# ifndef PI
static const SWIFT_Real PI        = 3.1415926535897932384626433832795029;
# endif

# ifndef PI_2
static const SWIFT_Real PI_2      = 1.5707963267948966192313216916397514;
# endif

# ifndef SQRT2
static const SWIFT_Real SQRT2     = 1.4142135623730950488016887242096981;
# endif

# ifndef SQRT1_2
static const SWIFT_Real SQRT1_2   = 0.7071067811865475244008443621048490;
# endif

#ifdef SWIFT_USE_FLOAT
static const SWIFT_Real SWIFT_INFINITY  = 3.4028234663852885981e+38;
#else
static const SWIFT_Real SWIFT_INFINITY  = 1.7976931348623157081e+308;
#endif

static const SWIFT_Real EPSILON15 = 1.0e-15;
static const SWIFT_Real EPSILON14 = 1.0e-14;
static const SWIFT_Real EPSILON13 = 1.0e-13;
static const SWIFT_Real EPSILON12 = 1.0e-12;
static const SWIFT_Real EPSILON11 = 1.0e-11;
static const SWIFT_Real EPSILON10 = 1.0e-10;
static const SWIFT_Real EPSILON9 = 1.0e-9;
static const SWIFT_Real EPSILON8 = 1.0e-8;
static const SWIFT_Real EPSILON7 = 1.0e-7;
static const SWIFT_Real EPSILON6 = 1.0e-6;
static const SWIFT_Real EPSILON5 = 1.0e-5;
static const SWIFT_Real EPSILON4 = 1.0e-4;
static const SWIFT_Real EPSILON3 = 1.0e-3;
static const SWIFT_Real EPSILON2 = 1.0e-2;
static const SWIFT_Real EPSILON1 = 1.0e-1;

// Standard min/max macros
# ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
# endif

# ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
# endif

// Special min and max function which accumulates min and max
inline void Min_And_Max( SWIFT_Real &mn, SWIFT_Real &mx, SWIFT_Real v )
{
    if( v < mn ) mn = v;
    else if( v > mx ) mx = v;
}


#define myfabs(x) ((x < 0) ? -x : x)


#ifdef WIN32
#include <stdlib.h>
// random numbers for windows
inline SWIFT_Real drand48( )
{
    return ((SWIFT_Real)rand()) / (RAND_MAX+1.0);
}
#endif

#endif


