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
// mesh.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#include <SWIFT_mesh.h>
#include <SWIFT_mesh_utils.h>

// Feature type identifiers
static const int SWIFT_VERTEX = 1;
static const int SWIFT_EDGE = 2;
static const int SWIFT_FACE = 3;

#ifdef SWIFT_DEBUG
#ifdef SWIFT_USE_FLOAT
static const SWIFT_Real REL_TOL1 = EPSILON6;
static const SWIFT_Real REL_TOL2 = EPSILON7;
static const SWIFT_Real REL_TOL_SQRT = EPSILON3;
#else
static const SWIFT_Real REL_TOL1 = EPSILON11;
static const SWIFT_Real REL_TOL2 = EPSILON12;
static const SWIFT_Real REL_TOL_SQRT = EPSILON6;
#endif
#endif


//////////////////////////////////////////////////////////////////////////////
// QSlim Hierarchy Creation Local functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Vertex functions
//////////////////////////////////////////////////////////////////////////////
int SWIFT_Tri_Vertex::Valence( ) const
{
    // The number of edges adjacent to the vertex is equivalent to the
    // number of faces that are adjacent since the mesh should be closed.
    SWIFT_Tri_Edge* e = edge->Twin()->Next();
    int valence;
    for( valence = 1; e != edge; e = e->Twin()->Next(), valence++ )
#ifdef SWIFT_DEBUG
    {
        if( valence == 1000 ) {
            cerr << "Warning: touched 1000 edges while computing valence"
                 << endl;
            break;
        }
    }
#else
    ;
#endif
    return valence;
}

