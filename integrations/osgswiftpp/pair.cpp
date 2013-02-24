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
// pair.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <math.h>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#include <SWIFT_mesh.h>
#include <SWIFT_object.h>
#ifdef SWIFT_FRONT_TRACKING
#include <SWIFT_front.h>
#endif
#ifdef SWIFT_PRIORITY_DIRECTION
#include <SWIFT_pqueue.h>
#endif
#include <SWIFT_pair.h>

#define CYCLE_DETECTION
#ifdef NO_CYCLE_DETECTION
#undef CYCLE_DETECTION
#endif



#define SWIFT_MAX_VALENCE 100


///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

#ifdef CYCLE_DETECTION
// The number of state transitions before cycle detection kicks in
static const int STATE_TRANS_CYCLE_DECL = 200;
// The maximum length of cycles that are detected
static const int CYCLE_LENGTH = 20;
// The maximum number of times the transformation is jittered
#define MAX_CYCLE_TRANSFORMATIONS 8

static const SWIFT_Real JITTER_SCALES [MAX_CYCLE_TRANSFORMATIONS][3] = {
    { 1, 0.5, 0.25 }, { 0, 1, .5 }, { 1, 0, 0.5 }, { 0.5, 1, 0 },
    { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0 } };
#ifdef SWIFT_USE_FLOAT
static const SWIFT_Real JITTER_NON_UNIFORM_SCALE_MULTIPLIER = 1e-6;
#else
static const SWIFT_Real JITTER_NON_UNIFORM_SCALE_MULTIPLIER = 1e-12;
#endif
#else
#ifdef SWIFT_DEBUG
// The number of state transitions before an infinite loop is declared
static const int STATE_TRANS_CYCLE_DECL = 10000;
#endif
#endif

// Feature type identifiers
static const int VERTEX = 1;
static const int EDGE = 2;
static const int FACE = 3;


///////////////////////////////////////////////////////////////////////////////
// Local variables
///////////////////////////////////////////////////////////////////////////////

// These are setup before a query
static SWIFT_Object* obj0;
static SWIFT_Object* obj1;

// Helper variables
static SWIFT_Real dist;
static SWIFT_Triple fdir;

// Forwarding variables
static SWIFT_Triple* t1xp = new SWIFT_Triple;
static SWIFT_Triple* h1xp = new SWIFT_Triple;
static SWIFT_Triple* u1xp = new SWIFT_Triple;
static SWIFT_Triple* t2xp = new SWIFT_Triple;
static SWIFT_Triple* h2xp = new SWIFT_Triple;
static SWIFT_Triple* u2xp = new SWIFT_Triple;
static SWIFT_Triple* fnxp = new SWIFT_Triple;
static SWIFT_Real dt12, dh12, dt21, dh21;
static SWIFT_Real dl12, dr12, dl21, dr21;
static SWIFT_Real lam_min1, lam_max1, lam_min2, lam_max2;

// Transformation variables
static SWIFT_Transformation trans01;
static SWIFT_Transformation trans10;
static SWIFT_Transformation* T01 = &trans01;
static SWIFT_Transformation* T10 = &trans10;

// State transition variables
static SWIFT_Tri_Vertex* v1;
static SWIFT_Tri_Vertex* v2;
static SWIFT_Tri_Edge* ve1;
static SWIFT_Tri_Edge* ve2;
static SWIFT_Tri_Edge* e1;
static SWIFT_Tri_Edge* e2;
static SWIFT_Tri_Face* f1;
static SWIFT_Tri_Face* f2;

// State saving variables
#ifndef SWIFT_FRONT_TRACKING
static RESULT_TYPE save_state;
static SWIFT_Tri_Vertex* save_v1;
static SWIFT_Tri_Vertex* save_v2;
static SWIFT_Tri_Edge* save_ve1;
static SWIFT_Tri_Edge* save_ve2;
static SWIFT_Tri_Edge* save_e1;
static SWIFT_Tri_Edge* save_e2;
static SWIFT_Tri_Face* save_f1;
static SWIFT_Tri_Face* save_f2;
#endif

// State flag for keeping track of the state across global function calls
static RESULT_TYPE state;
static RESULT_TYPE prev_state;


// feat0, feat1, distance, error, pair type lists for holding the
// contacts during traversal
static SWIFT_Array<void*> contact_list0;
static SWIFT_Array<void*> contact_list1;
static SWIFT_Array<SWIFT_Real> contact_listd;
static SWIFT_Array<RESULT_TYPE> contact_listt;
static SWIFT_Array<SWIFT_BV*> contact_listbv0;
static SWIFT_Array<SWIFT_BV*> contact_listbv1;


// Non-convex variables
static int level0;
static int level1;
static SWIFT_BV* bv0;
static SWIFT_BV* bv1;
#ifdef SWIFT_PIECE_CACHING
static SWIFT_Pair* pair;
#endif
#ifdef SWIFT_PRIORITY_DIRECTION
static SWIFT_Priority_Queue pqueue;
#endif
#ifdef SWIFT_FRONT_TRACKING
static SWIFT_Front* pairfront;
#else
static bool saved;
#endif
static int c1, c2;

#ifdef CYCLE_DETECTION
// Cycle detection variables
SWIFT_Array<void*> cycle_detector_feats( CYCLE_LENGTH<<1 );
#ifdef SWIFT_DEBUG
int cycle_counter = 0;
#endif
#endif




//////////////////////////////////////////////////////////////////////////////
// Function prototypes
//////////////////////////////////////////////////////////////////////////////


// Setup the state for either an intersection query or a distance query
inline void Setup_Pair_Query( SWIFT_Object* o0, SWIFT_Object* o1,
                              bool contacts );

// Setup the state for either an intersection query or a distance query
inline void Setup_BV_Query( RESULT_TYPE& start_state,
                            void* feat0, void* feat1 );


// Initialization functions

// Intialize the pair with a random pair of vertices
inline void Initialize_Randomly( RESULT_TYPE& start_state );

// Initialize the pair with a pair of vertices according to the lookup table
// fdir should have already been computed by the calling function to be the
// vector from object 1's center of mass to object 2's center of mass and be
// in object 1's local frame.
inline void Initialize_From_Scratch( RESULT_TYPE& start_state );
// Same thing but the roles of 1 and 2 are reversed (optimization)
inline void Initialize_From_Scratch2( RESULT_TYPE& start_state );

// Initialize the pair with the pair of features from the end of previous query
inline void Initialize_From_Previous( RESULT_TYPE& start_state,
                                      void* feat0, void* feat1 );


// State transition functions.
inline RESULT_TYPE Vertex_Vertex( );
inline RESULT_TYPE Vertex_Edge( );
inline RESULT_TYPE Vertex_Face( );
       RESULT_TYPE Edge_Edge( );
       RESULT_TYPE Edge_Face( SWIFT_Tri_Edge* edge1, SWIFT_Triple* tx,
                              SWIFT_Triple* hx, SWIFT_Triple* ux,
                              SWIFT_Tri_Edge*& edge2,
                              SWIFT_Tri_Vertex*& vert2, SWIFT_Tri_Face*& face2
                              //, int l1, int l2, SWIFT_Tri_Edge*& vedge2
                              , SWIFT_Tri_Edge*& vedge2
                              );

// Intersection and nearest features function for a pair of convex polyhedra
bool Walk_Convex_LC( RESULT_TYPE start_state );
// Distance of the nearest features
inline void Distance_After_Walk_Convex_LC( SWIFT_Real& distance );
inline bool Distance_Convex_LC( RESULT_TYPE start_state, SWIFT_Real& distance );

// Tolerance and distance functions for a pair of objects.  The compile
// flags determine what types of objects are possible
#ifdef SWIFT_FRONT_TRACKING
bool Tolerance_LC( RESULT_TYPE start_state, SWIFT_Real tolerance,
                   SWIFT_Front_Node* node, SWIFT_Front_Node*& parent );
#else
bool Tolerance_LC( RESULT_TYPE start_state, SWIFT_Real tolerance );
#endif
bool Distance_LC( RESULT_TYPE start_state,
                  SWIFT_Real tolerance, SWIFT_Real abs_error,
                  SWIFT_Real rel_error, SWIFT_Real& distance,
#ifdef SWIFT_FRONT_TRACKING
                  SWIFT_Front_Node* node, SWIFT_Front_Node*& parent,
#endif
                  bool contacts = false
                );

//////////////////////////////////////////////////////////////////////////////
// Initialization functions
//////////////////////////////////////////////////////////////////////////////

inline void Setup_Pair_Query( SWIFT_Object* o0, SWIFT_Object* o1,
                              bool contacts )
{
    // Set the objects and the pieces
    obj0 = o0; obj1 = o1;

    // Compute the T01 and T10 transformations
    trans01.Transform_From_To( o0->Transformation(), o1->Transformation() );
    trans10.Invert( trans01 );

    if( contacts ) {
        // Set up the contact lists
        contact_list0.Set_Length( 0 );
        contact_list1.Set_Length( 0 );
        contact_listd.Set_Length( 0 );
        contact_listt.Set_Length( 0 );
        contact_listbv0.Set_Length( 0 );
        contact_listbv1.Set_Length( 0 );
    }
}

inline void Setup_BV_Query( RESULT_TYPE& start_state,
                            void* feat0, void* feat1 )
{
    level0 = bv0->Level();
    level1 = bv1->Level();
#ifndef SWIFT_FRONT_TRACKING
    saved = false;
#endif

    if( feat1 == NULL ) {
        // There are no last starting features so initialize the starting
        // features based on the normal map stored in the objects.
        fdir = (trans10 * bv1->Center_Of_Mass()) -
               bv0->Center_Of_Mass();
        Initialize_From_Scratch( start_state );
    } else {
        // Initialize from the previous pair of features
        // Levels have already been set by the caller
        Initialize_From_Previous( start_state, feat0, feat1 );
    }
}

inline void Initialize_Randomly( RESULT_TYPE& start_state )
{
    const int rint1 = (int)(drand48() * bv0->Num_All_Faces()*3);
    const int fid1 = rint1 / 3;
    if( fid1 >= bv0->Num_Faces() ) {
        ve1 = bv0->Other_Faces()[fid1-bv0->Num_Faces()]->Edge1P();
    } else {
        ve1 = bv0->Faces()[fid1].Edge1P();
    }
    const int rint2 = (int)(drand48() * bv1->Num_All_Faces()*3);
    const int fid2 = rint2 / 3;
    if( fid2 >= bv1->Num_Faces() ) {
        ve2 = bv1->Other_Faces()[fid2-bv1->Num_Faces()]->Edge1P();
    } else {
        ve2 = bv1->Faces()[fid2].Edge1P();
    }
    v1 = ve1->Origin();
    v2 = ve2->Origin();
    start_state = CONTINUE_VV;
}

inline void Initialize_From_Scratch( RESULT_TYPE& start_state )
{
    // Use the lookup table to do vertex selection
    ve1 = bv0->Lookup_Edge( fdir );

    // Move the center of mass direction vector to be in obj1's local frame
    fdir &= trans01;
    fdir.Negate();
    ve2 = bv1->Lookup_Edge( fdir );
    v1 = ve1->Origin();
    v2 = ve2->Origin();

    // Set the state to reflect the fact that we are choosing two vertices.
    start_state = CONTINUE_VV;
}

inline void Initialize_From_Scratch2( RESULT_TYPE& start_state )
{
    // Use the lookup table to do vertex selection
    ve2 = bv1->Lookup_Edge( fdir );

    // Move the center of mass direction vector to be in obj1's local frame
    fdir &= trans10;
    fdir.Negate();
    ve1 = bv0->Lookup_Edge( fdir );
    v1 = ve1->Origin();
    v2 = ve2->Origin();

    // Set the state to reflect the fact that we are choosing two vertices.
    start_state = CONTINUE_VV;
}

inline void Initialize_From_Previous( RESULT_TYPE& start_state,
                                      void* feat0, void* feat1 )
{
    switch( start_state ) {
    case CONTINUE_VV:
        ve1 = (SWIFT_Tri_Edge*) feat0;
        v1 = ve1->Origin();
        ve2 = (SWIFT_Tri_Edge*) feat1;
        v2 = ve2->Origin();
        break;
    case CONTINUE_VE:
        ve1 = (SWIFT_Tri_Edge*) feat0;
        v1 = ve1->Origin();
        e2 = (SWIFT_Tri_Edge*) feat1;
        break;
    case CONTINUE_EV:
        e1 = (SWIFT_Tri_Edge*) feat0;
        ve2 = (SWIFT_Tri_Edge*) feat1;
        v2 = ve2->Origin();
        break;
    case CONTINUE_VF:
        ve1 = (SWIFT_Tri_Edge*) feat0;
        v1 = ve1->Origin();
        f2 = (SWIFT_Tri_Face*) feat1;
        break;
    case CONTINUE_FV:
        f1 = (SWIFT_Tri_Face*) feat0;
        ve2 = (SWIFT_Tri_Edge*) feat1;
        v2 = ve2->Origin();
        break;
    case CONTINUE_EE:
        e1 = (SWIFT_Tri_Edge*) feat0;
        e2 = (SWIFT_Tri_Edge*) feat1;
    default:
        break;
    }
}

void Jitter_Transformation( int which )
{
    SWIFT_Triple scale(
        1.0 + JITTER_SCALES[which][0] * JITTER_NON_UNIFORM_SCALE_MULTIPLIER,
        1.0 + JITTER_SCALES[which][1] * JITTER_NON_UNIFORM_SCALE_MULTIPLIER,
        1.0 + JITTER_SCALES[which][2] * JITTER_NON_UNIFORM_SCALE_MULTIPLIER );

    SWIFT_Triple inv_scale( 1.0 / scale.X(), 1.0 / scale.Y(), 1.0 / scale.Z() );

    trans01.Scale( scale );
    trans10.Scale( inv_scale );
}

void Reset_Transformation( )
{
    // Compute the T01 and T10 transformations
    trans01.Transform_From_To( obj0->Transformation(), obj1->Transformation() );
    trans10.Invert( trans01 );
}

//////////////////////////////////////////////////////////////////////////////
// Walking/Intersection functions
//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////
// Feature pair checks and closest feature functions.
//
// First there is a function to check for containment of a point in a vertex
// external voronoi region.
//
// There are four main feature pair functions to test if a given pair of
// features is the closest pair of features between two convex polyhedra.  Each
// one handles a different combination of feature types.  The four functions
// are: Vertex_Vertex, Vertex_Edge, Vertex_Face, and Edge_Edge.  In addition,
// there is a helper function called Edge_Face which resolves a specialized
// edge-face query.  It is called from the Edge_Edge function.
//
// Each of these functions takes as implicit parameters (global variables)
// the two features, two transformation matrices for transforming features from
// one frame to the other and vice versa.  The new pair of features are placed
// in the appropriate global variables.
// T01 is always the transformation matrix from the frame of the first
// feature in the parameter list to the frame of the second feature in the
// parameter list; T10 is the inverse transformation.
//
// If a pair of features processed by a check function is indeed a closest pair
// the distance between these features is placed in the dist variable and the
// separation state of the variables is passed back.  If the features are not a
// closest feature pair, a value is passed back indicating which state to
// proceed to next.  One or possibly both of the closest features are updated
// to form a new pair which is closer or of lower dimension than the previous
// pair.
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Vertex_Vertex:
//
// Checks to see if two vertices are the pair of closest features.
// The two vertices are given in v1 and v2.  If they are not, then either e1
// or e2 is set to an edge to walk to.
//////////////////////////////////////////////////////////////////////////////

inline RESULT_TYPE Vertex_Vertex( )
{
    // Check v1 against v2's region
    SWIFT_Tri_Edge* edge;

    // Transform v1 to v2's local coordinates
    if( prev_state != CONTINUE_VE ) {
        *t1xp = (*T01) * v1->Coords();
        edge = ve2;
    } else {
        edge = ve2->Twin( level1 )->Next();
    }

    do {
        if( (dt12 = edge->Distance( *t1xp )) >= 0.0 ) {
            e2 = edge;
            return CONTINUE_VE;
        }
        edge = edge->Twin( level1 )->Next();
    } while( edge != ve2 );


    // Check v2 against v1's region

    // Transform v2 to v1's local coordinates
    if( prev_state != CONTINUE_EV ) {
        *t2xp = (*T10) * v2->Coords();
        edge = ve1;
    } else {
        edge = ve1->Twin( level0 )->Next();
    }

    do {
        if( (dt12 = edge->Distance( *t2xp )) >= 0.0 ) {
            e1 = edge;
            return CONTINUE_EV;
        }
        edge = edge->Twin( level0 )->Next();
    } while( edge != ve1 );


    // Set the vector between closest points
    fdir = *t2xp - v1->Coords();
    c1 = c2 = CLASS_ORIGINAL;

    return DISJOINT;
}


//////////////////////////////////////////////////////////////////////////////
// Vertex_Edge:
//
// Checks to see if the vertex and the edge are the pair of closest features.
// The vertex and the edge are given in v1 and e2.  If they are not, then
// we walk to the v-v, e-e, or v-f cases.  The appropriate v, e, f pointers are
// set for the next state.
//////////////////////////////////////////////////////////////////////////////

