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
// SWIFT_mesh.h
//
// Description:
//      Classes to manage triangular mesh hierarchies for collision detection.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_MESH_H_
#define _SWIFT_MESH_H_

#include <math.h>


#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#include <SWIFT_lut.h>

//////////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////////
#ifndef _SWIFT_H_
typedef SWIFT_Real SWIFT_Orientation[9];
typedef SWIFT_Real SWIFT_Translation[3];
#endif

//////////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////////
#ifndef _SWIFT_H_
static const SWIFT_Orientation DEFAULT_ORIENTATION
                    = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
static const SWIFT_Translation DEFAULT_TRANSLATION = {0.0, 0.0, 0.0};
#endif


static const int CLASS_ORIGINAL = 0;
static const int CLASS_CONTAINED = 16;
static const int CLASS_FREE = 32;


//////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Tri_Vertex;
class SWIFT_Tri_Edge;
class SWIFT_Tri_Face;
class SWIFT_BV;
class SWIFT_Tri_Mesh;


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Vertex
//
// Description:
//      Vertex class for the triangular mesh.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Tri_Vertex {
  public:
    SWIFT_Tri_Vertex( )
    {   edge = NULL;
    }
    ~SWIFT_Tri_Vertex( ) { }

    // Get functions
    const SWIFT_Triple& Coords( ) const { return coords; }
    SWIFT_Tri_Edge* Adj_Edge( ) const { return edge; }

    // Set functions
    void Set_Coords( const SWIFT_Triple& p ) { coords = p; }
    void Set_Coords( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                                            { coords.Set_Value( x, y, z ); }
    void Translate( const SWIFT_Triple& t ) { coords += t; }
    void Scale( SWIFT_Real s ) { coords *= s; }
    void Set_Adj_Edge( SWIFT_Tri_Edge* e ) { edge = e; }

    // Computation functions
    int Valence( ) const;

    inline SWIFT_Tri_Edge* Adj_Edge( SWIFT_Tri_Vertex* v ) const;
    inline SWIFT_Tri_Face* Adj_Face( SWIFT_Tri_Vertex* v ) const;
    inline SWIFT_Triple Gathered_Normal( ) const;
    inline SWIFT_Triple Gathered_Direction( ) const;

#ifdef SWIFT_DEBUG
    bool Verify_Topology( int pos ) const;
#endif

    void Draw() const { coords.Send_VCoords_To_OpenGL(); }
  private:
    // Geometry info
    SWIFT_Triple coords;

    // Topology info
    SWIFT_Tri_Edge* edge;

};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Edge
//
// Description:
//      Edge class for the triangular mesh.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Tri_Edge {
  public:
    SWIFT_Tri_Edge( )
    {   next = twin = NULL, orig = NULL, face = NULL;
        twins = NULL;
    }
    ~SWIFT_Tri_Edge( ) { delete twins; }

    // Get functions
    const SWIFT_Triple& Direction( ) const { return u; }
    SWIFT_Real Length( ) const { return len; }
    SWIFT_Real Distance( ) const { return d; }
    SWIFT_Real Distance( const SWIFT_Triple& p ) const { return (p*u) - d; }
    SWIFT_Real Distance( SWIFT_Tri_Vertex* v ) const
                                            { return (v->Coords()*u) - d; }
    SWIFT_Real Face_Distance( const SWIFT_Triple& p ) const
                                            { return (p*fn) - fd; }
    SWIFT_Real Face_Distance( SWIFT_Tri_Vertex* v ) const
                                            { return (v->Coords()*fn) - fd; }
    const SWIFT_Triple& Face_Normal( ) const { return fn; }
    SWIFT_Real Face_Distance( ) const { return fd; }
    SWIFT_Triple Coords( ) const
                { return 0.5 * (orig->Coords() + next->Origin()->Coords()); }

    SWIFT_Tri_Edge* Prev( ) const { return next->Next(); }
    SWIFT_Tri_Edge* Next( ) const { return next; }
    // If the edge's face is ORIGINAL then this Twin points to the twin
    // ORIGINAL edge (which is part of an ORIGINAL face)
    SWIFT_Tri_Edge* Twin( ) const { return twin; }
    SWIFT_Tri_Vertex* Origin( ) const { return orig; }
    SWIFT_Tri_Vertex* Head( ) const { return next->Origin(); }
    SWIFT_Tri_Face* Adj_Face( ) const { return face; }


    // The level that this half-edge first appears in the hierarchy.  The root
    // is level 0.
    inline int Starting_Level( ) const;
    // Create the twins list to span a certain number of levels in the hierarchy
    void Create_Twins( int nt )
                        { delete twins; twins = new SWIFT_Tri_Edge*[nt]; }
    // Copy the main twin to be a twin at a certain level
    void Copy_Twin( int level ) { twins[level] = twin; }
    // Retrieve the twin for a certain level in the hierarchy
    SWIFT_Tri_Edge** Twin_Array( ) { return twins; }
    void Set_Twin_Array( SWIFT_Tri_Edge** ta ) { twins = ta; }
    SWIFT_Tri_Edge* Twin( int glevel ) const
                                    { return twins[glevel-Starting_Level()]; }

    // Nullify the twins array
    void Nullify_Twins( ) { twins = NULL; }

    // Retrieve the classification for a certain level in the hierarchy.  Note
    // that an edge has the most powerful classification between its own face
    // and its twin's face.
    inline int Classification( int glevel ) const;
    bool Is_Original( int glevel ) const
                        { return Classification( glevel ) == CLASS_ORIGINAL; }
    bool Is_Contained( int glevel ) const
                        { return Classification( glevel ) == CLASS_CONTAINED; }
    bool Is_Free( int glevel ) const
                        { return Classification( glevel ) == CLASS_FREE; }

    // Set functions
    void Set_Direction_N( const SWIFT_Triple& dir ) { u = dir; }
    void Set_Direction_N( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                                                { u.Set_Value( x, y, z ); }
    void Set_Direction_U( const SWIFT_Triple& dir ) { u = dir; u.Normalize(); }
    void Set_Direction_U( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                                { u.Set_Value( x, y, z ); u.Normalize(); }
    void Set_Length( SWIFT_Real l ) { len = l; }
    void Set_Distance( SWIFT_Real dist ) { d = dist; }
    void Set_Face_Distance( SWIFT_Real dist ) { fd = dist; }
    void Set_Face_Normal( const SWIFT_Triple& n ) { fn = n; }

    void Scale_Length( SWIFT_Real s ) { len *= s; }

    void Set_Direction_Length_To_Twin( )
                    { twin->Set_Length( len ); twin->Set_Direction_N( -u ); }

    void Set_Point_On_Plane( const SWIFT_Triple& p ) { d = (p*u); }
    void Set_Origin_On_Plane( ) { d = (orig->Coords()*u); }
    void Set_Origin_On_Plane_Twin( )
                { twin->Set_Origin_On_Plane(); d = - twin->Distance() - len; }
    void Set_Origin_On_Face_Plane( ) { fd = (orig->Coords()*fn); }

    void Set_Next( SWIFT_Tri_Edge* n ) { next = n; }
    void Set_Twin( SWIFT_Tri_Edge* t ) { twin = t; }
    void Set_Origin( SWIFT_Tri_Vertex* o ) { orig = o; }
    void Set_Adj_Face( SWIFT_Tri_Face* f ) { face = f; }


    // Boolean flag values
    inline int Marked( ) const;
    inline int Unmarked( ) const;
    // Set the boolean flags
    inline void Mark( );
    inline void Unmark( );

    // Query functions
    bool Inside( SWIFT_Tri_Vertex* v ) const { return ((v->Coords()*u) < d); }
    bool On( SWIFT_Tri_Vertex* v ) const { return ((v->Coords()*u) == d); }
    bool Outside( SWIFT_Tri_Vertex* v ) const { return ((v->Coords()*u) > d); }
    bool Inside_Tol( SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
                    { return (v->Coords()*u) <= (d + fabs(d) * tolerance); }
    bool On_Tol( SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
    {
        SWIFT_Real a = fabs(d) * tolerance;
        SWIFT_Real result = v->Coords()*u;
        return d - a <= result && result <= d + a;
    }
    bool Outside_Tol( SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
                    { return (d - fabs(d) * tolerance) <= (v->Coords()*u); }

    // Computation functions

    void Compute_Direction_Length( )
    {
        u = next->Origin()->Coords() - orig->Coords();
        len = u.Length();
        u /= len;
    }
    void Compute_Direction_Length_Twin( )
    {
        Compute_Direction_Length();
        twin->Set_Length( len );
        twin->Set_Direction_N( -u );
    }

    // These depend on the face normal and the edge direction being computed
    inline void Compute_Face_Plane( );
    void Compute_Voronoi_Planes( )
                            { Compute_Face_Plane(); Set_Origin_On_Plane(); }

#ifdef SWIFT_DEBUG
    bool Verify_Topology( int pos1, int pos2 ) const;
    bool Verify_Geometry( int pos1, int pos2 ) const;
#endif

    void Draw() const { orig->Draw(); next->Origin()->Draw(); }
  private:
    // Geometry info
    SWIFT_Triple u;   // The direction vector of the edge
    SWIFT_Triple fn;  // The edge-face voronoi plane normal
    SWIFT_Real len; // The length of the edge
    SWIFT_Real d;   // Distance of the plane from O defined by the origin and u
    SWIFT_Real fd;  // Distance of the edge-face voronoi plane from O

    // Topology info
    SWIFT_Tri_Edge* next;
    SWIFT_Tri_Edge* twin;
    SWIFT_Tri_Vertex* orig;
    SWIFT_Tri_Face* face;


    // The level twin pointers.  Because the face that owns this edge may be
    // shared at multiple levels of the hierarchy, this list stores the
    // adjacencies at multiple levels.  The method Twin( int level ) accesses
    // the correct element.
    SWIFT_Tri_Edge** twins;
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Face
//
// Description:
//      Triangle class for the triangular mesh.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Tri_Face {
  public:
    SWIFT_Tri_Face( )
    {
        Reset_Internal_Edge_Pointers();
        bit_field = 0;
    }
    ~SWIFT_Tri_Face( ) { }

    // Get functions
    const SWIFT_Triple& Normal( ) const { return normal; }
    SWIFT_Real Distance( ) const { return d; }
    SWIFT_Triple Coords( ) const { return (e1.Origin()->Coords() +
                        e2.Origin()->Coords() + e3.Origin()->Coords()) / 3.0; }
    const SWIFT_Triple& Coords1( ) const { return e1.Origin()->Coords(); }
    const SWIFT_Triple& Coords2( ) const { return e2.Origin()->Coords(); }
    const SWIFT_Triple& Coords3( ) const { return e3.Origin()->Coords(); }
    SWIFT_Tri_Vertex* Vertex1( ) const { return e1.Origin(); }
    SWIFT_Tri_Vertex* Vertex2( ) const { return e2.Origin(); }
    SWIFT_Tri_Vertex* Vertex3( ) const { return e3.Origin(); }

    SWIFT_Tri_Edge& Edge1( ) { return e1; }
    SWIFT_Tri_Edge* Edge1P( ) { return &e1; }
    SWIFT_Tri_Edge& Edge2( ) { return e2; }
    SWIFT_Tri_Edge* Edge2P( ) { return &e2; }
    SWIFT_Tri_Edge& Edge3( ) { return e3; }
    SWIFT_Tri_Edge* Edge3P( ) { return &e3; }
    int Edge_Id( const SWIFT_Tri_Edge* e ) const { return e - &e1; }
    SWIFT_Tri_Edge* EdgeP( int i ) { return &e1 + i; }
    SWIFT_Tri_Edge* EdgeP( SWIFT_Tri_Vertex* v )
    {
        if( v == e1.Origin() ) {
            return &e1;
        } else if( v == e2.Origin() ) {
            return &e2;
        } else {
#ifdef SWIFT_DEBUG
            if( v != e3.Origin() ) {
                cerr << "Error: Bad vertex passed to Face::EdgeP" << endl;
            }
#endif
            return &e3;
        }
    }

    int Bit_Field( ) { return bit_field; }

    // Level that face first appears going down hier.  The root is level 0.
    int Starting_Level( ) const { return (bit_field>>16) & 0x000000ff; }
    void Set_Level( int l )
            { bit_field = ((l<<16) & 0x00ff0000) | (bit_field & 0xff00ffff); }
    // The length of the twins arrays
    int Twins_Length( ) const { return (unsigned int)bit_field>>24; }
    // Record the length of the twins arrays
    void Set_Twins_Length( int tl )
                        { bit_field = (tl<<24) | (bit_field & 0x00ffffff); }
    // Get the classification of this face
    int Classification( ) const { return bit_field & 0x30; }
    // Set the classification of this face
    void Set_Classification( int c )
                                { bit_field = (bit_field & 0xffffffcf) | c; }
    bool Is_Original( ) const { return Classification() == CLASS_ORIGINAL; }
    bool Is_Contained( ) const { return Classification() == CLASS_CONTAINED; }
    bool Is_Free( ) const { return Classification() == CLASS_FREE; }

    // Set functions
    void Set_Bit_Field( int bf ) { bit_field = bf; }
    void Set_Normal_N( const SWIFT_Triple& n ) { normal = n; }
    void Set_Normal_N( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                                        { normal.Set_Value( x, y, z ); }
    void Set_Normal_U( const SWIFT_Triple& n )
                                        { normal = n; normal.Normalize(); }
    void Set_Normal_U( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                        { normal.Set_Value( x, y, z ); normal.Normalize(); }
    void Set_Distance( SWIFT_Real dist ) { d = dist; }
    void Set_Point_On_Plane( const SWIFT_Triple& p ) { d = (p*normal); }
    void Set_Point_On_Plane( ) { d = (e1.Origin()->Coords()*normal); }


    // Boolean flag values for the face
    int Marked( ) const { return bit_field & 0x8; }
    int Unmarked( ) const { return !Marked(); }
    // Boolean flag values for the edges
    int Marked_Edge( const SWIFT_Tri_Edge* e ) const
                                    { return bit_field & (1 << Edge_Id(e)); }
    int Unmarked_Edge( const SWIFT_Tri_Edge* e ) const
                                        { return !Marked_Edge(e); }
    // Set the boolean flags
    void Mark( ) { bit_field |= 0x8; }
    void Unmark( ) { bit_field &= 0xfffffff7; }
    void Mark_Edge( SWIFT_Tri_Edge* e ) { bit_field |= (1 << Edge_Id(e)); }
    void Unmark_Edge( SWIFT_Tri_Edge* e ) { bit_field &= (~(1 << Edge_Id(e))); }


    // Query functions
    SWIFT_Real Distance( SWIFT_Tri_Vertex* v ) const
                                    { return ((v->Coords()*normal) - d); }
    SWIFT_Real Distance( const SWIFT_Triple& p ) const
                                    { return (p*normal) - d; }

    bool Inside( const SWIFT_Triple& t ) const { return (t*normal) < d; }
    bool On( const SWIFT_Triple& t ) const { return (t*normal) == d; }
    bool Outside( const SWIFT_Triple& t ) const { return (t*normal) > d; }
    bool Inside( const SWIFT_Tri_Vertex* v ) const
                                            { return Inside( v->Coords() ); }
    bool On( const SWIFT_Tri_Vertex* v ) const { return On( v->Coords() ); }
    bool Outside( const SWIFT_Tri_Vertex* v ) const
                                            { return Outside( v->Coords() ); }
    bool Inside_Tol( const SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
                { return (v->Coords()*normal) <= (d + fabs(d) * tolerance); }
    bool On_Tol( SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
    {
        SWIFT_Real a = fabs(d) * tolerance;
        SWIFT_Real result = v->Coords()*normal;
        return d - a <= result && result <= d + a;
    }
    bool Outside_Tol( const SWIFT_Tri_Vertex* v, SWIFT_Real tolerance ) const
                { return (d - fabs(d) * tolerance) <= (v->Coords()*normal); }

    // Computation functions
    void Reset_Internal_Edge_Pointers( )
    {
        e1.Set_Next( &e2 ); e2.Set_Next( &e3 ); e3.Set_Next( &e1 );
        e1.Set_Adj_Face( this ); e2.Set_Adj_Face( this );
        e3.Set_Adj_Face( this );
    }

    SWIFT_Triple Computed_Normal( ) const
                { return (Coords2() - Coords1()) % (Coords3() - Coords2()); }


    void Compute_Plane_From_Edges( );
    void Compute_Plane_From_Edges( SWIFT_Tri_Edge* edge1 );
    SWIFT_Triple Centroid( )
    { return 0.333333333333333333333333 * (Coords1() + Coords2() + Coords3()); }

#ifdef SWIFT_DEBUG
    bool Verify_Topology( int pos ) const;
    bool Verify_Geometry( int pos ) const;
#endif

    void Draw() const
    {
        normal.Send_NCoords_To_OpenGL();
        e1.Origin()->Coords().Send_VCoords_To_OpenGL();
        e2.Origin()->Coords().Send_VCoords_To_OpenGL();
        e3.Origin()->Coords().Send_VCoords_To_OpenGL();
    }
  private:
    // Geometry info
    SWIFT_Triple normal;    // face plane normal
    SWIFT_Real d;           // distance of face plane from O

    // Edge/Topology info
    SWIFT_Tri_Edge e1;
    SWIFT_Tri_Edge e2;
    SWIFT_Tri_Edge e3;


    // Status bits: lowest 4 bits are the 4 mark flags for the 3 edges and then
    // the face.
    // The next higher 2 bits are for the classification of this face
    // The highest byte is for the twin lengths, the next byte is for the level
    int bit_field;
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_BV
//
// Description:
//      Bounding volume class
//////////////////////////////////////////////////////////////////////////////
class SWIFT_BV {
  public:
    // p is the parent of this bv.
    SWIFT_BV( SWIFT_BV* p = NULL ) { parent = p;
        level = 0;
    }
    ~SWIFT_BV( );

    // Get functions
    SWIFT_Triple& Center_Of_Mass( ) { return com; }
    SWIFT_Triple& Center( ) { return Center_Of_Mass(); }
    void Create_Lookup_Table( ) { lut.Create( this ); }
#ifdef SWIFT_DEBUG
    void Dump_Lookup_Table( )
    {   cerr << "BV has " << Num_Faces() << " faces and "
             << Num_Other_Faces() << " other faces" << endl;
        lut.Dump();
    }
#endif
    SWIFT_Lookup_Table& Lookup_Table( ) { return lut; }
    int Lookup_Table_Size( ) { return lut.Size(); }
    SWIFT_Tri_Edge* Lookup_Edge( const SWIFT_Triple& dir )
                                                { return lut.Lookup( dir ); }
    // The level 
    int Level( ) { return level; }
    SWIFT_BV* Parent( ) { return parent; }
    int Num_Children( ) { return children.Length(); }
    SWIFT_Array<SWIFT_BV*>& Children( ) { return children; }
    SWIFT_Array<SWIFT_Tri_Face>& Faces( ) { return faces; }
    SWIFT_Array<SWIFT_Tri_Face*>& Other_Faces( ) { return other_faces; }
    int Face_Id( SWIFT_Tri_Face* f ) { return faces.Position( f ); }
    bool Face_In_Range( SWIFT_Tri_Face* f )
                    { return Face_Id( f ) >= 0 && Face_Id( f ) < Num_Faces(); }

    SWIFT_Real Radius( ) { return radius; }

    // Set functions
    void Set_Center_Of_Mass( const SWIFT_Triple& c ) { com = c; }
    void Set_Center( const SWIFT_Triple& c ) { Set_Center_Of_Mass( c ); }
    void Set_Radius( SWIFT_Real r ) { radius = r; }
    void Set_Parent( SWIFT_BV* p ) { parent = p; }
    void Set_Children( SWIFT_Array<SWIFT_BV*>& c ) { children = c; }
    // The lists must be nullified and not destroyed after these funcs called.
    void Set_Faces( SWIFT_Array<SWIFT_Tri_Face>& fs );
    void Set_Faces( SWIFT_Array<SWIFT_Tri_Face>& vfs,
                    SWIFT_Array<SWIFT_Tri_Face>& efs );
    // The list must be nullified and not destroyed after this func is called.
    void Set_Other_Faces( SWIFT_Array<SWIFT_Tri_Face*>& fs );
    // Set the same classification to all faces newly created for this bv
    void Set_Faces_Classification( int c );
    // Set the level of this bv in the hierarchy.  Root is 0.
    void Set_Level( int l ) { level = l; }
    // Set the level of the faces in the hierarchy.
    void Set_Faces_Level( int l );
    void Increment_Level( ) { level++; }

    // Number of faces that are newly created in this bv
    int Num_Faces( ) { return faces.Length(); }
    // Number of faces created in other bvs but shared with this one
    int Num_Other_Faces( ) { return other_faces.Length(); }
    // Total number of all the faces.
    int Num_All_Faces( ) { return faces.Length() + other_faces.Length(); }


  // Query functions
    bool Is_Leaf( ) { return children.Empty(); }
    // Find an edge close to the coordinates given.  Note that it is not
    // necessarily the nearest.  Uses the closest feature function to find the
    // closest feature.  Then pick one of its edges if it is a vertex.  If it
    // is a face, find the vertex that is nearest to the coordinates, and
    // return the face edge that has the vertex as its origin.  If it is an
    // edge then return the edge or its next depending on which vertex is
    // closest to the given coordinates.
    // The given coordinates must lie outside of the bv.
    SWIFT_Tri_Edge* Close_Edge( const SWIFT_Triple& c );

    // Find the vertex that is extremal in the given direction given the
    // starting vertex as the origin of starte.  starte is set to an adjacent
    // edge of the extremal vertex.
    // The distance in the 'dir' direction is returned.
    SWIFT_Real Extremal_Vertex( const SWIFT_Triple& dir, int level,
                                SWIFT_Tri_Edge*& starte );


  // Computation functions
    void Compute_Center_Of_Mass( );
    void Compute_Center( ) { Compute_Center_Of_Mass(); }
    void Compute_Radius( );

  // Debug functions
#ifdef SWIFT_DEBUG
    void Verify( );
#endif


  private:

  // Private functions
    void Quicksort( SWIFT_Array<SWIFT_Tri_Edge*>& es, int p, int r );
    //void Compute_Geometry( );
    //void Scale( SWIFT_Real s );
    //void Compute_Geometry_After_Scale( SWIFT_Real s );
    //void Translate_To( const SWIFT_Triple& t );
    //void Compute_Geometry_After_Translate( SWIFT_Triple& t );

  // Data members
    // Number of faces created in other bvs but shared with this one
    SWIFT_Array<SWIFT_Tri_Face*> other_faces;
    // Number of faces that are newly created in this bv
    // Faces first appearing in this bv
    SWIFT_Array<SWIFT_Tri_Face> faces;

    SWIFT_BV* parent;
    SWIFT_Array<SWIFT_BV*> children;

    SWIFT_Lookup_Table lut;
    // The level of this bv
    int level;
    SWIFT_Triple com;
    SWIFT_Real radius;
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Tri_Mesh
//
// Description:
//      Triangular mesh class.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Tri_Mesh {
  public:
    SWIFT_Tri_Mesh( )
    {   ref = 0;
        big_arrays = false;
#ifndef SWIFT_ONLY_TRIS
        only_tris = true;
#endif
        no_dup_verts = true;
    }
    ~SWIFT_Tri_Mesh( ) { }
    void Prepare_For_Delete( );

  // Reference counting functions

    int Ref( ) { return ref; }
    void Increment_Ref( ) { ref++; }
    void Decrement_Ref( ) { ref--; }
    void Use_Big_Arrays( ) { big_arrays = true; }

  // Get functions

    SWIFT_Array<SWIFT_Tri_Vertex>& Vertices( ) { return verts; }
    SWIFT_Array<int>& Map_Vertex_Ids( ) { return map_vids; }
    SWIFT_Array<SWIFT_Tri_Face>& Faces( ) { return faces; }
    SWIFT_Array<int>& Map_Face_Ids( ) { return map_fids; }
    SWIFT_Array<SWIFT_Tri_Face*>& Other_Faces( ) { return other_faces; }
    SWIFT_Array<SWIFT_Tri_Edge*>& Edge_Twins( ) { return edge_twins; }
    SWIFT_Array<SWIFT_BV>& BVs( ) { return bvs; }

#ifndef SWIFT_ONLY_TRIS
    bool Only_Triangles( ) { return only_tris; }
#endif
    bool No_Duplicate_Vertices( ) { return no_dup_verts; }

    SWIFT_Triple& Center_Of_Mass( ) { return com; }
    SWIFT_Triple& Center( ) { return center; }
    SWIFT_Real Radius( ) { return radius; }
    SWIFT_BV* Root( ) { return bvs.Data(); }
    int Height( ) { return height; }

  // Set functions
#ifndef SWIFT_ONLY_TRIS
    void Set_Only_Triangles( bool ot ) { only_tris = ot; }
#endif
    void Set_No_Duplicate_Vertices( bool ndv ) { no_dup_verts = ndv; }

    void Set_Center_Of_Mass( const SWIFT_Triple& c ) { com = c; }
    void Set_Center( const SWIFT_Triple& c ) { center = c; }
    void Set_Radius( SWIFT_Real r ) { radius = r; }
    void Set_Height( int h ) { height = h; }
    void Set_Faces_Classification( int c );

  // Vertex, edge and face id query functions

    int Num_Vertices( ) { return verts.Length(); }
    int Vertex_Id( SWIFT_Tri_Vertex* v ) { return verts.Position( v ); }
    int Map_Vertex_Id( SWIFT_Tri_Vertex* v )
                    { return (no_dup_verts ? verts.Position( v )
                                           : map_vids[verts.Position( v )]); }
    bool Vertex_In_Range( SWIFT_Tri_Vertex* v )
                            { return verts.Position( v ) >= 0 &&
                                     verts.Position( v ) < Num_Vertices(); }
    int Num_Faces( ) { return faces.Length(); }
    int Face_Id( SWIFT_Tri_Face* f ) { return faces.Position( f ); }
#ifdef SWIFT_ONLY_TRIS
    int Map_Face_Id( SWIFT_Tri_Face* f ) { return faces.Position( f ); }
#else
    int Map_Face_Id( SWIFT_Tri_Face* f )
                    { return (only_tris ? faces.Position( f )
                                        : map_fids[faces.Position( f )]); }
#endif
    bool Face_In_Range( SWIFT_Tri_Face* f )
                    { return Face_Id( f ) >= 0 && Face_Id( f ) < Num_Faces(); }
    int Edge_Id( SWIFT_Tri_Edge* e )
    { return faces.Position( e->Adj_Face() )*3 + e->Adj_Face()->Edge_Id( e ); }
    SWIFT_Tri_Edge* EdgeP( int i ) { return faces[i/3].EdgeP(i%3); }
    bool Edge_In_Range( SWIFT_Tri_Edge* e )
                                    { return Face_In_Range( e->Adj_Face() ); }
    int Num_BVs( ) { return bvs.Length(); }

  // Creation functions

    // Create the hierarchy internally if hierarchy turned on.
    // The return value indicates success.
    bool Create( const SWIFT_Real* vs, const int* fs, int vn, int fn,
                 const SWIFT_Orientation& orient,
                 const SWIFT_Translation& trans,
                 SWIFT_Real sc, const int* fv = NULL
                );


    SWIFT_Tri_Mesh* Clone( const SWIFT_Orientation& orient,
                        const SWIFT_Translation& trans, SWIFT_Real sc = 1.0 );

    // The vfs subarrays should not be destroyed after calling this function
    // They can be nullified however.
    void Create_BV_Hierarchy( SPLIT_TYPE split,
                              SWIFT_Array<int>& piece_ids,
                              SWIFT_Array< SWIFT_Array<int> >& mfs,
                              SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face> >& vfs,
                              SWIFT_Array<SWIFT_Tri_Face*>& st_faces,    
                              SWIFT_Array<SWIFT_Tri_Edge*>& st_twins );
    void Create_Single_BV_Hierarchy( );
    void Compute_All_Hierarchy_Geometry( );


  // Debug functions
#ifdef SWIFT_DEBUG
    void Verify( );
    void Check_Face_Levels( );
    void Check_Face_Levels( SWIFT_BV* piece, int level );
#endif

    void Compute_Edge_Convexities( SWIFT_Array<bool>& ecs );
    void Accum_Bounding_Box( SWIFT_Triple& minc, SWIFT_Triple& maxc );
    void Compute_Bounding_Box( SWIFT_Triple& minc, SWIFT_Triple& maxc );
  private:

  // Private functions
    void Quicksort( SWIFT_Array<SWIFT_Tri_Edge*>& es, int p, int r );
    void Compute_Geometry( );
    void Translate_To( const SWIFT_Triple& t );
    void Compute_Geometry_After_Translate( );
    void Compute_Center_Of_Mass( );
    void Compute_Radius( );

    // Removes duplicate vertices. Allocates the vmapping and vnewindex arrays.
    // Also allocates the creates the map_vids array if needed.
    void Process_Vertices( const SWIFT_Real* vs, int vn,
                           int& vnn, const SWIFT_Real*& new_vs,
                           SWIFT_Array<int>& vmap, SWIFT_Array<int>& vnewi,
                           bool create_vids = true );

    // Transforms the vertices in vs into the verts (which is created before
    // this call to have the same number of verts).
    void Transform_Vertices( const SWIFT_Real* vs, int vn,
                             const SWIFT_Orientation& orient,
                             const SWIFT_Translation& trans, SWIFT_Real sc );

    // Transforms the vertices in the source mesh msrc into the verts (which
    // is created before this call to have the same number of verts).  Also
    // transforms the center of mass.  This fcn is used by the Clone fcn.
    void Transform_Vertices_And_COM( SWIFT_Tri_Mesh* msrc,
                            const SWIFT_Orientation& orient,
                            const SWIFT_Translation& trans, SWIFT_Real sc );


    // Triangulates faces.  Allocates the triang_edges array in which each
    // entry corresponds to a face index whose 3rd edge is the triang_edge.
    // May allocate the new_fs array (if only_tris is not set in which case the
    // caller should delete new_fs).  Also allocates the faces array and
    // creates map_fids if needed.
    void Process_Faces( const int* fs, int fn, const int* fv, int& tn,
                        const int*& new_fs, SWIFT_Array<int>& triang_edges,
                        bool create_fids = true );

    void Create_BV_Subtree( int& bvid, SPLIT_TYPE split,
                        SWIFT_Array<SWIFT_BV*>& leaves,
                        SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face*> >& ofmap,
                        SWIFT_Array<SWIFT_Tri_Face*>& st_faces,
                        SWIFT_Array<SWIFT_Tri_Edge*>& st_twins );

    void Create_Twins( SWIFT_BV* bv );

  // Data members
    SWIFT_Array<SWIFT_Tri_Vertex> verts;    // Vertex list
    SWIFT_Array<int> map_vids;              // 1-n mapping to original vert ids
    SWIFT_Array<SWIFT_Tri_Face> faces;      // Face (triangle) list
    SWIFT_Array<int> map_fids;              // n-1 mapping to original face ids
    SWIFT_Array<SWIFT_Tri_Face*> other_faces;
    SWIFT_Array<SWIFT_Tri_Edge*> edge_twins;
    SWIFT_Array<SWIFT_BV> bvs;
    int height;

    int ref;
    bool big_arrays;
    bool no_dup_verts;
#ifndef SWIFT_ONLY_TRIS
    bool only_tris;
#endif
    SWIFT_Triple com;
    SWIFT_Triple center;
    SWIFT_Real radius;

};

///////////////////////////////////////////////////////////////////////////////
// Inline functions
///////////////////////////////////////////////////////////////////////////////
inline SWIFT_Tri_Edge* SWIFT_Tri_Vertex::Adj_Edge( SWIFT_Tri_Vertex* v ) const
{
#ifdef SWIFT_ALLOW_BOUNDARY
    SWIFT_Tri_Edge* medge = edge;
    for( ; medge->Twin() != NULL; medge = medge->Twin()->Next() ) {
        if( medge->Next()->Origin() == v ) { 
            return medge;
        }   
    }   
    for( medge = edge->Prev()->Twin(); ; medge = medge->Prev()->Twin() ) {
        if( medge->Next()->Origin() == v ) {
            return medge;
        }
    }
#else
    SWIFT_Tri_Edge* medge = edge;
    for( ; ; medge = medge->Twin()->Next() ) {
        if( medge->Next()->Origin() == v ) {
            return medge;
        }
    }
#endif
}

inline SWIFT_Tri_Face* SWIFT_Tri_Vertex::Adj_Face( SWIFT_Tri_Vertex* v ) const
{
    return Adj_Edge( v )->Adj_Face();
}

inline SWIFT_Triple SWIFT_Tri_Vertex::Gathered_Normal( ) const
{
#ifdef SWIFT_ALLOW_BOUNDARY
    SWIFT_Tri_Edge* medge = edge->Twin() == NULL ? edge : edge->Twin()->Next();
    SWIFT_Triple tri = edge->Adj_Face()->Normal();
    for( ; medge != edge && medge->Twin() != NULL;
           medge = medge->Twin()->Next()
    ) {
        tri += medge->Adj_Face()->Normal();
    }
    if( medge->Twin() == NULL ) {
        for( medge = edge->Prev()->Twin(); medge != NULL;
             medge = medge->Prev()->Twin()
        ) {
            tri += medge->Adj_Face()->Normal();
        }
    }
#else   
    SWIFT_Tri_Edge* medge = edge->Twin()->Next();
    SWIFT_Triple tri = edge->Adj_Face()->Normal();
    for( ; medge != edge; medge = medge->Twin()->Next() ) {
        tri += medge->Adj_Face()->Normal();
    }
#endif  
    return tri;
}

inline SWIFT_Triple SWIFT_Tri_Vertex::Gathered_Direction( ) const
{
#ifdef SWIFT_ALLOW_BOUNDARY
    SWIFT_Tri_Edge* medge = edge->Twin() == NULL ? edge : edge->Twin()->Next();
    SWIFT_Triple tri = edge->Direction();
    for( ; medge != edge && medge->Twin() != NULL;
           medge = medge->Twin()->Next()
    ) {
        tri += medge->Direction();
    }
    if( medge->Twin() == NULL ) {
        for( medge = edge->Prev()->Twin(); medge != NULL;
             medge = medge->Prev()->Twin()
        ) {
            tri += medge->Direction();
        }
    }
#else   
    SWIFT_Tri_Edge* medge = edge->Twin()->Next();
    SWIFT_Triple tri = edge->Direction();
    for( ; medge != edge; medge = medge->Twin()->Next() ) {
        tri += medge->Direction();
    }
#endif  
    return tri;
}

inline int SWIFT_Tri_Edge::Starting_Level( ) const
                                            { return face->Starting_Level(); }
inline int SWIFT_Tri_Edge::Classification( int glevel ) const
{
    if( face->Classification() == CLASS_ORIGINAL ||
        Twin( glevel )->Adj_Face()->Classification() == CLASS_ORIGINAL
    ) {
        return CLASS_ORIGINAL;
    } else if( face->Classification() == CLASS_CONTAINED ||
        Twin( glevel )->Adj_Face()->Classification() == CLASS_CONTAINED
    ) {
        return CLASS_CONTAINED;
    } else {
        return CLASS_FREE;
    }
}
inline int SWIFT_Tri_Edge::Marked( ) const { return face->Marked_Edge( this ); }
inline int SWIFT_Tri_Edge::Unmarked( ) const { return !Marked(); }
inline void SWIFT_Tri_Edge::Mark( ) { face->Mark_Edge( this ); }
inline void SWIFT_Tri_Edge::Unmark( ) { face->Unmark_Edge( this ); }
inline void SWIFT_Tri_Edge::Compute_Face_Plane( )
                    { fn = face->Normal() % u; Set_Origin_On_Face_Plane(); }


#endif