#ifdef SWIFT_DEBUG
bool SWIFT_Tri_Vertex::Verify_Topology( int pos ) const
{
    bool result = true;

    if( edge == NULL ) {
        cerr << "Vertex at position " << pos << " does not have edge" << endl;
        result = false;
    }


    return result;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Edge functions
//////////////////////////////////////////////////////////////////////////////
#ifdef SWIFT_DEBUG
bool SWIFT_Tri_Edge::Verify_Topology( int pos1, int pos2 ) const
{
    bool result = true;

    if( orig == NULL ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not have vertex" << endl;
        result = false;
    }

    if( face == NULL ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not have face" << endl;
        result = false;
    } else if( face->Edge1P() != this && face->Edge2P() != this &&
               face->Edge3P() != this
    ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not point to the face that points to it" << endl;
        result = false;
    }

    if( next == NULL ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not have next" << endl;
        result = false;
    } else if( next->Next()->Next() != this ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not point to next correctly" << endl;
        result = false;
    }


    if( Marked() ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " is marked" << endl;
    }

    return result;
}

// The tests are formulated so as to fail if any of the numbers are NAN
// Since NAN (compare) x = FALSE for all x and all comparison operators.
bool SWIFT_Tri_Edge::Verify_Geometry( int pos1, int pos2 ) const
{
    bool result = true;

    if( twin != NULL ) {
        if( !(len == twin->Length()) ) {
            cerr << "Edge at position " << pos1 << ", " << pos2
                 << " does not have twin len" << endl;
            result = false;
        }

        if( !(-u == twin->Direction()) ) {
            cerr << "Edge at position " << pos1 << ", " << pos2
                 << " does not have twin -u" << endl;
            result = false;
        }
    }

    // Make sure that the direction and length match with the set endpoints
    SWIFT_Triple tempu = u;
    SWIFT_Triple temp1 = next->Origin()->Coords() - orig->Coords();
    SWIFT_Real lsq = temp1.Length_Sq();
    SWIFT_Real length_sq = len * len;

    if( !(length_sq / lsq < 1.0 + REL_TOL1 && length_sq / lsq > 1.0 - REL_TOL1)
    ) {
        cerr << "Edge at position " << pos1 << ", " << pos2 << " length = "
             << len << " does not match with endpoints length = "
             << sqrt( lsq ) << endl;
        result = false;
    }

    if( len < REL_TOL2 ) {
        cerr << "Edge at position " << pos1 << ", " << pos2 << " length = "
             << len << " is too short!" << endl;
        result = false;
    }

    tempu.Normalize();
    temp1.Normalize();
    length_sq = tempu * temp1;

    if( !(length_sq < 1.0 + REL_TOL2 && length_sq > 1.0 - REL_TOL2) ) {
        cerr << "Edge at position " << pos1 << ", " << pos2 << " direction = "
             << tempu << " does not match with endpoints direction = "
             << temp1 << endl;
        result = false;
    }

    // Attempt to make sure that the edge direction vector is unit length
    if( !(u.Length_Sq() < 1.0 + REL_TOL2 && u.Length_Sq() > 1.0 - REL_TOL2)
    ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not have a unit length u vector = " << u << endl;
        result = false;
    }

    length_sq = Distance( orig->Coords() );

    // Make sure that the origin is on the edge plane.
    if( !(length_sq < REL_TOL_SQRT && length_sq > -REL_TOL_SQRT) ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " origin is a distance away from the edge plane = "
             << length_sq << endl;
        result = false;
    }

    length_sq = Face_Distance( orig->Coords() );

    // Make sure that the origin is on the edge-face plane.
    if( !(length_sq < REL_TOL_SQRT && length_sq > -REL_TOL_SQRT) ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " origin is a distance away from the edge-face plane = "
             << length_sq << endl;
        result = false;
    }

    // Make sure that the edge-face plane is correct
    tempu = face->Normal() % u;
    length_sq = tempu * fn;
    if( !(length_sq < 1.0 + REL_TOL2 && length_sq > 1.0 - REL_TOL2) ) {
        cerr << "Edge at position " << pos1 << ", " << pos2
             << " does not have correct edge-face normal = " << fn
             << " should be = " << tempu << endl;
        result = false;
    }


    return result;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Face functions
//////////////////////////////////////////////////////////////////////////////
void SWIFT_Tri_Face::Compute_Plane_From_Edges( )
{
    // It is important for the Fill_Holes function that this use the directions
    // of e2 and e3 only!
    normal = e2.Direction() % e3.Direction();
    normal.Normalize();
    d = e1.Origin()->Coords() * normal;
}

void SWIFT_Tri_Face::Compute_Plane_From_Edges( SWIFT_Tri_Edge* edge1 )
{
    normal = edge1->Direction() % edge1->Next()->Direction();
    normal.Normalize();
    d = edge1->Origin()->Coords() * normal;
}

#ifdef SWIFT_DEBUG
bool SWIFT_Tri_Face::Verify_Topology( int pos ) const
{
    if( Marked() ) {
        cerr << "Face at position " << pos << " is marked" << endl;
    }

    return true;
}

// The tests are formulated so as to fail if any of the numbers are NAN
// Since NAN (compare) x = FALSE for all x and all comparison operators.
bool SWIFT_Tri_Face::Verify_Geometry( int pos ) const
{
    bool result = true;

    // Make sure that the edges that are set compute the same normal
    SWIFT_Triple temp1 = e1.Direction() % e2.Direction();

    temp1.Normalize();
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using e1 and e2 = "
             << temp1 << endl;
        cerr << "The edges are e1 = " << e1.Direction() << ", e2 = "
             << e2.Direction() << endl;
        result = false;
    }

    temp1 = e2.Direction() % e3.Direction();
    temp1.Normalize();
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using e2 and e3 = "
             << temp1 << endl;
        cerr << "The edges are e2 = " << e2.Direction() << ", e3 = "
             << e3.Direction() << endl;
        result = false;
    }

    temp1 = e3.Direction() % e1.Direction();
    temp1.Normalize();
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using e3 and e1 = "
             << temp1 << endl;
        cerr << "The edges are e3 = " << e3.Direction() << ", e1 = "
             << e1.Direction() << endl;
        result = false;
    }

    temp1 = (e1.Origin()->Coords() - e3.Origin()->Coords()) %
            (e2.Origin()->Coords() - e1.Origin()->Coords());
    temp1.Normalize( );
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using vertices 1 = "
             << temp1 << endl;
        result = false;
    }

    temp1 = (e2.Origin()->Coords() - e1.Origin()->Coords()) %
            (e3.Origin()->Coords() - e2.Origin()->Coords());
    temp1.Normalize( );
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using vertices 2 = "
             << temp1 << endl;
        result = false;
    }

    temp1 = (e3.Origin()->Coords() - e2.Origin()->Coords()) %
            (e1.Origin()->Coords() - e3.Origin()->Coords());
    temp1.Normalize( );
    if( !((temp1 * normal) < 1.0 + REL_TOL2 &&
         (temp1 * normal) > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos << " normal = " << normal
             << " does not match with that computed using vertices 3 = "
             << temp1 << endl;
        result = false;
    }

    // Attempt to make sure that the face normal vector is unit length
    if( !(normal.Length_Sq() < 1.0 + REL_TOL2 &&
          normal.Length_Sq() > 1.0 - REL_TOL2)
    ) {
        cerr << "Face at position " << pos
             << " does not have a unit length normal vector = " << normal
             << endl;
        result = false;
    }

    return result;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// SWIFT_BV public functions
//////////////////////////////////////////////////////////////////////////////

SWIFT_BV::~SWIFT_BV( )
{
}

// Convex hull bounding volumes
void SWIFT_BV::Set_Faces( SWIFT_Array<SWIFT_Tri_Face>& vfs )
{
    faces.Destroy();
    faces = vfs;
}

void SWIFT_BV::Set_Faces( SWIFT_Array<SWIFT_Tri_Face>& vfs,
                          SWIFT_Array<SWIFT_Tri_Face>& efs )
{
    faces.Destroy();
    if( efs.Length() == 0 ) {
        faces = vfs;
    } else {
        int i, j;
        faces.Create( vfs.Length() + efs.Length() );
        for( i = 0, j = 0; i < vfs.Length(); i++, j++ ) {
            faces[j] = vfs[i];
            faces[j].Reset_Internal_Edge_Pointers();
        }
        for( i = 0; i < efs.Length(); i++, j++ ) {
            faces[j] = efs[i];
            faces[j].Reset_Internal_Edge_Pointers();
        }
    }
}

void SWIFT_BV::Set_Other_Faces( SWIFT_Array<SWIFT_Tri_Face*>& fs )
{
    other_faces.Destroy();
    other_faces = fs;
}

void SWIFT_BV::Set_Faces_Classification( int c )
{
    int i;

    for( i = 0; i < Num_Faces(); i++ ) {
        faces[i].Set_Classification( c );
    }
}

void SWIFT_BV::Set_Faces_Level( int l )
{
    int i;

    for( i = 0; i < Num_Faces(); i++ ) {
        faces[i].Set_Level( l );
    }
    for( i = 0; i < Num_Other_Faces(); i++ ) {
        other_faces[i]->Set_Level( l );
    }
}

void SWIFT_BV::Compute_Center_Of_Mass( )
{
    int i;
    SWIFT_Real area_x2;
    SWIFT_Real total_area;
    SWIFT_Triple areav;

    com.Set_Value( 0.0, 0.0, 0.0 );
    total_area = 0.0;
    for( i = 0; i < Num_Faces(); i++ ) {
        areav = (faces[i].Edge1().Origin()->Coords() -
                 faces[i].Edge2().Origin()->Coords()) %
                (faces[i].Edge1().Origin()->Coords() -
                 faces[i].Edge3().Origin()->Coords());
        area_x2 = areav.Length();
        total_area += area_x2;
        com += area_x2 * (faces[i].Edge1().Origin()->Coords() +
                          faces[i].Edge2().Origin()->Coords() +
                          faces[i].Edge3().Origin()->Coords() );
    }
    for( i = 0; i < Num_Other_Faces(); i++ ) {
        areav = (other_faces[i]->Edge1().Origin()->Coords() -
                 other_faces[i]->Edge2().Origin()->Coords()) %
                (other_faces[i]->Edge1().Origin()->Coords() -
                 other_faces[i]->Edge3().Origin()->Coords());
        area_x2 = areav.Length();
        total_area += area_x2;
        com += area_x2 * (other_faces[i]->Edge1().Origin()->Coords() +
                          other_faces[i]->Edge2().Origin()->Coords() +
                          other_faces[i]->Edge3().Origin()->Coords() );
    }

    com /= 3.0 * total_area;
}

void SWIFT_BV::Compute_Radius( )
{
    int i;
    SWIFT_Real d;
    radius = 0.0;
    for( i = 0; i < Num_Faces(); i++ ) {
        d = Center().Dist_Sq( faces[i].Edge1().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
        d = Center().Dist_Sq( faces[i].Edge2().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
        d = Center().Dist_Sq( faces[i].Edge3().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
    }
    for( i = 0; i < Num_Other_Faces(); i++ ) {
        d = Center().Dist_Sq( other_faces[i]->Edge1().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
        d = Center().Dist_Sq( other_faces[i]->Edge2().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
        d = Center().Dist_Sq( other_faces[i]->Edge3().Origin()->Coords() );
        if( d > radius ) {
            radius = d;
        }
    }
    radius = sqrt( radius );
}

SWIFT_Tri_Edge* SWIFT_BV::Close_Edge( const SWIFT_Triple& pt )
{
    int i;
    SWIFT_Real dsq, mdsq;
    SWIFT_Tri_Edge* e;

    mdsq = SWIFT_INFINITY;
    for( i = 0; i < Num_Faces(); i++ ) {
        dsq = pt.Dist_Sq( faces[i].Edge1().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = faces[i].Edge1P();
        }
        dsq = pt.Dist_Sq( faces[i].Edge2().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = faces[i].Edge2P();
        }
        dsq = pt.Dist_Sq( faces[i].Edge3().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = faces[i].Edge3P();
        }
    }

    for( i = 0; i < Num_Other_Faces(); i++ ) {
        dsq = pt.Dist_Sq( other_faces[i]->Edge1().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = other_faces[i]->Edge1P();
        }
        dsq = pt.Dist_Sq( other_faces[i]->Edge2().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = other_faces[i]->Edge2P();
        }
        dsq = pt.Dist_Sq( other_faces[i]->Edge3().Origin()->Coords() );
        if( dsq < mdsq ) {
            mdsq = dsq;
            e = other_faces[i]->Edge3P();
        }
    }

    return e;
}

// Does not visit vertices that have already been visited.
SWIFT_Real SWIFT_BV::Extremal_Vertex( const SWIFT_Triple& dir,
                                      int level, SWIFT_Tri_Edge*& starte )
{
    SWIFT_Real d1, d2;
    SWIFT_Tri_Edge* e = starte->Twin( level );
    SWIFT_Tri_Edge* nexte = NULL;
    SWIFT_Tri_Edge* ende = e;

    d1 = dir * starte->Origin()->Coords();

    d2 = dir * e->Origin()->Coords();
    if( d2 > d1 ) {
        d1 = d2;
        nexte = e;
    }
    e = e->Next()->Twin( level );

    while( true ) {
        // Find the neighbor that has a greater distance
        for( ; e != ende; e = e->Next()->Twin( level ) ) {
            d2 = dir * e->Origin()->Coords();
            if( d2 > d1 ) {
                d1 = d2;
                nexte = e;
            }
        }
        if( nexte == NULL ) {
            break;
        }
        // Set up the edges for the next iteration
        e = nexte->Twin( level )->Next()->Twin( level )->Next()->Twin( level );
        ende = nexte->Prev();
        starte = nexte;
        nexte = NULL;
    }
    return d1;
}


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Mesh public functions
//////////////////////////////////////////////////////////////////////////////

void SWIFT_Tri_Mesh::Prepare_For_Delete( )
{
    if( big_arrays ) {
        // Have to nullify the bvs' faces arrays
        int i;
        for( i = 0; i < Num_BVs(); i++ ) {
            bvs[i].Faces().Nullify();
            bvs[i].Other_Faces().Nullify();
        }
        // Set all the edge twin pointers to NULL
        faces.Set_Length( faces.Max_Length() );
        for( i = 0; i < faces.Max_Length(); i++ ) {
            faces[i].Edge1().Nullify_Twins();
            faces[i].Edge2().Nullify_Twins();
            faces[i].Edge3().Nullify_Twins();
        }
    }
}

void SWIFT_Tri_Mesh::Set_Faces_Classification( int c )
{
    int i;

    for( i = 0; i < Num_Faces(); i++ ) {
        faces[i].Set_Classification( c );
    }
}

bool SWIFT_Tri_Mesh::Create( const SWIFT_Real* vs, const int* fs,
                             int vn, int fn, const SWIFT_Orientation& orient,
                             const SWIFT_Translation& trans, SWIFT_Real sc,
                             const int* fv
                             )
{
#if defined(SWIFT_HIERARCHY) && defined(SWIFT_QSLIM_HIER)
    // Create a QSlim simplification hierarchy
    int i, j;
    MxStdModel* mdl;
    const SWIFT_Real* qverts[MAX_LEVELS];
    const int* qfaces[MAX_LEVELS];
    int qvert_lens[MAX_LEVELS];
    int qface_lens[MAX_LEVELS];
    SWIFT_Orientation* qorient;
    SWIFT_Translation* qtrans;
    SWIFT_Real* qsc;
    int face_target;
    int vnn, tn;
    const SWIFT_Real* new_vs;
    const int* new_fs;
    SWIFT_Real* read_qverts;
    int* chull_faces;
    SWIFT_Array<int> triang_edges;
    SWIFT_Array<int> vmap; 
    SWIFT_Array<int> vnewi;

    // Initialize the other level arrays.
    for( i = 1; i < MAX_LEVELS; i++ ) {
        qverts[i] = NULL;
        qfaces[i] = NULL;
    }

    // Initialize the transformation arrays
    qorient = new SWIFT_Orientation[MAX_LEVELS];
    for( i = 0; i < MAX_LEVELS; i++ ) {
        for( j = 0; j < 9; j++ ) {
            qorient[i][j] = orient[j];
        }
    }

    qtrans = new SWIFT_Translation[MAX_LEVELS];
    for( i = 0; i < MAX_LEVELS; i++ ) {
        for( j = 0; j < 3; j++ ) {
            qtrans[i][j] = trans[j];
        }
    }

    qsc = new SWIFT_Real[MAX_LEVELS];
    for( i = 0; i < MAX_LEVELS; i++ ) {
        qsc[i] = sc;
    }

    // No need to transform the vertices because that will be done by the
    // other create call.  Duplicate vertices are removed here because we
    // want QSlim to behave properly.
    Process_Vertices( vs, vn, vnn, new_vs, vmap, vnewi );

    // Triangulate the faces and delete the faces array that is created
    Process_Faces( fs, fn, fv, tn, new_fs, triang_edges );

    // Take care of the vertices and the faces as a result of handling
    // duplication and triangulation
    qvert_lens[0] = vnn;
    qverts[0] = new_vs;
    qface_lens[0] = tn;
    if( no_dup_verts ) {
        // There were no duplicate vertices.  Simply assign the original verts.
        // And assign the new faces.
        qfaces[0] = new_fs;
    } else {
        // Remap the qfaces array
        chull_faces = new int[tn*3];
        for( i = 0, j = 0; i < tn; i++, j += 3 ) {
            chull_faces[j] = vnewi[ vmap[ new_fs[j] ] ];
            chull_faces[j+1] = vnewi[ vmap[ new_fs[j+1] ] ];
            chull_faces[j+2] = vnewi[ vmap[ new_fs[j+2] ] ];
        }
        qfaces[0] = chull_faces;
#ifdef WIN32
        delete (void*)new_fs;
#else
        delete new_fs;
#endif
    }

    // Write the arrays to the model
    //Write( &mdl, qverts[0], qfaces[0], qvert_lens[0], qface_lens[0] );

    // Compute the face target
    face_target = (int)(tratio*(SWIFT_Real)qface_lens[0]);
    for( j = 1; j < MAX_LEVELS && face_target > tcount; j++ ) {
        // Write the arrays to the model
        Write( &mdl, qverts[j-1], qfaces[j-1],
                     qvert_lens[j-1], qface_lens[j-1] );

        // Simplify the model to the desired face target
        SWIFT_QSlim( face_target, mdl );

        // Read the simplified model into the arrays
        Read( mdl, read_qverts, qvert_lens[j] );
        qverts[j] = read_qverts;

        // Make the model convex.  qfaces are allocated here.
        Compute_Convex_Hull( qverts[j], qvert_lens[j],
                             chull_faces, qface_lens[j] );
        qfaces[j] = chull_faces;

        // Compute the next face target
        face_target = (int)(tratio*(SWIFT_Real)face_target);

        delete mdl;
    }

    // Call the hierarchy-given creation function.  fv is given as NULL since
    // all the faces produced by qhull are triangles.
    Create( qverts, qfaces, qvert_lens, qface_lens, j, true, &triang_edges,
            qorient, qtrans, qsc );

    // Delete everything
    if( !no_dup_verts ) {
        delete qverts[0];
    }

#ifdef SWIFT_ONLY_TRIS
    if( !no_dup_verts ) {
#else
    if( !only_tris || !no_dup_verts ) {
#endif
        delete qfaces[0];
    }

    for( i = 1; i < j; i++ ) {
        delete qverts[i];
        delete qfaces[i];
    }

    delete qorient;
    delete qtrans;
    delete qsc;

    return true;
#else
    int i, j;
    int e, f;
    int vnn, tn;
    const SWIFT_Real* new_vs;
    const int* new_fs;
    SWIFT_Tri_Vertex* tv;
    SWIFT_Tri_Edge* te;
    SWIFT_Array<SWIFT_Tri_Edge*> sort_edges;
    SWIFT_Array<SWIFT_Tri_Face> temp_faces;
    SWIFT_Array<int> triang_edges;
    SWIFT_Array<int> vmap; 
    SWIFT_Array<int> vnewi;

    // Remove duplicate vertices
    Process_Vertices( vs, vn, vnn, new_vs, vmap, vnewi );

    // Create the vertices
    verts.Create( vnn );

    // Transform the vertices
    Transform_Vertices( new_vs, vnn, orient, trans, sc );

    // Delete the vertex array copy if there were duplicate vertices
    if( !no_dup_verts ) {
#ifdef WIN32
        delete (void*)new_vs;
#else
        delete new_vs;
#endif
    }

    // Triangulate faces.
    Process_Faces( fs, fn, fv, tn, new_fs, triang_edges );

    // Create the faces
    faces.Create( tn );

    // Create the connecting faces and edge ptrs.
    temp_faces.Create( tn );
    sort_edges.Create( tn*6 );

    // For each face, create one oriented as given and one oriented the
    // opposite direction
    for( e = 0, f = 0, i = 0; i < tn; i++ ) {
        // Create the face oriented as given
        tv = verts( vnewi[ vmap[ new_fs[f++] ] ] );
        tv->Set_Adj_Edge( faces[i].Edge1P() );
        faces[i].Edge1().Set_Origin( tv );
        faces[i].Edge1().Set_Twin( temp_faces[i].Edge2P() );
        sort_edges[e++] = faces[i].Edge1P();

        tv = verts( vnewi[ vmap[ new_fs[f++] ] ] );
        tv->Set_Adj_Edge( faces[i].Edge2P() );
        faces[i].Edge2().Set_Origin( tv );
        faces[i].Edge2().Set_Twin( temp_faces[i].Edge1P() );
        sort_edges[e++] = faces[i].Edge2P();

        tv = verts( vnewi[ vmap[ new_fs[f++] ] ] );
        tv->Set_Adj_Edge( faces[i].Edge3P() );
        faces[i].Edge3().Set_Origin( tv );
        faces[i].Edge3().Set_Twin( temp_faces[i].Edge3P() );
        sort_edges[e++] = faces[i].Edge3P();

        // Create the reversed face
        temp_faces[i].Edge1().Set_Origin( faces[i].Edge3().Origin() );
        temp_faces[i].Edge1().Set_Twin( faces[i].Edge2P() );
        sort_edges[e++] = temp_faces[i].Edge1P();

        temp_faces[i].Edge2().Set_Origin( faces[i].Edge2().Origin() );
        temp_faces[i].Edge2().Set_Twin( faces[i].Edge1P() );
        sort_edges[e++] = temp_faces[i].Edge2P();

        temp_faces[i].Edge3().Set_Origin( faces[i].Edge1().Origin() );
        temp_faces[i].Edge3().Set_Twin( faces[i].Edge3P() );
        sort_edges[e++] = temp_faces[i].Edge3P();
    }

#ifndef SWIFT_ONLY_TRIS
    // Don't need the new_fs array anymore
    if( !only_tris ) {
#ifdef WIN32
        delete (void*)new_fs;
#else
        delete new_fs;
#endif
    }
#endif

    // Sort the edges lexicographically
    Quicksort( sort_edges, 0, tn*6-1 );


    // Run through the sorted list and connect the mesh.
    tn *= 6;
    for( i = 0; i < tn; ) {
        for( ; i < tn && sort_edges[i]->Marked(); i++ ) {
            sort_edges[i]->Unmark();
        }

        if( i == tn ) break;

        j = i + 1;
        if( j < tn && sort_edges[i]->Origin() == sort_edges[j]->Origin() &&
            sort_edges[i]->Twin()->Origin() == sort_edges[j]->Twin()->Origin()
        ) {
            sort_edges[i]->Twin()->Mark();
            sort_edges[j]->Twin()->Mark();

            // Found a pair of edges that are equal.  But are there three?
            if( j < tn - 1 &&
                sort_edges[j+1]->Origin() == sort_edges[j]->Origin() &&
                sort_edges[j+1]->Twin()->Origin() ==
                                                sort_edges[j]->Twin()->Origin()
            ) {
                // User error report
                cerr << "Error: The mesh is not a 2-manifold --" << endl
                     << "       All edges may belong to at most 2 faces."
                     << endl
                     << "       An edge has been found that belongs to 3 or"
                     << " more faces."
                     << endl
                     << "       The vertices of the edge are "
                     << Map_Vertex_Id( sort_edges[j]->Origin() ) << ", "
                     << Map_Vertex_Id( sort_edges[j]->Twin()->Origin() )
                     << "." << endl;
                return false;
            } else {
                // Check the pair for proper orientation
                SWIFT_Tri_Face* fi = sort_edges[i]->Adj_Face();
                SWIFT_Tri_Face* fj = sort_edges[j]->Adj_Face();
                if( !((Face_Id( fi ) >= 0 && Face_Id( fi ) < Num_Faces()) ||
                      (Face_Id( fj ) >= 0 && Face_Id( fj ) < Num_Faces()))
                ) {
                    // User error report
                    cerr << "Error: The mesh is not oriented correctly --"
                         << endl
                         << "       All faces must be oriented CCW from the"
                         << " outside."
                         << endl
                         << "       An edge has been found to be going in the"
                         << " same direction for two"
                         << endl << "different faces."
                         << "  The vertices of the edge are "
                         << Map_Vertex_Id( sort_edges[j]->Origin() ) << ", "
                         << Map_Vertex_Id( sort_edges[j]->Twin()->Origin() )
                         << "." << endl;
                    return false;
                }

                // Connect the pair appropriately
                sort_edges[i]->Twin()->Set_Twin( sort_edges[j] );
                sort_edges[j]->Twin()->Set_Twin( sort_edges[i] );
                te = sort_edges[i]->Twin();
                sort_edges[i]->Set_Twin( sort_edges[j]->Twin() );
                sort_edges[j]->Set_Twin( te );
                i += 2;
                j += 2;
            }
        } else {
            // Found a lone edge.
            // It must be on the boundary so snip it.
            sort_edges[i]->Twin()->Mark();
            sort_edges[i]->Twin()->Set_Twin( NULL );
            sort_edges[i]->Set_Twin( NULL );
            i++;
            j++;
        }
    }

    Compute_Geometry();
    Compute_Center_Of_Mass();
    Compute_Radius();

    // If there are new edges resulting from face triangulation, then fix the
    // edge-face planes on them to be twin-identical
    for( i = 0; i < tn/6 - fn; i++ ) {
        faces[triang_edges[i]].Edge3().Twin()->Set_Face_Distance(
                            -faces[triang_edges[i]].Edge3().Face_Distance() );
        faces[triang_edges[i]].Edge3().Twin()->Set_Face_Normal(
                            -faces[triang_edges[i]].Edge3().Face_Normal() );
    }

#ifdef SWIFT_DEBUG
    cerr << "******* SWIFT_Tri_Mesh::Create()'1 Verification *******" << endl;
    Verify();
#endif



#endif
    return true;
}

SWIFT_Tri_Mesh* SWIFT_Tri_Mesh::Clone( const SWIFT_Orientation& orient,
                                       const SWIFT_Translation& trans,
                                       SWIFT_Real sc )
{
    int i, j;
    SWIFT_Tri_Mesh* result = new SWIFT_Tri_Mesh;

    result->Vertices().Create( Num_Vertices() );
    result->Faces().Create( Num_Faces() );

    result->Transform_Vertices_And_COM( this, orient, trans, sc );

    // Make the vertices point to the correct edges and give them coordinates
    for( i = 0; i < Num_Vertices(); i++ ) {
        j = Edge_Id( verts[i].Adj_Edge() );
        result->Vertices()[i].Set_Adj_Edge( result->EdgeP( j ) );
    }

    // Make the edges point to the correct vertices, edges, and faces
    for( i = 0; i < Num_Faces(); i++ ) {
        j = Edge_Id( faces[i].Edge1().Twin() );
        result->Faces()[i].Edge1().Set_Twin( result->EdgeP( j ) );
        result->Faces()[i].Edge1().Set_Origin( result->Vertices()(
                                    Vertex_Id( faces[i].Edge1().Origin() ) ) );

        j = Edge_Id( faces[i].Edge2().Twin() );
        result->Faces()[i].Edge2().Set_Twin( result->EdgeP( j ) );
        result->Faces()[i].Edge2().Set_Origin( result->Vertices()(
                                    Vertex_Id( faces[i].Edge2().Origin() ) ) );

        j = Edge_Id( faces[i].Edge3().Twin() );
        result->Faces()[i].Edge3().Set_Twin( result->EdgeP( j ) );
        result->Faces()[i].Edge3().Set_Origin( result->Vertices()(
                                    Vertex_Id( faces[i].Edge3().Origin() ) ) );
    }

    // Compute the geometry
    for( i = 0; i < Num_Faces(); i++ ) {
        result->Faces()[i].Edge1().Compute_Direction_Length_Twin();
        result->Faces()[i].Edge2().Compute_Direction_Length_Twin();
        result->Faces()[i].Edge3().Compute_Direction_Length_Twin();
        result->Faces()[i].Compute_Plane_From_Edges();
        result->Faces()[i].Edge1().Compute_Voronoi_Planes();
        result->Faces()[i].Edge2().Compute_Voronoi_Planes();
        result->Faces()[i].Edge3().Compute_Voronoi_Planes();
    }

    result->Set_Radius( sc * radius );

#ifdef SWIFT_DEBUG
    cerr << "******* SWIFT_Tri_Mesh::Clone() Verification *******" << endl;
    Verify(
            );
#endif

    return result;
}


void SWIFT_Tri_Mesh::Create_Twins( SWIFT_BV* bv )
{
    int i, j;
    SWIFT_Array<SWIFT_Tri_Edge*> sort_edges;
    SWIFT_Array<SWIFT_Tri_Face> temp_faces;
    SWIFT_Tri_Edge* tempe;
    int se, tf;
    const int nt = bv->Faces().Length() + bv->Other_Faces().Length();

    // First, create all the twin lists
    if( bv->Is_Leaf() ) {
        // Create and assign twins for all the faces in the leaves and the
        // original models faces too
        for( i = 0; i < bv->Faces().Length(); i++ ) {
            const int twins_len = bv->Level() -
                                  bv->Faces()[i].Starting_Level() + 1;
            bv->Faces()[i].Edge1().Create_Twins( twins_len );
            bv->Faces()[i].Edge2().Create_Twins( twins_len );
            bv->Faces()[i].Edge3().Create_Twins( twins_len );
            bv->Faces()[i].Set_Twins_Length( twins_len );
        }
        for( i = 0; i < bv->Other_Faces().Length(); i++ ) {
            const int twins_len = bv->Level() -
                                  bv->Other_Faces()[i]->Starting_Level() + 1;
            bv->Other_Faces()[i]->Edge1().Create_Twins( twins_len );
            bv->Other_Faces()[i]->Edge2().Create_Twins( twins_len );
            bv->Other_Faces()[i]->Edge3().Create_Twins( twins_len );
            bv->Other_Faces()[i]->Set_Twins_Length( twins_len );
        }
    } else {
        // First handle all the children
        for( i = 0; i < bv->Num_Children(); i++ ) {
            Create_Twins( bv->Children()[i] );
        }
        for( i = 0; i < bv->Faces().Length(); i++ ) {
            const int twins_len = bv->Level() -
                                  bv->Faces()[i].Starting_Level() + 1;
            bv->Faces()[i].Edge1().Create_Twins( twins_len );
            bv->Faces()[i].Edge2().Create_Twins( twins_len );
            bv->Faces()[i].Edge3().Create_Twins( twins_len );
            bv->Faces()[i].Set_Twins_Length( twins_len );
        }
    }

    // Second, figure out twins for this bv and set them appropriately
    sort_edges.Create( nt * 6 );
    temp_faces.Create( nt );

    // For each face, create one oriented as given and one oriented the
    // opposite direction
    for( se = 0, tf = 0, i = 0; i < bv->Faces().Length(); i++, tf++ ) {
        // Create the face oriented as given
        sort_edges[se++] = bv->Faces()[i].Edge1P();
        sort_edges[se++] = bv->Faces()[i].Edge2P();
        sort_edges[se++] = bv->Faces()[i].Edge3P();

        // Create the reversed face
        temp_faces[tf].Edge1().Set_Origin( bv->Faces()[i].Edge3().Origin() );
        temp_faces[tf].Edge1().Set_Twin( bv->Faces()[i].Edge2P() );
        bv->Faces()[i].Edge2().Set_Twin( temp_faces[tf].Edge1P() );
        sort_edges[se++] = temp_faces[tf].Edge1P();

        temp_faces[tf].Edge2().Set_Origin( bv->Faces()[i].Edge2().Origin() );
        temp_faces[tf].Edge2().Set_Twin( bv->Faces()[i].Edge1P() );
        bv->Faces()[i].Edge1().Set_Twin( temp_faces[tf].Edge2P() );
        sort_edges[se++] = temp_faces[tf].Edge2P();

        temp_faces[tf].Edge3().Set_Origin( bv->Faces()[i].Edge1().Origin() );
        temp_faces[tf].Edge3().Set_Twin( bv->Faces()[i].Edge3P() );
        bv->Faces()[i].Edge3().Set_Twin( temp_faces[tf].Edge3P() );
        sort_edges[se++] = temp_faces[tf].Edge3P();
    }

    for( i = 0; i < bv->Other_Faces().Length(); i++, tf++ ) {
        // Create the face oriented as given
        sort_edges[se++] = bv->Other_Faces()[i]->Edge1P();
        sort_edges[se++] = bv->Other_Faces()[i]->Edge2P();
        sort_edges[se++] = bv->Other_Faces()[i]->Edge3P();

        // Create the reversed face
        temp_faces[tf].Edge1().Set_Origin(
                                bv->Other_Faces()[i]->Edge3().Origin() );
        temp_faces[tf].Edge1().Set_Twin( bv->Other_Faces()[i]->Edge2P() );
        bv->Other_Faces()[i]->Edge2().Set_Twin( temp_faces[tf].Edge1P() );
        sort_edges[se++] = temp_faces[tf].Edge1P();

        temp_faces[tf].Edge2().Set_Origin(
                                bv->Other_Faces()[i]->Edge2().Origin() );
        temp_faces[tf].Edge2().Set_Twin( bv->Other_Faces()[i]->Edge1P() );
        bv->Other_Faces()[i]->Edge1().Set_Twin( temp_faces[tf].Edge2P() );
        sort_edges[se++] = temp_faces[tf].Edge2P();

        temp_faces[tf].Edge3().Set_Origin(
                                bv->Other_Faces()[i]->Edge1().Origin() );
        temp_faces[tf].Edge3().Set_Twin( bv->Other_Faces()[i]->Edge3P() );
        bv->Other_Faces()[i]->Edge3().Set_Twin( temp_faces[tf].Edge3P() );
        sort_edges[se++] = temp_faces[tf].Edge3P();
    }

    // Sort the edges lexicographically
    Quicksort( sort_edges, 0, nt*6-1 );


    // Run through the sorted list and connect the mesh.
    for( i = 0; i < nt*6; ) {
        for( ; i < nt*6 && sort_edges[i]->Marked(); i++ ) {
            sort_edges[i]->Unmark();
        }
        if( i == nt*6 ) break;

        j = i + 1;
        if( j < nt*6 &&
            sort_edges[i]->Origin() == sort_edges[j]->Origin() &&
            sort_edges[i]->Twin()->Origin() == sort_edges[j]->Twin()->Origin()
        ) {
            // Mark the two twins so that they do not get processed
            sort_edges[i]->Twin()->Mark();
            sort_edges[j]->Twin()->Mark();

            // Found a pair of edges that are equal.  But are there three?
            if( j < nt*6-1 &&
                sort_edges[j+1]->Origin() == sort_edges[j]->Origin() &&
                sort_edges[j+1]->Twin()->Origin() ==
                                                sort_edges[j]->Twin()->Origin()
            ) {
                cerr << "Internal Error: The bv mesh is not a 2-manifold --"
                     << endl
                     << "       All edges must belong to exactly 2 faces."
                     << endl
                     << "       An edge has been found that belongs to 3 or"
                     << " more faces."
                     << endl;
                return;
            } else {
                // Connect the pair appropriately
                sort_edges[i]->Twin()->Set_Twin( sort_edges[j] );
                sort_edges[j]->Twin()->Set_Twin( sort_edges[i] );
                tempe = sort_edges[i]->Twin();
                sort_edges[i]->Set_Twin( sort_edges[j]->Twin() );
                sort_edges[j]->Set_Twin( tempe );
                i += 2;
                j += 2;
            }
        } else {
            // Found a lone edge.
            cerr << "Internal Error: The bv mesh is not closed --" << endl
                 << "       All edges must belong to exactly 2 faces."
                 << endl
                 << "       An edge has been found that belongs to only 1 face."
                 << endl;
            return;
        }
    }

    // The edges on the faces of this bv have correct twins for this bv
    // Set the twins in the faces' twin lists
    for( i = 0; i < bv->Faces().Length(); i++ ) {
        const int j = bv->Level() - bv->Faces()[i].Starting_Level();
        bv->Faces()[i].Edge1().Copy_Twin( j );
        bv->Faces()[i].Edge2().Copy_Twin( j );
        bv->Faces()[i].Edge3().Copy_Twin( j );
    }

    for( i = 0; i < bv->Other_Faces().Length(); i++ ) {
        const int j = bv->Level() - bv->Other_Faces()[i]->Starting_Level();
        bv->Other_Faces()[i]->Edge1().Copy_Twin( j );
        bv->Other_Faces()[i]->Edge2().Copy_Twin( j );
        bv->Other_Faces()[i]->Edge3().Copy_Twin( j );
    }
}

void SWIFT_Tri_Mesh::Create_BV_Subtree( int& bvid, SPLIT_TYPE split,
                         SWIFT_Array<SWIFT_BV*>& leaves,
                         SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face*> >& ofmap,
                         SWIFT_Array<SWIFT_Tri_Face*>& st_faces,
                         SWIFT_Array<SWIFT_Tri_Edge*>& st_twins )
{
    const int this_bvid = bvid;
    int i, j, k;
    SWIFT_Array<SWIFT_BV*> children;

        children.Create( 2 );
        if( leaves.Length() == 2 ) {
            children[0] = leaves[0];
            children[0]->Set_Level( bvs[this_bvid].Level()+1 );
            children[0]->Set_Parent( bvs(this_bvid) );
            children[1] = leaves[1];
            children[1]->Set_Level( bvs[this_bvid].Level()+1 );
            children[1]->Set_Parent( bvs(this_bvid) );
            height = max( height, bvs[this_bvid].Level()+1 );
        } else {
            SWIFT_Triple max_dir;
            SWIFT_Array<SWIFT_BV*> leaves1;
            SWIFT_Array<SWIFT_BV*> leaves2;
            SWIFT_Array<SWIFT_Real> dir_vals( leaves.Length() );

            // Compute the convex hull of the center of masses of the leaves
            SWIFT_Array<SWIFT_Tri_Vertex> vs( leaves.Length() );
            for( i = 0; i < leaves.Length(); i++ ) {
                vs[i].Set_Coords( leaves[i]->Center_Of_Mass() );
            }
            Compute_Max_Spread_Direction( vs, max_dir );

            for( i = 0; i < leaves.Length(); i++ ) {
                dir_vals[i] = max_dir * leaves[i]->Center_Of_Mass();
            }

            if( split == MEDIAN ) {
                ::Quicksort( dir_vals, leaves, 0, dir_vals.Length()-1 );

                leaves1.Create( leaves.Length()>>1 );
                leaves2.Create( (leaves.Length()+1)>>1 );

                for( i = 0; i < (leaves.Length()>>1); i++ ) {
                    leaves1[i] = leaves[i];
                }
                for( j = 0; i < leaves.Length(); i++, j++ ) {
                    leaves2[j] = leaves[i];
                }
            } else if( split == MIDPOINT ) {
                SWIFT_Real minv = SWIFT_INFINITY, maxv = -SWIFT_INFINITY;

                // Determine the midpoint of the spread
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( dir_vals[i] > maxv ) {
                        maxv = dir_vals[i];
                    }
                    if( dir_vals[i] < minv ) {
                        minv = dir_vals[i];
                    }
                }
                minv = 0.5 * (minv+maxv);

                leaves1.Create( leaves.Length() );
                leaves2.Create( leaves.Length() );
                leaves1.Set_Length( 0 );
                leaves2.Set_Length( 0 );

                // Separate the leaves
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( minv > dir_vals[i] ) {
                        leaves1.Add( leaves[i] );
                    } else {
                        leaves2.Add( leaves[i] );
                    }
                }
            } else if( split == MEAN ) {
                SWIFT_Real mean = 0.0;

                // Determine the mean
                for( i = 0; i < leaves.Length(); i++ ) {
                    mean += dir_vals[i];
                }
                mean /= leaves.Length();

                leaves1.Create( leaves.Length() );
                leaves2.Create( leaves.Length() );
                leaves1.Set_Length( 0 );
                leaves2.Set_Length( 0 );

                // Separate the leaves
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( mean > dir_vals[i] ) {
                        leaves1.Add( leaves[i] );
                    } else {
                        leaves2.Add( leaves[i] );
                    }
                }
            } else {
                // Try to find a well-defined gap
                SWIFT_Real largest_gap = 0.0;
                SWIFT_Real max_spread = 0.0;

                ::Quicksort( dir_vals, leaves, 0, dir_vals.Length()-1 );

                // Compute the largest and the second largest gaps
                for( i = 1; i < leaves.Length(); i++ ) {
                    if( dir_vals[i]-dir_vals[i-1] > largest_gap ) {
                        max_spread = largest_gap;
                        largest_gap = dir_vals[i]-dir_vals[i-1];
                        j = i;
                    } else if( dir_vals[i]-dir_vals[i-1] > max_spread ) {
                        max_spread = dir_vals[i]-dir_vals[i-1];
                    }
                }

                // The largest gap must be within the central 60% of the leaves
                // and must be 2 times as large as the second largest gap and
                // must span at least 10% of the entire range
                if( j > leaves.Length()*0.2 && j < leaves.Length()*0.8 &&
                    largest_gap > max_spread*2.0 &&
                    largest_gap > (dir_vals.Last()-dir_vals[0])*0.1
                ) {
                    leaves1.Create( j );
                    leaves2.Create( leaves.Length()-j );
                    for( i = 0; i < j; i++ ) {
                        leaves1[i] = leaves[i];
                    }
                    for( j = 0; i < leaves.Length(); i++, j++ ) {
                        leaves2[j] = leaves[i];
                    }
                } else {
                    // Do mean
                    SWIFT_Real mean = 0.0;

                    leaves1.Create( leaves.Length() );
                    leaves2.Create( leaves.Length() );
                    leaves1.Set_Length( 0 );
                    leaves2.Set_Length( 0 );

                    // Determine the mean
                    for( i = 0; i < leaves.Length(); i++ ) {
                        mean += dir_vals[i];
                    }
                    mean /= leaves.Length();

                    for( i = 0; i < leaves.Length(); i++ ) {
                        if( mean > dir_vals[i] ) {
                            leaves1.Add( leaves[i] );
                        } else {
                            leaves2.Add( leaves[i] );
                        }
                    }
                }
            }

            if( leaves1.Length() > 1 ) {
                ++bvid;
                children[0] = bvs(bvid);
                children[0]->Set_Parent( bvs(this_bvid) );
                // This must be set before the child is created
                children[0]->Set_Level( bvs[this_bvid].Level()+1 );
                Create_BV_Subtree( bvid, split, leaves1, ofmap, st_faces,
                                                                st_twins );
            } else {
                children[0] = leaves1[0];
                children[0]->Set_Level( bvs[this_bvid].Level()+1 );
                children[0]->Set_Parent( bvs(this_bvid) );
            }
            if( leaves2.Length() > 1 ) {
                ++bvid;
                children[1] = bvs(bvid);
                children[1]->Set_Parent( bvs(this_bvid) );
                // This must be set before the child is created
                children[1]->Set_Level( bvs[this_bvid].Level()+1 );
                Create_BV_Subtree( bvid, split, leaves2, ofmap, st_faces,
                                                                st_twins );
            } else {
                children[1] = leaves2[0];
                children[1]->Set_Level( bvs[this_bvid].Level()+1 );
                children[1]->Set_Parent( bvs(this_bvid) );
            }
        }

        // These must be set after the children are created
        children[0]->Set_Faces_Level( bvs[this_bvid].Level()+1 );
        children[1]->Set_Faces_Level( bvs[this_bvid].Level()+1 );

    // Create the mesh of this bv
    SWIFT_Array<SWIFT_Tri_Vertex*> vs( Num_Vertices() );
    SWIFT_Array<bool> vinclude( Num_Vertices() );
    SWIFT_Array<int> vmapping( Num_Vertices() );
    SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face*> > vfmap( Num_Vertices() );
    SWIFT_Array<SWIFT_Tri_Face> new_faces;
    SWIFT_Array<int> new_face_ids;
    SWIFT_Array<SWIFT_Tri_Face*> matched_faces;
    SWIFT_Array<SWIFT_Tri_Face*> other_faces;
    int* fs;
    int fn;

    // Create and initialize the vertex list and create the vfmap lists
    // Also handle the vertex mappings
    vmapping.Set_Length( 0 );
    vs.Set_Length( 0 );

    for( i = 0; i < Num_Vertices(); i++ ) {
        vinclude[i] = false;
    }
    for( i = 0; i < children.Length(); i++ ) {
        for( j = 0; j < children[i]->Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Faces()[j].Edge1().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Faces()[j].Edge1().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
            k = Vertex_Id( children[i]->Faces()[j].Edge2().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Faces()[j].Edge2().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
            k = Vertex_Id( children[i]->Faces()[j].Edge3().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Faces()[j].Edge3().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
        }
        for( j = 0; j < children[i]->Other_Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge1().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Other_Faces()[j]->Edge1().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge2().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Other_Faces()[j]->Edge2().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge3().Origin() );
            if( !vinclude[k] ) {
                // This vertex has not yet been included
                vs.Add( children[i]->Other_Faces()[j]->Edge3().Origin() );
                vinclude[k] = true;
                vmapping.Add( k );
            }
        }
    }

    Compute_Convex_Hull( vs, fs, fn );

    bvs[this_bvid].Set_Children( children );

    // For every child insert its faces into the vfmap table.  Also compute
    // the child's lookup table here.
    for( i = 0; i < children.Length(); i++ ) {

        children[i]->Compute_Radius();  // LUT needs this
        //children[i]->Compute_Volume();
        children[i]->Create_Lookup_Table();
        //children[i]->Dump_Lookup_Table();

        // Process the child's owned faces
        for( j = 0; j < children[i]->Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Faces()[j].Edge1().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );

            k = Vertex_Id( children[i]->Faces()[j].Edge2().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );

            k = Vertex_Id( children[i]->Faces()[j].Edge3().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );
        }

        // Process the child's other faces
        for( j = 0; j < children[i]->Other_Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge1().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );

            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge2().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );

            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge3().Origin() );
#ifdef SWIFT_DEBUG
            if( !vinclude[k] ) {
cerr << "****************** vertex not included" << endl;
            }
#endif
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );
        }
    }

    matched_faces.Create( fn );
    matched_faces.Set_Length( 0 );
    new_face_ids.Create( fn );
    new_face_ids.Set_Length( 0 );

    for( i = 0; i < fn*3; i++ ) {
        fs[i] = vmapping[fs[i]];
    }

    // Second thing to do is find faces on the convex hull
    for( i = 0; i < fn*3; i += 3 ) {
        for( j = 0; j < vfmap[fs[i]].Length(); j++ ) {
            k = Vertex_Id( vfmap[fs[i]][j]->Edge1().Origin() );
            if( k == fs[i] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i+1] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i+2]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            } else if( k == fs[i+1] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i+2] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            } else if( k == fs[i+2] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i+1]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            }
        }
        if( j == vfmap[fs[i]].Length() ) {
            new_face_ids.Add( i );
        }
    }

    // Copy to the other faces
    other_faces.Copy_Length( matched_faces );

    // Create and fill in the new faces
    new_faces.Create( new_face_ids.Length() );

    for( i = 0; i < new_face_ids.Length(); i++ ) {
        // Set the vertices
        new_faces[i].Edge1().Set_Origin( verts(fs[new_face_ids[i]]) );
        new_faces[i].Edge2().Set_Origin( verts(fs[new_face_ids[i]+1]) );
        new_faces[i].Edge3().Set_Origin( verts(fs[new_face_ids[i]+2]) );

        // Try to find this face in the original faces.  If it exists there,
        // make it have class ORIGINAL
        SWIFT_Array<SWIFT_Tri_Face*>& search_fs = ofmap[fs[new_face_ids[i]]];
        for( j = 0; j < search_fs.Length(); j++ ) {
            k = Vertex_Id( search_fs[j]->Edge1().Origin() );
            if( k == fs[new_face_ids[i]] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]+1] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]+2]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    break;
                }
            } else if( k == fs[new_face_ids[i]+1] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]+2] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    break;
                }
            } else if( k == fs[new_face_ids[i]+2] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]+1]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    break;
                }
            }
        }

        if( j == search_fs.Length() ) {
            // Did not find the face.  Set its classification to be FREE.
            new_faces[i].Set_Classification( CLASS_FREE );
        } else {
            st_faces.Add_Grow( new_faces(i), 100 );
            st_twins.Add_Grow( new_faces[i].Edge1().Twin(), 300 );
            st_twins.Add( new_faces[i].Edge2().Twin() );
            st_twins.Add( new_faces[i].Edge3().Twin() );
        }

        // Compute the geometry
        new_faces[i].Edge1().Compute_Direction_Length();
        new_faces[i].Edge2().Compute_Direction_Length();
        new_faces[i].Edge3().Compute_Direction_Length();
        new_faces[i].Compute_Plane_From_Edges();
        new_faces[i].Edge1().Compute_Voronoi_Planes();
        new_faces[i].Edge2().Compute_Voronoi_Planes();
        new_faces[i].Edge3().Compute_Voronoi_Planes();
    }

    bvs[this_bvid].Set_Faces( new_faces );
    bvs[this_bvid].Set_Other_Faces( other_faces );
    bvs[this_bvid].Compute_Center_Of_Mass();

    // Cleanup the lists so that they do not get destroyed
    delete fs;
    new_faces.Nullify();
    other_faces.Nullify();
    children.Nullify();
}