inline RESULT_TYPE Vertex_Edge( )
{
// Forward from VV, VF, or EE (not through EF?)
    // Transform v to e's local coordinates
    if( prev_state == DISJOINT ||
        (prev_state == CONTINUE_EE && state == CONTINUE_EV)
    ) {
        *t1xp = (*T01) * v1->Coords();
    }

    if( prev_state != CONTINUE_VF && prev_state != CONTINUE_FV ) {
        dl12 = e2->Face_Distance( *t1xp );
    }
    dr12 = e2->Twin( level1 )->Face_Distance( *t1xp );

    // Check v against the edge-face planes of the edge
    if( dl12 >= 0.0 ) {
        e1 = e2;
        f2 = e2->Adj_Face();
        return CONTINUE_VF;
    } else if( dr12 >= 0.0 ) {
        e1 = e2->Twin( level1 );
        dl12 = dr12;
        f2 = e1->Adj_Face();
        return CONTINUE_VF;
    }

    // Check v against the vertex-edge planes of the edge

    // Don't have to compute vertex violation of the edge's ve planes if
    // already done in EE state
    if( !(prev_state == CONTINUE_EE && state == CONTINUE_VE) ) {
        // Have to compute dh12
        dh12 = e2->Twin( level1 )->Distance( *t1xp );
        if( prev_state != CONTINUE_VV ) {
            dt12 = e2->Distance( *t1xp );
        }
    }

    if( dh12 < 0.0 ) {
        v2 = e2->Next()->Origin();
        ve2 = e2->Twin( level1 );
        return CONTINUE_VV;
    } else if( dt12 < 0.0 ) {
        v2 = e2->Origin();
        ve2 = e2;
        return CONTINUE_VV;
    }


    // The vertex lies in the edge's v-region.  Check e against the v's planes
    if( prev_state != CONTINUE_EE ) {
        *h2xp = (*T10) * e2->Next()->Origin()->Coords();
        if( !(prev_state == CONTINUE_VV && state == CONTINUE_EV) ) {
            // Only have to transform the tail if we came from VV and it was
            // transformed there already (we are in the EV state).
            *t2xp = (*T10) * e2->Origin()->Coords();
        }
        *u2xp = (*T10) & e2->Direction(); // Normalized
    }

    SWIFT_Tri_Edge* edge;
    SWIFT_Real lambda;
    SWIFT_Tri_Edge* et = NULL;
    SWIFT_Tri_Edge* eh = NULL;

    if( prev_state == CONTINUE_EE ) {
        // lam_min1 and lam_max1 are initialized correctly in EE
        edge = ve1->Twin( level0 )->Next();
    } else {
        lam_min1 = 0.0; lam_max1 = 1.0;
        edge = ve1;
    }

    do {
        dt21 = edge->Distance( *t2xp );
        dh21 = edge->Distance( *h2xp );

        if( dt21 < 0.0 ) {
            // The tail is inside the plane
            if( dh21 >= 0.0 && (lambda = dt21 / (dt21 - dh21)) < lam_max1 ) {
                lam_max1 = lambda;
                eh = edge;
                if( lam_min1 > lam_max1 ) {
                    break;
                }
            }
        } else {
            // The tail is not inside the plane
            if( dh21 >= 0.0 ) {
                // The head is not inside the plane
                e1 = edge;
                lam_min1 = 0.0; lam_max1 = 1.0;
                return CONTINUE_EE;
            }
            // The head is inside the plane
            if( (lambda = dt21 / (dt21 - dh21)) > lam_min1 ) {
                lam_min1 = lambda;
                et = edge;
                if( lam_min1 > lam_max1 ) {
                    break;
                }
            }
        }
        edge = edge->Twin( level0 )->Next();
    } while( edge != ve1 );

    // Check the derivatives at the ends of the clipped edge
    if( et != NULL ) {
        if( (*t2xp + (lam_min1 * e2->Length()) * (*u2xp) -
            v1->Coords()) * (*u2xp) > 0.0
        ) {
            e1 = et;
            lam_max1 = lam_min1;
            lam_min1 = 0.0;
            return CONTINUE_EE;
        }
    }
    if( eh != NULL ) {
        if( (*t2xp + (lam_max1 * e2->Length()) * (*u2xp) -
            v1->Coords()) * (*u2xp) < 0.0
        ) {
            e1 = eh;
            lam_min1 = lam_max1;
            lam_max1 = 1.0;
            return CONTINUE_EE;
        }
    }

    const SWIFT_Triple fdir2 = *h2xp - v1->Coords();
    fdir = fdir2 - (fdir2 * (*u2xp)) * (*u2xp);

    c1 = CLASS_ORIGINAL;
    c2 = e2->Classification( level1 );

    return DISJOINT;
}


//////////////////////////////////////////////////////////////////////////////
// Vertex_Face:
//
// Checks to see if the vertex and the face are the pair of closest features.
// The vertex and the face are given in v1 and f2.  If they are not, then
// we walk to the penetration, v-e, e-f->v-f, e-f->e-e, e-f->penetration,
// or e-f->v-f->penetration cases.  The appropriate v, e, f pointers are
// set for the next state.
//
// There is special code to handle the edge-face case that may arise if the
// vertex is in the face region but the corresponding point on the plane is
// not in the vertex region.
//
// When penetration is returned after calling this function, the features
// could either be the original vertex and face or another vertex and the
// original face.
//////////////////////////////////////////////////////////////////////////////

inline RESULT_TYPE Vertex_Face( )
{

    if( prev_state < CONTINUE_VF ) {
        // Transform v to f's local coordinates
        if( prev_state == DISJOINT ) {
            *t1xp = (*T01) * v1->Coords();
        }

        // Not necessary to use dl12 here...  But remember that it is a
        // forwarding variable TO here.

        // It is important to note that the e-f plane which is maximally
        // violated must be chosen and not simply the first one (incorrect).
        if( prev_state >= CONTINUE_VE ) {
            dr21 = f2->Edge2P() == e1 ? dl12 :
                                       f2->Edge2().Face_Distance( *t1xp );
            dr12 = f2->Edge3P() == e1 ? dl12 :
                                       f2->Edge3().Face_Distance( *t1xp );
            dl12 = f2->Edge1P() == e1 ? dl12 :
                                        f2->Edge1().Face_Distance( *t1xp );
        } else {
            dl12 = f2->Edge1().Face_Distance( *t1xp );
            dr21 = f2->Edge2().Face_Distance( *t1xp );
            dr12 = f2->Edge3().Face_Distance( *t1xp );
        }

        // Check v against the edge-face planes of the face
        e2 = f2->Edge1P();
        if( dr21 < dl12 ) {
            dl12 = dr21;
            e2 = f2->Edge2P();
        }
        if( dr12 < dl12 ) {
            dl12 = dr12;
            e2 = f2->Edge3P();
        }

        // Check if v is in f's v-region
        if( dl12 < 0.0 ) {
            return CONTINUE_VE;
        }

        // No edge-face plane was violated.

        // Transform f's normal to v's local coordinates
        *fnxp = (*T10) & f2->Normal();

        if( prev_state != LOCAL_MINIMUM ) {
            // Take the distance to the plane.
            dist = f2->Distance( *t1xp );

            // Take the distance from the vertex to the face plane and negate
            // the normal if necessary.
            if( dist < 0.0 ) {
                fnxp->Negate();
            }
        }
    } else if( prev_state >= CONTINUE_EE ) {
        // Transform f's normal to v's local coordinates
        *fnxp = (*T10) & f2->Normal();

        // The distance is already computed and is passed in the dist variable
        if( dist < 0.0 ) {
            fnxp->Negate();
        }
    }

    // Look for an edge that points into the face.
    SWIFT_Tri_Edge* edge = ve1;
    SWIFT_Real minfd;
    minfd = SWIFT_INFINITY;
    if( prev_state >= CONTINUE_VF ) {
        edge = edge->Twin( level0 )->Next();
    }

    do {
        dr21 = (*fnxp) * edge->Direction();
        if( dr21 < minfd ) {
            minfd = dr21;
            e1 = edge;
        }
        edge = edge->Twin( level0 )->Next();
    } while( edge != ve1 );

    if( minfd >= 0.0 ) {
        // The distance is already in the dist variable.

        // Check for local minimum
        if( dist < 0.0 ) {
            return LOCAL_MINIMUM;
        } else {
            c1 = CLASS_ORIGINAL;
            c2 = f2->Classification();
            return DISJOINT;
        }
    }

    // Resolve the edge-face case.  We have found an edge that is pointing into
    // the face.  minfd holds the dot product of the edge and the face normal.

    // Transform the head of the edge to f's local coordinates and compute the
    // distance of it to the first and second edge-face planes.
    *h1xp = (*T01) * e1->Next()->Origin()->Coords();

    dr21 = f2->Edge1().Face_Distance( *h1xp );
    dr12 = f2->Edge2().Face_Distance( *h1xp );

    // Find out which edge-face planes the head of the edge violates.
    // e2 will holds first one (if possible), edge holds possible second one.
    edge = NULL;
    e2 = NULL;
    if( dr21 < 0.0 ) {
        e2 = f2->Edge1P();
        if( dr12 < 0.0 ) {
            edge = f2->Edge2P();
        } else {
            if( (dr12 = f2->Edge3().Face_Distance( *h1xp )) < 0.0 ) {
                edge = f2->Edge3P();
            }
        }
    } else if( dr12 < 0.0 ) {
        dr21 = dr12;
        e2 = f2->Edge2P();
        if( (dr12 = f2->Edge3().Face_Distance( *h1xp )) < 0.0 ) {
            edge = f2->Edge3P();
        }
    } else {
        if( (dr21 = f2->Edge3().Face_Distance( *h1xp )) < 0.0 ) {
            e2 = f2->Edge3P();
        }
    }

    // Check if any of the edge-face planes were violated.
    if( e2 == NULL ) {
        // Check for penetration
        minfd = dist;
        dist = f2->Distance( *h1xp );
        if( (dist < 0.0 && minfd > 0.0) || (dist > 0.0 && minfd < 0.0) ) {
            // Previous continuation was CONTINUE_VF
            c1 = e1->Classification( level0 );
            c2 = f2->Classification();
            return PENETRATION;
        }

        v1 = e1->Next()->Origin();
        ve1 = e1->Twin( level0 );

        // Swap the head and the tail
        SWIFT_Triple* temp_xp = t1xp; t1xp = h1xp; h1xp = temp_xp;

        return CONTINUE_VF;
    }

    // One or more of the edge-face planes is pierced by the edge.
    // Find out which one is pierced first.

    *u1xp = (*T01) & e1->Direction();
    SWIFT_Real ux_dot = *u1xp * e2->Face_Normal();

    if( edge == NULL ) {
        dr12 = dr21;
    } else {
        SWIFT_Real ux_dot2 = *u1xp * edge->Face_Normal();
        // There were two edge-face planes pierced.
        if( dr12 * ux_dot > dr21 * ux_dot2 ) {
            // the other edge clips first
            e2 = edge;
            ux_dot = ux_dot2;
        } else {
            dr12 = dr21;
        }
    }

    dr21 = f2->Distance( *h1xp );

    // minfd contains the dot product of the edge direction and the face norm.
    // dr12 contains the distance that the head is from the nearest clip edge
    // ux_dot contains the dot product of the edge direction and the nearest
    //      clip edge edge-face normal.
    // dr21 contains the distance that the head is from the face plane.
    // dist contains the distance that the tail is from the face plane.

    if( (dr21 > 0.0 && dist < 0.0 && -dr12 * minfd > dr21 * ux_dot) ||
        (dr21 < 0.0 && dist > 0.0 && dr12 * minfd < dr21 * ux_dot)
    ) {
        c1 = e1->Classification( level0 );
        c2 = f2->Classification();
        return PENETRATION;
    }

    // Continue onto the E-E case.  e1 is set to the edge coming from the
    // input vertex and e2 is the edge belonging to the input face so
    // the parameters are set correctly.
    return CONTINUE_EE;
}


//////////////////////////////////////////////////////////////////////////////
// Edge_Edge:
//
// Checks to see if the two edges are the pair of closest features.
// The edges are given in e1 and e2.  If they are not the closest, then
// we walk to the v-v, v-e, e-v, e-f->v-f, e-f->penetration cases.
// The appropriate v, e, f pointers are set for the next state.
//
// A special function is called to handle the edge face cases that arise when
// the closest points on the edges are on the interiors and violate each
// other's regions.
//
// When penetration is returned after calling this function, the features
// could either be the original edges or one of the edges the same and the
// other different.
//////////////////////////////////////////////////////////////////////////////

// Computes the closest LOCAL points on the edges.  The vector that spans the
// two points is also computed in EDGE2's LOCAL coordinates pointing from edge1
// to edge2.
void Compute_Closest_Points_Edge_Edge(
    SWIFT_Tri_Edge* edge1, SWIFT_Tri_Edge* edge2, SWIFT_Triple& tri1,
    //SWIFT_Triple& tri2, SWIFT_Transformation& T_1_2, SWIFT_Triple* sv = NULL )
    SWIFT_Triple& tri2, SWIFT_Transformation& T_1_2, SWIFT_Real* dist = NULL )
{
    SWIFT_Triple tx = T_1_2 * edge1->Origin()->Coords();
    SWIFT_Triple ux = T_1_2 & edge1->Direction();   // Vector transform
    tri2 = edge2->Origin()->Coords() - tx;
    SWIFT_Real d1_dot_v1 = ux * tri2;
    SWIFT_Real d2_dot_v1 = edge2->Direction() * tri2;
    SWIFT_Real d = edge2->Direction() * ux;
    // t is distance along e1 and u is distance along e2
    SWIFT_Real t, u = 1.0 - d * d;  // denom

    if( u == 0.0 ) {
        t = 0.0;
        u = -d2_dot_v1;
    } else {
        t = (d1_dot_v1 - d2_dot_v1 * d) / u;
        if( t < 0.0 ) {
            t = 0.0;
            u = -d2_dot_v1;
        } else if( t > edge1->Length() ) {
            t = edge1->Length();
            u = t*d - d2_dot_v1;
        } else {
            u = t*d - d2_dot_v1;
        }
    }

    if( u < 0.0 ) {
        tri2 = edge2->Origin()->Coords();
        if( d1_dot_v1 < 0.0 ) {
            t = 0.0;
            tri1 = edge1->Origin()->Coords();
        } else if( d1_dot_v1 > edge1->Length() ) {
            t = edge1->Length();
            tri1 = edge1->Next()->Origin()->Coords();
        } else {
            t = d1_dot_v1;
            tri1 = edge1->Origin()->Coords() + t * edge1->Direction();
        }
    } else if( u > edge2->Length() ) {
        t = edge2->Length() * d + d1_dot_v1;
        tri2 = edge2->Next()->Origin()->Coords();
        if( t < 0.0 ) {
            t = 0.0;
            tri1 = edge1->Origin()->Coords();
        } else if( t > edge1->Length() ) {
            t = edge1->Length();
            tri1 = edge1->Next()->Origin()->Coords();
        } else {
            tri1 = edge1->Origin()->Coords() + t * edge1->Direction();
        }
    } else { 
        // The points lie on the interiors of the edges
        tri1 = edge1->Origin()->Coords() + t * edge1->Direction();
        tri2 = edge2->Origin()->Coords() + u * edge2->Direction();
    }

    if( dist != NULL ) {
        *dist = tri2.Dist( tx + t * ux );
    }
}

