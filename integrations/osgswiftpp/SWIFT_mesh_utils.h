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
// SWIFT_mesh_utils.h
//
// Description:
//      Utility functions for mesh usage
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_MESH_UTILS_H_
#define _SWIFT_MESH_UTILS_H_

#include <math.h>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>

extern "C" {
#include <qhull/qhull_a.h>
}

// Call this before calling any other mesh utils functions
void Mesh_Utils_Initialize();

// Compute the minimum and maximum spread directions and values.  Input is an
// array of vertices, an array of vertex indices (3 per face), the number of
// faces and a center of mass.
void Compute_Spread( SWIFT_Array<SWIFT_Tri_Vertex>& vs, int* fs, int fn,
                     bool have_com,
                     const SWIFT_Triple& com, SWIFT_Triple& min_dir,
                     SWIFT_Triple& mid_dir, SWIFT_Triple& max_dir );


void Compute_Convex_Hull( coordT* vs, int vn, int*& fs, int& fn );
void Compute_Convex_Hull( SWIFT_Array<SWIFT_Tri_Vertex*>& vs,
                          int*& fs, int& fn );
void Compute_Convex_Hull( SWIFT_Array<SWIFT_Tri_Vertex>& vs,
                          int*& fs, int& fn );
void Compute_Convex_Hull( SWIFT_Array<SWIFT_Triple>& vs,
                          SWIFT_Array<SWIFT_Tri_Vertex>& cs,
                          int*& fs, int& fn );

void Compute_Min_And_Max_Spread( SWIFT_Tri_Mesh* m, SWIFT_BV* p,
                                 SWIFT_Real& min_spread,
                                 SWIFT_Real& max_spread );

// Given a point list computes the spread of them based on their cvx hull
// If compute_spreads is false, the center of mass of the convex hull is
// computed and returned in center and the spreads are not set, otherwise the
// spreads are computed and the center is set to the average of the spreads --
// Note that this is the same as the the center in an LCS where the dirs are
// the columns of R in LCS->WCS.
void Compute_Spread( SWIFT_Array<SWIFT_Tri_Vertex>& vs, SWIFT_Triple& center,
                     bool compute_spreads,
                     SWIFT_Triple& min_dir, SWIFT_Real& min_spread,
                     SWIFT_Triple& mid_dir, SWIFT_Real& mid_spread,
                     SWIFT_Triple& max_dir, SWIFT_Real& max_spread );


inline void Compute_Max_Spread_Direction( SWIFT_Array<SWIFT_Tri_Vertex>& vs,
                                          SWIFT_Triple& max_dir )
{
    SWIFT_Triple min_dir, mid_dir, center;
    SWIFT_Real min_spread, mid_spread, max_spread;
    Compute_Spread( vs, center, false, min_dir, min_spread, mid_dir, mid_spread,
                    max_dir, max_spread );
}


void Quicksort( SWIFT_Array<SWIFT_Real>& vals, SWIFT_Array<SWIFT_BV*>& leaves,
                int p, int r );

#endif