#ifdef SWIFT_DEBUG
void SWIFT_Tri_Mesh::Check_Face_Levels( )
{
    Check_Face_Levels( bvs(0), 0 );
}

void SWIFT_Tri_Mesh::Check_Face_Levels( SWIFT_BV* bv, int level )
{
    int i;
    for( i = 0; i < bv->Num_Children(); i++ ) {
        Check_Face_Levels( bv->Children()[i], level+1 );
    }
    for( i = 0; i < bv->Num_Faces(); i++ ) {
        if( bv->Faces()[i].Starting_Level() > level ||
            bv->Faces()[i].Starting_Level() < 0
        ) {
            cerr << "*********** Face at pos " << i << " at level " << level
                 << " does not have correct level "
                 << bv->Faces()[i].Starting_Level() << endl;
        }
    }
    for( i = 0; i < bv->Num_Other_Faces(); i++ ) {
        if( bv->Other_Faces()[i]->Starting_Level() > level ||
            bv->Other_Faces()[i]->Starting_Level() < 0
        ) {
            cerr << "*********** Other Face at pos " << i << " at level "
                 << level << " does not have correct level "
                 << bv->Other_Faces()[i]->Starting_Level() << endl;
        }
    }
}
#endif

// When computing the hierarchy it is desirable to keep the edge directions
// and lengths consistent.
void SWIFT_Tri_Mesh::Create_BV_Hierarchy( SPLIT_TYPE split,
                              SWIFT_Array<int>& piece_ids,
                              SWIFT_Array< SWIFT_Array<int> >& mfs,
                              SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face> >& vfs,
                              SWIFT_Array<SWIFT_Tri_Face*>& st_faces,    
                              SWIFT_Array<SWIFT_Tri_Edge*>& st_twins )
{
    // Create the convex hull hierarchy
    int i;
    SWIFT_Array<SWIFT_Tri_Face*> other_faces;

    // Create the bounding volumes
    bvs.Create( 2 * mfs.Length() - 1 );

    // Set all the faces in the original mesh to be original faces
    Set_Faces_Classification( CLASS_ORIGINAL );

    height = 0;


    // Check to see if the object is a simple convex object
    if( mfs.Length() == 1 ) {
        other_faces.Create( mfs[0].Length() );
        for( i = 0; i < mfs[0].Length(); i++ ) {
            other_faces[i] = faces(mfs[0][i]);
        }
        bvs[0].Set_Other_Faces( other_faces );

        bvs[0].Set_Center_Of_Mass( com );

        // Cleanup the list
        other_faces.Nullify();

        // This does not harm the twins for this mesh
        Create_Twins( bvs(0) );

        bvs[0].Compute_Radius();
        //bvs[0].Compute_Volume();
        // Create the root's lookup table
        bvs[0].Create_Lookup_Table();
        //bvs[0].Dump_Lookup_Table();

        // Done since there is only 1 piece
        return;
    }

    int j, k, l, bvid;
    SWIFT_Array<SWIFT_BV*> leaves;
    SWIFT_Array<SWIFT_BV*> children;
    SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face*> > ofmap( Num_Vertices() );

    st_faces.Create( Num_Faces() );
    st_twins.Create( 3*Num_Faces() );
    st_faces.Set_Length( 0 );
    st_twins.Set_Length( 0 );

    // Create the original face mapping
    for( i = 0; i < Num_Faces(); i++ ) {
        ofmap[Vertex_Id( faces[i].Edge1().Origin() )].Add_Grow( faces(i), 10 );
        ofmap[Vertex_Id( faces[i].Edge2().Origin() )].Add_Grow( faces(i), 10 );
        ofmap[Vertex_Id( faces[i].Edge3().Origin() )].Add_Grow( faces(i), 10 );
    }

    leaves.Create( mfs.Length() );

    // Figure out if there are faces mentioned multiple times in the mfs list
    for( i = 0, k = 0; i < mfs.Length(); i++ ) {
        for( j = 0; j < mfs[i].Length(); j++ ) {
            if( faces[mfs[i][j]].Marked() ) {
                k++;
            }
            faces[mfs[i][j]].Mark();
        }
    }

    // Unmark the faces for the next round which is to actually copy stuff
    for( j = 0; j < Num_Faces(); j++ ) {
        faces[j].Unmark();
    }

    SWIFT_Array<SWIFT_Tri_Face> extra_faces( k );

    // Create the lowest level pieces
    for( i = 0, l = mfs.Length()-1; i < mfs.Length(); i++, l++ ) {
        int p;

        leaves[i] = bvs(l);

        // Create the other faces
        other_faces.Create( mfs[i].Length() );
        other_faces.Set_Length( 0 );
        extra_faces.Set_Length( 0 );

        for( j = 0; j < mfs[i].Length(); j++ ) {
            if( faces[mfs[i][j]].Marked() ) {
                // Need to create an extra face
                extra_faces.Add( faces[mfs[i][j]] );
                extra_faces.Last().Unmark();
            } else {
                other_faces.Add( faces(mfs[i][j]) );
                faces[mfs[i][j]].Mark();
            }
        }
        leaves[i]->Set_Other_Faces( other_faces );
        other_faces.Nullify();

        // Set the contained faces and any duplicate original faces
        for( p = 0; p < vfs[i].Length(); p++ ) {
            // Try to find this face in the original faces.  If it exists there,
            // make it have class ORIGINAL
            const int vid0 = Vertex_Id( vfs[i][p].Edge1().Origin() );
            const int vid1 = Vertex_Id( vfs[i][p].Edge2().Origin() );
            const int vid2 = Vertex_Id( vfs[i][p].Edge3().Origin() );
            SWIFT_Array<SWIFT_Tri_Face*>& search_fs = ofmap[vid0];
            for( j = 0; j < search_fs.Length(); j++ ) {
                k = Vertex_Id( search_fs[j]->Edge1().Origin() );
                if( k == vid0 ) {
                    if( Vertex_Id( search_fs[j]->Edge2().Origin() ) == vid1 &&
                        Vertex_Id( search_fs[j]->Edge3().Origin() ) == vid2
                    ) {
                        // This face matches.  Set the class to be ORIGINAL.
                        // Also set the twins to point to main model faces.
                        vfs[i][p].Set_Classification( CLASS_ORIGINAL );
                        vfs[i][p].Edge1().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                        vfs[i][p].Edge2().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                        vfs[i][p].Edge3().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                        break;
                    }
                } else if( k == vid1 ) {
                    if( Vertex_Id( search_fs[j]->Edge2().Origin() ) == vid2 &&
                        Vertex_Id( search_fs[j]->Edge3().Origin() ) == vid0
                    ) {
                        // This face matches.  Set the class to be ORIGINAL.
                        // Also set the twins to point to main model faces.
                        vfs[i][p].Set_Classification( CLASS_ORIGINAL );
                        vfs[i][p].Edge1().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                        vfs[i][p].Edge2().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                        vfs[i][p].Edge3().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                        break;
                    }
                } else if( k == vid2 ) {
                    if( Vertex_Id( search_fs[j]->Edge2().Origin() ) == vid0 &&
                        Vertex_Id( search_fs[j]->Edge3().Origin() ) == vid1
                    ) {
                        // This face matches.  Set the class to be ORIGINAL.
                        // Also set the twins to point to main model faces.
                        vfs[i][p].Set_Classification( CLASS_ORIGINAL );
                        vfs[i][p].Edge1().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                        vfs[i][p].Edge2().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                        vfs[i][p].Edge3().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                        break;
                    }
                }
            }

            if( j == search_fs.Length() ) {
                // Did not find the face.  Set its classification CONTAINED.
                vfs[i][p].Set_Classification( CLASS_CONTAINED );
            } else {
                st_faces.Add_Grow( vfs[i](p), 100 );
                st_twins.Add_Grow( vfs[i][p].Edge1().Twin(), 300 );
                st_twins.Add( vfs[i][p].Edge2().Twin() );
                st_twins.Add( vfs[i][p].Edge3().Twin() );
            }
        }
        leaves[i]->Set_Faces( vfs[i], extra_faces );

        // Compute the center of mass
        leaves[i]->Compute_Center_Of_Mass();
    }

    // Finally unmark the faces
    for( j = 0; j < Num_Faces(); j++ ) {
        faces[j].Unmark();
    }

        children.Create( 2 );
        if( leaves.Length() == 2 ) {
            children[0] = leaves[0];
            children[0]->Set_Level( 1 );
            children[0]->Set_Parent( bvs(0) );
            children[1] = leaves[1];
            children[1]->Set_Level( 1 );
            children[1]->Set_Parent( bvs(0) );
            height = max( height, 1 );
        } else {
            SWIFT_Triple max_dir;
            SWIFT_Array<SWIFT_BV*> leaves1;
            SWIFT_Array<SWIFT_BV*> leaves2;
            SWIFT_Array<SWIFT_Real> dir_vals( leaves.Length() );

            // Compute the convex hull of the center of masses of the leaves
            SWIFT_Array<SWIFT_Tri_Vertex> vs( leaves.Length() );
            for( i = 0; i < leaves.Length(); i++ ) {
                vs[i].Set_Coords( leaves[i]->Center_Of_Mass() );
            }
            Compute_Max_Spread_Direction( vs, max_dir );

            for( i = 0; i < leaves.Length(); i++ ) {
                dir_vals[i] = max_dir * leaves[i]->Center_Of_Mass();
            }
            if( split == MEDIAN ) {
                ::Quicksort( dir_vals, leaves, 0, dir_vals.Length()-1 );

                leaves1.Create( leaves.Length()>>1 );
                leaves2.Create( (leaves.Length()+1)>>1 );

                for( i = 0; i < (leaves.Length()>>1); i++ ) {
                    leaves1[i] = leaves[i];
                }
                for( j = 0; i < leaves.Length(); i++, j++ ) {
                    leaves2[j] = leaves[i];
                }
            } else if( split == MIDPOINT ) {
                SWIFT_Real minv = SWIFT_INFINITY, maxv = -SWIFT_INFINITY;

                // Determine the midpoint of the spread
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( dir_vals[i] > maxv ) {
                        maxv = dir_vals[i];
                    }
                    if( dir_vals[i] < minv ) {
                        minv = dir_vals[i];
                    }
                }
                minv = 0.5 * (minv+maxv);

                leaves1.Create( leaves.Length() );
                leaves2.Create( leaves.Length() );
                leaves1.Set_Length( 0 );
                leaves2.Set_Length( 0 );

                // Separate the leaves
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( minv > dir_vals[i] ) {
                        leaves1.Add( leaves[i] );
                    } else {
                        leaves2.Add( leaves[i] );
                    }
                }
            } else if( split == MEAN ) {
                SWIFT_Real mean = 0.0;

                // Determine the mean
                for( i = 0; i < leaves.Length(); i++ ) {
                    mean += dir_vals[i];
                }
                mean /= leaves.Length();

                leaves1.Create( leaves.Length() );
                leaves2.Create( leaves.Length() );
                leaves1.Set_Length( 0 );
                leaves2.Set_Length( 0 );

                // Separate the leaves
                for( i = 0; i < leaves.Length(); i++ ) {
                    if( mean > dir_vals[i] ) {
                        leaves1.Add( leaves[i] );
                    } else {
                        leaves2.Add( leaves[i] );
                    }
                }
            } else {
                // Try to find a well-defined gap
                ::Quicksort( dir_vals, leaves, 0, dir_vals.Length()-1 );

                // Compute the largest and the second largest gaps
                SWIFT_Real largest_gap = 0.0;
                SWIFT_Real max_spread = 0.0;

                for( i = 1; i < leaves.Length(); i++ ) {
                    if( dir_vals[i]-dir_vals[i-1] > largest_gap ) {
                        max_spread = largest_gap;
                        largest_gap = dir_vals[i]-dir_vals[i-1];
                        j = i;
                    } else if( dir_vals[i]-dir_vals[i-1] > max_spread ) {
                        max_spread = dir_vals[i]-dir_vals[i-1];
                    }
                }

                // The largest gap must be within the central 60% of the leaves
                // and must be 2 times as large as the second largest gap and
                // must span at least 10% of the entire range
                if( j > leaves.Length()*0.2 && j < leaves.Length()*0.8 &&
                    largest_gap > max_spread*2.0 &&
                    largest_gap > (dir_vals.Last()-dir_vals[0])*0.1
                ) {
                    leaves1.Create( j );
                    leaves2.Create( leaves.Length()-j );
                    for( i = 0; i < j; i++ ) {
                        leaves1[i] = leaves[i];
                    }
                    for( j = 0; i < leaves.Length(); i++, j++ ) {
                        leaves2[j] = leaves[i];
                    }
                } else {
                    // Do mean
                    SWIFT_Real mean = 0.0;

                    leaves1.Create( leaves.Length() );
                    leaves2.Create( leaves.Length() );
                    leaves1.Set_Length( 0 );
                    leaves2.Set_Length( 0 );

                    // Determine the mean
                    for( i = 0; i < leaves.Length(); i++ ) {
                        mean += dir_vals[i];
                    }
                    mean /= leaves.Length();

                    for( i = 0; i < leaves.Length(); i++ ) {
                        if( mean > dir_vals[i] ) {
                            leaves1.Add( leaves[i] );
                        } else {
                            leaves2.Add( leaves[i] );
                        }
                    }
                }
            }

            bvid = 0;
            if( leaves1.Length() > 1 ) {
                ++bvid;
                children[0] = bvs(bvid);
                children[0]->Set_Level( 1 );
                children[0]->Set_Parent( bvs(0) );
                Create_BV_Subtree( bvid, split, leaves1, ofmap, st_faces,
                                                                st_twins );
            } else {
                children[0] = leaves1[0];
                children[0]->Set_Level( 1 );
                children[0]->Set_Parent( bvs(0) );
            }
            if( leaves2.Length() > 1 ) {
                ++bvid;
                children[1] = bvs(bvid);
                children[1]->Set_Level( 1 );
                children[1]->Set_Parent( bvs(0) );
                Create_BV_Subtree( bvid, split, leaves2, ofmap, st_faces,
                                                                st_twins );
            } else {
                children[1] = leaves2[0];
                children[1]->Set_Level( 1 );
                children[1]->Set_Parent( bvs(0) );
            }
        }
        children[0]->Set_Faces_Level( 1 );
        children[1]->Set_Faces_Level( 1 );

    // Create the mesh of this bv
    SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face*> > vfmap( Num_Vertices() );
    SWIFT_Array<SWIFT_Tri_Face> new_faces;
    SWIFT_Array<int> new_face_ids;
    SWIFT_Array<SWIFT_Tri_Face*> matched_faces;
    int* fs;
    int fn;

    Compute_Convex_Hull( verts, fs, fn );

    bvs[0].Set_Children( children );

    // For every child insert its faces into the vfmap table.  Also compute
    // the child's lookup table here.
    for( i = 0; i < children.Length(); i++ ) {

        children[i]->Compute_Radius();  // LUT needs this
        //children[i]->Compute_Volume();
        children[i]->Create_Lookup_Table();
        //children[i]->Dump_Lookup_Table();

        // Process the child's owned faces
        for( j = 0; j < children[i]->Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Faces()[j].Edge1().Origin() );
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );

            k = Vertex_Id( children[i]->Faces()[j].Edge2().Origin() );
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );

            k = Vertex_Id( children[i]->Faces()[j].Edge3().Origin() );
            vfmap[k].Add_Grow( children[i]->Faces()(j), 10 );
        }

        // Process the child's other faces
        for( j = 0; j < children[i]->Other_Faces().Length(); j++ ) {
            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge1().Origin() );
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );

            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge2().Origin() );
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );

            k = Vertex_Id( children[i]->Other_Faces()[j]->Edge3().Origin() );
            vfmap[k].Add_Grow( children[i]->Other_Faces()[j], 10 );
        }
    }

    matched_faces.Create( fn );
    matched_faces.Set_Length( 0 );
    new_face_ids.Create( fn );
    new_face_ids.Set_Length( 0 );

    // Second thing to do is find faces on the convex hull
    for( i = 0; i < fn*3; i += 3 ) {
        for( j = 0; j < vfmap[fs[i]].Length(); j++ ) {
            k = Vertex_Id( vfmap[fs[i]][j]->Edge1().Origin() );
            if( k == fs[i] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i+1] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i+2]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            } else if( k == fs[i+1] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i+2] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            } else if( k == fs[i+2] ) {
                if( Vertex_Id( vfmap[fs[i]][j]->Edge2().Origin() ) == fs[i] &&
                    Vertex_Id( vfmap[fs[i]][j]->Edge3().Origin() ) == fs[i+1]
                ) {
                    // This face matches
                    matched_faces.Add( vfmap[fs[i]][j] );
                    break;
                }
            }
        }
        if( j == vfmap[fs[i]].Length() ) {
            new_face_ids.Add( i );
        }
    }

    // Copy to the other faces
    other_faces.Copy_Length( matched_faces );

    // Create and fill in the new faces
    new_faces.Create( new_face_ids.Length() );

    for( i = 0; i < new_face_ids.Length(); i++ ) {
        // Set the vertices
        new_faces[i].Edge1().Set_Origin( verts(fs[new_face_ids[i]]) );
        new_faces[i].Edge2().Set_Origin( verts(fs[new_face_ids[i]+1]) );
        new_faces[i].Edge3().Set_Origin( verts(fs[new_face_ids[i]+2]) );

        // Try to find this face in the original faces.  If it exists there,
        // make it have class ORIGINAL
        SWIFT_Array<SWIFT_Tri_Face*>& search_fs = ofmap[fs[new_face_ids[i]]];
        for( j = 0; j < search_fs.Length(); j++ ) {
            k = Vertex_Id( search_fs[j]->Edge1().Origin() );
            if( k == fs[new_face_ids[i]] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]+1] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]+2]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    break;
                }
            } else if( k == fs[new_face_ids[i]+1] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]+2] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    break;
                }
            } else if( k == fs[new_face_ids[i]+2] ) {
                if( Vertex_Id( search_fs[j]->Edge2().Origin() ) ==
                                                    fs[new_face_ids[i]] &&
                    Vertex_Id( search_fs[j]->Edge3().Origin() ) ==
                                                    fs[new_face_ids[i]+1]
                ) {
                    // This face matches.  Set the class to be ORIGINAL.  Also
                    // set the twins to point to main model faces.
                    new_faces[i].Set_Classification( CLASS_ORIGINAL );
                    new_faces[i].Edge1().Set_Twin(
                                                search_fs[j]->Edge2().Twin() );
                    new_faces[i].Edge2().Set_Twin(
                                                search_fs[j]->Edge3().Twin() );
                    new_faces[i].Edge3().Set_Twin(
                                                search_fs[j]->Edge1().Twin() );
                    break;
                }
            }
        }

        if( j == search_fs.Length() ) {
            // Did not find the face.  Set its classification to be FREE.
            new_faces[i].Set_Classification( CLASS_FREE );
        } else {
            st_faces.Add_Grow( new_faces(i), 100 );
            st_twins.Add_Grow( new_faces[i].Edge1().Twin(), 300 );
            st_twins.Add( new_faces[i].Edge2().Twin() );
            st_twins.Add( new_faces[i].Edge3().Twin() );
        }

        // Compute the geometry
        new_faces[i].Edge1().Compute_Direction_Length();
        new_faces[i].Edge2().Compute_Direction_Length();
        new_faces[i].Edge3().Compute_Direction_Length();
        new_faces[i].Compute_Plane_From_Edges();
        new_faces[i].Edge1().Compute_Voronoi_Planes();
        new_faces[i].Edge2().Compute_Voronoi_Planes();
        new_faces[i].Edge3().Compute_Voronoi_Planes();
    }

    bvs[0].Set_Faces( new_faces );
    bvs[0].Set_Other_Faces( other_faces );
    bvs[0].Compute_Center_Of_Mass();

    // All the new faces in the root node are free faces.  Actually, this is
    // true for all internal nodes in the hierarchy.  All faces that exist  
    // at this level should be marked as such since the marking is done
    // bottom up.  Finally compute the center of mass.
    bvs[0].Set_Level( 0 );
    bvs[0].Set_Faces_Level( 0 );

    bvs[0].Compute_Radius();
    //bvs[0].Compute_Volume();
    // Create the root's lookup table
    bvs[0].Create_Lookup_Table();
    //bvs[0].Dump_Lookup_Table();

    // Cleanup the lists
    delete fs;
    new_faces.Nullify();
    other_faces.Nullify();
    children.Nullify();

    // Save the twins for this mesh
    SWIFT_Array<SWIFT_Tri_Edge*> save_twins( Num_Faces()*3 );

    for( i = 0, j = 0; i < Num_Faces(); i++ ) {
        save_twins[j++] = faces[i].Edge1().Twin();
        save_twins[j++] = faces[i].Edge2().Twin();
        save_twins[j++] = faces[i].Edge3().Twin();
    }

    Create_Twins( bvs(0) );

    // Restore the twins for this mesh
    for( i = 0, j = 0; i < Num_Faces(); i++ ) {
        faces[i].Edge1().Set_Twin( save_twins[j++] );
        faces[i].Edge2().Set_Twin( save_twins[j++] );
        faces[i].Edge3().Set_Twin( save_twins[j++] );
    }

    // Restore the twins for the other original faces
    for( i = 0, j = 0; i < st_faces.Length(); i++, j += 3 ) {
        st_faces[i]->Edge1().Set_Twin( st_twins[j] );
        st_faces[i]->Edge2().Set_Twin( st_twins[j+1] );
        st_faces[i]->Edge3().Set_Twin( st_twins[j+2] );
    }