RESULT_TYPE Edge_Edge( )
{
    SWIFT_Real d1, d2;

    // Handle the swap of the previously computed lambda values
    if( prev_state == CONTINUE_VE ) {
        lam_min2 = lam_min1; lam_max2 = lam_max1;
    }

    // Don't have to transform the first edge if it was already done in EV
    // Don't have to clip against the tail if it was already done in EV
    if( prev_state != CONTINUE_EV ) {
        // Transform e1 to e2's coordinates
        if( prev_state != CONTINUE_VF && prev_state != CONTINUE_EF ) {
            *h1xp = (*T01) * e1->Next()->Origin()->Coords();
            *u1xp = (*T01) & e1->Direction();
            if( prev_state != CONTINUE_VE ) {
                *t1xp = (*T01) * e1->Origin()->Coords();
            }
        }
        if( prev_state != CONTINUE_VE ) {
            dt12 = e2->Distance( *t1xp );
        }
        d1 = e2->Distance( *h1xp );
        lam_min1 = 0.0; lam_max1 = 1.0;
        // Test the vertices of edge1 against edge2

        // Test against the vertex-edge planes
        if( dt12 < 0.0 ) {
            if( d1 < 0.0 ) {
                // Edge is fully clipped out
                v2 = e2->Origin();
                ve2 = e2;
                lam_min1 = 0.0; lam_max1 = 1.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_EV;
            }
            lam_min1 = dt12 / (dt12 - d1);
            // Check the derivative
            if( (*t1xp + (lam_min1 * e1->Length()) * (*u1xp) -
                e2->Origin()->Coords()) * (*u1xp) > 0.0
            ) {
                v2 = e2->Origin();
                ve2 = e2;
                lam_max1 = lam_min1; lam_min1 = 0.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_EV;
            }
        } else if( d1 < 0.0 ) {
            lam_max1 = dt12 / (dt12 - d1);
            // Check the derivative
            if( (*t1xp + (lam_max1 * e1->Length()) * (*u1xp) -
                e2->Origin()->Coords()) * (*u1xp) < 0.0
            ) {
                v2 = e2->Origin();
                ve2 = e2;
                lam_min1 = lam_max1; lam_max1 = 1.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_EV;
            }
        }
    } else {
        dt21 = dt12; dh21 = dh12;
    }

    if( prev_state != CONTINUE_VE ) {
        dh12 = e2->Twin( level1 )->Distance( *t1xp );
    }
    d2 = e2->Twin( level1 )->Distance( *h1xp );
    if( dh12 < 0.0 ) {
        if( d2 < 0.0 ) {
            // Edge is fully clipped out
            v2 = e2->Next()->Origin();
            ve2 = e2->Twin( level1 );
            lam_min1 = 0.0; lam_max1 = 1.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_EV;
        }
        lam_min1 = dh12 / (dh12 - d2);
        // Check the derivative
        if( (*t1xp + (lam_min1 * e1->Length()) * (*u1xp) -
            e2->Next()->Origin()->Coords()) * (*u1xp) > 0.0
        ) {
            v2 = e2->Next()->Origin();
            ve2 = e2->Twin( level1 );
            lam_max1 = lam_min1; lam_min1 = 0.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_EV;
        }
    } else if( d2 < 0.0 ) {
        lam_max1 = dh12 / (dh12 - d2);
        // Check the derivative
        if( (*t1xp + (lam_max1 * e1->Length()) * (*u1xp) -
            e2->Next()->Origin()->Coords()) * (*u1xp) < 0.0
        ) {
            v2 = e2->Next()->Origin();
            ve2 = e2->Twin( level1 );
            lam_min1 = lam_max1; lam_max1 = 1.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_EV;
        }
    }

    // Test the vertices of edge2 against edge1

    // Don't have to transform the second edge if it was already done in VE
    // Don't have to clip against the tail if it was already done in VE
    // Have to us the d?21 variables here because the d?12 variables hold
    // information for if we go to VE.
    if( prev_state != CONTINUE_VE ) {
        // Transform e2 to e1's coordinates

        if( prev_state != CONTINUE_FV && prev_state != CONTINUE_FE ) {
            *h2xp = (*T10) * e2->Next()->Origin()->Coords();
            *u2xp = (*T10) & e2->Direction();
            if( prev_state != CONTINUE_EV ) {
                *t2xp = (*T10) * e2->Origin()->Coords();
            }
        }
        if( prev_state != CONTINUE_EV ) {
            dt21 = e1->Distance( *t2xp );
        }
        // Use the variable dr21 here since it is not needed
        dr21 = e1->Distance( *h2xp );
        lam_min2 = 0.0; lam_max2 = 1.0;
        // Test against the vertex-edge planes
        if( dt21 < 0.0 ) {
            if( dr21 < 0.0 ) {
                // Edge is fully clipped out
                v1 = e1->Origin();
                ve1 = e1;
                lam_min1 = 0.0; lam_max1 = 1.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_VE;
            }
            lam_min2 = dt21 / (dt21 - dr21);
            // Check the derivative
            if( (*t2xp + (lam_min2 * e2->Length()) * (*u2xp) -
                e1->Origin()->Coords()) * (*u2xp) > 0.0
            ) {
                v1 = e1->Origin();
                ve1 = e1;
                lam_max1 = lam_min2; lam_min1 = 0.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_VE;
            }
        } else if( dr21 < 0.0 ) {
            lam_max2 = dt21 / (dt21 - dr21);
            // Check the derivative
            if( (*t2xp + (lam_max2 * e2->Length()) * (*u2xp) -
                e1->Origin()->Coords()) * (*u2xp) < 0.0
            ) {
                v1 = e1->Origin();
                ve1 = e1;
                lam_min1 = lam_max2; lam_max1 = 1.0;
                prev_state = CONTINUE_EE;
                return CONTINUE_VE;
            }
        }
    }

    if( prev_state != CONTINUE_EV ) {
        dh21 = e1->Twin( level0 )->Distance( *t2xp );
    }
    dl21 = e1->Twin( level0 )->Distance( *h2xp );
    if( dh21 < 0.0 ) {
        if( dl21 < 0.0 ) {
            // Edge is fully clipped out

            // Swap the head and the tail
            SWIFT_Triple* temp_xp = t1xp; t1xp = h1xp; h1xp = temp_xp;

            if( prev_state == CONTINUE_EV ) {
                // Have to compute dt12
                dt12 = e2->Distance( *t1xp );
            } else {
                dt12 = d1;
            }
            dh12 = d2;

            v1 = e1->Next()->Origin();
            ve1 = e1->Twin( level0 );
            lam_min1 = 0.0; lam_max1 = 1.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_VE;
        }
        lam_min2 = dh21 / (dh21 - dl21);
        // Check the derivative
        if( (*t2xp + (lam_min2 * e2->Length()) * (*u2xp) -
            e1->Next()->Origin()->Coords()) * (*u2xp) > 0.0
        ) {
            // Swap the head and the tail
            SWIFT_Triple* temp_xp = t1xp; t1xp = h1xp; h1xp = temp_xp;

            if( prev_state == CONTINUE_EV ) {
                // Have to compute dt12
                dt12 = e2->Distance( *t1xp );
            } else {
                dt12 = d1;
            }
            dh12 = d2;

            v1 = e1->Next()->Origin();
            ve1 = e1->Twin( level0 );
            lam_max1 = lam_min2; lam_min1 = 0.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_VE;
        }
    } else if( dl21 < 0.0 ) {
        lam_max2 = dh21 / (dh21 - dl21);
        // Check the derivative
        if( (*t2xp + (lam_max2 * e2->Length()) * (*u2xp) -
            e1->Next()->Origin()->Coords()) * (*u2xp) < 0.0
        ) {
            // Swap the head and the tail
            SWIFT_Triple* temp_xp = t1xp; t1xp = h1xp; h1xp = temp_xp;

            if( prev_state == CONTINUE_EV ) {
                // Have to compute dt12
                dt12 = e2->Distance( *t1xp );
            } else {
                dt12 = d1;
            }
            dh12 = d2;

            v1 = e1->Next()->Origin();
            ve1 = e1->Twin( level0 );
            lam_min1 = lam_max2; lam_max1 = 1.0;
            prev_state = CONTINUE_EE;
            return CONTINUE_VE;
        }
    }

    SWIFT_Tri_Edge* et2 = NULL;
    SWIFT_Tri_Edge* eh2 = NULL;
    SWIFT_Tri_Edge* et1 = NULL;
    SWIFT_Tri_Edge* eh1 = NULL;
    SWIFT_Real lambda;
    bool fully_clipped1 = false;
    bool fully_clipped2 = false;
    bool done = false;

    // Test against the edge face planes

    if( prev_state == CONTINUE_EV || prev_state == CONTINUE_FV
                                  || prev_state == CONTINUE_FE
    ) {
        dl21 = dl12; dr21 = dr12;
    }

    if( prev_state != CONTINUE_VE && prev_state != CONTINUE_EF ) {
        dl12 = e2->Face_Distance( *t1xp );
    }
    if( prev_state == CONTINUE_VF || prev_state == CONTINUE_EF ) {
        dt12 = dr12;
    } else {
        dt12 = e2->Face_Distance( *h1xp );
    }
    if( dl12 >= 0.0 ) {
        if( dt12 >= 0.0 ) {
            // Edge is fully clipped out
            done = true;
            et2 = eh2 = e2;
            fully_clipped2 = true;
        } else if( (lambda = dl12 / (dl12 - dt12)) > lam_min1 ) {
            lam_min1 = lambda;
            et2 = e2;
            done = lam_max1 < lam_min1;
        }
    } else if( dt12 >= 0.0 && (lambda = dl12 / (dl12 - dt12)) < lam_max1 ) {
        lam_max1 = lambda;
        eh2 = e2;
        done = lam_max1 < lam_min1;
    }
    if( !done ) {
        if( prev_state != CONTINUE_VE ) {
            dr12 = e2->Twin( level1 )->Face_Distance( *t1xp );
        }
        dh12 = e2->Twin( level1 )->Face_Distance( *h1xp );
        if( dr12 >= 0.0 ) {
            if( dh12 >= 0.0 ) {
                // Edge is fully clipped out
                et2 = eh2 = e2->Twin( level1 );
                fully_clipped2 = true;
            } else if( (lambda = dr12 / (dr12 - dh12)) > lam_min1 ) {
                lam_min1 = lambda;
                et2 = e2->Twin( level1 );
            }
        } else if( dh12 >= 0.0 && (lambda = dr12 / (dr12 - dh12)) < lam_max1 ) {
            lam_max1 = lambda;
            eh2 = e2->Twin( level1 );
        }
    }

    // Test against the edge face planes
    if( prev_state != CONTINUE_EV &&  prev_state != CONTINUE_FE ) {
        dl21 = e1->Face_Distance( *t2xp );
    }
    if( prev_state == CONTINUE_FV || prev_state == CONTINUE_FE ) {
        dt21 = dr21;
    } else {
        dt21 = e1->Face_Distance( *h2xp );
    }
    done = false;
    if( dl21 >= 0.0 ) {
        if( dt21 >= 0.0 ) {
            // Edge is fully clipped out
            et1 = eh1 = e1;
            done = true;
            fully_clipped1 = true;
        } else if( (lambda = dl21 / (dl21 - dt21)) > lam_min2 ) {
            lam_min2 = lambda;
            et1 = e1;
            done = lam_max2 < lam_min2;
        }
    } else if( dt21 >= 0.0 && (lambda = dl21 / (dl21 - dt21)) < lam_max2 ) {
        lam_max2 = lambda;
        eh1 = e1;
        done = lam_max2 < lam_min2;
    }
    if( !done ) {
        if( prev_state != CONTINUE_EV ) {
            dr21 = e1->Twin( level0 )->Face_Distance( *t2xp );
        }
        dh21 = e1->Twin( level0 )->Face_Distance( *h2xp );
        if( dr21 >= 0.0 ) {
            if( dh21 >= 0.0 ) {
                // Edge is fully clipped out
                et1 = eh1 = e1->Twin( level0 );
                fully_clipped1 = true;
            } else if( (lambda = dr21 / (dr21 - dh21)) > lam_min2 ) {
                lam_min2 = lambda;
                et1 = e1->Twin( level0 );
            }
        } else if( dh21 >= 0.0 && (lambda = dr21 / (dr21 - dh21)) < lam_max2 ) {
            lam_max2 = lambda;
            eh1 = e1->Twin( level0 );
        }
    }

    // Check the derivatives crossing to faces if the edges were clipped by the
    // edge-face planes
    if( et2 != NULL ) {
        if( !fully_clipped2 ) {
            d1 = et2->Adj_Face()->Distance(
                                *t1xp + (lam_min1 * e1->Length()) * (*u1xp) );
        }
        d2 = et2->Adj_Face()->Normal() * (*u1xp);

        if( fully_clipped2 || d1 < 0.0 && d2 < 0.0 || d1 > 0.0 && d2 > 0.0 ) {
            if( !fully_clipped2 || (et2 == e2 && dt12 < dl12)
                                || (et2 != e2 && dh12 < dr12)
            ) {
                SWIFT_Triple* temp_xp = h1xp;
                h1xp = t1xp;
                t1xp = temp_xp;
                u1xp->Negate();
                e1 = e1->Twin( level0 );
                dt12 = (et2 == e2) ? dt12 : dh12;
                dr21 = -d2;
            } else {
                dt12 = (et2 == e2) ? dl12 : dr12;
                dr21 = d2;
            }
            e2 = et2;
            f2 = e2->Adj_Face();
            prev_state = CONTINUE_EF;
            RESULT_TYPE result = Edge_Face( e1, t1xp, h1xp, u1xp, e2, v1, f2
                                                         , ve1
                                                                );
            if( result == CONTINUE_VF ) {
                // Swap the head and the tail
                SWIFT_Triple* temp_xp = t1xp;
                t1xp = h1xp;
                h1xp = temp_xp;
            }
            return result;
        }
    }
    if( eh2 != NULL ) {
        d1 = eh2->Adj_Face()->Distance(
                                *t1xp + (lam_max1 * e1->Length()) * (*u1xp) );
        d2 = eh2->Adj_Face()->Normal() * (*u1xp);

        if( d1 < 0.0 && d2 > 0.0 || d1 > 0.0 && d2 < 0.0 ) {
            dt12 = (eh2 == e2) ? dl12 : dr12;
            dr21 = d2;
            e2 = eh2;
            f2 = e2->Adj_Face();
            prev_state = CONTINUE_EF;
            RESULT_TYPE result = Edge_Face( e1, t1xp, h1xp, u1xp, e2, v1, f2
                                                         , ve1
                                                                );
            if( result == CONTINUE_VF ) {
                // Swap the head and the tail
                SWIFT_Triple* temp_xp = t1xp;
                t1xp = h1xp;
                h1xp = temp_xp;
            }
            return result;
        }
    }
    if( et1 != NULL ) {
        if( !fully_clipped1 ) {
            d1 = et1->Adj_Face()->Distance(
                                *t2xp + (lam_min2 * e2->Length()) * (*u2xp) );
        }
        d2 = et1->Adj_Face()->Normal() * (*u2xp);

        if( fully_clipped1 || d1 < 0.0 && d2 < 0.0 || d1 > 0.0 && d2 > 0.0 ) {
            if( !fully_clipped1 || (et1 == e1 && dt21 < dl21)
                                || (et1 != e1 && dh21 < dr21)
            ) {
                SWIFT_Triple* temp_xp = h2xp;
                h2xp = t2xp;
                t2xp = temp_xp;
                u2xp->Negate();
                e2 = e2->Twin( level1 );
                dt12 = (et1 == e1) ? dt21 : dh21;
                dr21 = -d2;
            } else {
                dt12 = (et1 == e1) ? dl21 : dr21;
                dr21 = d2;
            }
            int tempi = level1; level1 = level0; level0 = tempi;
            e1 = et1;
            f1 = e1->Adj_Face();
            prev_state = CONTINUE_FE;
            RESULT_TYPE result = Edge_Face( e2, t2xp, h2xp, u2xp, e1, v2, f1
                                                         , ve2
                                                                );
            tempi = level1; level1 = level0; level0 = tempi;
            tempi = c2; c2 = c1; c1 = tempi;
            if( result == CONTINUE_VF ) {
                // Swap the head and the tail
                SWIFT_Triple* temp_xp = t2xp;
                t2xp = h2xp;
                h2xp = temp_xp;
                result = CONTINUE_FV;
            }
            return result;
        }
    }
    if( eh1 != NULL ) {
        d1 = eh1->Adj_Face()->Distance(
                                *t2xp + (lam_max2 * e2->Length()) * (*u2xp) );
        d2 = eh1->Adj_Face()->Normal() * (*u2xp);

        if( d1 < 0.0 && d2 > 0.0 || d1 > 0.0 && d2 < 0.0 ) {
            int tempi = level1; level1 = level0; level0 = tempi;
            dt12 = (eh1 == e1) ? dl21 : dr21;
            dr21 = d2;
            e1 = eh1;
            f1 = e1->Adj_Face();
            prev_state = CONTINUE_FE;
            RESULT_TYPE result = Edge_Face( e2, t2xp, h2xp, u2xp, e1, v2, f1
                                                         , ve2
                                                                );
            tempi = level1; level1 = level0; level0 = tempi;
            tempi = c2; c2 = c1; c1 = tempi;
            if( result == CONTINUE_VF ) {
                // Swap the head and the tail
                SWIFT_Triple* temp_xp = t2xp;
                t2xp = h2xp;
                h2xp = temp_xp;
                result = CONTINUE_FV;
            }
            return result;
        }
    }
    prev_state = CONTINUE_EE;

    c1 = e1->Classification( level0 );
    c2 = e2->Classification( level1 );
    return DISJOINT;
}


