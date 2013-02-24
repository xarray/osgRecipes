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
// SWIFT_lut.h
//
// Description:
//      Classes to manage lookup tables for convex pieces
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_LUT_H_
#define _SWIFT_LUT_H_

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>

typedef enum { LUT_NONE = -1, LUT_22_5 = 0, LUT_11_25 = 1, LUT_5_625 = 2 }
                                                                    LUT_TYPE;

class SWIFT_BV;
class SWIFT_Tri_Edge;
class SWIFT_Tri_Vertex;
class SWIFT_Tri_Mesh;

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Lookup_Table
//
// Description:
//      Class to manage a lookup table for a convex piece.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Lookup_Table {
  public:
    SWIFT_Lookup_Table( ) { }
    ~SWIFT_Lookup_Table( ) { }

    int Type( ) { return (int)type; }
    void Set_Type( int t ) { type = (LUT_TYPE)t; }
    int Size( ) { return ((8<<(int)type)-1) * (16<<(int)type) + 2; }
    SWIFT_Array<SWIFT_Tri_Edge*>& Table( ) { return lut; }
    void Set_Table( SWIFT_Array<SWIFT_Tri_Edge*>& t ) { lut = t; }
    void Create( SWIFT_BV* p );
    // Lookup table query function
    SWIFT_Tri_Edge* Lookup( const SWIFT_Triple& dir )
                { return type == LUT_NONE ? NULL : Lookup_Internal( dir ); }

#ifdef SWIFT_DEBUG
    void Dump( );
#endif

  private:
    SWIFT_Tri_Edge* Lookup_Internal( const SWIFT_Triple& dir );

    LUT_TYPE type;
    SWIFT_Array<SWIFT_Tri_Edge*> lut;
};

#endif


