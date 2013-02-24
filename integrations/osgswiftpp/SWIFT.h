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
// SWIFT.h
//
// Description:
//      Class to manage an entire scene.  This is the calling interface for
//  the SWIFT system.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_H_
#define _SWIFT_H_


#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_array.h>

//////////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////////
typedef SWIFT_Real SWIFT_Orientation[9];
typedef SWIFT_Real SWIFT_Translation[3];

typedef SWIFT_Real SWIFT_Matrix[9];

typedef enum { MEDIAN, MIDPOINT, MEAN, GAP } SPLIT_TYPE;

//////////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////////

// Object creation box settings
static const int BOX_SETTING_DEFAULT = 0;
static const int BOX_SETTING_CUBE = 1;
static const int BOX_SETTING_DYNAMIC = 2;
static const int BOX_SETTING_CHOOSE = 3;

// Feature type identifiers
static const int SWIFT_VERTEX = 1;
static const int SWIFT_EDGE = 2;
static const int SWIFT_FACE = 3;

// Default scene configuration
static const bool DEFAULT_BP = true;                        // Broad phase on
static const bool DEFAULT_GS = true;                        // Global sort on

// Default object configuration
static const bool DEFAULT_FIXED = false;                    // Moving
static const SWIFT_Orientation DEFAULT_ORIENTATION          // Identity
                    = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
static const SWIFT_Translation DEFAULT_TRANSLATION = {0.0, 0.0, 0.0};// Identity
static const SWIFT_Real DEFAULT_SCALE = 1.0;                // Identity

const SPLIT_TYPE DEFAULT_SPLIT_TYPE = MIDPOINT;

static const int DEFAULT_BOX_SETTING = BOX_SETTING_DEFAULT;
static const SWIFT_Real DEFAULT_BOX_ENLARGE_REL = 0.0;      // No enlargement
static const SWIFT_Real DEFAULT_BOX_ENLARGE_ABS = 0.0;      // No enlargement
static const int* DEFAULT_FACE_VALENCES = NULL;             // Triangular model
static const int DEFAULT_COPY = -1;                         // No copy
static const SWIFT_Real DEFAULT_CUBE_ASPECT_RATIO = 2.0;

// Default query configuration
static SWIFT_Real** NO_DISTANCES = NULL;
static SWIFT_Real** NO_NEAREST_PTS = NULL;
static SWIFT_Real** NO_NORMALS = NULL;
static int** NO_FEAT_TYPES = NULL;
static int** NO_FEAT_IDS = NULL;


