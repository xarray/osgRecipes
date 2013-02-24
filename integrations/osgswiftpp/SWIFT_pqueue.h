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
// SWIFT_pqueue.h
//
// Description:
//      Classes to manage a bounded priority queue
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_PQUEUE_H_
#define _SWIFT_PQUEUE_H_

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#ifdef SWIFT_FRONT_TRACKING
#include <SWIFT_front.h>
#endif

#define PQ_SIZE 1000

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Distance_Pair
//
// Description:
//      Class to manage a priority queue for distance pairs to expand
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Distance_Pair {
  public:
    SWIFT_Distance_Pair( ) { }
    ~SWIFT_Distance_Pair( ) { }

    SWIFT_Real Distance( ) const { return dist; }
    void Set_Distance( SWIFT_Real d ) { dist = d; }
    int Class0( ) const { return class0; }
    int Class1( ) const { return class1; }
    void Set_Class0( int c ) { class0 = c; }
    void Set_Class1( int c ) { class1 = c; }
#ifdef SWIFT_FRONT_TRACKING
    SWIFT_Front_Node* Front_Node( ) const { return fn; }
    void Set_Front_Node( SWIFT_Front_Node* f ) { fn = f; }
#endif
    SWIFT_BV* BV0( ) const { return bv0; }
    SWIFT_BV* BV1( ) const { return bv1; }
    void Set_BV0( SWIFT_BV* p ) { bv0 = p; }
    void Set_BV1( SWIFT_BV* p ) { bv1 = p; }

  private:
    SWIFT_Real dist;
    int class0;
    int class1;
#ifdef SWIFT_FRONT_TRACKING
    SWIFT_Front_Node* fn;
#endif
    SWIFT_BV* bv0;
    SWIFT_BV* bv1;
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Priority_Queue
//
// Description:
//      Class to manage a priority queue for distance pairs to expand
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Priority_Queue {
  public:
    SWIFT_Priority_Queue( ) : queue( PQ_SIZE ) { queue.Set_Length( 0 ); }
    ~SWIFT_Priority_Queue( ) { }

    bool Full( ) const { return queue.Max_Length() == queue.Length(); }
    bool Empty( ) const { return queue.Empty(); }

    const SWIFT_Distance_Pair& Highest( ) { return queue[0]; }

    // Remove the highest priority element from the queue
    void Remove_Highest( SWIFT_Distance_Pair& dp )
    {
#ifdef SWIFT_DEBUG
        if( queue.Empty() ) {
            cerr << "(((((((((((( Error: Cannot remove highest "
                 << "from empty q ))))))))))))" << endl;
        }
#endif
        dp = queue[0];
        queue[0] = queue.Last();
        queue.Decrement_Length();
        Heapify( 0 );
    }

    // Insert a new pair into the tree
    void Insert(
#ifdef SWIFT_FRONT_TRACKING
                 SWIFT_Front_Node* fn,
#endif
                 SWIFT_BV* bv0, SWIFT_BV* bv1,
                 int c0, int c1,
                 SWIFT_Real d )
    {
#ifdef SWIFT_DEBUG
        if( Full() ) {
            cerr << "(((((((((((( Error: Cannot insert into "
                 << "full q ))))))))))))" << endl;
        }
#endif
        queue.Increment_Length();
#ifdef SWIFT_FRONT_TRACKING
        queue.Last().Set_Front_Node( fn );
#endif
        queue.Last().Set_BV0( bv0 );
        queue.Last().Set_BV1( bv1 );
        queue.Last().Set_Class0( c0 );
        queue.Last().Set_Class1( c1 );
        queue.Last().Set_Distance( d );
        Up_Heap( queue.Length()-1 );
    }

    void Reset( ) { queue.Set_Length( 0 ); }

  private:
    void Heapify( int i );
    void Up_Heap( int i );

    SWIFT_Array<SWIFT_Distance_Pair> queue;
};

#endif


