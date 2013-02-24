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
// SWIFT_boxnode.h
//
// Description:
//      Classes to manage nodes for bounding box sorting.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_BOXNODE_H_
#define _SWIFT_BOXNODE_H_

#include <SWIFT_config.h>
#include <SWIFT_common.h>

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Box_Node
//
// Description:
//      Class to manage a node for axis aligned bounding box sorting.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Box_Node {
  public:
    SWIFT_Box_Node( ) { val = 0.0; }
    ~SWIFT_Box_Node( ) { }

    // Get functions
    SWIFT_Real Value( ) { return val; }
    // Object id.  Indexes into the objects list.
    int Id( ) { return id; }
    // Index into the sorted box lists
    int Idx( ) { return idx; }
    bool Is_Max( ) { return is_max; }

    // Set functions
    void Set_Value( SWIFT_Real v ) { val = v; }
    void Set_Id( int i ) { id = i; }
    void Set_Idx( int i ) { idx = i; }
    void Set_Is_Max( bool im ) { is_max = im; }

  private:
    SWIFT_Real val;
    int id;
    int idx;
    bool is_max;
};

#endif