//////////////////////////////////////////////////////////////////////////////
// Edge_Face:
//
// Figure out which state to transition to when given an edge (edge1) and a
// face (edge2->face).
// This is called from the edge-edge case where the pair of nearest points
// are on the interiors of the two edges.  It can transition to v-f, e-e, or
// penetration cases.  In the e-e case, edge2 is set to the edge that whose e-f
// plane is pierced so the new pair is edge1 and edge2.
//
// We just have to decide to step to the vertex-face case if the head of
// the edge is closest to the face or to the edge-edge case if the head of
// the edge is not in the face's V-region.  Or decide that penetration exists.
//////////////////////////////////////////////////////////////////////////////
RESULT_TYPE Edge_Face( SWIFT_Tri_Edge* edge1, SWIFT_Triple* tx,
                       SWIFT_Triple* hx, SWIFT_Triple* ux,
                       SWIFT_Tri_Edge*& edge2,
                       SWIFT_Tri_Vertex*& vert1, SWIFT_Tri_Face*& face2
                       , SWIFT_Tri_Edge*& vedge1
                       )
{
    // From knowing which edge of the face we are entering from, we set e4 to
    // be the entering edge and e3 and edge2 to be the other edges in their
    // order around the face.  We also compute the distance of the tail of the
    // input edge from the entering edge-face plane (dt12) to later determine if
    // the tail is on the face side or not.  We compute the distance of the
    // input edge's head from the two (non-entering) edges of the face
    // (dh1/dh2).

    SWIFT_Tri_Edge* e3 = edge2->Next();
    SWIFT_Tri_Edge* e4 = edge2->Prev();
    SWIFT_Real dh1 = e3->Face_Distance( *hx );
    SWIFT_Real dh2 = e4->Face_Distance( *hx );
    SWIFT_Real dhf = dist = face2->Distance( *hx );
    SWIFT_Real dt1 = e3->Face_Distance( *tx );
    SWIFT_Real dt2 = e4->Face_Distance( *tx );
    SWIFT_Real un1, un2, unf;
    bool clipped_out = false;

    if( dr21 > 0.0 ) {
        dhf = -dhf;
        unf = -dr21;
    } else {
        unf = dr21;
    }

    // Check results of clipping the head against the other two edges
    // If neither edge clipped the head, then proceed to VF.
    // Set e3 to be the edge that clipped the head first.
    // Set dh1 to be distance of the head from the first clipping plane.
    // Set un1 to the dot product of the first edge face plane normal and
    // the direction vector of the entering edge.
    if( dh1 < 0.0 ) {
        // Head was clipped by the first edge
        if( dh2 < 0.0 ) {
            if( dt1 > 0.0 && dt2 > 0.0 ) {
                // Head was also clipped by either the second or
                // third edge
                un1 = *ux * e3->Face_Normal();
                un2 = *ux * e4->Face_Normal();
                if( dh1 * un2 < dh2 * un1 ) {
                    // The second edge clipped it first
                    SWIFT_Tri_Edge* tempe = e3; e3 = e4; e4 = tempe;
                    // Use un1 as a temp
                    un1 = dt1; dt1 = dt2; dt2 = un1;
                    un1 = dh1; dh1 = dh2; dh2 = un1;
                    un1 = un2;
                //} else {
                    // Head was not clipped by the second edge
                    // Everything is set correctly
                }
            } else if( dt2 > 0.0 ) {
                // The first edge fully clipped the clip edge
                edge2 = e3;
                dl12 = dt1; dr12 = dh1;
                return CONTINUE_EE;
            } else if( dt1 > 0.0 ) {
                // The second edge fully clipped the clip edge
                edge2 = e4;
                dl12 = dt2; dr12 = dh2;
                return CONTINUE_EE;
            } else {
                // The head and the tail were both clipped out by both edges
                clipped_out = true;
            }
        } else {
            // The head was only clipped out by the first edge
            if( dt1 > 0.0 && dt2 > 0.0 ) {
                un1 = *ux * e3->Face_Normal();
            } else if( dt2 > 0.0 ) {
                // The tail was clipped out by only the first edge
                // Walk to the first edge
                edge2 = e3;
                dl12 = dt1; dr12 = dh1;
                return CONTINUE_EE;
            } else if( dt1 > 0.0 ) {
                // The tail was clipped out by only the second edge
                un1 = *ux * e3->Face_Normal();
                un2 = *ux * e4->Face_Normal();

                // Figure out if the whole edge was clipped out
                if( dt2 * un1 < dt1 * un2 ) {
                    // The entire edge was not clipped out

                    // Check for penetration
                    if( dh2 * unf < dhf * un2 ) {
                        if( dh1 * unf < dhf * un1 ) {
                            c1 = edge1->Classification( level0 );
                            c2 = face2->Classification();
                            return PENETRATION;
                        } else {
                            edge2 = e3;
                            dl12 = dt1; dr12 = dh1;
                            return CONTINUE_EE;
                        }
                    } else {
                        edge2 = e4;
                        dl12 = dt2; dr12 = dh2;
                        return CONTINUE_EE;
                    }
                } else {
                    // The entire edge was clipped out
                    clipped_out = true;
                }
            } else {
                // The tail was clipped out by both edges.  Go to the 1st edge.
                edge2 = e3;
                dl12 = dt1; dr12 = dh1;
                return CONTINUE_EE;
            }
        }
    } else {
        // Head was not clipped by the first edge
        if( dh2 < 0.0 ) {
            // Head was clipped by the second edge
            if( dt1 > 0.0 && dt2 > 0.0 ) {
                SWIFT_Tri_Edge* tempe = e3; e3 = e4; e4 = tempe;
                // Use un1 as a temp
                un1 = dt1; dt1 = dt2; dt2 = un1;
                un1 = dh1; dh1 = dh2; dh2 = un1;
                un1 = *ux * e3->Face_Normal();
            } else if( dt2 > 0.0 ) {
                // The tail was clipped out by only the first edge
                un1 = *ux * e3->Face_Normal();
                un2 = *ux * e4->Face_Normal();

                // Figure out if the whole edge was clipped out
                if( dt1 * un2 < dt2 * un1 ) {
                    // The entire edge was not clipped out.

                    // Check for penetration
                    if( dh1 * unf < dhf * un1 ) {
                        if( dh2 * unf < dhf * un2 ) {
                            c1 = edge1->Classification( level0 );
                            c2 = face2->Classification();
                            return PENETRATION;
                        } else {
                            edge2 = e4;
                            dl12 = dt2; dr12 = dh2;
                            return CONTINUE_EE;
                        }
                    } else {
                        edge2 = e3;
                        dl12 = dt1; dr12 = dh1;
                        return CONTINUE_EE;
                    }
                } else {
                    // The entire edge was clipped out
                    clipped_out = true;
                }
            } else if( dt1 > 0.0 ) {
                // The tail was clipped out by only the second edge
                // Walk to the second edge
                edge2 = e4;
                dl12 = dt2; dr12 = dh2;
                return CONTINUE_EE;
            } else {
                // The tail was clipped out by both edges.  Go to the 2nd edge.
                edge2 = e4;
                dl12 = dt2; dr12 = dh2;
                return CONTINUE_EE;
            }
        } else {
            // Head was not clipped at all.  It is not possible for the tail
            // to be clipped out by both edges.

            if( dt1 < 0.0 ) {
                // Tail clipped out by first edge only
                if( dhf > 0.0 ) {
                    // Nearest point to the face is the head of the edge
                    vert1 = edge1->Next()->Origin();
                    vedge1 = edge1->Twin( level0 );
                    return CONTINUE_VF;
                } else {
                    // Head is under the face
                    un1 = *ux * e3->Face_Normal();

                    // Check for penetration or go to the first edge
                    if( dh1 * unf < dhf * un1 ) {
                        c1 = edge1->Classification( level0 );
                        c2 = face2->Classification();
                        return PENETRATION;
                    }

                    // Walk to the first edge
                    edge2 = e3;
                    dl12 = dt1; dr12 = dh1;
                    return CONTINUE_EE;
                }
            } else if( dt2 < 0.0 ) {
                // Tail clipped out by second edge only
                if( dhf > 0.0 ) {
                    // Nearest point to the face is the head of the edge
                    vert1 = edge1->Next()->Origin();
                    vedge1 = edge1->Twin( level0 );
                    return CONTINUE_VF;
                } else {
                    // Head is under the face
                    un2 = *ux * e4->Face_Normal();

                    // Check for penetration or go to the second edge
                    if( dh2 * unf < dhf * un2 ) {
                        c1 = edge1->Classification( level0 );
                        c2 = face2->Classification();
                        return PENETRATION;
                    }

                    // Walk to the second edge
                    edge2 = e4;
                    dl12 = dt2; dr12 = dh2;
                    return CONTINUE_EE;
                }
#ifdef SWIFT_DEBUG
            } else if( dt1 < 0.0 && dt2 < 0.0 ) {
cerr << "********* Error: Head not clipped but tail clipped by both" << endl;
#endif
            } else {
                // Check for penetration
                dt1 = dr21 > 0.0 ? -face2->Distance(*tx) : face2->Distance(*tx);
                if( dhf < 0.0 && dt1 > 0.0 ) {
                    c1 = edge1->Classification( level0 );
                    c2 = face2->Classification();
                    return PENETRATION;
                }

                // Walk to head.
                vert1 = edge1->Next()->Origin();
                vedge1 = edge1->Twin( level0 );
                return CONTINUE_VF;
            }
        }
    }

    if( clipped_out ) {
        un1 = e4->Distance( *tx );
        un2 = e4->Distance( *hx );

        if( dt2 < 0.0 && un1 > 0.0 ) {
            if( un2 < 0.0 ) {
                if( e3->Twin( level1 )->Distance( *hx ) > 0.0 ) {
                    // Decide which edge to go to.
                    edge2 = ((*tx + (un1 / (un1-un2)) * (*ux) -
                            edge2->Origin()->Coords()) * (*ux) < 0.0) ? e3 : e4;
                } else {
                    edge2 = e4;
                }
            } else {
                edge2 = e4;
            }
        } else {
            if( dh2 < 0.0 && un2 > 0.0 ) {
                if( e3->Twin( level1 )->Distance( *tx ) > 0.0 ) {
                    // Decide which edge to go to.
                    edge2 = ((*tx + (un1 / (un1-un2)) * (*ux) -
                            edge2->Origin()->Coords()) * (*ux) > 0.0) ? e3 : e4;
                } else {
                    edge2 = e4;
                }
            } else {
                // Go to e3 or to the vertex but since the vertex is a
                // subset of e3 then we simply go to e3.
                edge2 = e3;
            }
        }

        if( edge2 == e3 ) {
            dl12 = dt1; dr12 = dh1;
        } else {
            dl12 = dt2; dr12 = dh2;
        }
        return CONTINUE_EE;
    }

    // Check for penetration.  For this second test, dhf < 0.0, dh1 < 0.0,
    // unf < 0.0, un1 > 0.0.  Check that the tail is not also under the face.
    un2 = face2->Distance( *tx );
    if( dhf < 0.0 && (dr21 > 0.0 && un2 < 0.0 || dr21 < 0.0 && un2 > 0.0) &&
        dh1 * unf < dhf * un1
    ) {
        // The face is pierced by the edge.  edge2 is set the original
        // edge across which we entered which is correct.
        //dist = dhf;
        c1 = edge1->Classification( level0 );
        c2 = face2->Classification();
        return PENETRATION;
    }

    // Walk to the edge whose edge-face plane clipped the head first
    edge2 = e3;
    dl12 = dt1; dr12 = dh1;

    return CONTINUE_EE;
}


///////////////////////////////////////////////////////////////////////////////
// Intersection and distance functions for a pair of convex polyhedra
///////////////////////////////////////////////////////////////////////////////

// Determines nearest features and intersection between two convex polyhedra.
// Walk to the nearest features.  Uses an improved LC closest features alg.
// If there is intersection returns true otherwise returns false.
bool Walk_Convex_LC( RESULT_TYPE start_state )
{
    // For looping and swapping
    int i;
#ifdef CYCLE_DETECTION
    // For cycle detection counting
    int j = 0, k = 0, l = 0;
    bool jittered_transformation = false;
#else
#ifdef SWIFT_DEBUG
    int r = 0;
#endif
#endif


    prev_state = DISJOINT;

    do {
        switch( state = start_state ) {
        case CONTINUE_VV:
            start_state = Vertex_Vertex();
            prev_state = CONTINUE_VV;
            break;
        case CONTINUE_EV:
            T10 = &trans01;
            T01 = &trans10;
            { SWIFT_Triple* temp_xp = t1xp; t1xp = t2xp; t2xp = temp_xp;
              temp_xp = h1xp; h1xp = h2xp; h2xp = temp_xp;
              temp_xp = u1xp; u1xp = u2xp; u2xp = temp_xp; }
            e2 = e1;
            ve1 = ve2;
            i = level0; level0 = level1; level1 = i;
            v1 = v2;
        case CONTINUE_VE:
            start_state = Vertex_Edge();
            prev_state = state;

            if( state == CONTINUE_EV ) {
                // Unswap the results
                SWIFT_Triple* temp_xp = t1xp; t1xp = t2xp; t2xp = temp_xp;
                temp_xp = h1xp; h1xp = h2xp; h2xp = temp_xp;
                temp_xp = u1xp; u1xp = u2xp; u2xp = temp_xp;
                i = level0; level0 = level1; level1 = i;
                switch( start_state ) {
                case CONTINUE_VV:
                    { SWIFT_Tri_Vertex* tempv = v1; v1 = v2; v2 = tempv;
                      SWIFT_Tri_Edge* tempe = ve1; ve1 = ve2; ve2 = tempe;
                    }
                    break;
                case CONTINUE_EE:
                    { SWIFT_Tri_Edge* tempe = e1; e1 = e2; e2 = tempe;
                    }
                    break;
                case CONTINUE_VF:
                    start_state = CONTINUE_FV;
                    f1 = f2;
                    ve2 = ve1;
                    v2 = v1;
                    break;
                default: // DISJOINT
                    // Swap the edge back (vertex is ok) since Vertex_Edge
                    // may have set e1
                    e1 = e2;
                    i = c1; c1 = c2; c2 = i;
                    break;
                }
                T01 = &trans01;
                T10 = &trans10;
            }
            break;
        case CONTINUE_FV:
            T10 = &trans01;
            T01 = &trans10;
            { SWIFT_Triple* temp_xp = t1xp; t1xp = t2xp; t2xp = temp_xp;
              temp_xp = h1xp; h1xp = h2xp; h2xp = temp_xp;
              temp_xp = u1xp; u1xp = u2xp; u2xp = temp_xp; }
            f2 = f1;
            ve1 = ve2;
            i = level0; level0 = level1; level1 = i;
            v1 = v2;
        case CONTINUE_VF:
            start_state = Vertex_Face();
            prev_state = state;

            if( state == CONTINUE_FV ) {
                // Unswap the results
                SWIFT_Triple* temp_xp = t1xp; t1xp = t2xp; t2xp = temp_xp;
                temp_xp = h1xp; h1xp = h2xp; h2xp = temp_xp;
                temp_xp = u1xp; u1xp = u2xp; u2xp = temp_xp;
                i = level0; level0 = level1; level1 = i;
                switch( start_state ) {
                case CONTINUE_VV:
                    { SWIFT_Tri_Vertex* tempv = v1; v1 = v2; v2 = tempv;
                      SWIFT_Tri_Edge* tempe = ve1; ve1 = ve2; ve2 = tempe;
                    }
                    break;
                case CONTINUE_VE:
                    start_state = CONTINUE_EV;
                    e1 = e2;
                    ve2 = ve1;
                    v2 = v1;
                    break;
                case CONTINUE_EE:
                    { SWIFT_Tri_Edge* tempe = e1; e1 = e2; e2 = tempe;
                    }
                    break;
                case CONTINUE_VF:
                    start_state = CONTINUE_FV;
                    f1 = f2;
                    ve2 = ve1;
                    v2 = v1;
                    break;
                case PENETRATION:
                    f1 = f2;
                    ve2 = ve1;
                    i = c1; c1 = c2; c2 = i;
                    v2 = v1;
                    break;
                case LOCAL_MINIMUM:
                  { // Handle the local minimum
                    v2 = v1;
                    ve2 = ve1;
                    SWIFT_Real dist2;
                    dist = -SWIFT_INFINITY;
                    for( i = 0; i < bv0->Num_Faces(); i++ ) {
                        dist2 = bv0->Faces()[i].Distance( *t2xp );
                        if( dist2 > dist ) {
                            dist = dist2;
                            f1 = bv0->Faces()(i);
                        }
                    }
                    for( i = 0; i < bv0->Num_Other_Faces(); i++ ) {
                        dist2 = bv0->Other_Faces()[i]->Distance( *t2xp );
                        if( dist2 > dist ) {
                            dist = dist2;
                            f1 = bv0->Other_Faces()[i];
                        }
                    }
                    // Check for penetration
                    if( dist < 0.0 ) {
                        start_state = PENETRATION;
                        if( bv0->Is_Leaf() ) {
                            c1 = CLASS_ORIGINAL;
                            c2 = CLASS_ORIGINAL;
                        } else if( bv1->Is_Leaf() ) {
                            c1 = CLASS_FREE;
                            c2 = CLASS_ORIGINAL;
                        } else {
                            c1 = CLASS_FREE;
                            c2 = CLASS_FREE;
                        }
                    } else {
                        prev_state = LOCAL_MINIMUM;
                        start_state = CONTINUE_FV;
                    }
                  } break;
                default: // DISJOINT
                    i = c1; c1 = c2; c2 = i;
                    break;
                }
                T01 = &trans01;
                T10 = &trans10;
            } else if( start_state == PENETRATION ) {
            } else if( start_state == LOCAL_MINIMUM ) {
                // Handle the local minimum
                SWIFT_Real dist2;
                dist = -SWIFT_INFINITY;
                for( i = 0; i < bv1->Num_Faces(); i++ ) {
                    dist2 = bv1->Faces()[i].Distance( *t1xp );
                    if( dist2 > dist ) {
                        dist = dist2;
                        f2 = bv1->Faces()(i);
                    }
                }
                for( i = 0; i < bv1->Num_Other_Faces(); i++ ) {
                    dist2 = bv1->Other_Faces()[i]->Distance( *t1xp );
                    if( dist2 > dist ) {
                        dist = dist2;
                        f2 = bv1->Other_Faces()[i];
                    }
                }
                // Check for penetration
                if( dist < 0.0 ) {
                    start_state = PENETRATION;
                    if( bv1->Is_Leaf() ) {
                        c1 = CLASS_ORIGINAL;
                        c2 = CLASS_ORIGINAL;
                    } else if( bv0->Is_Leaf() ) {
                        c1 = CLASS_ORIGINAL;
                        c2 = CLASS_FREE;
                    } else {
                        c1 = CLASS_FREE;
                        c2 = CLASS_FREE;
                    }
                } else {
                    prev_state = LOCAL_MINIMUM;
                    start_state = CONTINUE_VF;
                }
            }
            break;
        case CONTINUE_EE:
            // It is the responsibility of Edge_Edge() to set prev_state.
            start_state = Edge_Edge();

            break;
        default:
            break;
        }

#ifdef CYCLE_DETECTION
        // Handle cycles
        if( ++k >= STATE_TRANS_CYCLE_DECL ) {
#ifdef SWIFT_DEBUG
            if( k == STATE_TRANS_CYCLE_DECL ) {
                cycle_counter++;
                cerr << "****************** Entered Cycle Detection : "
                     << "counter = " << cycle_counter << endl;
            }
#endif
            // We may actually be done the query
            if( start_state == DISJOINT || start_state == PENETRATION ) {
                break;
            }

            // We must do some checking for cycles
            if( k == STATE_TRANS_CYCLE_DECL ) {
                // First time in the detector for this potential cycle.
                // Initialize the cycle detector.
                for( j = 0; j < cycle_detector_feats.Length(); j++ ) {
                    cycle_detector_feats[j] = NULL;
                }
                j = 0;
            }

            void* cf0; void* cf1;
            switch( start_state ) {
            case CONTINUE_VV: cf0 = (void*)v1; cf1 = (void*)v2; break;
            case CONTINUE_VE: cf0 = (void*)v1; cf1 = (void*)e2; break;
            case CONTINUE_VF: cf0 = (void*)v1; cf1 = (void*)f2; break;
            case CONTINUE_EV: cf0 = (void*)e1; cf1 = (void*)v2; break;
            case CONTINUE_EE: cf0 = (void*)e1; cf1 = (void*)e2; break;
            case CONTINUE_FV: cf0 = (void*)f1; cf1 = (void*)v2; break;
            default: break;
            }
            for( i = 0; i < cycle_detector_feats.Length() &&
                (cf0 != cycle_detector_feats[i] ||
                 cf1 != cycle_detector_feats[i+1]); i += 2 );
            if( i != cycle_detector_feats.Length() ) {
                // Found a cycle
#ifdef SWIFT_DEBUG
                cerr << "***************** Found a cycle" << endl;
#endif

                // There was a cycle.  If there have been the maximum number
                // of cycles detected, then just return disjoint.
                if( ++l >= MAX_CYCLE_TRANSFORMATIONS ) {
                    // We have jittered the transformation too many times.
                    // Try initializing to a random pair and start over.
                    k = 0; l = 0;
                    Initialize_Randomly( start_state );
                    Reset_Transformation();
                    jittered_transformation = false;
                } else {
                    // Jitter the transformation and try to proceed.
                    Jitter_Transformation( k );

                    jittered_transformation = true;
                    // Do not reset k since we are close to the answer.
                }
            } else {
                // There was no cycle detected.  Add the pair to the detector.
                cycle_detector_feats[j<<1] = cf0;
                cycle_detector_feats[(j<<1)+1] = cf1;
                j = (j+1) % CYCLE_LENGTH;
            }
        }
#else
#ifdef SWIFT_DEBUG
        if( r++ == STATE_TRANS_CYCLE_DECL ) {
            cerr << "SWIFT infinite loop detected: Exiting..." << endl;
            exit( -1 );
        }
#endif
#endif
    } while( start_state != DISJOINT && start_state != PENETRATION );

    if( jittered_transformation ) {
        Reset_Transformation();
    }
    return (start_state == PENETRATION);
}