#ifdef SWIFT_DEBUG
    Check_Face_Levels( );
#endif
}

void SWIFT_Tri_Mesh::Create_Single_BV_Hierarchy( )
{
    // Create the convex hull hierarchy
    int i;
    SWIFT_Array<SWIFT_Tri_Face*> other_faces;

    // Create the bounding volumes
    bvs.Create( 1 );

    // Set all the faces in the original mesh to be original faces
    Set_Faces_Classification( CLASS_ORIGINAL );

    height = 0;


    other_faces.Create( faces.Length() );
    for( i = 0; i < faces.Length(); i++ ) {
        other_faces[i] = faces(i);
    }
    bvs[0].Set_Other_Faces( other_faces );

    bvs[0].Set_Center_Of_Mass( com );

    // Cleanup the list
    other_faces.Nullify();

    // This does not harm the twins for this mesh
    Create_Twins( bvs(0) );

    bvs[0].Compute_Radius();
    //bvs[0].Compute_Volume();
    // Create the root's lookup table
    bvs[0].Create_Lookup_Table();
    //bvs[0].Dump_Lookup_Table();
}

// All edges should be unmarked when this function is called.  When this
// function terminates, they will still be unmarked.
void SWIFT_Tri_Mesh::Compute_All_Hierarchy_Geometry( )
{
    int i, j;

    // First, compute the main mesh geometry
    Compute_Geometry();

    // Second, compute the geometry of the internal nodes from the bottom up,
    // Checking their twins for stuff already computed
    for( i = Num_BVs()-1; i >= 0; i-- ) {
        const int lev = bvs[i].Level();
        for( j = 0; j < bvs[i].Num_Faces(); j++ ) {
#ifdef SWIFT_DEBUG
            if( bvs[i].Faces()[j].Edge1().Twin( lev ) == NULL ||
                bvs[i].Faces()[j].Edge2().Twin( lev ) == NULL ||
                bvs[i].Faces()[j].Edge3().Twin( lev ) == NULL
            ) {
                cerr << "Found null edge when computing all hier geom" << endl;
            }
#endif
            if( bvs[i].Faces()[j].Classification() == CLASS_ORIGINAL ) {
                // Can copy the geometry from the main mesh face
                SWIFT_Tri_Edge** e1 = bvs[i].Faces()[j].Edge1().Twin_Array();
                SWIFT_Tri_Edge** e2 = bvs[i].Faces()[j].Edge2().Twin_Array();
                SWIFT_Tri_Edge** e3 = bvs[i].Faces()[j].Edge3().Twin_Array();
                int bf = bvs[i].Faces()[j].Bit_Field();
                SWIFT_Tri_Face* f = bvs[i].Faces()[j].Edge1().Twin()->Twin()->
                                                                    Adj_Face();
                if( f->Vertex1() == bvs[i].Faces()[j].Vertex1() ) {
                    bvs[i].Faces()[j].Edge1() = f->Edge1();
                    bvs[i].Faces()[j].Edge2() = f->Edge2();
                    bvs[i].Faces()[j].Edge3() = f->Edge3();
                } else if( f->Vertex2() == bvs[i].Faces()[j].Vertex1() ) {
                    bvs[i].Faces()[j].Edge1() = f->Edge2();
                    bvs[i].Faces()[j].Edge2() = f->Edge3();
                    bvs[i].Faces()[j].Edge3() = f->Edge1();
                } else {
                    bvs[i].Faces()[j].Edge1() = f->Edge3();
                    bvs[i].Faces()[j].Edge2() = f->Edge1();
                    bvs[i].Faces()[j].Edge3() = f->Edge2();
                }
                bvs[i].Faces()[j].Set_Normal_N( f->Normal() );
                bvs[i].Faces()[j].Set_Distance( f->Distance() );
                bvs[i].Faces()[j].Reset_Internal_Edge_Pointers();
                bvs[i].Faces()[j].Edge1().Set_Twin_Array( e1 );
                bvs[i].Faces()[j].Edge2().Set_Twin_Array( e2 );
                bvs[i].Faces()[j].Edge3().Set_Twin_Array( e3 );
                bvs[i].Faces()[j].Set_Bit_Field( bf );
            }
        }

        for( j = 0; j < bvs[i].Num_Faces(); j++ ) {
            if( bvs[i].Faces()[j].Classification() == CLASS_ORIGINAL ) {
                continue;
            }
            if( bvs[i].Faces()[j].Edge1().Unmarked() ) {
                SWIFT_Tri_Edge* e = bvs[i].Faces()[j].Edge1().Twin( lev );
                SWIFT_Tri_Face* f = e->Adj_Face();
                if( !bvs[i].Face_In_Range( f ) ||
                    f->Classification() == CLASS_ORIGINAL
                ) {
                    // Copy the geometry from the already computed face
                    bvs[i].Faces()[j].Edge1().Set_Length( e->Length() );
                    bvs[i].Faces()[j].Edge1().Set_Direction_N(
                                                            -e->Direction() );
                } else {
                    // Compute direction and length of the edge and its twin
                    bvs[i].Faces()[j].Edge1().Compute_Direction_Length();
                    e->Set_Length( bvs[i].Faces()[j].Edge1().Length() );
                    e->Set_Direction_N(
                                    -bvs[i].Faces()[j].Edge1().Direction() );
                    // Compute edge distance
                    e->Set_Origin_On_Plane();
                    e->Mark();
                }
                bvs[i].Faces()[j].Edge1().Set_Distance(
                                            -e->Distance() - e->Length() );
            } else {
                bvs[i].Faces()[j].Edge1().Unmark();
            }
            if( bvs[i].Faces()[j].Edge2().Unmarked() ) {
                SWIFT_Tri_Edge* e = bvs[i].Faces()[j].Edge2().Twin( lev );
                SWIFT_Tri_Face* f = e->Adj_Face();
                if( !bvs[i].Face_In_Range( f ) ||
                    f->Classification() == CLASS_ORIGINAL
                ) {
                    // Copy the geometry from the already computed face
                    bvs[i].Faces()[j].Edge2().Set_Length( e->Length() );
                    bvs[i].Faces()[j].Edge2().Set_Direction_N(
                                                            -e->Direction() );
                } else {
                    // Compute direction and length of the edge and its twin
                    bvs[i].Faces()[j].Edge2().Compute_Direction_Length();
                    e->Set_Length( bvs[i].Faces()[j].Edge2().Length() );
                    e->Set_Direction_N(
                                    -bvs[i].Faces()[j].Edge2().Direction() );
                    // Compute edge distance
                    e->Set_Origin_On_Plane();
                    e->Mark();
                }
                bvs[i].Faces()[j].Edge2().Set_Distance(
                                            -e->Distance() - e->Length() );
            } else {
                bvs[i].Faces()[j].Edge2().Unmark();
            }
            if( bvs[i].Faces()[j].Edge3().Unmarked() ) {
                SWIFT_Tri_Edge* e = bvs[i].Faces()[j].Edge3().Twin( lev );
                SWIFT_Tri_Face* f = e->Adj_Face();
                if( !bvs[i].Face_In_Range( f ) ||
                    f->Classification() == CLASS_ORIGINAL
                ) {
                    // Copy the geometry from the already computed face
                    bvs[i].Faces()[j].Edge3().Set_Length( e->Length() );
                    bvs[i].Faces()[j].Edge3().Set_Direction_N(
                                                            -e->Direction() );
                } else {
                    // Compute direction and length of the edge and its twin
                    bvs[i].Faces()[j].Edge3().Compute_Direction_Length();
                    e->Set_Length( bvs[i].Faces()[j].Edge3().Length() );
                    e->Set_Direction_N(
                                    -bvs[i].Faces()[j].Edge3().Direction() );
                    // Compute edge distance
                    e->Set_Origin_On_Plane();
                    e->Mark();
                }
                bvs[i].Faces()[j].Edge3().Set_Distance(
                                            -e->Distance() - e->Length() );
            } else {
                bvs[i].Faces()[j].Edge3().Unmark();
            }
            // Compute face plane
            bvs[i].Faces()[j].Compute_Plane_From_Edges();
            bvs[i].Faces()[j].Edge1().Compute_Face_Plane();
            bvs[i].Faces()[j].Edge2().Compute_Face_Plane();
            bvs[i].Faces()[j].Edge3().Compute_Face_Plane();
        }
        // No need to compute the radius of the bv, just the COM
        bvs[i].Compute_Center_Of_Mass();
    }

    // Compute the radius and the center of mass of the main mesh
    Compute_Center_Of_Mass();
    Compute_Radius();
}


