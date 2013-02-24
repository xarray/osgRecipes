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
// object.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_mesh.h>
#include <SWIFT_object.h>


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Object public functions
//////////////////////////////////////////////////////////////////////////////

SWIFT_Object::SWIFT_Object( )
{
    mesh = NULL;
    min_bns[0].Set_Is_Max( false ), max_bns[0].Set_Is_Max( true ),
    min_bns[1].Set_Is_Max( false ), max_bns[1].Set_Is_Max( true ),
    min_bns[2].Set_Is_Max( false ), max_bns[2].Set_Is_Max( true );

    transform.Identity();
}


SWIFT_Object::~SWIFT_Object( )
{
    if( mesh->Ref() == 0 ) {
        mesh->Prepare_For_Delete();
        delete mesh;
    } else {
        mesh->Decrement_Ref();
    }
}

void SWIFT_Object::Get_Box_Nodes( int i,
                            SWIFT_Box_Node** min_0, SWIFT_Box_Node** max_0,
                            SWIFT_Box_Node** min_1, SWIFT_Box_Node** max_1,
                            SWIFT_Box_Node** min_2, SWIFT_Box_Node** max_2 )
{
    *min_0 = min_bns;   *max_0 = max_bns;
    *min_1 = min_bns+1; *max_1 = max_bns+1;
    *min_2 = min_bns+2; *max_2 = max_bns+2;
}

void SWIFT_Object::Set_Id( int i )
{
    min_bns[0].Set_Id( i ); max_bns[0].Set_Id( i );
    min_bns[1].Set_Id( i ); max_bns[1].Set_Id( i );
    min_bns[2].Set_Id( i ); max_bns[2].Set_Id( i );
}

void SWIFT_Object::Initialize( SWIFT_Tri_Mesh* m, bool f, bool uc,
                               SWIFT_Real ber, SWIFT_Real bea, bool cp )
{
    mesh = m;

    fixed = f;
    cube = uc;
    enlargement = ber * Radius() + bea;

    if( uc ) {
        // Create the cube
        radius = Radius() + enlargement;
    } else {
        // Initialize the dynamic bounding box
        min_es[0] = min_es[1] = min_es[2] = max_es[0] = 
        max_es[1] = max_es[2] = mesh->Root()->Other_Faces().Empty() ?
                                    mesh->Root()->Faces()[0].Edge1P() :
                                    mesh->Root()->Other_Faces()[0]->Edge1P();
    }
    Update_Boxes();
}