inline void Distance_After_Walk_Convex_LC( SWIFT_Real& distance )
{
    // Compute the distance of the objects based on the set globals from walk
    if( state == CONTINUE_FV || state == CONTINUE_VF ) {
        // The distance is stored in the dist variable
        distance = dist;
    } else if( state == CONTINUE_EE ) {
        // The fdir vector is not set at all
        SWIFT_Triple tri1, tri2;
        Compute_Closest_Points_Edge_Edge( e1, e2, tri1, tri2, trans01,
                                          &distance );
    } else {
        // The fdir vector spans the nearest points
        distance = fdir.Length();
    }
}

inline bool Distance_Convex_LC( RESULT_TYPE start_state, SWIFT_Real& distance )
{
    if( Walk_Convex_LC( start_state ) ) {
        // There is penetration
        distance = -1.0;
        return true;
    }

    Distance_After_Walk_Convex_LC( distance );

    return false;

}


///////////////////////////////////////////////////////////////////////////////
// Global Query functions
///////////////////////////////////////////////////////////////////////////////

inline void Add_To_Contact_List( SWIFT_Real distance )
{
    contact_listd.Add_Grow( distance, 10 );
    contact_listt.Add_Grow( state, 10 );
    contact_listbv0.Add_Grow( bv0, 10 );
    contact_listbv1.Add_Grow( bv1, 10 );
    switch( state ) {
    case CONTINUE_VV:
        contact_list0.Add_Grow( (void*) ve1, 10 );
        contact_list1.Add_Grow( (void*) ve2, 10 );
        break;
    case CONTINUE_VE:
        contact_list0.Add_Grow( (void*) ve1, 10 );
        contact_list1.Add_Grow( (void*) e2, 10 );
        break;
    case CONTINUE_EV:
        contact_list0.Add_Grow( (void*) e1, 10 );
        contact_list1.Add_Grow( (void*) ve2, 10 );
        break;
    case CONTINUE_VF:
        contact_list0.Add_Grow( (void*) ve1, 10 );
        contact_list1.Add_Grow( (void*) f2, 10 );
        break;
    case CONTINUE_FV:
        contact_list0.Add_Grow( (void*) f1, 10 );
        contact_list1.Add_Grow( (void*) ve2, 10 );
        break;
    case CONTINUE_EE:
        contact_list0.Add_Grow( (void*) e1, 10 );
        contact_list1.Add_Grow( (void*) e2, 10 );
    default:
        break;
    }
}

inline void Extract_Features( RESULT_TYPE state, void*& feat0, void*& feat1 )
{
    switch( state ) {
    case CONTINUE_VV:
        feat0 = (void*)ve1;
        feat1 = (void*)ve2;
        break;
    case CONTINUE_VE:
        feat0 = (void*)ve1;
        feat1 = (void*)e2;
        break;
    case CONTINUE_EV:
        feat0 = (void*)e1;
        feat1 = (void*)ve2;
        break;
    case CONTINUE_VF:
        feat0 = (void*)ve1;
        feat1 = (void*)f2;
        break;
    case CONTINUE_FV:
        feat0 = (void*)f1;
        feat1 = (void*)ve2;
        break;
    case CONTINUE_EE:
        feat0 = (void*)e1;
        feat1 = (void*)e2;
    default:
        break;
    }
}

#ifdef SWIFT_FRONT_TRACKING
inline void Save_State( SWIFT_Front_Node* node )
{
    // Save the features
    void* feat0;
    void* feat1;
    Extract_Features( state, feat0, feat1 );

    // Save the state id
    node->Set_Feature0( feat0 );
    node->Set_Feature1( feat1 );
    node->Set_State( state );
}
#endif

// returns intersection: true or false
#ifdef SWIFT_FRONT_TRACKING
bool Tolerance_LC( RESULT_TYPE start_state, SWIFT_Real tolerance,
                   SWIFT_Front_Node* node, SWIFT_Front_Node*& parent )
#else
bool Tolerance_LC( RESULT_TYPE start_state, SWIFT_Real tolerance )
#endif
{
    bool result = Walk_Convex_LC( start_state );
    if( tolerance != 0.0 ) {
        SWIFT_Real dist;
        Distance_After_Walk_Convex_LC( dist );
        result = result || dist < tolerance;
    }

#ifdef SWIFT_FRONT_TRACKING
    SWIFT_Front_Node* new_node;
    if( node != NULL ) {
        // This is the top level node (first call) and parent == node
        Save_State( node );
        node->Set_Close( result );
        new_node = node;
    } else {
        // There is a parent -- check if this pair intersected and try to
        // expand the front
        if( result ) {
            // There was intersection so we need to create a new pair in the
            // front tree
            new_node = new SWIFT_Front_Node;
            Save_State( new_node );
            new_node->Set_Parent( parent );
            new_node->Set_BV0( bv0 );
            new_node->Set_BV1( bv1 );
            parent = new_node;
        } else {
            parent = NULL;
        }
    }
#else
    if( !saved ) {
        // Save the features if this is the top level node
        saved = true;
        save_v1 = v1;
        save_v2 = v2;
        save_ve1 = ve1;
        save_ve2 = ve2;
        save_e1 = e1;
        save_e2 = e2;
        save_f1 = f1;
        save_f2 = f2;
        save_state = state;
    }
#endif

    if( result ) {

        if( c1 == CLASS_FREE ) {
            // Save the bv pointers
            SWIFT_BV* save_bv0 = bv0;
            SWIFT_BV* save_bv1 = bv1;

            // Go down on bv0
            SWIFT_Triple st = trans10 * save_bv1->Center_Of_Mass();
            bv0 = save_bv0->Children()[0]; bv1 = save_bv1;
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv0->Center_Of_Mass();
            Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( Tolerance_LC( start_state, tolerance, NULL, new_node ) ) {
                // Link in the subtree and the non-visited subtree
                parent->Set_Left_Child( new_node );
                new_node = new SWIFT_Front_Node;
                new_node->Set_Parent( parent );
                new_node->Set_BV0( save_bv0->Children()[1] );
                new_node->Set_BV1( save_bv1 );
                new_node->Set_Uninitialized();
                parent->Set_Right_Child( new_node );
                pairfront->Count_Drop();
                return true;
            }
#else
            if( Tolerance_LC( start_state, tolerance ) ) {
                return true;
            }
#endif
            bv0 = save_bv0->Children()[1]; bv1 = save_bv1;
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv0->Center_Of_Mass();
            Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
            parent->Set_Left_Child( new_node );
            new_node = parent;
            result = Tolerance_LC( start_state, tolerance, NULL, new_node );
            parent->Set_Right_Child( new_node );
            if( parent->Left_Child() == NULL ) {
                if( parent->Right_Child() != NULL ) {
                    // Have to create a left child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0->Children()[0] );
                    new_node->Set_BV1( save_bv1 );
                    new_node->Set_Uninitialized();
                    parent->Set_Left_Child( new_node );
                    pairfront->Count_Drop();
                }
            } else {
                if( parent->Right_Child() == NULL ) {
                    // Have to create a right child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0->Children()[1] );
                    new_node->Set_BV1( save_bv1 );
                    new_node->Set_Uninitialized();
                    parent->Set_Right_Child( new_node );
                }
                pairfront->Count_Drop();
            }
#else
            result = Tolerance_LC( start_state, tolerance );
#endif
            return result;
        } else if( c2 == CLASS_FREE ) {
            // Save the bv pointers
            SWIFT_BV* save_bv0 = bv0;
            SWIFT_BV* save_bv1 = bv1;

            // Go down on bv1
            SWIFT_Triple st = trans01 * save_bv0->Center_Of_Mass();
            bv0 = save_bv0; bv1 = save_bv1->Children()[0];
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv1->Center_Of_Mass();
            Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( Tolerance_LC( start_state, tolerance, NULL, new_node ) ) {
                // Link in the subtree and the non-visited subtree
                parent->Set_Left_Child( new_node );
                new_node = new SWIFT_Front_Node;
                new_node->Set_Parent( parent );
                new_node->Set_BV0( save_bv0 );
                new_node->Set_BV1( save_bv1->Children()[1] );
                new_node->Set_Uninitialized();
                parent->Set_Right_Child( new_node );
                pairfront->Count_Drop();
                return true;
            }
#else
            if( Tolerance_LC( start_state, tolerance ) ) {
                return true;
            }
#endif
            bv0 = save_bv0; bv1 = save_bv1->Children()[1];
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv1->Center_Of_Mass();
            Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
            parent->Set_Left_Child( new_node );
            new_node = parent;
            result = Tolerance_LC( start_state, tolerance, NULL, new_node );
            parent->Set_Right_Child( new_node );
            if( parent->Left_Child() == NULL ) {
                if( parent->Right_Child() != NULL ) {
                    // Have to create a left child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0 );
                    new_node->Set_BV1( save_bv1->Children()[0] );
                    new_node->Set_Uninitialized();
                    parent->Set_Left_Child( new_node );
                    pairfront->Count_Drop();
                }
            } else {
                if( parent->Right_Child() == NULL ) {
                    // Have to create a right child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0 );
                    new_node->Set_BV1( save_bv1->Children()[1] );
                    new_node->Set_Uninitialized();
                    parent->Set_Right_Child( new_node );
                }
                pairfront->Count_Drop();
            }
#else
            result = Tolerance_LC( start_state, tolerance );
#endif
            return result;
        } else {
            return true;
        }
    }
    return false;
}

// returns intersection: true or false
bool Distance_LC( RESULT_TYPE start_state,
                  SWIFT_Real tolerance, SWIFT_Real abs_error,
                  SWIFT_Real rel_error, SWIFT_Real& distance,
#ifdef SWIFT_FRONT_TRACKING
                  SWIFT_Front_Node* node, SWIFT_Front_Node*& parent,
#endif
                  bool contacts
                )
{

    SWIFT_Real loc_dist;
    bool result = Distance_Convex_LC( start_state, loc_dist );
    const bool recur = result || (loc_dist < tolerance &&
          (loc_dist < distance - abs_error || loc_dist * rel_error < distance));

#ifdef SWIFT_FRONT_TRACKING
    SWIFT_Front_Node* new_node;
    if( node != NULL ) {
        // This is the top level node (first call) and parent == node
        Save_State( node );
        node->Set_Close( result );
        new_node = node;
    } else {
        // There is a parent -- check if this pair intersected and try to
        // expand the front
        if( result ) {
            // There was intersection so we need to create a new pair in the
            // front tree.  Note that we do not have to set Close() here since
            // this node will not have an attempted raise on it.
            new_node = new SWIFT_Front_Node;
            Save_State( new_node );
            new_node->Set_Parent( parent );
            new_node->Set_BV0( bv0 );
            new_node->Set_BV1( bv1 );
            parent = new_node;
        } else {
            parent = NULL;
        }
    }
#else
    if( !saved ) {
        // Save the features if this is the top level node
        saved = true;
        save_v1 = v1;
        save_v2 = v2;
        save_ve1 = ve1;
        save_ve2 = ve2;
        save_e1 = e1;
        save_e2 = e2;
        save_f1 = f1;
        save_f2 = f2;
        save_state = state;
    }
#endif

    if( recur ) {
        // There is recursion
        if( c1 == CLASS_FREE ) {
            // Save the bv pointers
            SWIFT_BV* save_bv0 = bv0;
            SWIFT_BV* save_bv1 = bv1;

            // Go down on bv0 and compute the distances
            SWIFT_Triple st = trans10 * save_bv1->Center_Of_Mass();
            loc_dist = distance;
            bv0 = save_bv0->Children()[0]; bv1 = save_bv1;
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv0->Center_Of_Mass();
            Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                             loc_dist, NULL, new_node, contacts )
            ) {
                // Link in the subtree and the non-visited subtree.  This might
                // only happen when parent != NULL (intersection at prev level)
                parent->Set_Left_Child( new_node );
                new_node = new SWIFT_Front_Node;
                new_node->Set_Parent( parent );
                new_node->Set_BV0( save_bv0->Children()[1] );
                new_node->Set_BV1( save_bv1 );
                new_node->Set_Uninitialized();
                parent->Set_Right_Child( new_node );
                pairfront->Count_Drop();
                distance = -1.0;
                return true;
            }
#else
            if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                             loc_dist, contacts )
            ) {
                distance = -1.0;
                return true;
            }
#endif
            distance = min( distance, loc_dist );
            loc_dist = distance;
            bv0 = save_bv0->Children()[1]; bv1 = save_bv1;
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv0->Center_Of_Mass();
            Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( parent != NULL ) {
                parent->Set_Left_Child( new_node );
                new_node = parent;
                result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node, contacts );
                parent->Set_Right_Child( new_node );
                if( parent->Left_Child() == NULL ) {
                    if( parent->Right_Child() != NULL ) {
                        // Have to create a left child
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( parent );
                        new_node->Set_BV0( save_bv0->Children()[0] );
                        new_node->Set_BV1( save_bv1 );
                        new_node->Set_Uninitialized();
                        parent->Set_Left_Child( new_node );
                        pairfront->Count_Drop();
                    }
                } else {
                    if( parent->Right_Child() == NULL ) {
                        // Have to create a right child
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( parent );
                        new_node->Set_BV0( save_bv0->Children()[1] );
                        new_node->Set_BV1( save_bv1 );
                        new_node->Set_Uninitialized();
                        parent->Set_Right_Child( new_node );
                    }
                    pairfront->Count_Drop();
                }
#ifdef SWIFT_DEBUG
} else if( new_node != NULL ) {
cerr << "!!!!!!!!!!!!!!!! new node is null !!!!!!!!!!!!!!!!!" << endl;
#endif
            } else {
                result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node, contacts );
            }
#else
            result = Distance_LC( start_state, tolerance, abs_error, rel_error,
                                  loc_dist, contacts );