#ifdef SWIFT_DEBUG
void SWIFT_Tri_Mesh::Verify(
                            )
{
    int i;

cerr << "------------- Beginning Verification -------------" << endl;

    for( i = 0; i < Num_Vertices(); i++ ) {
        // Verify the vertex connectivity
        verts[i].Verify_Topology( i );
    }

    for( i = 0; i < Num_Faces(); i++ ) {
        // Verify the face connectivity
        if( faces[i].Verify_Topology( i ) ) {
            faces[i].Verify_Geometry( i );
        }

        // Verify the edge connectivity
        if( faces[i].Edge1().Verify_Topology( i, 1 ) ) {
            faces[i].Edge1().Verify_Geometry( i, 1 );
        }
        if( faces[i].Edge2().Verify_Topology( i, 2 ) ) {
            faces[i].Edge2().Verify_Geometry( i, 2 );
        }
        if( faces[i].Edge3().Verify_Topology( i, 3 ) ) {
            faces[i].Edge3().Verify_Geometry( i, 3 );
        }
    }



cerr << "------------- Done Verification -------------" << endl;
}
#endif

void SWIFT_Tri_Mesh::Compute_Edge_Convexities( SWIFT_Array<bool>& ecs )
{
    int i, j;
    bool ce1, ce2, ce3;

    ecs.Create( Num_Faces()*3 );
    for( i = 0, j = 0; i < Num_Faces(); i++, j += 3 ) {
        ce1 = faces[i].Edge1().Twin() == NULL ? false :
              faces[i].Inside( faces[i].Edge1().Twin()->Prev()->Origin() );
        ce2 = faces[i].Edge2().Twin() == NULL ? false :
              faces[i].Inside( faces[i].Edge2().Twin()->Prev()->Origin() );
        ce3 = faces[i].Edge3().Twin() == NULL ? false :
              faces[i].Inside( faces[i].Edge3().Twin()->Prev()->Origin() );
        ecs[j] = ce1;
        ecs[j+1] = ce2;
        ecs[j+2] = ce3;
        if( faces[i].Edge1().Twin() != NULL ) {
            ecs[ Edge_Id( faces[i].Edge1().Twin() ) ] = ce1;
        }
        if( faces[i].Edge2().Twin() != NULL ) {
            ecs[ Edge_Id( faces[i].Edge2().Twin() ) ] = ce2;
        }
        if( faces[i].Edge3().Twin() != NULL ) {
            ecs[ Edge_Id( faces[i].Edge3().Twin() ) ] = ce3;
        }
    }
}