//////////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Object;
class SWIFT_Box_Node;
class SWIFT_Pair;
class SWIFT_File_Reader;


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Scene
//
// Description:
//      Class to manage a scene composed of objects.  This class can import
//  object geometry, test objects for intersection, tolerance verification,
//  compute distance or contacts between them, find closest features, closest
//  points, contact normals, and produce reports on the results.  Also provided
//  are mechanisms to activate or deactivate certain parts of the scene and to
//  move objects in a scene.
//
//  Further documentation can be found included in the system distribution.
//
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Scene {
  public:

///////////////////////////////////////////////////////////////////////////////
// Scene Creation (Configuration) and Deletion Methods
///////////////////////////////////////////////////////////////////////////////

    // Configure the scene.  Turn the broad phase (sweep and prune) algorithm
    // on or off.  Turn on global sorting or local sorting (by setting
    // global_sort to false).
    SWIFT_Scene( bool broad_phase = DEFAULT_BP,
                 bool global_sort = DEFAULT_GS );

    ~SWIFT_Scene( );


///////////////////////////////////////////////////////////////////////////////
// Object Creation methods
///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Each of the following Add_*_Object methods returns true if the object
    // addition succeeded otherwise false and an error message is printed to
    // stderr.
    ///////////////////////////////////////////////////////////////////////////

    // Add a convex object to the scene.  SWIFT expects this object to be
    // convex.  If it is not, the behavior is undefined.  Since only polyhedral
    // geometry is given here, a non-convex object cannot be added using this
    // function since it must first be decomposed before it is passed into
    // SWIFT.  The input geometry is given in arrays.  To copy from a previously
    // added object (convex or non-convex), use the Copy_Object function.
    bool Add_Convex_Object(
                const SWIFT_Real* vertices, const int* faces,
                int num_vertices, int num_faces, int& id,
                bool fixed = DEFAULT_FIXED,
                const SWIFT_Orientation& orient = DEFAULT_ORIENTATION,
                const SWIFT_Translation& trans = DEFAULT_TRANSLATION,
                SWIFT_Real scale = DEFAULT_SCALE,
                int box_setting = DEFAULT_BOX_SETTING,
                SWIFT_Real box_enl_rel = DEFAULT_BOX_ENLARGE_REL,
                SWIFT_Real box_enl_abs = DEFAULT_BOX_ENLARGE_ABS,
                const int* face_valences = DEFAULT_FACE_VALENCES,
                SWIFT_Real cube_aspect_ratio = DEFAULT_CUBE_ASPECT_RATIO );

    // Add a polyhedral object to the scene by file.  If the file is plain
    // geometry (the description of a polyhedron), it must be convex (see
    // comment for Add_Convex_Object).  If the file is a decomposition file or
    // a convex hierarchy file, then the object is allowed to be non-convex.
    // To copy from a previously added object (convex or non-convex), use the
    // Copy_Object function.
    bool Add_General_Object(
                const char* filename, int& id,
                bool fixed = DEFAULT_FIXED,
                const SWIFT_Orientation& orient = DEFAULT_ORIENTATION,
                const SWIFT_Translation& trans = DEFAULT_TRANSLATION,
                SWIFT_Real scale = DEFAULT_SCALE,
                SPLIT_TYPE split = DEFAULT_SPLIT_TYPE,
                int box_setting = DEFAULT_BOX_SETTING,
                SWIFT_Real box_enl_rel = DEFAULT_BOX_ENLARGE_REL,
                SWIFT_Real box_enl_abs = DEFAULT_BOX_ENLARGE_ABS,
                SWIFT_Real cube_aspect_ratio = DEFAULT_CUBE_ASPECT_RATIO );

    // Copy an object in the scene.  There will not be any replication of
    // object geometry.  The copied object will be identical to the object
    // being copied from.  Id of the newly created object is returned in id.
    bool Copy_Object( int copy_oid, int& id );


///////////////////////////////////////////////////////////////////////////////
// Object Deletion methods
///////////////////////////////////////////////////////////////////////////////

    // Delete the object with the specified id from the scene.  Its id may
    // be reused for a subsequently added object.
    void Delete_Object( int id );

///////////////////////////////////////////////////////////////////////////////
// Object Rigid Transformation methods
///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Note that in all of the following Set_*_Transformation(s) methods, the
    // only transformations that are allowed are translations and rotations.
    // Sacling and skewing and any other transformations are not permitted.
    //
    // For maximum query performance, avoid setting an object's transformation
    // more than once per query.  Each object's transformation must be set at
    // least once after its creation to ensure correct querying.
    ///////////////////////////////////////////////////////////////////////////

    // Set the transformation for the object given by id i.
    // R should have length 9 and be a 3x3 rotation matrix in row-major form
    // T is the translation vector and should be of length 3.
    void Set_Object_Transformation( int id, const SWIFT_Real* R,
                                            const SWIFT_Real* T );

    // Set the transformation for the object given by id i.
    // R should be of length 12 and be a 3x4 transformation matrix in
    // row-major form: [R|T]. 
    void Set_Object_Transformation( int id, const SWIFT_Real* RT );

    // Set the transformation for all objects.  They must be given in the same
    // order that the objects were added to the scene in.
    // R should have length 9 x number of moving objects and be a series of
    // 3x3 rotation matrices in row-major form.
    // T should have length 3 x number of moving objects and be a series of
    // translation vectors.
    void Set_All_Object_Transformations( const SWIFT_Real* R,
                                         const SWIFT_Real* T );

    // Set the transformation for all objects.  They must be given in the same
    // order that the objects were added to the scene in.
    // R should have length 12 x number of moving objects and be a series of
    // 3x4 transformation matrices in row-major form: [R|T]. 
    void Set_All_Object_Transformations( const SWIFT_Real* RT );


///////////////////////////////////////////////////////////////////////////////
// Pair Activation methods
///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Activate or deactivate pairs of objects.  A pair, the set of pairs
    // containing an object, or all the pairs may be activated or deactivated.
    // When a pair is active, it is tracked and reported on.
    //
    // By default, when an object is added, all of the pairs it is a member of
    // are active unless any of those pairs are composed of two fixed objects.
    // Furthermore, no pairs of two fixed objects are ever activated.
    ///////////////////////////////////////////////////////////////////////////

    // Activate a pair of objects whose ids are given as id1 and id2.
    void Activate( int id1, int id2 );

    // Activate all pairs for object with a certain id.
    void Activate( int id );

    // Activate all pairs
    void Activate( );

    // Deactivate a pair of objects whose ids are given as id1 and id2.
    void Deactivate( int id1, int id2 );

    // Deactivate all pairs for object with a certain id.
    void Deactivate( int id );

    // Deactivate all pairs.
    void Deactivate( );


///////////////////////////////////////////////////////////////////////////////
// Query methods
///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Query geometric properties of the objects
    ///////////////////////////////////////////////////////////////////////////

    // Get the center of mass for object with given id.  This is NOT the center
    // of the bounding box of the object but the actual physical center of mass.
    // The center of mass that is returned is the center of mass of the object
    // after its input transformation was applied but not any of the other
    // transformations that may have been set for it.
    // This function does not compute anything.  It is just a copy.
    void Get_Center_Of_Mass( int id,
                             SWIFT_Real& x, SWIFT_Real& y, SWIFT_Real& z );

    ///////////////////////////////////////////////////////////////////////////
    // Query various proximity statuses from the scene.  A return value of true
    // from any of the Query methods indicates that intersection was detected
    // in the scene and false indicates that none of the objects in the scene
    // are intersecting.  See the system documentation for more information on
    // any of the queries.
    //
    // All the arrays are managed by the SWIFT system.  Do NOT allocate or
    // deallocate them.
    ///////////////////////////////////////////////////////////////////////////

    // Intersection detection query.  The return value indicates if there was
    // an intersecting pair in the scene.  If the early_exit parameter is set to
    // be true, then the computation is stopped when the first intersection is
    // found and true is returned but no pairs are reported.  If early_exit is
    // set to false, the object ids array will be allocated and filled in by
    // this method if intersection is detected.  There are 2 * num_pairs
    // elements in the array.  For example, oids[0] and oids[1] are an  
    // intersecting pair (if num_pairs > 0).
    bool Query_Intersection( bool early_exit, int& num_pairs, int** oids );

    // Tolerance verification query.  If the distance between any objects are
    // closer than the given tolerance the objects are reported.  The oids,
    // early_exit, and num_pairs parameters as well as the return value,
    // function in the same manner as the Query_Intersection method.
    bool Query_Tolerance_Verification( bool early_exit, SWIFT_Real tolerance,   
                                       int& num_pairs, int** oids );

    // Approximate distance computation query.  The desired absolute and
    // relative error are given in the abs_error and rel_error parameters.
    // If the exact distance between objects is less than the tolerance,
    // it is reported to within the specified error.  There are num_pairs
    // elements in the distances array.  If any of the objects are intersecting,
    // the distance is given as -1.0 (this is not a measure of penetration).
    // The oids, early_exit, and num_pairs parameters as well as the return
    // value, function in the same manner as the Query_Intersection method.
    bool Query_Approximate_Distance(
                    bool early_exit, SWIFT_Real tolerance,
                    SWIFT_Real abs_error, SWIFT_Real rel_error,
                    int& num_pairs, int** oids, SWIFT_Real** distances );


    // Exact distance computation query.  It is quite similar to the
    // approximate distance query except that the reported distances are exact.
    // No distances below the given tolerance are reported.
    bool Query_Exact_Distance(
                    bool early_exit, SWIFT_Real tolerance, int& num_pairs,
                    int** oids, SWIFT_Real** distances );


    // Contact determination query.  It is quite similar to the exact distance
    // query except more information is available than just distance.
    // There are various things that may be computed for a contact.  They are:
    // the distance, the nearest points, the contact normals, and the nearest
    // features.  They are selected for reporting by passing an array pointer to
    // be assigned by SWIFT.  To not select an item for reporting, simply pass
    // NULL for that array.  The default is that all items are deselected.
    // There is the possibility that there are multiple contacts per pair.  The
    // optional num_contacts array gives the number of contacts for each pair.
    // It should be set non-NULL if any information other than the oids is
    // required.
    // See the included documentation for a more detailed description of the
    // reported items.
    bool Query_Contact_Determination(
                bool early_exit, SWIFT_Real tolerance, int& num_pairs,
                int** oids, int** num_contacts,
                SWIFT_Real** distances = NO_DISTANCES,
                SWIFT_Real** nearest_pts = NO_NEAREST_PTS,
                SWIFT_Real** normals = NO_NORMALS,
                int** feature_types = NO_FEAT_TYPES,
                int** feature_ids = NO_FEAT_IDS );


///////////////////////////////////////////////////////////////////////////////
// Plug-In Registration methods
///////////////////////////////////////////////////////////////////////////////

    // Register a file reader.  A file reader does not have to be registered
    // for every scene created but only has to be registered once for the
    // entire program.  For more information on file reader construction, see
    // the included documentation and SWIFT_fileio.h.  The return value
    // indicates success.
    bool Register_File_Reader( const char* magic_number,
                               SWIFT_File_Reader* file_reader ) const;


  private:
///////////////////////////////////////////////////////////////////////////////
// Private methods
///////////////////////////////////////////////////////////////////////////////
    // Scene maintenance methods
    void Initialize_Object_In_Scene( SWIFT_Object* cobj, int& id );

    // Bounding box sorting and update methods
    inline void Update_Overlap( int axis, int id1, int id2 );
    inline void Sort_Global( int axis );
    void Sort_Global( );
    inline void Sort_Local( int oid, int axis );
    void Sort_Local( int oid );

///////////////////////////////////////////////////////////////////////////////
// Private data
///////////////////////////////////////////////////////////////////////////////
    // Scene configuration
    //      Sweep and Prune
    bool bp; // Broad phase enabled?
    bool gs; // Global sorting enabled?

    // The objects in the scene
    SWIFT_Array<SWIFT_Object*> objects;
    SWIFT_Array<int> free_oids;

    // Mapping of external object ids to internal object ids
    SWIFT_Array<int> object_ids;

    // Mapping of internal object ids to external object ids
    SWIFT_Array<int> user_object_ids;


    // The sweep and prune lists which contain internal piece ids
    SWIFT_Array<SWIFT_Box_Node*> sorted[3];

    // Pair count
    int total_pairs;

    // The tracked list of pairs that are overlapping
    SWIFT_Pair* overlapping_pairs;

    // Reporting lists
    SWIFT_Array<int> ois;           // object ids
    SWIFT_Array<SWIFT_Real> ds;     // distances
    SWIFT_Array<int> ncs;           // num contacts
    SWIFT_Array<SWIFT_Real> nps;    // nearest points
    SWIFT_Array<SWIFT_Real> cns;    // contact normals
    SWIFT_Array<int> pis;           // piece ids
    SWIFT_Array<int> fts;           // feature types
    SWIFT_Array<int> fis;           // feature ids
};

#endif