#endif
            distance = min( distance, loc_dist );
            return result;
        } else if( c2 == CLASS_FREE ) {
            // Save the bv pointers
            SWIFT_BV* save_bv0 = bv0;
            SWIFT_BV* save_bv1 = bv1;

            // Go down on bv1 and compute the distances
            SWIFT_Triple st = trans01 * save_bv0->Center_Of_Mass();
            loc_dist = distance;
            bv0 = save_bv0; bv1 = save_bv1->Children()[0];
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv1->Center_Of_Mass();
            Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                             loc_dist, NULL, new_node, contacts )
            ) {
                // Link in the subtree and the non-visited subtree.  This might
                // only happen when parent != NULL (intersection at prev level)
                parent->Set_Left_Child( new_node );
                new_node = new SWIFT_Front_Node;
                new_node->Set_Parent( parent );
                new_node->Set_BV0( save_bv0 );
                new_node->Set_BV1( save_bv1->Children()[1] );
                new_node->Set_Uninitialized();
                parent->Set_Right_Child( new_node );
                pairfront->Count_Drop();
                distance = -1.0;
                return true;
            }
#else
            if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                             loc_dist, contacts )
            ) {
                distance = -1.0;
                return true;
            }
#endif
            distance = min( distance, loc_dist );
            loc_dist = distance;
            bv0 = save_bv0; bv1 = save_bv1->Children()[1];
            level0 = bv0->Level(); level1 = bv1->Level();
            // Randomly initialize the features
            fdir = st - bv1->Center_Of_Mass();
            Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
            if( parent != NULL ) {
                parent->Set_Left_Child( new_node );
                new_node = parent;
                result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node, contacts );
                parent->Set_Right_Child( new_node );
                if( parent->Left_Child() == NULL ) {
                    if( parent->Right_Child() != NULL ) {
                        // Have to create a left child
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( parent );
                        new_node->Set_BV0( save_bv0 );
                        new_node->Set_BV1( save_bv1->Children()[0] );
                        new_node->Set_Uninitialized();
                        parent->Set_Left_Child( new_node );
                        pairfront->Count_Drop();
                    }
                } else {
                    if( parent->Right_Child() == NULL ) {
                        // Have to create a right child
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( parent );
                        new_node->Set_BV0( save_bv0 );
                        new_node->Set_BV1( save_bv1->Children()[1] );
                        new_node->Set_Uninitialized();
                        parent->Set_Right_Child( new_node );
                    }
                    pairfront->Count_Drop();
                }
#ifdef SWIFT_DEBUG
} else if( new_node != NULL ) {
cerr << "!!!!!!!!!!!!!!!! new node is null !!!!!!!!!!!!!!!!!" << endl;
#endif
            } else {
                result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node, contacts );
            }
#else
            result = Distance_LC( start_state, tolerance, abs_error, rel_error,
                                  loc_dist, contacts );
#endif
            distance = min( distance, loc_dist );
            return result;
        } else {
#ifndef SWIFT_PIECE_CACHING
            distance = min( distance, loc_dist );
#endif
            if( contacts ) {
#ifdef SWIFT_PIECE_CACHING
                distance = min( distance, loc_dist );
#endif
                if( !result ) {
                    // This is the closest pair of features that there can ever
                    // be on these two subtrees.  Add the distance etc. to the
                    // reporting lists and early exit.
                    Add_To_Contact_List( distance );
                }
#ifdef SWIFT_PIECE_CACHING
            } else {
                if( loc_dist < distance ) {
                    pair->Save_To_Cache_State();
                    distance = loc_dist;
                }
#endif
            }
            return result;
        }
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////
// SWIFT_Pair public functions
///////////////////////////////////////////////////////////////////////////////

bool SWIFT_Pair::Tolerance( SWIFT_Object* o0, SWIFT_Object* o1,
                            SWIFT_Real tolerance )
{
    RESULT_TYPE start_state;
    bool result;

    // Setup the pair specific query stuff
    result = false;
    Setup_Pair_Query( o0, o1,
                      false );

#ifdef SWIFT_PIECE_CACHING
    // Try to compute a valid distance on this cached bv
    pair = this;
    if( Cache_BV0() != NULL && (obj0->Mesh()->Root() != Cache_BV0() ||
                                obj1->Mesh()->Root() != Cache_BV1())
    ) {
        bv0 = Cache_BV0(); bv1 = Cache_BV1();
        Setup_BV_Query( cache_state, cache_feat0, cache_feat1 );
        result = Walk_Convex_LC( cache_state );
        if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
            if( result ) {
                Save_To_Cache_State();
                return true;
            } else {
                SWIFT_Real loc_dist;
                Distance_After_Walk_Convex_LC( loc_dist );
                if( loc_dist < tolerance ) {
                    Save_To_Cache_State();
                    return true;
                }
            }
        }
    }
    // Did not find intersection for the cached bv.  There is no longer a
    // cached bv.  One may be instated later.
    Set_Cache_BV0( NULL );
#endif

#ifdef SWIFT_FRONT_TRACKING
    // Start of the front tracking loop

    // Perform a BFS on the front tree and gather the leaves, then process them
    // from deepest to shallowest and process the marked leaf first.
    int i;
    front.Alloc_Queue();
    front.Gather_Leaves();
    pairfront = &front;
     //<< " leaves in the front tree" << endl;
    for( i = front.First_Leaf(); i <= front.Last_Leaf(); i++ ) {
        SWIFT_Front_Node* leaf = front.Leaves()[i];
        bv0 = leaf->BV0(); bv1 = leaf->BV1();
        start_state = leaf->State();
        Setup_BV_Query( start_state, leaf->Feature0(), leaf->Feature1() );
#else
    // Setup the bv specific query stuff
    bv0 = o0->Mesh()->Root(); bv1 = o1->Mesh()->Root();
#endif
#ifndef SWIFT_FRONT_TRACKING
    start_state = State();
    Setup_BV_Query( start_state, feat0, feat1 );
#endif

// ------------------------- Start tolerance call -------------------------
#ifdef SWIFT_FRONT_TRACKING
    result = Tolerance_LC( start_state, tolerance, leaf, leaf );
#else
    result = Tolerance_LC( start_state, tolerance );
#endif
// ------------------------- End tolerance call -------------------------

#ifdef SWIFT_FRONT_TRACKING
        // End of the front tracking loop
        if( result ) {
            // There was intersection
            break;
        }

        if( leaf->Is_Right_Child() && leaf->Left_Sibling()->Is_Leaf() &&
            !leaf->Close() && !leaf->Left_Sibling()->Close()
        ) {
            // Raise
            delete leaf->Left_Sibling();
            leaf->Parent()->Make_Leaf();
            leaf->Parent()->Set_Uninitialized();
            delete leaf;
            front.Count_Raise();
        }
    }
#endif


#ifndef SWIFT_FRONT_TRACKING
    Save_State();
#endif

    return result;
}


#ifdef SWIFT_PRIORITY_DIRECTION
bool SWIFT_Pair::Handle_Full_Queue( SWIFT_Real tolerance, SWIFT_Real abs_error,
                                    SWIFT_Real rel_error, SWIFT_Real& distance
#ifdef SWIFT_FRONT_TRACKING
                                    , SWIFT_Front_Node* parent
#endif 
                                    )
{
    bool result;
    RESULT_TYPE start_state;
    SWIFT_Real loc_dist = distance;
    SWIFT_BV* save_bv0 = bv0;
    SWIFT_BV* save_bv1 = bv1;
#ifdef SWIFT_FRONT_TRACKING
    SWIFT_Front_Node* new_node;
#endif

    if( c1 == CLASS_FREE ) {
        // Go down on bv0
        SWIFT_Triple st = trans10 * save_bv1->Center_Of_Mass();
        bv0 = save_bv0->Children()[0]; bv1 = save_bv1;
        level0 = bv0->Level(); level1 = bv1->Level();
        // Randomly initialize the features
        fdir = st - bv0->Center_Of_Mass();
        Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
        new_node = parent;
        if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                                                 loc_dist, NULL, new_node )
        ) {
            // Link in the subtree and the non-visited subtree.  This might
            // only happen when parent != NULL (intersection at prev level)
            parent->Set_Left_Child( new_node );
            new_node = new SWIFT_Front_Node;
            new_node->Set_Parent( parent );
            new_node->Set_BV0( save_bv0->Children()[1] );
            new_node->Set_BV1( save_bv1 );
            new_node->Set_Uninitialized();
            parent->Set_Right_Child( new_node );
            front.Count_Drop();
            distance = -1.0;
            return true;
        }
#else
        if( Distance_LC( start_state, tolerance, abs_error, rel_error, loc_dist
        ) ) {
            distance = -1.0;
            return true;
        }
#endif

        distance = min( distance, loc_dist );
        loc_dist = distance;
        bv0 = save_bv0->Children()[1]; bv1 = save_bv1;
        level0 = bv0->Level(); level1 = bv1->Level();
        // Randomly initialize the features
        fdir = st - bv0->Center_Of_Mass();
        Initialize_From_Scratch( start_state );
#ifdef SWIFT_FRONT_TRACKING
        if( parent != NULL ) {
            parent->Set_Left_Child( new_node );
            new_node = parent;
        }
        result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node );
        if( parent != NULL ) {
            parent->Set_Right_Child( new_node );
            if( parent->Left_Child() == NULL ) {
                if( parent->Right_Child() != NULL ) {
                    // Have to create a left child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0->Children()[0] );
                    new_node->Set_BV1( save_bv1 );
                    new_node->Set_Uninitialized();
                    parent->Set_Left_Child( new_node );
                    front.Count_Drop();
                }
            } else {
                if( parent->Right_Child() == NULL ) {
                    // Have to create a right child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0->Children()[1] );
                    new_node->Set_BV1( save_bv1 );
                    new_node->Set_Uninitialized();
                    parent->Set_Right_Child( new_node );
                }
                front.Count_Drop();
            }
        }
#else
        result = Distance_LC( start_state, tolerance, abs_error, rel_error,
                                                                 loc_dist );
#endif
        distance = min( distance, loc_dist );
    } else {
        // Go down on bv1
        SWIFT_Triple st = trans01 * save_bv0->Center_Of_Mass();
        bv0 = save_bv0; bv1 = save_bv1->Children()[0];
        level0 = bv0->Level(); level1 = bv1->Level();
        // Randomly initialize the features
        fdir = st - bv1->Center_Of_Mass();
        Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
        new_node = parent;
        if( Distance_LC( start_state, tolerance, abs_error, rel_error,
                                                 loc_dist, NULL, new_node )
        ) {
            // Link in the subtree and the non-visited subtree.  This might
            // only happen when parent != NULL (intersection at prev level)
            parent->Set_Left_Child( new_node );
            new_node = new SWIFT_Front_Node;
            new_node->Set_Parent( parent );
            new_node->Set_BV0( save_bv0 );
            new_node->Set_BV1( save_bv1->Children()[1] );
            new_node->Set_Uninitialized();
            parent->Set_Right_Child( new_node );
            front.Count_Drop();
            distance = -1.0;
            return true;
        }
#else
        if( Distance_LC( start_state, tolerance, abs_error, rel_error, loc_dist
        ) ) {
            distance = -1.0;
            return true;
        }
#endif

        loc_dist = distance;
        bv0 = save_bv0; bv1 = save_bv1->Children()[1];
        level0 = bv0->Level(); level1 = bv1->Level();
        // Randomly initialize the features
        fdir = st - bv1->Center_Of_Mass();
        Initialize_From_Scratch2( start_state );
#ifdef SWIFT_FRONT_TRACKING
        if( parent != NULL ) {
            parent->Set_Left_Child( new_node );
            new_node = parent;
        }
        result = Distance_LC( start_state, tolerance, abs_error,
                              rel_error, loc_dist, NULL, new_node );
        if( parent != NULL ) {
            parent->Set_Right_Child( new_node );
            if( parent->Left_Child() == NULL ) {
                if( parent->Right_Child() != NULL ) {
                    // Have to create a left child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0 );
                    new_node->Set_BV1( save_bv1->Children()[0] );
                    new_node->Set_Uninitialized();
                    parent->Set_Left_Child( new_node );
                    front.Count_Drop();
                }
            } else {
                if( parent->Right_Child() == NULL ) {
                    // Have to create a right child
                    new_node = new SWIFT_Front_Node;
                    new_node->Set_Parent( parent );
                    new_node->Set_BV0( save_bv0 );
                    new_node->Set_BV1( save_bv1->Children()[1] );
                    new_node->Set_Uninitialized();
                    parent->Set_Right_Child( new_node );
                }
                front.Count_Drop();
            }
        }
#else
        result = Distance_LC( start_state, tolerance, abs_error, rel_error,
                                                                 loc_dist );
#endif
        distance = min( distance, loc_dist );
    }
    return result;
}
#endif

bool SWIFT_Pair::Distance( SWIFT_Object* o0, SWIFT_Object* o1,
                           SWIFT_Real tolerance,
                           SWIFT_Real abs_error, SWIFT_Real rel_error,
                           SWIFT_Real& distance )
{
    RESULT_TYPE start_state;
    bool result;

    // Setup the pair specific query stuff
    distance = SWIFT_INFINITY;
    result = false;
    rel_error += 1.0;
    Setup_Pair_Query( o0, o1,
                      false );

#ifdef SWIFT_PIECE_CACHING
    // Try to compute a valid distance on this cached bv
    pair = this;
    if( cache_bv0 != NULL && (obj0->Mesh()->Root() != cache_bv0 ||
                              obj1->Mesh()->Root() != cache_bv1)
    ) {
        bv0 = cache_bv0; bv1 = cache_bv1;
        Setup_BV_Query( cache_state, cache_feat0, cache_feat1 );
        result = Walk_Convex_LC( cache_state );
        if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
            if( result ) {
                Save_To_Cache_State();
                distance = -1.0;
                return true;
            } else {
                Distance_After_Walk_Convex_LC( distance );
            }
        }
    }
#endif
#ifdef SWIFT_PRIORITY_DIRECTION
#ifdef SWIFT_PIECE_CACHING
    if( distance != -1.0 ) {
#endif
        int i;
        SWIFT_Real loc_dist;
        pqueue.Reset();

#ifdef SWIFT_FRONT_TRACKING
        front.Alloc_Queue();
        front.Gather_Leaves();
        pairfront = &front;
        distance = SWIFT_INFINITY;

        for( i = front.First_Leaf(); i <= front.Last_Leaf(); i++ ) {
            SWIFT_Front_Node* leaf = front.Leaves()[i];
            bv0 = leaf->BV0(); bv1 = leaf->BV1();
            start_state = leaf->State();
            Setup_BV_Query( start_state, leaf->Feature0(), leaf->Feature1());
            result = Distance_Convex_LC( start_state, loc_dist );
            Save_State( leaf );
            leaf->Set_Close( result );
            if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
                if( loc_dist < distance ) {
                    Save_To_Cache_State();
                    distance = loc_dist;
                }
#else
                distance = min( distance, loc_dist );
#endif
                if( result ) {
                    pqueue.Reset();
                    break;
                }
            } else if( result || (loc_dist < tolerance &&
                                  (loc_dist < distance - abs_error ||
                                   loc_dist * rel_error < distance))
            ) {
                // Insert this expansion into the queue
                if( pqueue.Full() ) {
#ifdef SWIFT_DEBUG
cerr << "Full Queue when processing leaves" << endl;
#endif
                    // There is no room for this node in the queue so
                    // resolve this pair of recursion branches
                    loc_dist = distance;
                    result = Handle_Full_Queue( tolerance, abs_error,
                                                rel_error, loc_dist, leaf );
                    distance = min( distance, loc_dist );
                    if( result ) {
                        pqueue.Reset();
                        break;
                    }
                } else {
                    pqueue.Insert( leaf, bv0, bv1, c1, c2, loc_dist );
                }
            }
            // Try to raise the front
            if( leaf->Is_Right_Child() && leaf->Left_Sibling()->Is_Leaf() &&
                !leaf->Close() && !leaf->Left_Sibling()->Close()
            ) {
                // Raise
                //delete leaf->Left_Sibling();
                leaf->Parent()->Make_Leaf();
                leaf->Parent()->Set_Uninitialized();
                //delete leaf;
                front.Count_Raise();
            }
        }
#else // !SWIFT_FRONT_TRACKING
        // Insert the two roots
        bv0 = o0->Mesh()->Root(); bv1 = o1->Mesh()->Root();
        start_state = State();
        Setup_BV_Query( start_state, feat0, feat1 );
        result = Distance_Convex_LC( start_state, loc_dist );
        if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
            Save_To_Cache_State();
#endif
            if( result ) {
                distance = -1.0;
            } else {
                distance = loc_dist;
            }
        } else {
            pqueue.Insert( bv0, bv1, c1, c2, loc_dist );
        }
        save_v1 = v1; save_v2 = v2; save_ve1 = ve1; save_ve2 = ve2;
        save_e1 = e1; save_e2 = e2; save_f1 = f1; save_f2 = f2;
        save_state = state;
