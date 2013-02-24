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
// scene.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <iostream>


#include <SWIFT.h>
#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_array.h>
#include <SWIFT_mesh.h>
#include <SWIFT_mesh_utils.h>
#include <SWIFT_boxnode.h>
#include <SWIFT_object.h>
#include <SWIFT_pair.h>
#include <SWIFT_fileio.h>


///////////////////////////////////////////////////////////////////////////////
// File Reading Objects
///////////////////////////////////////////////////////////////////////////////

SWIFT_File_Read_Dispatcher file_dispatcher;
SWIFT_Basic_File_Reader basic_file_reader;
SWIFT_Obj_File_Reader obj_file_reader;


///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

// Segment size for the object array which is grown dynamically.  This should
// be a power of 2.
static const int OBJECT_SEGMENT_SIZE = 4;

// How big to create the reporting lists and how much to grow them by
static const int REPORTING_LIST_CREATION_SIZE = 100;
static const int REPORTING_LIST_GROW_SIZE = 100;


///////////////////////////////////////////////////////////////////////////////
// Static functions
///////////////////////////////////////////////////////////////////////////////
static bool Use_Cube( SWIFT_Tri_Mesh* m, bool fixed, int box_setting,
                      SWIFT_Real cube_aspect_ratio_threshold )
{
    if( fixed ) {
        return false;
    } else {
        if( box_setting == BOX_SETTING_CUBE ) {
            return true;
        } else if( box_setting == BOX_SETTING_DYNAMIC ) {
            return false;
        } else {
            // Choose the box setting based on aspect ratio
            SWIFT_Real min_spread, max_spread;
            Compute_Min_And_Max_Spread( m, m->Root(), min_spread, max_spread );
            return max_spread / min_spread < cube_aspect_ratio_threshold;
        }
    }
}