void SWIFT_Tri_Mesh::Accum_Bounding_Box( SWIFT_Triple& minc,
                                         SWIFT_Triple& maxc )
{
    int i;

    for( i = 0; i < Num_Vertices(); i++ ) {
        if( verts[i].Coords().X() < minc.X() ) {
            minc.Set_X( verts[i].Coords().X() );
        }
        if( verts[i].Coords().X() > maxc.X() ) {
            maxc.Set_X( verts[i].Coords().X() );
        }
        if( verts[i].Coords().Y() < minc.Y() ) {
            minc.Set_Y( verts[i].Coords().Y() );
        }
        if( verts[i].Coords().Y() > maxc.Y() ) {
            maxc.Set_Y( verts[i].Coords().Y() );
        }
        if( verts[i].Coords().Z() < minc.Z() ) {
            minc.Set_Z( verts[i].Coords().Z() );
        }
        if( verts[i].Coords().Z() > maxc.Z() ) {
            maxc.Set_Z( verts[i].Coords().Z() );
        }
    }
}

void SWIFT_Tri_Mesh::Compute_Bounding_Box( SWIFT_Triple& minc,
                                           SWIFT_Triple& maxc )
{
    minc = SWIFT_Triple( SWIFT_INFINITY, SWIFT_INFINITY, SWIFT_INFINITY );
    maxc = SWIFT_Triple( -SWIFT_INFINITY, -SWIFT_INFINITY, -SWIFT_INFINITY );

    Accum_Bounding_Box( minc, maxc );
}