#endif

        while( !pqueue.Empty() &&
               pqueue.Highest().Distance() < distance - abs_error &&
               pqueue.Highest().Distance() * rel_error < distance
        ) {
            // Take the head of the queue and test its children
            SWIFT_Distance_Pair dp;
            pqueue.Remove_Highest( dp );
            if( dp.Class0() == CLASS_FREE ) {
                // Go down on bv0
                SWIFT_Triple st = trans10 * dp.BV1()->Center_Of_Mass();
                bv0 = dp.BV0()->Children()[0]; bv1 = dp.BV1();
                level0 = bv0->Level(); level1 = bv1->Level();
                // Initialize the features
                fdir = st - bv0->Center_Of_Mass();
                Initialize_From_Scratch( start_state );
                result = Distance_Convex_LC( start_state, loc_dist );
#ifdef SWIFT_FRONT_TRACKING
                SWIFT_Front_Node* new_node;
                if( result ) {
                    // There was intersection so we need to create a new pair
                    // in the front tree.  Note that we do not have to set
                    // Close() here since this node will not have an attempted
                    // raise on it.
                    new_node = new SWIFT_Front_Node;
                    Save_State( new_node );
                    new_node->Set_Parent( dp.Front_Node() );
                    new_node->Set_BV0( bv0 );
                    new_node->Set_BV1( bv1 );
                    // dp.Front_Node() != NULL
                    dp.Front_Node()->Set_Left_Child( new_node );
                } else {
                    if( dp.Front_Node() != NULL ) {
                        dp.Front_Node()->Set_Left_Child( NULL );
                    }
                }
#endif
                if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
                    if( loc_dist < distance ) {
                        Save_To_Cache_State();
                        distance = loc_dist;
                    }
#else
                    distance = min( distance, loc_dist );
#endif
                    if( result ) {
                        // Have to create sibling front node
#ifdef SWIFT_FRONT_TRACKING
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( dp.Front_Node() );
                        new_node->Set_BV0( dp.BV0()->Children()[1] );
                        new_node->Set_BV1( bv1 );
                        new_node->Set_Uninitialized();
                        // dp.Front_Node() != NULL
                        dp.Front_Node()->Set_Right_Child( new_node );
                        front.Count_Drop();
#endif
                        break;
                    }
                } else if( result || (loc_dist < tolerance &&
                                      (loc_dist < distance - abs_error ||
                                       loc_dist * rel_error < distance))
                ) {
                    // Insert this expansion into the queue -- there is room
                    pqueue.Insert(
#ifdef SWIFT_FRONT_TRACKING
                                    dp.Front_Node() == NULL ? NULL :
                                            dp.Front_Node()->Left_Child(),
#endif
                                    bv0, bv1, c1, c2, loc_dist );
                }

                bv0 = dp.BV0()->Children()[1]; bv1 = dp.BV1();
                level0 = bv0->Level(); level1 = bv1->Level();
                // Randomly initialize the features
                fdir = st - bv0->Center_Of_Mass();
                Initialize_From_Scratch( start_state );
                result = Distance_Convex_LC( start_state, loc_dist );
#ifdef SWIFT_FRONT_TRACKING
                if( result ) {
                    // There was intersection so we need to create a new pair
                    // in the front tree.  Note that we do not have to set
                    // Close() here since this node will not have an attempted
                    // raise on it.
                    new_node = new SWIFT_Front_Node;
                    Save_State( new_node );
                    new_node->Set_Parent( dp.Front_Node() );
                    new_node->Set_BV0( bv0 );
                    new_node->Set_BV1( bv1 );
                    // dp.Front_Node() != NULL
                    dp.Front_Node()->Set_Right_Child( new_node );
                } else {
                    if( dp.Front_Node() != NULL ) {
                        dp.Front_Node()->Set_Right_Child( NULL );
                    }
                }
                if( dp.Front_Node() != NULL ) {
                    if( dp.Front_Node()->Left_Child() == NULL ) {
                        if( dp.Front_Node()->Right_Child() != NULL ) {
                            // Have to create a left child
                            new_node = new SWIFT_Front_Node;
                            new_node->Set_Parent( dp.Front_Node() );
                            new_node->Set_BV0( dp.BV0()->Children()[0] );
                            new_node->Set_BV1( bv1 );
                            new_node->Set_Uninitialized();
                            dp.Front_Node()->Set_Left_Child( new_node );
                            front.Count_Drop();
                        }
                    } else {
                        if( dp.Front_Node()->Right_Child() == NULL ) {
                            // Have to create a right child
                            new_node = new SWIFT_Front_Node;
                            new_node->Set_Parent( dp.Front_Node() );
                            new_node->Set_BV0( dp.BV0()->Children()[1] );
                            new_node->Set_BV1( bv1 );
                            new_node->Set_Uninitialized();
                            dp.Front_Node()->Set_Right_Child( new_node );
                        }
                        front.Count_Drop();
                    }
                }
#endif
                if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
                    if( loc_dist < distance ) {
                        Save_To_Cache_State();
                        distance = loc_dist;
                    }
#else
                    distance = min( distance, loc_dist );
#endif
                    if( result ) {
                        break;
                    }
                } else if( result || (loc_dist < tolerance &&
                                      (loc_dist < distance - abs_error ||
                                       loc_dist * rel_error < distance))
                ) {
                    // Insert this expansion into the queue
                    if( pqueue.Full() ) {
#ifdef SWIFT_DEBUG
cerr << "Full Queue when processing queue" << endl;
#endif
                        // There is no room for this node in the queue so
                        // resolve this pair of recursion branches
                        loc_dist = distance;
                        result = Handle_Full_Queue(
                                    tolerance, abs_error, rel_error, loc_dist
#ifdef SWIFT_FRONT_TRACKING
                                    , dp.Front_Node() == NULL ? NULL :
                                        dp.Front_Node()->Right_Child()
#endif
                                    );
                        distance = min( distance, loc_dist );
                        if( result ) {
                            break;
                        }
                    } else {
                        pqueue.Insert(
#ifdef SWIFT_FRONT_TRACKING
                                        dp.Front_Node() == NULL ? NULL :
                                            dp.Front_Node()->Right_Child(),
#endif
                                        bv0, bv1, c1, c2, loc_dist );
                    }
                }
            } else {
                // Go down on bv1
                SWIFT_Triple st = trans01 * dp.BV0()->Center_Of_Mass();
                bv0 = dp.BV0(); bv1 = dp.BV1()->Children()[0];
                level0 = bv0->Level(); level1 = bv1->Level();
                // Randomly initialize the features
                fdir = st - bv1->Center_Of_Mass();
                Initialize_From_Scratch2( start_state );
                result = Distance_Convex_LC( start_state, loc_dist );
#ifdef SWIFT_FRONT_TRACKING
                SWIFT_Front_Node* new_node;
                if( result ) {
                    // There was intersection so we need to create a new pair
                    // in the front tree.  Note that we do not have to set
                    // Close() here since this node will not have an attempted
                    // raise on it.
                    new_node = new SWIFT_Front_Node;
                    Save_State( new_node );
                    new_node->Set_Parent( dp.Front_Node() );
                    new_node->Set_BV0( bv0 );
                    new_node->Set_BV1( bv1 );
                    // dp.Front_Node() is not NULL
                    dp.Front_Node()->Set_Left_Child( new_node );
                } else {
                    if( dp.Front_Node() != NULL ) {
                        dp.Front_Node()->Set_Left_Child( NULL );
                    }
                }
#endif
                if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
                    if( loc_dist < distance ) {
                        Save_To_Cache_State();
                        distance = loc_dist;
                    }
#else
                    distance = min( distance, loc_dist );
#endif
                    if( result ) {
#ifdef SWIFT_FRONT_TRACKING
                        new_node = new SWIFT_Front_Node;
                        new_node->Set_Parent( dp.Front_Node() );
                        new_node->Set_BV0( bv0 );
                        new_node->Set_BV1( dp.BV1()->Children()[1] );
                        new_node->Set_Uninitialized();
                        // dp.Front_Node() is not NULL
                        dp.Front_Node()->Set_Right_Child( new_node );
                        front.Count_Drop();
#endif
                        break;
                    }
                } else if( result || (loc_dist < tolerance &&
                                      (loc_dist < distance - abs_error ||
                                       loc_dist * rel_error < distance))
                ) {
                    // Insert this expansion into the queue -- there is room
                    pqueue.Insert(
#ifdef SWIFT_FRONT_TRACKING
                                    dp.Front_Node() == NULL ? NULL :
                                        dp.Front_Node()->Left_Child(),
#endif
                                    bv0, bv1, c1, c2, loc_dist );
                }

                bv0 = dp.BV0(); bv1 = dp.BV1()->Children()[1];
                level0 = bv0->Level(); level1 = bv1->Level();
                // Randomly initialize the features
                fdir = st - bv1->Center_Of_Mass();
                Initialize_From_Scratch2( start_state );
                result = Distance_Convex_LC( start_state, loc_dist );
#ifdef SWIFT_FRONT_TRACKING
                if( result ) {
                    // There was intersection so we need to create a new pair
                    // in the front tree.  Note that we do not have to set
                    // Close() here since this node will not have an attempted
                    // raise on it.
                    new_node = new SWIFT_Front_Node;
                    Save_State( new_node );
                    new_node->Set_Parent( dp.Front_Node() );
                    new_node->Set_BV0( bv0 );
                    new_node->Set_BV1( bv1 );
                    dp.Front_Node()->Set_Right_Child( new_node );
                } else {
                    if( dp.Front_Node() != NULL ) {
                        dp.Front_Node()->Set_Right_Child( NULL );
                    }
                }
                if( dp.Front_Node() != NULL ) {
                    if( dp.Front_Node()->Left_Child() == NULL ) {
                        if( dp.Front_Node()->Right_Child() != NULL ) {
                            // Have to create a left child
                            new_node = new SWIFT_Front_Node;
                            new_node->Set_Parent( dp.Front_Node() );
                            new_node->Set_BV0( bv0 );
                            new_node->Set_BV1( dp.BV1()->Children()[0] );
                            new_node->Set_Uninitialized();
                            dp.Front_Node()->Set_Left_Child( new_node );
                            front.Count_Drop();
                        }
                    } else {
                        if( dp.Front_Node()->Right_Child() == NULL ) {
                            // Have to create a right child
                            new_node = new SWIFT_Front_Node;
                            new_node->Set_Parent( dp.Front_Node() );
                            new_node->Set_BV0( bv0 );
                            new_node->Set_BV1( dp.BV1()->Children()[1] );
                            new_node->Set_Uninitialized();
                            dp.Front_Node()->Set_Right_Child( new_node );
                        }
                        front.Count_Drop();
                    }
                }
#endif
                if( c1 != CLASS_FREE && c2 != CLASS_FREE ) {
#ifdef SWIFT_PIECE_CACHING
                    if( loc_dist < distance ) {
                        Save_To_Cache_State();
                        distance = loc_dist;
                    }
#else
                    distance = min( distance, loc_dist );
#endif
                    if( result ) {
                        break;
                    }
                } else if( result || (loc_dist < tolerance &&
                                      (loc_dist < distance - abs_error ||
                                       loc_dist * rel_error < distance))
                ) {
                    // Insert this expansion into the queue
                    if( pqueue.Full() ) {
#ifdef SWIFT_DEBUG
cerr << "Full Queue when processing queue" << endl;
#endif
                        // There is no room for this node in the queue so
                        // resolve this pair of recursion branches
                        loc_dist = distance;
                        result = Handle_Full_Queue(
                                    tolerance, abs_error, rel_error, loc_dist
#ifdef SWIFT_FRONT_TRACKING
                                    , dp.Front_Node() == NULL ? NULL :
                                        dp.Front_Node()->Right_Child()
#endif
                                    );
                        distance = min( distance, loc_dist );
                        if( result ) {
                            break;
                        }
                    } else {
                        pqueue.Insert(
#ifdef SWIFT_FRONT_TRACKING
                                        dp.Front_Node() == NULL ? NULL :
                                            dp.Front_Node()->Right_Child(),
#endif
                                        bv0, bv1, c1, c2, loc_dist );
                    }
                }
            }
        }
#ifdef SWIFT_PIECE_CACHING
    } else {
#ifndef SWIFT_FRONT_TRACKING
        // This root has gone incoherent if the intersection was not on the
        // root.  Set it to be uninitialized.
        if( !bv0->Is_Root() || !bv1->Is_Root() ) {
            feat1 = NULL;
        }
#endif
    }
#endif
#else
#ifdef SWIFT_FRONT_TRACKING
    // Start of the front tracking loop

    // Perform a BFS on the front tree and gather the leaves, then process them
    // from deepest to shallowest and process the marked leaf first.
    int i;
    front.Alloc_Queue();
    front.Gather_Leaves();
    pairfront = &front;
    distance = SWIFT_INFINITY;
     //<< " leaves in the front tree" << endl;
    for( i = front.First_Leaf(); i <= front.Last_Leaf(); i++ ) {
        SWIFT_Front_Node* leaf = front.Leaves()[i];
        bv0 = leaf->BV0(); bv1 = leaf->BV1();
        start_state = leaf->State();
        Setup_BV_Query( start_state, leaf->Feature0(), leaf->Feature1() );
#else
    // Setup the bv specific query stuff
    bv0 = o0->Mesh()->Root(); bv1 = o1->Mesh()->Root();
#endif
#endif
#ifndef SWIFT_FRONT_TRACKING
#ifndef SWIFT_PRIORITY_DIRECTION
    start_state = State();
    Setup_BV_Query( start_state, feat0, feat1 );
#endif
#endif

#ifndef SWIFT_PRIORITY_DIRECTION
// ------------------------- Start dist call -------------------------
#ifdef SWIFT_FRONT_TRACKING
    result = Distance_LC( start_state, tolerance,
                          abs_error, rel_error, distance, leaf, leaf );
#else
    result = Distance_LC( start_state, tolerance,
                          abs_error, rel_error, distance );
#endif
// ------------------------- End dist call -------------------------
#endif

#ifdef SWIFT_FRONT_TRACKING
#ifndef SWIFT_PRIORITY_DIRECTION
        // End of the front tracking loop
        if( result ) {
            // There was intersection
            break;
        }

        if( leaf->Is_Right_Child() && leaf->Left_Sibling()->Is_Leaf() &&
            !leaf->Close() && !leaf->Left_Sibling()->Close()
        ) {
            // Raise
            delete leaf->Left_Sibling();
            leaf->Parent()->Make_Leaf();
            leaf->Parent()->Set_Uninitialized();
            delete leaf;
            front.Count_Raise();
        }
    }
#endif
#endif


#ifndef SWIFT_FRONT_TRACKING
    Save_State();
#endif


    return result;
}

bool SWIFT_Pair::Contacts( SWIFT_Object* o0, SWIFT_Object* o1,
                           SWIFT_Real tolerance, SWIFT_Real& distance,
                           int& num_contacts )
{
    RESULT_TYPE start_state = State();
    bool result;

    // Setup the pair specific query stuff
    distance = SWIFT_INFINITY;
    Setup_Pair_Query( o0, o1,
                      true );

#ifdef SWIFT_FRONT_TRACKING
    // Start of the front tracking loop

    // Perform a BFS on the front tree and gather the leaves, then process them
    // from deepest to shallowest and process the marked leaf first.
    int i;
    front.Alloc_Queue();
    front.Gather_Leaves();
    pairfront = &front;
    distance = SWIFT_INFINITY;
     //<< " leaves in the front tree" << endl;
    for( i = front.First_Leaf(); i <= front.Last_Leaf(); i++ ) {
        SWIFT_Front_Node* leaf = front.Leaves()[i];
        bv0 = leaf->BV0(); bv1 = leaf->BV1();
        start_state = leaf->State();
        Setup_BV_Query( start_state, leaf->Feature0(), leaf->Feature1() );
#else
    // Setup the bv specific query stuff
    bv0 = o0->Mesh()->Root(); bv1 = o1->Mesh()->Root();
#endif
#ifndef SWIFT_FRONT_TRACKING
    start_state = State();
    Setup_BV_Query( start_state, feat0, feat1 );
#endif

// ------------------------- Start contacts call -------------------------
#ifdef SWIFT_FRONT_TRACKING
    result = Distance_LC( start_state, tolerance, 0.0, 1.0, distance,
                                                       leaf, leaf, true );
#else
    result = Distance_LC( start_state, tolerance, 0.0, 1.0, distance, true );
#endif
// ------------------------- End contacts call -------------------------

#ifdef SWIFT_FRONT_TRACKING
        // End of the front tracking loop
        if( result ) {
            // There was intersection
            break;
        }

        if( leaf->Is_Right_Child() && leaf->Left_Sibling()->Is_Leaf() &&
            !leaf->Close() && !leaf->Left_Sibling()->Close()
        ) {
            // Raise
            delete leaf->Left_Sibling();
            leaf->Parent()->Make_Leaf();
            leaf->Parent()->Set_Uninitialized();
            delete leaf;
            front.Count_Raise();
        }
    }
#endif


#ifndef SWIFT_FRONT_TRACKING
    Save_State();
#endif

    num_contacts = contact_listd.Length();

    return result;
}


///////////////////////////////////////////////////////////////////////////////
// List Fillers
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Pair::Distances( SWIFT_Array<SWIFT_Real>& dists )
{
    int i;
    for( i = 0; i < contact_listd.Length(); i++ ) {
        dists.Add( contact_listd[i] );
    }
}