static bool Is_Identity( const SWIFT_Orientation& orient,
                         const SWIFT_Translation& trans, SWIFT_Real scale )
{
    if( scale == 1.0 ) {
        if( trans == NULL ||
            (trans[0] == 0.0 && trans[1] == 0.0 && trans[2] == 0.0)
        ) {
            return orient == NULL ||
                   (orient[0] == 1.0 && orient[1] == 0.0 && orient[2] == 0.0 &&
                    orient[3] == 0.0 && orient[4] == 1.0 && orient[5] == 0.0 &&
                    orient[6] == 0.0 && orient[7] == 0.0 && orient[8] == 1.0);
        } else {
            return false;
        }
    } else {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Scene Creation methods
///////////////////////////////////////////////////////////////////////////////

SWIFT_Scene::SWIFT_Scene( bool broad_phase, bool global_sort )
{
    // Create the lists
    objects.Create( OBJECT_SEGMENT_SIZE );
    object_ids.Create( OBJECT_SEGMENT_SIZE );
    user_object_ids.Create( OBJECT_SEGMENT_SIZE );
    objects.Set_Length( 0 );
    object_ids.Set_Length( 0 );
    user_object_ids.Set_Length( 0 );

    total_pairs = 0;
    overlapping_pairs = NULL;

    // Register the file readers
    basic_file_reader.Register_Yourself( file_dispatcher );
    obj_file_reader.Register_Yourself( file_dispatcher );

    bp = broad_phase;
    gs = global_sort;

    if( bp ) {
        sorted[0].Create( OBJECT_SEGMENT_SIZE<<1 );
        sorted[1].Create( OBJECT_SEGMENT_SIZE<<1 );
        sorted[2].Create( OBJECT_SEGMENT_SIZE<<1 );
        sorted[0].Set_Length( 0 );
        sorted[1].Set_Length( 0 );
        sorted[2].Set_Length( 0 );
    }

    // Create the initial reporting lists
    ds.Create( REPORTING_LIST_CREATION_SIZE );
    ds.Set_Length( 0 );
    nps.Create( REPORTING_LIST_CREATION_SIZE*6 );
    nps.Set_Length( 0 );
    cns.Create( REPORTING_LIST_CREATION_SIZE*3 );
    cns.Set_Length( 0 );
    fts.Create( REPORTING_LIST_CREATION_SIZE<<1 );
    fts.Set_Length( 0 );
    fis.Create( REPORTING_LIST_CREATION_SIZE<<2 );
    fis.Set_Length( 0 );

    // Initialize non-object modules
    Mesh_Utils_Initialize();
    SWIFT_File_IO_Initialize();
}


///////////////////////////////////////////////////////////////////////////////
// Scene Deletion methods
///////////////////////////////////////////////////////////////////////////////

SWIFT_Scene::~SWIFT_Scene( )
{
    int i;

    for( i = 0; i < objects.Length(); i++ ) {
        // Just delete the objects.  The boxes and the geometry are included
        // in the objects.
        delete objects[i];
    }
}


//////////////////////////////////////////////////////////////////////////////
// Object Creation methods
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Adds convex polyhedral object from arrays
bool SWIFT_Scene::Add_Convex_Object(
    const SWIFT_Real* vs, const int* fs, int vn, int fn, int& id, bool fixed,
    const SWIFT_Orientation& orient, const SWIFT_Translation& trans,
    SWIFT_Real scale, int box_setting, SWIFT_Real box_enl_rel,
    SWIFT_Real box_enl_abs, const int* fv, SWIFT_Real cube_aspect_ratio )
{
    SWIFT_Tri_Mesh* stm;
    bool use_cube;

    stm = new SWIFT_Tri_Mesh;

    if( box_setting == BOX_SETTING_DEFAULT ) {
        box_setting = fixed ? BOX_SETTING_DYNAMIC : BOX_SETTING_CUBE;
    }

    if( !stm->Create( vs, fs, vn, fn, orient, trans, scale, fv ) ) {
        // Delete everything
        delete stm;
        return false;
    }

    stm->Create_Single_BV_Hierarchy();

    use_cube = Use_Cube( stm, fixed, box_setting, cube_aspect_ratio );

    SWIFT_Object* cobj = new SWIFT_Object;

    cobj->Initialize( stm, fixed, use_cube, box_enl_rel, box_enl_abs, false );

    Initialize_Object_In_Scene( cobj, id );

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// Adds general polyhedral object from file
bool SWIFT_Scene::Add_General_Object(
    const char* f, int& id, bool fixed, const SWIFT_Orientation& orient,
    const SWIFT_Translation& trans, SWIFT_Real scale,
    SPLIT_TYPE split,
    int box_setting, SWIFT_Real box_enl_rel, SWIFT_Real box_enl_abs,
    SWIFT_Real cube_aspect_ratio )
{
    SWIFT_Tri_Mesh* stm;
    bool use_cube;


    SWIFT_FILE_TYPE ft;
    if( !SWIFT_File_Type( f, ft ) ) {
        return false;
    }

    if( box_setting == BOX_SETTING_DEFAULT ) {
        box_setting = fixed ? BOX_SETTING_DYNAMIC : BOX_SETTING_CUBE;
    }

    switch( ft ) {
    case FT_CHIER:
        if( !SWIFT_File_Read_Hierarchy( f, stm, orient, trans, scale ) ) { 
            return false;
        }
        stm->Compute_All_Hierarchy_Geometry();
        break;
    case FT_DECOMP:
#ifdef SWIFT_DECOMP
        // Dummy return
        return true;
#else
        if( !SWIFT_File_Read_Decomposition(
                                        f, stm, orient, trans, scale, split )
        ) {
            return false;
        }
#endif
        break;
    case FT_OTHER:
      { SWIFT_Real* vs = NULL;
        int* fs = NULL;
        int vn, fn;
        int* fv = NULL;

        if( !file_dispatcher.Read( f, vs, fs, vn, fn, fv ) ) {
            // Delete everything
            delete vs; delete fs; delete fv;

            const int new_obj_id = free_oids.Length() == 0 ? objects.Length()
                                                          : free_oids.Last();
            cerr << "Error (Add_General_Object obj " << new_obj_id
                 << "): File read failed" << endl;
            return false;
        }

        stm = new SWIFT_Tri_Mesh;

        if( !stm->Create( vs, fs, vn, fn, orient, trans, scale, fv ) ) {
            // Delete everything
            delete stm; delete vs; delete fs; delete fv;

            const int new_obj_id = free_oids.Length() == 0 ? objects.Length()
                                                          : free_oids.Last();
            cerr << "Error (Add_General_Object obj " << new_obj_id
                 << "): Mesh creation failed" << endl;
            return false;
        }

        delete vs; delete fs; delete fv;
        stm->Create_Single_BV_Hierarchy();
      }
        break;
    default:
        break;
    }

    use_cube = Use_Cube( stm, fixed, box_setting, cube_aspect_ratio );

    SWIFT_Object* cobj = new SWIFT_Object;

    cobj->Initialize( stm, fixed, use_cube, box_enl_rel, box_enl_abs, false );

    Initialize_Object_In_Scene( cobj, id );

    return true;
}


bool SWIFT_Scene::Copy_Object( int copy_oid, int& id )
{
#ifdef SWIFT_DEBUG
    const int new_obj_id = free_oids.Length() == 0 ? objects.Length()
                                                  : free_oids.Last();
    if( copy_oid < 0 || copy_oid >= object_ids.Length() ) {
        cerr << "Error (Copy_Object obj " << new_obj_id
             << "): Invalid object id given to copy a piece: "
             << copy_oid << endl;
        return false;
    }

    if( object_ids[copy_oid] == -1 ) {
        cerr << "Error (Copy_Object obj " << new_obj_id
             << "): Object to be copied is deleted: " << copy_oid << endl;
        return false;
    }
#endif

    SWIFT_Object* cobj = new SWIFT_Object;

    *cobj = *objects[object_ids[copy_oid]];
    cobj->Mesh()->Increment_Ref();
    cobj->Pairs().Nullify();

    Initialize_Object_In_Scene( cobj, id );

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Object Deletion methods
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Scene::Delete_Object( int id )
{
#ifdef SWIFT_DEBUG
    // Check the validity of the id
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: Object id out of range given to Delete_Object("
             << id << "): " << id << endl;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: Object with id given to Delete_Object("
             << id << "): already deleted" << endl;
    }
#endif

    int i, j;
    SWIFT_Object* free_obj = objects[object_ids[id]];

    if( bp ) {
        // Remove the boxes from the sorted lists and compress the lists
        for( j = 0; j < 3; j++ ) {
            // Fix the boxes before the first deleted index
            for( i = 0; i < free_obj->Min_Box_Node(j)->Idx(); i++ ) {
                if( sorted[j][i]->Id() > object_ids[id] ) {
                    sorted[j][i]->Set_Id( sorted[j][i]->Id()-1 );
                }
            }
            // Fix the boxes between the two deleted indices
            for( i = free_obj->Min_Box_Node(j)->Idx();
                 i < free_obj->Max_Box_Node(j)->Idx()-1; i++
            ) {
                sorted[j][i] = sorted[j][i+1];
                sorted[j][i]->Set_Idx( i );
                if( sorted[j][i]->Id() > object_ids[id] ) {
                    sorted[j][i]->Set_Id( sorted[j][i]->Id()-1 );
                }
            }
            // Fix the boxes after the second deleted index
            for( ; i < sorted[j].Length()-2; i++ ) {
                sorted[j][i] = sorted[j][i+2];
                sorted[j][i]->Set_Idx( i );
                if( sorted[j][i]->Id() > object_ids[id] ) {
                    sorted[j][i]->Set_Id( sorted[j][i]->Id()-1 );
                }
            }
            sorted[j].Decrement_Length();
            sorted[j].Decrement_Length();
        }
    }
    
    // Delete from the overlapping pairs list
    Deactivate( id );

    // Decrement the number of pairs in the scene
    total_pairs -= free_obj->Num_Pairs();

    // Compress all the object stuff and renumber internal oids
    for( i = object_ids[id]+1; i < objects.Length(); i++ ) {
        objects[i-1] = objects[i];
        user_object_ids[i-1] = user_object_ids[i];
        object_ids[user_object_ids[i]] = i-1;

        // Find the pair to delete.
        for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
            if( !objects[i]->Pairs()[j].Deleted() ) {
                // Renumber the pair
                objects[i]->Pairs()[j].Set_Id1(
                                            objects[i]->Pairs()[j].Id1()-1 );
                if( objects[i]->Pairs()[j].Id0() == object_ids[id] ) {
                    objects[i]->Pairs()[j].Delete();
                    j++;
                    break;
                } else if( objects[i]->Pairs()[j].Id0() > object_ids[id] ) {
                    // Did not find it.  Must have been a pair of fixed objects.
                    break;
                }
            }
        }

        // Renumber the remaining pairs
        for( ; j < objects[i]->Num_Pairs(); j++ ) {
            if( !objects[i]->Pairs()[j].Deleted() ) {
                objects[i]->Pairs()[j].Set_Id0(
                                            objects[i]->Pairs()[j].Id0()-1 );
                objects[i]->Pairs()[j].Set_Id1(
                                            objects[i]->Pairs()[j].Id1()-1 );
            }
        }

        // This sets the box node ids
        objects[i]->Set_Id( i-1 );
    }
    objects.Decrement_Length();
    user_object_ids.Decrement_Length();

    // Put the index on the reuse list
    free_oids.Add_Grow( id, 10 );

    // Set the index to invalid
    object_ids[id] = -1;

    // Free up the memory
    delete free_obj;
}

///////////////////////////////////////////////////////////////////////////////
// Object Transformation methods
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Scene::Set_Object_Transformation( int id, const SWIFT_Real* R,
                                                     const SWIFT_Real* T )
{
#ifdef SWIFT_DEBUG
    // Check the validity of the id
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: Object id out of range given to "
             << "Set_Object_Transformation(" << id << ")" << endl;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: Object given to Set_Object_Transformation("
             << id << "): already deleted" << endl;
    }
#endif
    if( bp ) {
        objects[object_ids[id]]->Set_Transformation( R, T );
        if( !gs ) {
            Sort_Local( object_ids[id] );
        }
    } else {
        objects[object_ids[id]]->Set_Transformation_No_Boxes( R, T );
    }
}

void SWIFT_Scene::Set_Object_Transformation( int id, const SWIFT_Real* R )
{
#ifdef SWIFT_DEBUG
    // Check the validity of the id
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: Object id out of range given to "
             << "Set_Object_Transformation(" << id << ")" << endl;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: Object given to Set_Object_Transformation("
             << id << "): already deleted" << endl;
    }
#endif
    if( bp ) {
        objects[object_ids[id]]->Set_Transformation( R );
        if( !gs ) {
            Sort_Local( object_ids[id] );
        }
    } else {
        objects[object_ids[id]]->Set_Transformation_No_Boxes( R );
    }
}

void SWIFT_Scene::Set_All_Object_Transformations( const SWIFT_Real* R,
                                                  const SWIFT_Real* T )
{
    int i;
    const SWIFT_Real* Rp = R;
    const SWIFT_Real* Tp = T;

    if( bp ) {
        for( i = 0; i < objects.Length(); i++ ) {
            if( !objects[i]->Fixed() ) {
                objects[i]->Set_Transformation( Rp, Tp );
                Rp += 9; Tp += 3;
            }
        }
        Sort_Global();
    } else {
        for( i = 0; i < objects.Length(); i++ ) {
            if( !objects[i]->Fixed() ) {
                objects[i]->Set_Transformation_No_Boxes( Rp, Tp );
                Rp += 9; Tp += 3;
            }
        }
    }
}

void SWIFT_Scene::Set_All_Object_Transformations( const SWIFT_Real* R )
{
    int i;
    const SWIFT_Real* Rp = R;

    if( bp ) {
        for( i = 0; i < objects.Length(); i++ ) {
            if( !objects[i]->Fixed() ) {
                objects[i]->Set_Transformation( Rp );
                Rp += 12;
            }
        }
        Sort_Global();
    } else {
        for( i = 0; i < objects.Length(); i++ ) {
            if( !objects[i]->Fixed() ) {
                objects[i]->Set_Transformation_No_Boxes( Rp );
                Rp += 12;
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Pair Activation methods
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Scene::Activate( int id1, int id2 )
{
#ifdef SWIFT_DEBUG
    if( id1 < 0 || id1 >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Activate(" << id1
             << "," << id2 << "): " << id1 << endl;
        return;
    } else if( object_ids[id1] == -1 ) {
        cerr << "Error: deleted object id given to Activate(" << id1
             << "," << id2 << "): " << id1 << endl;
        return;
    }
    if( id2 < 0 || id2 >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Activate(" << id1
             << "," << id2 << "): " << id2 << endl;
        return;
    } else if( object_ids[id2] == -1 ) {
        cerr << "Error: deleted object id given to Activate(" << id1
             << "," << id2 << "): " << id2 << endl;
        return;
    }
    if( id1 == id2 ) {
        cerr << "Warning: object ids equal given to Activate(" << id1
             << "," << id2 << "): Has no effect" << endl;
        return;
    }
#endif

    int j;

    id1 = object_ids[id1];
    id2 = object_ids[id2];

    if( !objects[id1]->Fixed() || !objects[id2]->Fixed() ) {
        if( id1 > id2 ) {
            for( j = 0; objects[id1]->Pairs()[j].Id0() < id2; j++ );

            for( ; j < objects[id1]->Num_Pairs() &&
                   //objects[id1]->Pairs()[j].Id0() < objects[id2+1]->Id(); j++
                   objects[id1]->Pairs()[j].Id0() <= id2; j++
            ) {
                if( !objects[id1]->Pairs()[j].Deleted() &&
                    objects[id1]->Pairs()[j].Inactive() &&
                    objects[id1]->Pairs()[j].Overlapping()
                ) {
                    // Add it to the overlapping list.
                    objects[id1]->Pairs()[j].Set_Next( overlapping_pairs );
                    objects[id1]->Pairs()[j].Set_Prev( NULL );
                    if( overlapping_pairs != NULL ) {
                        overlapping_pairs->Set_Prev( objects[id1]->Pairs()(j) );
                    }
                    overlapping_pairs = objects[id1]->Pairs()(j);
                }
                objects[id1]->Pairs()[j].Set_Active();
            }
        } else if( id1 < id2 ) {
            for( j = 0; objects[id2]->Pairs()[j].Id0() < id1; j++ );

            for( ; j < objects[id2]->Num_Pairs() &&
                   //objects[id2]->Pairs()[j].Id0() < objects[id1+1]->Id(); j++
                   objects[id2]->Pairs()[j].Id0() <= id1; j++
            ) {
                if( !objects[id2]->Pairs()[j].Deleted() &&
                    objects[id2]->Pairs()[j].Inactive() &&
                    objects[id2]->Pairs()[j].Overlapping()
                ) {
                    // Add it to the overlapping list.
                    objects[id2]->Pairs()[j].Set_Next( overlapping_pairs );
                    objects[id2]->Pairs()[j].Set_Prev( NULL );
                    if( overlapping_pairs != NULL ) {
                        overlapping_pairs->Set_Prev( objects[id2]->Pairs()(j) );
                    }
                    overlapping_pairs = objects[id2]->Pairs()(j);
                }
                objects[id2]->Pairs()[j].Set_Active();
            }
        }
    }
}

void SWIFT_Scene::Activate( int id )
{
#ifdef SWIFT_DEBUG
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Activate(" << id << ")"
             << endl;
        return;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: deleted object id given to Activate(" << id << ")"
             << endl;
        return;
    }
#endif

    int j, k;

    id = object_ids[id];

    // Take care of the object's pairs
    for( j = 0; j < objects[id]->Num_Pairs(); j++ ) {
        if( !objects[id]->Pairs()[j].Deleted() &&
            objects[id]->Pairs()[j].Inactive() &&
            objects[id]->Pairs()[j].Overlapping()
        ) {
            // Add it to the overlapping list.
            objects[id]->Pairs()[j].Set_Next( overlapping_pairs );
            objects[id]->Pairs()[j].Set_Prev( NULL );
            if( overlapping_pairs != NULL ) {
                overlapping_pairs->Set_Prev( objects[id]->Pairs()(j) );
            }
            overlapping_pairs = objects[id]->Pairs()(j);
        }
        objects[id]->Pairs()[j].Set_Active();
    }

    // Take care of the pairs of all subsequent objects
    for( k = id+1; k < objects.Length(); k++ ) {
        if( !objects[id]->Fixed() || !objects[k]->Fixed() ) {
            for( j = 0; objects[k]->Pairs()[j].Id0() < id; j++ );

            for( ; j < objects[k]->Num_Pairs() &&
                   //objects[k]->Pairs()[j].Id0() < objects[i+1]->Id(); j++
                   objects[k]->Pairs()[j].Id0() <= id; j++
            ) {
                if( !objects[k]->Pairs()[j].Deleted() &&
                    objects[k]->Pairs()[j].Inactive() &&
                    objects[k]->Pairs()[j].Overlapping()
                ) {
                    // Add it to the overlapping list.
                    objects[k]->Pairs()[j].Set_Next( overlapping_pairs );
                    objects[k]->Pairs()[j].Set_Prev( NULL );
                    if( overlapping_pairs != NULL ) {
                        overlapping_pairs->Set_Prev( objects[k]->Pairs()(j) );
                    }
                    overlapping_pairs = objects[k]->Pairs()(j);
                }
                objects[k]->Pairs()[j].Set_Active();
            }
        }
    }
}

void SWIFT_Scene::Activate( )
{
    int i, j;

    // Start off with an empty overlap list
    overlapping_pairs = NULL;

    for( i = 0; i < objects.Length(); i++ ) {
        for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
            if( !objects[i]->Pairs()[j].Deleted() ) {
                objects[i]->Pairs()[j].Set_Active();
                if( objects[i]->Pairs()[j].Overlapping() ) {
                    // Add it to the overlapping list.
                    objects[i]->Pairs()[j].Set_Next( overlapping_pairs );
                    objects[i]->Pairs()[j].Set_Prev( NULL );
                    if( overlapping_pairs != NULL ) {
                        overlapping_pairs->Set_Prev( objects[i]->Pairs()(j) );
                    }
                    overlapping_pairs = objects[i]->Pairs()(j);
                }
            }
        }
    }
}

void SWIFT_Scene::Deactivate( int id1, int id2 )
{
#ifdef SWIFT_DEBUG
    if( id1 < 0 || id1 >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Deactivate(" << id1
             << "," << id2 << "): " << id1 << endl;
        return;
    } else if( object_ids[id1] == -1 ) {
        cerr << "Error: deleted object id given to Deactivate(" << id1
             << "," << id2 << "): " << id1 << endl;
        return;
    }
    if( id2 < 0 || id2 >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Deactivate(" << id1
             << "," << id2 << "): " << id2 << endl;
        return;
    } else if( object_ids[id2] == -1 ) {
        cerr << "Error: deleted object id given to Deactivate(" << id1
             << "," << id2 << "): " << id2 << endl;
        return;
    }
    if( id1 == id2 ) {
        cerr << "Warning: object ids equal given to Deactivate(" << id1
             << "," << id2 << "): Has no effect" << endl;
        return;
    }
#endif

    int j;

    id1 = object_ids[id1];
    id2 = object_ids[id2];

    if( !objects[id1]->Fixed() || !objects[id2]->Fixed() ) {
        if( id1 > id2 ) {
            for( j = 0; objects[id1]->Pairs()[j].Id0() < id2; j++ );

            for( ; j < objects[id1]->Num_Pairs() &&
                   //objects[id1]->Pairs()[j].Id0() < objects[id2+1]->Id(); j++
                   objects[id1]->Pairs()[j].Id0() <= id2; j++
            ) {
                objects[id1]->Pairs()[j].Set_Inactive();
            }
        } else if( id1 < id2 ) {
            for( j = 0; objects[id2]->Pairs()[j].Id0() < id1; j++ );

            for( ; j < objects[id2]->Num_Pairs() &&
                   //objects[id2]->Pairs()[j].Id0() < objects[id1+1]->Id(); j++
                   objects[id2]->Pairs()[j].Id0() <= id1; j++
            ) {
                objects[id2]->Pairs()[j].Set_Inactive();
            }
        }
    }

    // Remove pairs from the overlapping list
    while( overlapping_pairs != NULL &&
           ( (overlapping_pairs->Id0() == id1 &&
              overlapping_pairs->Id1() == id2) ||
             (overlapping_pairs->Id0() == id2 &&
              overlapping_pairs->Id1() == id1) )
    ) {
        overlapping_pairs = overlapping_pairs->Next();
    }

    if( overlapping_pairs != NULL ) {
        SWIFT_Pair* pair = overlapping_pairs->Next();
        overlapping_pairs->Set_Prev( NULL );
        while( pair != NULL ) {
            if( (pair->Id0() == id1 && pair->Id1() == id2) ||
                (pair->Id0() == id2 && pair->Id1() == id1)
            ) {
                pair->Prev()->Set_Next( pair->Next() );
                if( pair->Next() != NULL ) {
                    pair->Next()->Set_Prev( pair->Prev() );
                }
            }
            pair = pair->Next();
        }
    }
}

void SWIFT_Scene::Deactivate( int id )
{
#ifdef SWIFT_DEBUG
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Deactivate(" << id
             << ")" << endl;
        return;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: deleted object id given to Deactivate(" << id << ")"
             << endl;
        return;
    }
#endif

    int j, k;

    id = object_ids[id];

    // Take care of the object's pairs
    for( j = 0; j < objects[id]->Num_Pairs(); j++ ) {
        objects[id]->Pairs()[j].Set_Inactive();
    }

    // Take care of the pairs of all subsequent objects
    for( k = id+1; k < objects.Length(); k++ ) {
        if( !objects[id]->Fixed() || !objects[k]->Fixed() ) {
            for( j = 0; objects[k]->Pairs()[j].Id0() < id; j++ );

            for( ; j < objects[k]->Num_Pairs() &&
                   //objects[k]->Pairs()[j].Id0() < objects[id+1]->Id(); j++
                   objects[k]->Pairs()[j].Id0() <= id; j++
            ) {
                objects[k]->Pairs()[j].Set_Inactive();
            }
        }
    }

    // Remove pairs from the overlapping list
    while( overlapping_pairs != NULL &&
           (overlapping_pairs->Id0() == id || overlapping_pairs->Id1() == id)
    ) {
        overlapping_pairs = overlapping_pairs->Next();
    }

    if( overlapping_pairs != NULL ) {
        SWIFT_Pair* pair = overlapping_pairs->Next();
        overlapping_pairs->Set_Prev( NULL );
        while( pair != NULL ) {
            if( pair->Id0() == id || pair->Id1() == id ) {
                pair->Prev()->Set_Next( pair->Next() );
                if( pair->Next() != NULL ) {
                    pair->Next()->Set_Prev( pair->Prev() );
                }
            }
            pair = pair->Next();
        }
    }
}

void SWIFT_Scene::Deactivate( )
{
    int i, j;

    for( i = 0; i < objects.Length(); i++ ) {
        for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
            objects[i]->Pairs()[j].Set_Inactive();
        }
    }

    // No pairs are overlapping
    overlapping_pairs = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Query methods
///////////////////////////////////////////////////////////////////////////////

void SWIFT_Scene::Get_Center_Of_Mass( int id, SWIFT_Real& x,
                                      SWIFT_Real& y, SWIFT_Real& z )
{
#ifdef SWIFT_DEBUG
    if( id < 0 || id >= object_ids.Length() ) {
        cerr << "Error: out of range object id given to Deactivate(" << id
             << ")" << endl;
        return;
    } else if( object_ids[id] == -1 ) {
        cerr << "Error: deleted object id given to Deactivate(" << id << ")"
             << endl;
        return;
    }
#endif
    objects[object_ids[id]]->Mesh()->Center_Of_Mass().Get_Value( x, y, z );
}

bool SWIFT_Scene::Query_Intersection(
                                bool early_exit, int& num_pairs, int** oids )
{
    int i, j, k;
    int o1, o2;

    ois.Ensure_Length( total_pairs<<1 );
    *oids = ois.Data();

    num_pairs = 0;


    if( bp ) {
        if( gs ) {
            // Do global bounding box sort
            Sort_Global();
        }

        SWIFT_Pair* pair = overlapping_pairs;

        k = 0;
        while( pair != NULL ) {
            o1 = pair->Id0();
            o2 = pair->Id1();
            if( pair->Tolerance( objects[o1], objects[o2],
                                 0.0 )
            ) {
                if( early_exit ) {
                    num_pairs = 0;
                    return true;
                }
                ois[k] = user_object_ids[o1];
                ois[k+1] = user_object_ids[o2];
                k += 2;
            }
            pair = pair->Next();
        }
    } else {
        // Do an all pairs test
        for( i = 1, k = 0; i < objects.Length(); i++ ) {
            // Objects are compressed on deletion so this object valid
            for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
                if( objects[i]->Pairs()[j].Inactive() ||
                    objects[i]->Pairs()[j].Deleted()
                ) {
                    continue;
                }
                o1 = objects[i]->Pairs()[j].Id0();
                o2 = objects[i]->Pairs()[j].Id1();
                if( objects[i]->Pairs()[j].Tolerance( objects[o1], objects[o2],
                                                      0.0 )
                ) {
                    if( early_exit ) {
                        num_pairs = 0;
                        return true;
                    }
                    ois[k] = user_object_ids[o1];
                    ois[k+1] = user_object_ids[o2];
                    k += 2;
                }
            }
        }
    }


    num_pairs = k>>1;
    return num_pairs != 0;
}


bool SWIFT_Scene::Query_Tolerance_Verification( bool early_exit,
                            SWIFT_Real tolerance, int& num_pairs, int** oids )
{
    int i, j, k;
    int o1, o2;

    ois.Ensure_Length( total_pairs<<1 );
    *oids = ois.Data();

    num_pairs = 0;

    if( bp ) {
        if( gs ) {
            // Do global bounding box sort
            Sort_Global();
        }

        SWIFT_Pair* pair = overlapping_pairs;

        k = 0;
        while( pair != NULL ) {
            o1 = pair->Id0();
            o2 = pair->Id1();
            if( pair->Tolerance( objects[o1], objects[o2], tolerance ) ) {
                if( early_exit ) {
                    num_pairs = 0;
                    return true;
                }
                ois[k] = user_object_ids[o1];
                ois[k+1] = user_object_ids[o2];
                k += 2;
            }
            pair = pair->Next();
        }
    } else {
        // Do an all pairs test
        for( i = 1, k = 0; i < objects.Length(); i++ ) {
            // Objects are compressed on deletion so this object valid
            for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
                if( objects[i]->Pairs()[j].Inactive() ||
                    objects[i]->Pairs()[j].Deleted()
                ) {
                    continue;
                }
                o1 = objects[i]->Pairs()[j].Id0();
                o2 = objects[i]->Pairs()[j].Id1();
                if( objects[i]->Pairs()[j].Tolerance( objects[o1], objects[o2],
                                                      tolerance )
                ) {
                    if( early_exit ) {
                        num_pairs = 0;
                        return true;
                    }
                    ois[k] = user_object_ids[o1];
                    ois[k+1] = user_object_ids[o2];
                    k += 2;
                }
            }
        }
    }

    num_pairs = k>>1;
    return num_pairs != 0;
}


bool SWIFT_Scene::Query_Approximate_Distance(
                    bool early_exit, SWIFT_Real distance_tolerance,
                    SWIFT_Real abs_error, SWIFT_Real rel_error, int& num_pairs,
                    int** oids, SWIFT_Real** distances )
{
    int i, j, k;
    int o1, o2;
    SWIFT_Real dist;
    bool intersection = false;

    ois.Ensure_Length( total_pairs<<1 );
    *oids = ois.Data();

    num_pairs = 0;

    distance_tolerance = distance_tolerance < 0.0 ? 0.0 : distance_tolerance;
    abs_error = abs_error < 0.0 ? 0.0 : abs_error;
    rel_error = rel_error < 0.0 ? 0.0 : rel_error;

    if( bp ) {
        if( gs ) {
            // Do global bounding box sort
            Sort_Global();
        }

        SWIFT_Pair* pair = overlapping_pairs;

        j = 0; k = 0;
        while( pair != NULL ) {
            o1 = pair->Id0();
            o2 = pair->Id1();
            if( pair->Distance( objects[o1], objects[o2],
                                distance_tolerance, abs_error, rel_error, dist )
            ) {
                // There is intersection
                intersection = true;
                if( early_exit ) {
                    // Force the number of reported pairs to be 0
                    k = 0;
                    break;
                }
            }

            if( dist <= distance_tolerance ) {
                // Process all the pairs that met the tolerance.
                // This works even if they are intersecting
                ois[k] = user_object_ids[o1];
                ois[k+1] = user_object_ids[o2];
                ds.Add_Grow( dist, REPORTING_LIST_GROW_SIZE );
                k += 2;
            }

            pair = pair->Next();
        }
    } else {
        // Do an all pairs test
        for( i = 1, k = 0; i < objects.Length(); i++ ) {
            // Objects are compressed on deletion so this object valid
            for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
                if( objects[i]->Pairs()[j].Inactive() ||
                    objects[i]->Pairs()[j].Deleted()
                ) {
                    continue;
                }
                o1 = objects[i]->Pairs()[j].Id0();
                o2 = objects[i]->Pairs()[j].Id1();
                if( objects[i]->Pairs()[j].Distance( objects[o1], objects[o2],
                            distance_tolerance, abs_error, rel_error, dist )
                ) {
                    // There is intersection
                    intersection = true;
                    if( early_exit ) {
                        // Force the number of reported pairs to be 0
                        k = 0;
                        break;
                    }
                }

                if( dist <= distance_tolerance ) {
                    // Process all the pairs that met the tolerance.
                    // This works even if they are intersecting
                    ois[k] = user_object_ids[o1];
                    ois[k+1] = user_object_ids[o2];
                    ds.Add_Grow( dist, REPORTING_LIST_GROW_SIZE );
                    k += 2;
                }
            }
            if( intersection && early_exit ) {
                break;
            }
        }
    }

    *distances = ds.Data();
    ds.Set_Length( 0 );

    num_pairs = k>>1;
    return intersection;
}


bool SWIFT_Scene::Query_Exact_Distance(
                        bool early_exit, SWIFT_Real tolerance, int& num_pairs,
                        int** oids, SWIFT_Real** distances )
{
    int i, j, k;
    int o1, o2;
    SWIFT_Real dist;
    bool intersection = false;

    ois.Ensure_Length( total_pairs<<1 );
    *oids = ois.Data();

    num_pairs = 0;

    tolerance = tolerance < 0.0 ? 0.0 : tolerance;

    if( bp ) {
        if( gs ) {
            // Do global bounding box sort
            Sort_Global();
        }

        SWIFT_Pair* pair = overlapping_pairs;

        j = 0; k = 0;
        while( pair != NULL ) {
            o1 = pair->Id0();
            o2 = pair->Id1();
            if( pair->Distance( objects[o1], objects[o2],
                                tolerance,
                                0.0, 0.0,
                                dist )
            ) {
                // There is intersection
                intersection = true;
                if( early_exit ) {
                    // Force the number of reported pairs to be 0
                    k = 0;
                    break;
                }
            }

            if( dist <= tolerance ) {
                // Process all the pairs that met the tolerance.
                // This works even if they are intersecting
                ois[k] = user_object_ids[o1];
                ois[k+1] = user_object_ids[o2];
                ds.Add_Grow( dist, REPORTING_LIST_GROW_SIZE );
                k += 2;
            }

            pair = pair->Next();
        }
    } else {
        // Do an all pairs test
        for( i = 1, k = 0; i < objects.Length(); i++ ) {
            // Objects are compressed on deletion so this object valid
            for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
                if( objects[i]->Pairs()[j].Inactive() ||
                    objects[i]->Pairs()[j].Deleted()
                ) {
                    continue;
                }
                o1 = objects[i]->Pairs()[j].Id0();
                o2 = objects[i]->Pairs()[j].Id1();
                if( objects[i]->Pairs()[j].Distance( objects[o1], objects[o2],
                        tolerance,
                        0.0, 0.0,
                        dist )
                ) {
                    // There is intersection
                    intersection = true;
                    if( early_exit ) {
                        // Force the number of reported pairs to be 0
                        k = 0;
                        break;
                    }
                }

                if( dist <= tolerance ) {
                    // Process all the pairs that met the tolerance.
                    // This works even if they are intersecting
                    ois[k] = user_object_ids[o1];
                    ois[k+1] = user_object_ids[o2];
                    ds.Add_Grow( dist, REPORTING_LIST_GROW_SIZE );
                    k += 2;
                }
            }
            if( intersection && early_exit ) {
                break;
            }
        }
    }

    *distances = ds.Data();
    ds.Set_Length( 0 );

    num_pairs = k>>1;
    return intersection;
}


bool SWIFT_Scene::Query_Contact_Determination(
        bool early_exit, SWIFT_Real tolerance, int& num_pairs, int** oids,
        int** num_contacts, SWIFT_Real** distances, SWIFT_Real** nearest_pts,
        SWIFT_Real** normals, int** feature_types, int** feature_ids )
{
    int i, j, k;
    int o1, o2;
    int num_cs;
    SWIFT_Real dist;
    bool intersection = false;
    bool this_intersection;

    ois.Ensure_Length( total_pairs<<1 );
    *oids = ois.Data();

    ncs.Ensure_Length( total_pairs );
    *num_contacts = ncs.Data();

    num_pairs = 0;

    tolerance = tolerance < 0.0 ? 0.0 : tolerance;

    if( bp ) {
        if( gs ) {
            // Do global bounding box sort
            Sort_Global();
        }

        SWIFT_Pair* pair = overlapping_pairs;

        k = 0;
        while( pair != NULL ) {
            this_intersection = false;
            o1 = pair->Id0();
            o2 = pair->Id1();
            if( pair->Contacts( objects[o1], objects[o2],
                                tolerance, dist, num_cs )
            ) {
                // There is intersection
                intersection = true;
                this_intersection = true;
                if( early_exit ) {
                    // Force the number of reported pairs to be 0
                    k = 0;
                    break;
                }
            }

            if( dist <= tolerance ) {
                // Process all the contacts that met the tolerance.
                // This works even if they are intersecting
                ois[k] = user_object_ids[o1];
                ois[k+1] = user_object_ids[o2];

                if( this_intersection ) {
                    ncs[k>>1] = -1;
                    if( distances != NULL ) {
                        ds.Fit_Grow( 1, REPORTING_LIST_GROW_SIZE );
                        ds.Add( dist );
                    }
                    if( nearest_pts != NULL ) {
                        nps.Fit_Grow( 6, REPORTING_LIST_GROW_SIZE*6 );
                        nps.Set_Length( nps.Length()+6 );
                    }
                    if( normals != NULL ) {
                        cns.Fit_Grow( 3, REPORTING_LIST_GROW_SIZE*3 );
                        cns.Set_Length( cns.Length()+3 );
                    }
                    if( feature_types != NULL && feature_ids != NULL ) {
                        fts.Fit_Grow( 2, REPORTING_LIST_GROW_SIZE<<1 );
                        fts.Set_Length( fts.Length()+2 );
                        fis.Fit_Grow( 4, REPORTING_LIST_GROW_SIZE<<2 );
                        fis.Set_Length( fis.Length()+4 );
                    }
                } else {
                    ncs[k>>1] = num_cs;
                    if( distances != NULL ) {
                        ds.Fit_Grow( num_cs, REPORTING_LIST_GROW_SIZE );
                        pair->Distances( ds );
                    }
                    if( nearest_pts != NULL ) {
                        nps.Fit_Grow( num_cs*6, REPORTING_LIST_GROW_SIZE*6 );
                        pair->Contact_Points( nps );
                    }
                    if( normals != NULL ) {
                        cns.Fit_Grow( num_cs*3, REPORTING_LIST_GROW_SIZE*3 );
                        pair->Contact_Normals( cns );
                    }
                    if( feature_types != NULL && feature_ids != NULL ) {
                        fts.Fit_Grow( num_cs<<1, REPORTING_LIST_GROW_SIZE<<1 );
                        fis.Fit_Grow( num_cs<<2, REPORTING_LIST_GROW_SIZE<<2 );
                        pair->Contact_Features( fts, fis );
                    }
                }
                k += 2;
            }

            pair = pair->Next();
        }
    } else {
        // Do an all pairs test
        for( i = 1, k = 0; i < objects.Length(); i++ ) {
            // Objects are compressed on deletion so this object valid
            for( j = 0; j < objects[i]->Num_Pairs(); j++ ) {
                if( objects[i]->Pairs()[j].Inactive() ||
                    objects[i]->Pairs()[j].Deleted()
                ) {
                    continue;
                }
                this_intersection = false;
                o1 = objects[i]->Pairs()[j].Id0();
                o2 = objects[i]->Pairs()[j].Id1();
                if( objects[i]->Pairs()[j].Contacts( objects[o1], objects[o2],
                                                     tolerance, dist, num_cs )
                ) {
                    // There is intersection
                    intersection = true;
                    this_intersection = true;
                    if( early_exit ) {
                        // Force the number of reported pairs to be 0
                        k = 0;
                        break;
                    }
                }

                if( dist <= tolerance ) { 
                    // Process all the contacts that met the tolerance.
                    // This works even if they are intersecting
                    ois[k] = user_object_ids[o1];
                    ois[k+1] = user_object_ids[o2];

                    if( this_intersection ) {
                        ncs[k>>1] = -1;
                        if( distances != NULL ) {
                            ds.Fit_Grow( 1, REPORTING_LIST_GROW_SIZE );
                            ds.Add( dist );
                        }
                        if( nearest_pts != NULL ) {
                            nps.Fit_Grow( 6, REPORTING_LIST_GROW_SIZE*6 );
                            nps.Set_Length( nps.Length()+6 );
                        }
                        if( normals != NULL ) {
                            cns.Fit_Grow( 3, REPORTING_LIST_GROW_SIZE*3 );
                            cns.Set_Length( cns.Length()+3 );
                        }
                        if( feature_types != NULL && feature_ids != NULL ) {
                            fts.Fit_Grow( 2, REPORTING_LIST_GROW_SIZE<<1 );
                            fts.Set_Length( fts.Length()+2 );
                            fis.Fit_Grow( 4, REPORTING_LIST_GROW_SIZE<<2 );
                            fis.Set_Length( fis.Length()+4 );
                        }
                    } else {
                        ncs[k>>1] = num_cs;
                        if( distances != NULL ) {
                            ds.Fit_Grow( num_cs, REPORTING_LIST_GROW_SIZE );
                            objects[i]->Pairs()[j].Distances( ds );
                        }
                        if( nearest_pts != NULL ) {
                            nps.Fit_Grow( num_cs*6,
                                          REPORTING_LIST_GROW_SIZE*6 );
                            objects[i]->Pairs()[j].Contact_Points( nps );
                        }
                        if( normals != NULL ) {
                            cns.Fit_Grow( num_cs*3,
                                          REPORTING_LIST_GROW_SIZE*3 );
                            objects[i]->Pairs()[j].Contact_Normals( cns );
                        }
                        if( feature_types != NULL && feature_ids != NULL ) {
                            fts.Fit_Grow( num_cs<<1,
                                          REPORTING_LIST_GROW_SIZE<<1 );
                            fis.Fit_Grow( num_cs<<2,
                                          REPORTING_LIST_GROW_SIZE<<2 );
                            objects[i]->Pairs()[j].Contact_Features( fts, fis );
                        }
                    }
                    k += 2;
                }
            }
            if( intersection && early_exit ) {
                break;
            }
        }
    }

    if( distances != NULL ) {
        *distances = ds.Data();
        ds.Set_Length( 0 );
    }
    if( nearest_pts != NULL ) {
        *nearest_pts = nps.Data();
        nps.Set_Length( 0 );
    }
    if( normals != NULL ) {
        *normals = cns.Data();
        cns.Set_Length( 0 );
    }
    if( feature_types != NULL && feature_ids != NULL ) {
        *feature_types = fts.Data();
        *feature_ids = fis.Data();
        fts.Set_Length( 0 );
        fis.Set_Length( 0 );
    }

    num_pairs = k>>1;
    return intersection;
}


///////////////////////////////////////////////////////////////////////////////
// Plug-In Registration methods
///////////////////////////////////////////////////////////////////////////////

bool SWIFT_Scene::Register_File_Reader( const char* magic_number,
                                        SWIFT_File_Reader* file_reader ) const
{
    return file_dispatcher.Register( magic_number, file_reader );
}

//////////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////////

void SWIFT_Scene::Initialize_Object_In_Scene( SWIFT_Object* cobj, int& id )
{
    int i, j, k, l;

    cobj->Set_Id( objects.Length() );

    if( free_oids.Length() == 0 ) {
        // Need a new id
        id = objects.Length();
        if( objects.Max_Length() == objects.Length() ) {
            // Object list is full.  Grow it.
            objects.Grow( OBJECT_SEGMENT_SIZE );
            object_ids.Grow( OBJECT_SEGMENT_SIZE );
            user_object_ids.Grow( OBJECT_SEGMENT_SIZE );
            // Only one box per object
            if( bp && sorted[0].Length() == sorted[0].Max_Length() ) {
                sorted[0].Grow( OBJECT_SEGMENT_SIZE<<1 );
                sorted[1].Grow( OBJECT_SEGMENT_SIZE<<1 );
                sorted[2].Grow( OBJECT_SEGMENT_SIZE<<1 );
            }
        }
        object_ids.Increment_Length();
    } else {
        // Reuse one of the free ids
        id = free_oids.Last();
        free_oids.Decrement_Length();
    }
    object_ids[id] = objects.Length();
    objects.Increment_Length();
    objects.Last() = cobj;
    user_object_ids.Increment_Length();
    user_object_ids.Last() = id;

    // Count up the pairs
    for( i = 0, j = 0; i < objects.Length()-1; i++ ) {
        if( !cobj->Fixed() || !objects[i]->Fixed() ) {
            j++;
        }
    }
    total_pairs += j;
    cobj->Pairs().Create( j );

    // Set up the pairs for this object
    for( i = 0, l = 0; i < objects.Length()-1; i++ ) {
        if( !cobj->Fixed() || !objects[i]->Fixed() ) {
#ifdef SWIFT_FRONT_TRACKING
            cobj->Pairs()[l].Set_Roots( objects[i]->Mesh()->Root(),
                                        cobj->Mesh()->Root() );
#endif
            cobj->Pairs()[l].Set_Id0( i );
            cobj->Pairs()[l++].Set_Id1( objects.Length()-1 );
        }
    }

    if( bp ) {
        // Set the box nodes and initialize them in the sorted list
        j = sorted[0].Length();
        k = j+1;
        i = 0;
            sorted[0].Increment_Length();
            sorted[0].Increment_Length();
            sorted[1].Increment_Length();
            sorted[1].Increment_Length();
            sorted[2].Increment_Length();
            sorted[2].Increment_Length();
            cobj->Get_Box_Nodes( i, &(sorted[0][j]), &(sorted[0][k]),
                                    &(sorted[1][j]), &(sorted[1][k]),
                                    &(sorted[2][j]), &(sorted[2][k]) );
            sorted[0][j]->Set_Idx( j );
            sorted[0][k]->Set_Idx( k );
            sorted[1][j]->Set_Idx( j );
            sorted[1][k]->Set_Idx( k );
            sorted[2][j]->Set_Idx( j );
            sorted[2][k]->Set_Idx( k );
        Sort_Local( objects.Length()-1 );
    }
}

inline void SWIFT_Scene::Update_Overlap( int axis, int id1, int id2 )
{
    int j;
    SWIFT_Pair* pair;
    bool poverlapping;
    const int oid1 = id1;
    const int oid2 = id2;

    // Check to see if a pair exists for the two ids
    if( oid1 == oid2 || (objects[oid1]->Fixed() && objects[oid2]->Fixed()) ) {
        return;
    }

    // Find the pair
    if( oid1 > oid2 ) {
        for( j = 0; objects[oid1]->Pairs()[j].Id0() != id2; j++ );

        pair = objects[oid1]->Pairs()( j );
    } else {
        for( j = 0; objects[oid2]->Pairs()[j].Id0() != id1; j++ );

        pair = objects[oid2]->Pairs()( j );
    }

    poverlapping = pair->Active() && pair->Overlapping();

    // Potentially changes the Overlapping() status of the pair
    pair->Toggle_Overlap( axis );

    if( poverlapping ) {
        // Pair is done overlapping. Remove it from the overlapping pairs list.
        if( pair->Next() != NULL ) {
            pair->Next()->Set_Prev( pair->Prev() );
        }
        if( pair->Prev() != NULL ) {
            pair->Prev()->Set_Next( pair->Next() );
        } else {
            overlapping_pairs = pair->Next();
        }
        // Set it uninitialized
        pair->Set_Uninitialized();
    } else if( pair->Active() && pair->Overlapping() ) {
        // The pair is starting to overlap.  Add it to the overlapping list.
        pair->Set_Next( overlapping_pairs );
        pair->Set_Prev( NULL );
        if( overlapping_pairs != NULL ) {
            overlapping_pairs->Set_Prev( pair );
        }
        overlapping_pairs = pair;
    }
}

inline void SWIFT_Scene::Sort_Global( int axis )
{
    int i, j;
    SWIFT_Box_Node* tempc;

    // Do insertion sort on the list.
    for( i = 1; i < sorted[axis].Length(); i++ ) {
        tempc = sorted[axis][i];
        for( j = i; j > 0 && tempc->Value() < sorted[axis][j-1]->Value(); j--
        ) {
            sorted[axis][j] = sorted[axis][j-1];
            sorted[axis][j]->Set_Idx( j );
            if( tempc->Is_Max() != sorted[axis][j]->Is_Max() ) {
                Update_Overlap( axis, tempc->Id(), sorted[axis][j]->Id() );
            }
        }
        sorted[axis][j] = tempc;
        tempc->Set_Idx( j );
    }
}

void SWIFT_Scene::Sort_Global( )
{
    int i;
    for( i = 0; i < 3; i++ ) {
        Sort_Global( i );
    }
}

inline void SWIFT_Scene::Sort_Local( int oid, int axis )
{
    SWIFT_Box_Node* bn;
    int i;

    // Try to move the min to the left
    bn = objects[oid]->Min_Box_Node( axis );
    i = bn->Idx();
    for( ; i != 0 && sorted[axis][i-1]->Value() > bn->Value(); i-- ) {
        sorted[axis][i] = sorted[axis][i-1];
        sorted[axis][i]->Set_Idx( i );
        if( sorted[axis][i]->Is_Max() ) {
            Update_Overlap( axis, bn->Id(), sorted[axis][i]->Id() );
        }
    }

    sorted[axis][i] = bn;
    bn->Set_Idx( i );

    // Try to move the max to the right
    bn = objects[oid]->Max_Box_Node( axis );
    i = bn->Idx();
    for( ; i != sorted[axis].Length()-1 &&
           sorted[axis][i+1]->Value() < bn->Value(); i++
    ) {
        sorted[axis][i] = sorted[axis][i+1];
        sorted[axis][i]->Set_Idx( i );
        if( !sorted[axis][i]->Is_Max() ) {
            Update_Overlap( axis, bn->Id(), sorted[axis][i]->Id() );
        }
    }

    sorted[axis][i] = bn;
    bn->Set_Idx( i );

    // Try to move the min to the right
    bn = objects[oid]->Min_Box_Node( axis );
    i = bn->Idx();
    for( ; sorted[axis][i+1]->Value() < bn->Value(); i++ ) {
        sorted[axis][i] = sorted[axis][i+1];
        sorted[axis][i]->Set_Idx( i );
        if( sorted[axis][i]->Is_Max() ) {
            Update_Overlap( axis, bn->Id(), sorted[axis][i]->Id() );
        }
    }

    sorted[axis][i] = bn;
    bn->Set_Idx( i );

    // Try to move the max to the left
    bn = objects[oid]->Max_Box_Node( axis );
    i = bn->Idx();
    for( ; sorted[axis][i-1]->Value() > bn->Value(); i-- ) {
        sorted[axis][i] = sorted[axis][i-1];
        sorted[axis][i]->Set_Idx( i );
        if( !sorted[axis][i]->Is_Max() ) {
            Update_Overlap( axis, bn->Id(), sorted[axis][i]->Id() );
        }
    }

    sorted[axis][i] = bn;
    bn->Set_Idx( i );
}

void SWIFT_Scene::Sort_Local( int oid )
{
    int i;
    for( i = 0; i < 3; i++ ) {
        Sort_Local( oid, i );
    }
}