///////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Mesh private functions
///////////////////////////////////////////////////////////////////////////////
void SWIFT_Tri_Mesh::Quicksort( SWIFT_Array<SWIFT_Tri_Edge*>& es, int p, int r )
{
    if( p < r ) {
        // Compute a random element to use as the pivot
        int rn = (int) ((SWIFT_Real)(r-p+1) * drand48()) + p;
        int i = p-1;
        int j = r+1;
        SWIFT_Tri_Edge* x = es[rn];
        SWIFT_Tri_Edge* te;

        // Swap the random element into the first position
        es[rn] = es[p];
        es[p] = x;

        while( true ) {
            j--;
            while( es[j]->Origin() > x->Origin() ||
                   (es[j]->Origin() == x->Origin() &&
                    es[j]->Twin()->Origin() > x->Twin()->Origin())
            ) {
                j--;
            }
            i++;
            while( es[i]->Origin() < x->Origin() ||
                   (es[i]->Origin() == x->Origin() &&
                    es[i]->Twin()->Origin() < x->Twin()->Origin())
            ) {
                i++;
            }
            if( i < j ) {
                te = es[i];
                es[i] = es[j];
                es[j] = te;
            } else {
                break;
            }
        }

        Quicksort( es, p, j );
        Quicksort( es, j+1, r );
    }
}


