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
// pqueue.cpp
//
//////////////////////////////////////////////////////////////////////////////

#ifdef SWIFT_PRIORITY_DIRECTION
#include <iostream>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#include <SWIFT_mesh.h>
#include <SWIFT_pqueue.h>

///////////////////////////////////////////////////////////////////////////////
// Scene Creation methods
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Priority_Queue::Heapify( int i )
{
    int l = (i<<1)+1;
    int r = l+1;
    int highest = (l < queue.Length() &&
                   queue[l].Distance() < queue[i].Distance()) ? l : i;

    if( r < queue.Length() && queue[r].Distance() < queue[highest].Distance()) {
        highest = r;
    }

    while( highest != i ) {
        SWIFT_Distance_Pair tempdp = queue[i];
        queue[i] = queue[highest];
        queue[highest] = tempdp;

        i = highest;
        l = (i<<1)+1;
        r = l+1;
        highest = (l < queue.Length() &&
                   queue[l].Distance() < queue[i].Distance()) ? l : i;

        if( r < queue.Length() &&
            queue[r].Distance() < queue[highest].Distance()
        ) {
            highest = r;
        }
    }
}

void SWIFT_Priority_Queue::Up_Heap( int i )
{
    int parent = (i-1)>>1;

    while( i > 0 && queue[parent].Distance() >= queue[i].Distance() ) {
        SWIFT_Distance_Pair tempdp = queue[i];
        queue[i] = queue[parent];
        queue[parent] = tempdp;

        i = parent;
        parent = (i-1)>>1;
    }

}

#endif