// If SWIFT_ALLOW_BOUNDARY is on and a face id is returned, it may be -1
// which means it is a contained face
void SWIFT_Pair::Contact_Features( SWIFT_Array<int>& ftypes,
                                   SWIFT_Array<int>& fids )
{
    int i;

    for( i = 0; i < contact_listt.Length(); i++ ) {
        switch( contact_listt[i] ) {
        case CONTINUE_VV:
            ftypes.Add( VERTEX );
            ftypes.Add( VERTEX );
            fids.Add( obj0->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list0[i])->Origin() ) );
            fids.Add( obj1->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list1[i])->Origin() ) );
            break;
        case CONTINUE_VE:
            ftypes.Add( VERTEX );
            ftypes.Add( EDGE );
            fids.Add( obj0->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list0[i])->Origin() ) );
            Report_Edge1( i, fids );
            break;
        case CONTINUE_EV:
            ftypes.Add( EDGE );
            ftypes.Add( VERTEX );
            Report_Edge0( i, fids );
            fids.Add( obj1->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list1[i])->Origin() ) );
            break;
        case CONTINUE_VF:
            ftypes.Add( VERTEX );
            ftypes.Add( FACE );
            fids.Add( obj0->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list0[i])->Origin() ) );
#ifdef SWIFT_ALLOW_BOUNDARY
            fids.Add( ((SWIFT_Tri_Face*)contact_list1[i])->Classification()
                        != CLASS_ORIGINAL ? -1 : obj1->Mesh()->Map_Face_Id(
                            ((SWIFT_Tri_Face*)contact_list1[i])->Edge1().
                                            Twin()->Twin()->Adj_Face() ) );
#else
            fids.Add( obj1->Mesh()->Map_Face_Id(
                            ((SWIFT_Tri_Face*)contact_list1[i])->Edge1().
                                            Twin()->Twin()->Adj_Face() ) );
#endif
            break;
        case CONTINUE_FV:
            ftypes.Add( FACE );
            ftypes.Add( VERTEX );
#ifdef SWIFT_ALLOW_BOUNDARY
            fids.Add( ((SWIFT_Tri_Face*)contact_list0[i])->Classification()
                        != CLASS_ORIGINAL ? -1 : obj0->Mesh()->Map_Face_Id(
                            ((SWIFT_Tri_Face*)contact_list0[i])->Edge1().
                                            Twin()->Twin()->Adj_Face() ) );
#else
            fids.Add( obj0->Mesh()->Map_Face_Id(
                            ((SWIFT_Tri_Face*)contact_list0[i])->Edge1().
                                            Twin()->Twin()->Adj_Face() ) );
#endif
            fids.Add( obj1->Mesh()->Map_Vertex_Id(
                            ((SWIFT_Tri_Edge*)contact_list1[i])->Origin() ) );
            break;
        case CONTINUE_EE:
            ftypes.Add( EDGE );
            ftypes.Add( EDGE );
            Report_Edge0( i, fids );
            Report_Edge1( i, fids );
            break;
        default:
            break;
        }
    }
}

void SWIFT_Pair::Contact_Points( SWIFT_Array<SWIFT_Real>& points )
{
    int i;
    SWIFT_Triple tri;
    SWIFT_Real* points_ptr = points.Data() + points.Length();

    for( i = 0; i < contact_listt.Length(); i++, points_ptr += 6 ) {
        switch( contact_listt[i] ) {
        case CONTINUE_VV:
            ((SWIFT_Tri_Edge*)contact_list0[i])->Origin()->Coords().
                                                    Get_Value( points_ptr );
            ((SWIFT_Tri_Edge*)contact_list1[i])->Origin()->Coords().
                                                    Get_Value( points_ptr+3 );
            break;
        case CONTINUE_VE:
          { const SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list1[i]);
            const SWIFT_Tri_Vertex* v =
                                ((SWIFT_Tri_Edge*)contact_list0[i])->Origin();
            v->Coords().Get_Value( points_ptr );
            tri = e->Origin()->Coords() + ((trans01 * v->Coords() -
                  e->Origin()->Coords()) * e->Direction()) * e->Direction();
            tri.Get_Value( points_ptr+3 );
          } break;
        case CONTINUE_EV:
          { const SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list0[i]);
            const SWIFT_Tri_Vertex* v =
                                ((SWIFT_Tri_Edge*)contact_list1[i])->Origin();
            tri = e->Origin()->Coords() + ((trans10 * v->Coords() -
                  e->Origin()->Coords()) * e->Direction()) * e->Direction();
            tri.Get_Value( points_ptr );
            v->Coords().Get_Value( points_ptr+3 );
          } break;
        case CONTINUE_VF:
          { const SWIFT_Tri_Face* f = ((SWIFT_Tri_Face*)contact_list1[i]);
            const SWIFT_Tri_Vertex* v =
                                ((SWIFT_Tri_Edge*)contact_list0[i])->Origin();
            v->Coords().Get_Value( points_ptr );
            tri = trans01 * v->Coords();
            tri -= ((tri - f->Coords1()) * f->Normal()) * f->Normal();
            tri.Get_Value( points_ptr+3 );
          } break;
        case CONTINUE_FV:
          { const SWIFT_Tri_Face* f = ((SWIFT_Tri_Face*)contact_list0[i]);
            const SWIFT_Tri_Vertex* v =
                                ((SWIFT_Tri_Edge*)contact_list1[i])->Origin();
            tri = trans10 * v->Coords();
            tri -= ((tri - f->Coords1()) * f->Normal()) * f->Normal();
            tri.Get_Value( points_ptr );
            v->Coords().Get_Value( points_ptr+3 );
          } break;
        case CONTINUE_EE:
          { SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list0[i]);
            SWIFT_Tri_Edge* ee = ((SWIFT_Tri_Edge*)contact_list1[i]);
            SWIFT_Triple tri2;
            Compute_Closest_Points_Edge_Edge( e, ee, tri, tri2, trans01 );
            tri.Get_Value( points_ptr );
            tri2.Get_Value( points_ptr+3 );
          } break;
        default:
            break;
        }
    }

    // Fix up the length of the points array
    points.Set_Length( points.Length() + 6*contact_listt.Length() );
}

void SWIFT_Pair::Contact_Normals( SWIFT_Array<SWIFT_Real>& normals )
{
    int i;
    SWIFT_Triple tri;
    SWIFT_Real* normals_ptr = normals.Data() + normals.Length();

    for( i = 0; i < contact_listt.Length(); i++, normals_ptr += 3 ) {
        switch( contact_listt[i] ) {
        case CONTINUE_VV:
            tri = (obj1->Transformation() &
                   ((SWIFT_Tri_Edge*)contact_list1[i])->Origin()->
                                                        Gathered_Normal()) -
                  (obj0->Transformation() &
                   ((SWIFT_Tri_Edge*)contact_list0[i])->Origin()->
                                                        Gathered_Normal());
#ifdef SWIFT_USE_FLOAT
            if( tri.Length_Sq() < EPSILON7 ) {
#else
            if( tri.Length_Sq() < EPSILON13 ) {
#endif
                // Use the edge method
                tri = (obj0->Transformation() &
                       ((SWIFT_Tri_Edge*)contact_list0[i])->Origin()->
                                                        Gathered_Direction()) -
                      (obj1->Transformation() &
                       ((SWIFT_Tri_Edge*)contact_list1[i])->Origin()->
                                                        Gathered_Direction());
            }
            tri.Normalize();
            break;
        case CONTINUE_VE:
          { SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list1[i]);
#ifdef SWIFT_USE_FLOAT
            if( contact_listd[i] < EPSILON7 ) {
#else
            if( contact_listd[i] < EPSILON13 ) {
#endif
                // Just take the average of the face normals
                e = e->Origin()->Adj_Edge( e->Next()->Origin() );
                tri = e->Adj_Face()->Normal() + e->Twin()->Adj_Face()->Normal();
#ifdef SWIFT_USE_FLOAT
                if( tri.Length_Sq() < EPSILON7 ) {
#else
                if( tri.Length_Sq() < EPSILON13 ) {
#endif
                    // Do the cross product method
                    tri = e->Twin()->Adj_Face()->Normal() % e->Direction();
                }
            } else {
                // Compute weighted combination of vertex' angle about the edge
                tri = trans01 *
                      ((SWIFT_Tri_Edge*)contact_list0[i])->Origin()->Coords();
                tri -= e->Origin()->Coords() +
                        (((tri - e->Origin()->Coords()) * e->Direction()) *
                         e->Direction());
            }
            tri.Normalize();
            tri &= obj1->Transformation();
          } break;
        case CONTINUE_EV:
          { SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list0[i]);
#ifdef SWIFT_USE_FLOAT
            if( contact_listd[i] < EPSILON7 ) {
#else
            if( contact_listd[i] < EPSILON13 ) {
#endif
                // Just take the average of the face normals
                e = e->Origin()->Adj_Edge( e->Next()->Origin() );
                tri = -e->Adj_Face()->Normal() -e->Twin()->Adj_Face()->Normal();
#ifdef SWIFT_USE_FLOAT
                if( tri.Length_Sq() < EPSILON7 ) {
#else
                if( tri.Length_Sq() < EPSILON13 ) {
#endif
                    // Do the cross product method
                    tri = e->Adj_Face()->Normal() % e->Direction();
                }
            } else {
                // Compute weighted combination of vertex' angle about the edge
                tri = trans10 *
                      ((SWIFT_Tri_Edge*)contact_list1[i])->Origin()->Coords();
                tri -= e->Origin()->Coords() +
                        (((tri - e->Origin()->Coords()) * e->Direction()) *
                         e->Direction());
                tri.Negate();
            }
            tri.Normalize();
            tri &= obj0->Transformation();
          } break;
        case CONTINUE_VF:
          { // Transform the face normal to world coordinates
            const SWIFT_Tri_Face* f = ((SWIFT_Tri_Face*)contact_list1[i]);
            tri = obj1->Transformation() & f->Normal();
          } break;
        case CONTINUE_FV:
          { // Transform the face normal to world coordinates
            const SWIFT_Tri_Face* f = ((SWIFT_Tri_Face*)contact_list0[i]);
            tri = -(obj0->Transformation() & f->Normal());
          } break;
        case CONTINUE_EE:
          { SWIFT_Tri_Edge* e = ((SWIFT_Tri_Edge*)contact_list0[i]);
            SWIFT_Tri_Edge* ee = ((SWIFT_Tri_Edge*)contact_list1[i]);
            // Compute the cross product of the two edge directions
            tri = e->Direction() % (trans10 & ee->Direction());
#ifdef SWIFT_USE_FLOAT
            if( tri.Length_Sq() < EPSILON7 ) {
#else
            if( tri.Length_Sq() < EPSILON13 ) {
#endif
                // Handle nearly parallel edges by averaging face normals
                e = e->Origin()->Adj_Edge( e->Next()->Origin() );
                ee = ee->Origin()->Adj_Edge( ee->Next()->Origin() );
                tri = - (obj0->Transformation() & (e->Adj_Face()->Normal() +
                                            e->Twin()->Adj_Face()->Normal()))
                      + (obj1->Transformation() & (ee->Adj_Face()->Normal() +
                                            ee->Twin()->Adj_Face()->Normal()));
#ifdef SWIFT_USE_FLOAT
                if( tri.Length_Sq() < EPSILON7 ) {
#else
                if( tri.Length_Sq() < EPSILON13 ) {
#endif
                    // Use the cross product method
                    tri = ee->Twin()->Adj_Face()->Normal() % ee->Direction();
                    tri &= obj1->Transformation();
                }
            } else {
                // Determine which way the vector should point
                if( (contact_listbv0[i]->Center_Of_Mass() -
                        e->Origin()->Coords()) * tri < 0.0
                ) {
                    tri.Negate();
                }
                tri &= obj0->Transformation();
            }
            tri.Normalize();
          } break;
        default:
            break;
        }
        tri.Get_Value( normals_ptr );
    }

    // Fix up the length of the normals array
    normals.Set_Length( normals.Length() + 3*contact_listt.Length() );
}


///////////////////////////////////////////////////////////////////////////////
// SWIFT_Pair private functions
///////////////////////////////////////////////////////////////////////////////

#ifdef SWIFT_PIECE_CACHING
inline void SWIFT_Pair::Save_To_Cache_State( )
{
    void* feat0;
    void* feat1;
    Extract_Features( state, feat0, feat1 );
    Set_Cache_Feature0( feat0 );
    Set_Cache_Feature1( feat1 );
    Set_Cache_State( state );
    Set_Cache_BV0( bv0 );
    Set_Cache_BV1( bv1 );
}
#endif

inline void SWIFT_Pair::Report_Edge0( int i, SWIFT_Array<int>& fids )
{
#ifdef SWIFT_REPORT_EDGE_IDS
    SWIFT_Tri_Face* f = ((SWIFT_Tri_Edge*)contact_list0[i])->Adj_Face();
    if( f->Classification() == CLASS_ORIGINAL ) {
        // We use the twin edge here to ensure that we end up on the
        // main mesh which will yield the correct ids.
        f = ((SWIFT_Tri_Edge*)contact_list0[i])->Twin()->Adj_Face();
        fids.Add( obj0->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list0[i])->Twin() ) );
    } else { // Must be CONTAINED
#ifdef SWIFT_ALLOW_BOUNDARY
        // Check to see if the neighboring face is also CONTAINED
        f = ((SWIFT_Tri_Edge*)contact_list0[i])->Twin(
                            contact_listbv0[i]->Level() )->Adj_Face();
        if( f->Classification() == CLASS_CONTAINED ) {
            // Both faces are contained -- report -1
            fids.Add( -1 );
        } else {
            // Simply use the twin
            f = ((SWIFT_Tri_Edge*)contact_list0[i])->Twin(
                    contact_listbv0[i]->Level() )->Twin()->Adj_Face();
            fids.Add( obj0->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list0[i])->Twin(
                            contact_listbv0[i]->Level() )->Twin() ) );
        }
#else
        // Simply use the twin
        f = ((SWIFT_Tri_Edge*)contact_list0[i])->Twin(
                    contact_listbv0[i]->Level() )->Twin()->Adj_Face();
        fids.Add( obj0->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list0[i])->Twin(
                            contact_listbv0[i]->Level() )->Twin() ) );
#endif
    }
#else
    fids.Add( obj0->Mesh()->Map_Vertex_Id(
                    ((SWIFT_Tri_Edge*)contact_list0[i])->Origin() ) );
    fids.Add( obj0->Mesh()->Map_Vertex_Id(
            ((SWIFT_Tri_Edge*)contact_list0[i])->Next()->Origin() ) );
#endif
}

inline void SWIFT_Pair::Report_Edge1( int i, SWIFT_Array<int>& fids )
{
#ifdef SWIFT_REPORT_EDGE_IDS
    SWIFT_Tri_Face* f = ((SWIFT_Tri_Edge*)contact_list1[i])->Adj_Face();
    if( f->Classification() == CLASS_ORIGINAL ) {
        // We use the twin edge here to ensure that we end up on the
        // main mesh which will yield the correct ids.
        f = ((SWIFT_Tri_Edge*)contact_list1[i])->Twin()->Adj_Face();
        fids.Add( obj1->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list1[i])->Twin() ) );
    } else { // Must be CONTAINED
#ifdef SWIFT_ALLOW_BOUNDARY
        // Check to see if the neighboring face is also CONTAINED
        f = ((SWIFT_Tri_Edge*)contact_list1[i])->Twin(
                            contact_listbv1[i]->Level() )->Adj_Face();
        if( f->Classification() == CLASS_CONTAINED ) {
            // Both faces are contained -- report -1
            fids.Add( -1 );
        } else {
            // Simply use the twin
            f = ((SWIFT_Tri_Edge*)contact_list1[i])->Twin(
                    contact_listbv1[i]->Level() )->Twin()->Adj_Face();
            fids.Add( obj1->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list1[i])->Twin(
                            contact_listbv1[i]->Level() )->Twin() ) );
        }
#else
        // Simply use the twin
        f = ((SWIFT_Tri_Edge*)contact_list1[i])->Twin(
                    contact_listbv1[i]->Level() )->Twin()->Adj_Face();
        fids.Add( obj1->Mesh()->Face_Id( f )*3 + f->Edge_Id(
                    ((SWIFT_Tri_Edge*)contact_list1[i])->Twin(
                            contact_listbv1[i]->Level() )->Twin() ) );
#endif
    }
#else
    fids.Add( obj1->Mesh()->Map_Vertex_Id(
                    ((SWIFT_Tri_Edge*)contact_list1[i])->Origin() ) );
    fids.Add( obj1->Mesh()->Map_Vertex_Id(
            ((SWIFT_Tri_Edge*)contact_list1[i])->Next()->Origin() ) );
#endif
}

#ifndef SWIFT_FRONT_TRACKING
// Takes the last known state before penetration or disjointness was detected
// and saves those features as well as their type encoded as a CONTINUE_* value
// encoded in the bit_field bits of the pair.
inline void SWIFT_Pair::Save_State( )
{
    // Save the features
    switch( save_state ) {
    case CONTINUE_VV:
        feat0 = (void*) save_ve1;
        feat1 = (void*) save_ve2;
        break;
    case CONTINUE_VE:
        feat0 = (void*) save_ve1;
        feat1 = (void*) save_e2;
        break;
    case CONTINUE_EV:
        feat0 = (void*) save_e1;
        feat1 = (void*) save_ve2;
        break;
    case CONTINUE_VF:
        feat0 = (void*) save_ve1;
        feat1 = (void*) save_f2;
        break;
    case CONTINUE_FV:
        feat0 = (void*) save_f1;
        feat1 = (void*) save_ve2;
        break;
    case CONTINUE_EE:
        feat0 = (void*) save_e1;
        feat1 = (void*) save_e2;
    default:
        break;
    }


    // Save the state id
    Set_State( save_state );
}
#endif