// All edges should be unmarked when this function is called.  When this
// function terminates, they will still be unmarked.
void SWIFT_Tri_Mesh::Compute_Geometry( )
{
    int i;

    // Compute edge lengths, directions, vertex-edge planes, and face planes
    for( i = 0; i < Num_Faces(); i++ ) {
        if( faces[i].Edge1().Unmarked() ) {
            if( faces[i].Edge1().Twin() != NULL ) {
                // Compute direction and length of the edge and its twin
                faces[i].Edge1().Compute_Direction_Length_Twin();
                // Compute edge distance
                faces[i].Edge1().Set_Origin_On_Plane_Twin();
                faces[i].Edge1().Twin()->Mark();
            } else {
                faces[i].Edge1().Compute_Direction_Length();
                faces[i].Edge1().Set_Origin_On_Plane();
            }
        } else {
            faces[i].Edge1().Unmark();
        }
        if( faces[i].Edge2().Unmarked() ) {
            if( faces[i].Edge2().Twin() != NULL ) {
                faces[i].Edge2().Compute_Direction_Length_Twin();
                faces[i].Edge2().Set_Origin_On_Plane_Twin();
                faces[i].Edge2().Twin()->Mark();
            } else {
                faces[i].Edge2().Compute_Direction_Length();
                faces[i].Edge2().Set_Origin_On_Plane();
            }
        } else {
            faces[i].Edge2().Unmark();
        }
        if( faces[i].Edge3().Unmarked() ) {
            if( faces[i].Edge3().Twin() != NULL ) {
                faces[i].Edge3().Compute_Direction_Length_Twin();
                faces[i].Edge3().Set_Origin_On_Plane_Twin();
                faces[i].Edge3().Twin()->Mark();
            } else {
                faces[i].Edge3().Compute_Direction_Length();
                faces[i].Edge3().Set_Origin_On_Plane();
            }
        } else {
            faces[i].Edge3().Unmark();
        }
        // Compute face plane
        faces[i].Compute_Plane_From_Edges();
        faces[i].Edge1().Compute_Face_Plane();
        faces[i].Edge2().Compute_Face_Plane();
        faces[i].Edge3().Compute_Face_Plane();
    }
}

void SWIFT_Tri_Mesh::Translate_To( const SWIFT_Triple& t )
{   
    int i; 

    for( i = 0; i < Num_Vertices(); i++ ) {
        verts[i].Translate( t );
    }
    com += t;
    Compute_Geometry_After_Translate();
}       
    
// All edges should be unmarked when this function is called.  When this
// function terminates, they will still be unmarked.
void SWIFT_Tri_Mesh::Compute_Geometry_After_Translate( )
{
    int i;

    // Recompute face and edge distances
    for( i = 0; i < Num_Faces(); i++ ) {
        faces[i].Set_Point_On_Plane();
        faces[i].Edge1().Set_Origin_On_Plane();
        faces[i].Edge1().Set_Origin_On_Face_Plane();
        faces[i].Edge2().Set_Origin_On_Plane();
        faces[i].Edge2().Set_Origin_On_Face_Plane();
        faces[i].Edge3().Set_Origin_On_Plane(); 
        faces[i].Edge3().Set_Origin_On_Face_Plane();
    }
}

void SWIFT_Tri_Mesh::Compute_Center_Of_Mass( )
{
    int i;
    SWIFT_Real area_x2;
    SWIFT_Real total_area;
    SWIFT_Triple areav;

    com.Set_Value( 0.0, 0.0, 0.0 );
    total_area = 0.0;
    for( i = 0; i < Num_Faces(); i++ ) {
        areav = (faces[i].Edge1().Origin()->Coords() -
                 faces[i].Edge2().Origin()->Coords()) %
                (faces[i].Edge1().Origin()->Coords() -
                 faces[i].Edge3().Origin()->Coords());
        area_x2 = areav.Length();
        total_area += area_x2;
        com += area_x2 * (faces[i].Edge1().Origin()->Coords() +
                          faces[i].Edge2().Origin()->Coords() +
                          faces[i].Edge3().Origin()->Coords() );
    }

    com /= 3.0 * total_area;
}

void SWIFT_Tri_Mesh::Compute_Radius( )
{
    int i;
    SWIFT_Real d;
    radius = 0.0;
    for( i = 0; i < Num_Vertices(); i++ ) {
        d = Center().Dist_Sq( verts[i].Coords() );
        if( d > radius ) {
            radius = d;
        }
    }
    radius = sqrt( radius );
}

void SWIFT_Tri_Mesh::Process_Vertices(
            const SWIFT_Real* vs, int vn, int& vnn, const SWIFT_Real*& new_vs,
            SWIFT_Array<int>& vmap, SWIFT_Array<int>& vnewi, bool create_vids )
{
    int i, j;
    int initial_creation;
    SWIFT_Real xbuck_res;
    SWIFT_Real ybuck_res;
    SWIFT_Real zbuck_res;
    SWIFT_Triple minc, maxc;
    SWIFT_Array<SWIFT_Triple> vcoords;
    SWIFT_Array<int> buckets[21][21][21];
    const SWIFT_Real* vs_ptr;

    // Remove duplicate vertices

    // First copy all the coordinates to the vertices and compute the
    // bounding box
    vcoords.Create( vn ); 
    vmap.Create( vn );
    vnewi.Create( vn );
    minc = SWIFT_Triple( SWIFT_INFINITY, SWIFT_INFINITY, SWIFT_INFINITY );
    maxc = SWIFT_Triple( -SWIFT_INFINITY, -SWIFT_INFINITY, -SWIFT_INFINITY );
    for( i = 0, vs_ptr = vs; i < vn; i++, vs_ptr += 3 ) {
        vcoords[i] = SWIFT_Triple( vs_ptr[0], vs_ptr[1], vs_ptr[2] );
        if( vs_ptr[0] < minc.X() ) { minc.Set_X( vs_ptr[0] ); }
        if( vs_ptr[0] > maxc.X() ) { maxc.Set_X( vs_ptr[0] ); }
        if( vs_ptr[1] < minc.Y() ) { minc.Set_Y( vs_ptr[1] ); }
        if( vs_ptr[1] > maxc.Y() ) { maxc.Set_Y( vs_ptr[1] ); }
        if( vs_ptr[2] < minc.Z() ) { minc.Set_Z( vs_ptr[2] ); }
        if( vs_ptr[2] > maxc.Z() ) { maxc.Set_Z( vs_ptr[2] ); }
    }

    xbuck_res = (maxc.X() - minc.X()) / 20.0;
    ybuck_res = (maxc.Y() - minc.Y()) / 20.0;
    zbuck_res = (maxc.Z() - minc.Z()) / 20.0;

    initial_creation = vn / 8000 + 1;

    vnn = 0;
    no_dup_verts = true;
    for( i = 0; i < vn; i++ ) {
        int xbuck = (int)floor((vcoords[i].X() - minc.X()) / xbuck_res);
        int ybuck = (int)floor((vcoords[i].Y() - minc.Y()) / ybuck_res);
        int zbuck = (int)floor((vcoords[i].Z() - minc.Z()) / zbuck_res);
        SWIFT_Array<int>& buck_list = buckets[xbuck][ybuck][zbuck];

        if( buck_list.Length() == 0 ) {
            // First time accessing this bucket
            buck_list.Create( initial_creation );
            buck_list.Set_Length( 1 );
            buck_list.Set_Last( i );
            vmap[i] = i;
            vnewi[i] = vnn++;
        } else {
            // Search for the item in the list
            for( j = 0; j < buck_list.Length(); j++ ) {
                if( vcoords[i].X() == vcoords[ buck_list[j] ].X() &&
                    vcoords[i].Y() == vcoords[ buck_list[j] ].Y() &&
                    vcoords[i].Z() == vcoords[ buck_list[j] ].Z()
                ) {
                    break;
                }
            }

            if( j == buck_list.Length() ) {
                // Item not found
                if( buck_list.Length() == buck_list.Max_Length() ) {
                    buck_list.Grow_Double();
                }
                buck_list.Increment_Length();
                buck_list.Set_Last( i );
                vmap[i] = i;
                vnewi[i] = vnn++;
            } else {
                // Item is found at position j
                vmap[i] = buck_list[j];
                no_dup_verts = false;
            }
        }
    }

    // Create and fill in the new_vs and the map_vids if duplicate vertices
    if( !no_dup_verts ) {
        SWIFT_Real* temp_vs = new SWIFT_Real[vnn*3];
        if( create_vids ) {
            int k;
            map_vids.Create( vnn );
            for( i = 0, j = 0, k = 0; i < vn; i++ ) {
                if( i == vmap[i] ) {
                    temp_vs[j++] = vcoords[ vmap[i] ].X();
                    temp_vs[j++] = vcoords[ vmap[i] ].Y();
                    temp_vs[j++] = vcoords[ vmap[i] ].Z();
                    map_vids[k++] = vmap[i];
                }
            }
        } else {
            for( i = 0, j = 0; i < vn; i++ ) {
                if( i == vmap[i] ) {
                    temp_vs[j++] = vcoords[ vmap[i] ].X();
                    temp_vs[j++] = vcoords[ vmap[i] ].Y();
                    temp_vs[j++] = vcoords[ vmap[i] ].Z();
                }
            }
        }
        new_vs = temp_vs;
    } else {
        new_vs = vs;
    }
}

void SWIFT_Tri_Mesh::Transform_Vertices( const SWIFT_Real* vs, int vn,
                                const SWIFT_Orientation& orient,
                                const SWIFT_Translation& trans, SWIFT_Real sc )
{
    int i, j;
    SWIFT_Triple T = SWIFT_Triple( trans[0], trans[1], trans[2] );
    SWIFT_Matrix33 R;

    R.Set_Value( orient );

    for( i = 0, j = 0; i < Num_Vertices(); i++, j += 3 ) {
        SWIFT_Triple coords( vs[j], vs[j+1], vs[j+2] );
        verts[i].Set_Coords( sc * (R * coords) + T );
    }
}

void SWIFT_Tri_Mesh::Transform_Vertices_And_COM( SWIFT_Tri_Mesh* msrc,
                                const SWIFT_Orientation& orient,
                                const SWIFT_Translation& trans, SWIFT_Real sc )
{
    int i;
    SWIFT_Triple T = SWIFT_Triple( trans[0], trans[1], trans[2] );
    SWIFT_Matrix33 R;

    R.Set_Value( orient );

    for( i = 0; i < Num_Vertices(); i++ ) {
        verts[i].Set_Coords( sc * (R * msrc->Vertices()[i].Coords()) + T );
    }

    // Transform the center of mass
    Set_Center_Of_Mass( sc * (R * msrc->Center_Of_Mass()) + T );
}

void SWIFT_Tri_Mesh::Process_Faces( const int* fs, int fn, const int* fv,
                                    int& tn, const int*& new_fs,
                                    SWIFT_Array<int>& triang_edges,
                                    bool create_fids )
{
#ifdef SWIFT_ONLY_TRIS
    tn = fn;
    new_fs = fs;
#else
    int i, j;
    int* temp_fs;

    only_tris = true;

    if( fv != NULL ) {
        tn = 0;
        for( i = 0; i < fn; i++ ) {
            if( fv[i] != 3 ) {
                only_tris = false;
            }
            tn += fv[i] - 2;
        }
    } else {
        tn = fn;
    }

    if( !only_tris ) {
        // Triangulate into the temp_fs array
        int b0, b1;
        int v, e, f, j_3;

        f = 0;
        e = 0;
        if( create_fids ) {
            map_fids.Create( tn );
        }
        temp_fs = new int[tn*3];
        triang_edges.Create( tn - fn );
        for( i = 0, j = 0, j_3 = 0; i < fn; i++ ) {
            v = f + fv[i] - 1;  // Store the last index of this face
            b0 = f;
            b1 = f+1;

            while( true ) {
                if( create_fids ) {
                    map_fids[j_3] = i;
                }

                // Create the left face
                temp_fs[j++] = fs[b0];
                temp_fs[j++] = fs[b1++];
                temp_fs[j++] = fs[b1];
                j_3++;

                // Test completion
                if( b1 == v ) {
                    break;
                }

                // Set an internal edge
                triang_edges[e++] = j_3-1;

                if( create_fids ) {
                    map_fids[j_3] = i;
                }

                // Create the right face
                temp_fs[j++] = fs[v];
                temp_fs[j++] = fs[b0];
                temp_fs[j++] = fs[b1];
                j_3++;

                b0 = v;  // Advance b0
                v--;

                // Test completion
                if( b1 == v ) {
                    break;
                }

                // Set an internal edge
                triang_edges[e++] = j_3-1;
            }

            f += fv[i];
        }

        new_fs = temp_fs;
    } else {
        new_fs = fs;
    }
#endif
}


