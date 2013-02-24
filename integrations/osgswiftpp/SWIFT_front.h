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
// SWIFT_front.h
//
// Description:
//      Classes to manage a front tracking data structure
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_FRONT_H_
#define _SWIFT_FRONT_H_

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Front_Node
//
// Description:
//      Class to manage a front node in the front tree (SWIFT_Front).
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Front_Node {
  public:
    //SWIFT_Front_Node( ) { parent = NULL; rchild = NULL; lchild = NULL; }
    SWIFT_Front_Node( ) { Make_Leaf(); }
    ~SWIFT_Front_Node( ) { delete lchild; delete rchild; }

    RESULT_TYPE State( ) const { return state; }
    void* Feature0( ) const { return feat0; }
    void* Feature1( ) const { return feat1; }
    SWIFT_BV* BV0( ) const { return bv0; }
    SWIFT_BV* BV1( ) const { return bv1; }
    SWIFT_Front_Node* Parent( ) const { return parent; }
    SWIFT_Front_Node* Left_Child( ) const { return lchild; }
    SWIFT_Front_Node* Right_Child( ) const { return rchild; }
    SWIFT_Front_Node* Left_Sibling( ) const { return parent->Left_Child(); }
    SWIFT_Front_Node* Right_Sibling( ) const { return parent->Right_Child(); }
    // Could be intersecting or within a tolerance
    bool Close( ) const { return close; }

    void Set_State( RESULT_TYPE s ) { state = s; }
    void Set_Feature0( void* f ) { feat0 = f; }
    void Set_Feature1( void* f ) { feat1 = f; }
    void Set_BV0( SWIFT_BV* bv ) { bv0 = bv; }
    void Set_BV1( SWIFT_BV* bv ) { bv1 = bv; }
    void Set_Parent( SWIFT_Front_Node* n ) { parent = n; }
    void Set_Left_Child( SWIFT_Front_Node* n ) { lchild = n; }
    void Set_Right_Child( SWIFT_Front_Node* n ) { rchild = n; }
    void Set_Close( bool i ) { close = i; }
    void Set_Uninitialized( ) { feat1 = NULL; }

    bool Is_Leaf( ) const { return lchild == NULL; }
    bool Is_Left_Child( ) const { return parent->Left_Child() == this; }
    bool Is_Right_Child( ) const { return parent->Right_Child() == this; }
    bool Initialized( ) const { return feat1 != NULL; }

    void Make_Leaf( ) { lchild = rchild = NULL; }
    void Reset( ) {
             feat1 = NULL;
             parent = NULL; lchild = NULL; rchild = NULL; }

  private:
    bool close;

    RESULT_TYPE state;
    // if feat1 == NULL then the pair is not initialized
    void* feat0;
    void* feat1;
    SWIFT_BV* bv0;
    SWIFT_BV* bv1;

    SWIFT_Front_Node* parent;
    SWIFT_Front_Node* lchild;
    SWIFT_Front_Node* rchild;
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Front
//
// Description:
//      Class to manage a front tree.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Front {
  public:
    SWIFT_Front( ) { root.Set_Parent( &root ); size = 1; }
    ~SWIFT_Front( ) { Delete(); }

    void Delete( ) { delete root.Left_Child(); root.Set_Left_Child( NULL );
                     delete root.Right_Child(); root.Set_Right_Child( NULL ); }

    int Num_Leaves( ) const { return size; }
    int Size( ) const { return size; }
    SWIFT_Front_Node& Root( ) { return root; }
    SWIFT_Array<SWIFT_Front_Node*>& Leaves( ) { return queue; }
    int First_Leaf( ) const { return first; }
    int Last_Leaf( ) const { return last; }

    void Count_Raise( ) { size--; }
    void Count_Drop( ) { size++; }
    void Alloc_Queue( )
    {
        if( (size<<2) < queue.Max_Length() || (size<<1) > queue.Max_Length() ) {
            queue.Recreate( size<<1 );
        }
    }
    void Gather_Leaves( )
    {
        // Do BFS and gather the leaves at the end of the queue in deepest first
        first = queue.Length();
        last = queue.Length()-1;

        // Handle the case of root is a leaf
        if( root.Is_Leaf() ) {
            queue[last] = &root;
            first = last;
            return;
        }

        int start = 0, middle = 0;
        queue[0] = &root;
        while( middle >= start ) {
            // Warning: This order is critical
            if( queue[start]->Right_Child()->Is_Leaf() ) {
                queue[--first] = queue[start]->Right_Child();
            } else {
                queue[++middle] = queue[start]->Right_Child();
            }
            if( queue[start]->Left_Child()->Is_Leaf() ) {
                queue[--first] = queue[start]->Left_Child();
            } else {
                queue[++middle] = queue[start]->Left_Child();
            }
            start++;
        }
    }

  private:
    int size;
    int first, last;

    SWIFT_Front_Node root;
    SWIFT_Array<SWIFT_Front_Node*> queue;
};

#endif


