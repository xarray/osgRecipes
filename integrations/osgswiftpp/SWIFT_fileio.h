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
// SWIFT_fileio.h
//
// Description:
//      Abstract base class for derivation of new file readers.  File I/O
//  dispatch class with which to register new readers.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_FILEIO_H_
#define _SWIFT_FILEIO_H_

#include <fstream>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_array.h>


//////////////////////////////////////////////////////////////////////////////
// SWIFT_File_Reader
//
// Description:
//      Abstract base class for a file reader.  New file readers can be
//  derived.  For an examples of how to go about creating one of these, see the 
//  SWIFT_TRI_File_Reader below.
//
//  If you create a file reader that is for a standard file type and you think
//  will be of use to others, please let us know and we can add it to the
//  distribution and give you credit.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_File_Reader {
  public:
    SWIFT_File_Reader( ) { }
    virtual ~SWIFT_File_Reader( ) { }

    // When this function is called, the file is opened and the stream pointer
    // is situated at the beginning of the file.  Do NOT close the file when
    // done.  The ifstream parameter is to be used for reading the file.
    //
    // Allocate *vs for the coordinates of the vertices.  Set vn to the number
    // of vertices.  Allocate *fs for the face vertex indices.  If faces have
    // more than three vertices, that is ok.  Just make sure that the *fs
    // array is long enough.  Set fn to be the number of faces.  fv is used if
    // some faces are non-triangular.  If ALL faces are triangular,
    // set *fv = NULL.  Otherwise, allocate *fs to a length equal to the
    // number of faces.  Set each entry equal to the number of vertices in the
    // corresponding face.
    virtual bool Read( ifstream& fin, SWIFT_Real*& vs, int*& fs,
                                      int& vn, int& fn, int*& fv ) = 0;

  private:
};


//////////////////////////////////////////////////////////////////////////////
// SWIFT_File_Read_Dispatcher
//
// Description:
//      Class to dispatch file reads to various file readers depending on the
//  magic number at the beginning of the file.  Also handles opening and
//  closing of the file.  Only one file may be read at a time.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_File_Read_Dispatcher {
  public:
    SWIFT_File_Read_Dispatcher( );

    // This will not delete the file readers that were registered.
    ~SWIFT_File_Read_Dispatcher( );

    // The magic number for the file type must be a single ascii token.
    // The file reader to handle this type of file is given by the abstract
    // base class above.  Note that a reader may be registered more than once
    // for a differnent magic number each time.
    bool Register( const char* magic_number, SWIFT_File_Reader* file_reader );

    // Used by SWIFT to perform a file read.  This function will open the
    // file, read the magic number, dispatch to the correct file reader, and
    // then close the file.  Error messages are printed out if something fails.
    bool Read( const char* filename, SWIFT_Real*& vs, int*& fs,
                                     int& vn, int& fn, int*& fv );

  private:
    ifstream fin;
    SWIFT_Array<char*> magic_nums;
    SWIFT_Array<SWIFT_File_Reader*> readers;
};


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Basic_File_Reader
//
// Description:
//      Basic file reader provided by SWIFT.  TRI files and POLY files are both
// read and are a format created for SWIFT.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Basic_File_Reader : public SWIFT_File_Reader {
  public:
    SWIFT_Basic_File_Reader( ) { }
    ~SWIFT_Basic_File_Reader( ) { }

    void Register_Yourself( SWIFT_File_Read_Dispatcher& disp )
    {
        disp.Register( "TRI", this );
        disp.Register( "POLY", this );
    }

    bool Read( ifstream& fin, SWIFT_Real*& vs, int*& fs,
                              int& vn, int& fn, int*& fv );

  private:
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Obj_File_Reader
//
// Description:
//      Obj file reader provided for SWIFT.  OBJ files are
// read and output in a SWIFT usable format.
// SWIFT_OBJ_File_Reader written by John Lucas
// johnl@vt.edu
// http://www.research.cs.vt.edu/
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Obj_File_Reader : public SWIFT_File_Reader {
  public:
    SWIFT_Obj_File_Reader( ) { }
    ~SWIFT_Obj_File_Reader( ) { }

    void Register_Yourself( SWIFT_File_Read_Dispatcher& disp )
    {
        disp.Register( "#OBJ", this );
		disp.Register( "OBJ", this );
    }

    bool Read( ifstream& fin, SWIFT_Real*& vs, int*& fs,
                              int& vn, int& fn, int*& fv );

  private:
};

typedef enum { FT_CHIER, FT_DECOMP, FT_OTHER } SWIFT_FILE_TYPE;

// Call this before calling any other file io functions
void SWIFT_File_IO_Initialize( );

// This function checks the file type and returns it in ft.
bool SWIFT_File_Type( const char* filename, SWIFT_FILE_TYPE& ft );

// This function reads a convex decomposition file into the parameters.
// Note that the mesh is created directly here.  The mesh is fully computed.
// The hierarchy is not created however.
// The vfs are all computed except for the twin pointers.  This should be
// resolved when the pieces are combined in a hierarchy.  Note that the edge
// directions and lengths should be made consistent across twins as well.
bool SWIFT_File_Read_Decomposition(
                        const char* filename, SWIFT_Tri_Mesh*& mesh,
#ifdef SWIFT_DECOMP
                        bool create_hierarchy,
                        SWIFT_Array<int>& piece_ids,
                        SWIFT_Array< SWIFT_Array<int> >& mfs,
                        SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face> >& vfs,
#endif
                        const SWIFT_Orientation& orient = DEFAULT_ORIENTATION,
                        const SWIFT_Translation& trans = DEFAULT_TRANSLATION,
                        SWIFT_Real scale = DEFAULT_SCALE,
                        SPLIT_TYPE split = DEFAULT_SPLIT_TYPE );

// This function reads a convex hierarchy file into the parameters.
// Note that the mesh is created directly here.  The mesh is fully computed.
bool SWIFT_File_Read_Hierarchy(
                        const char* filename, SWIFT_Tri_Mesh*& mesh,
                        const SWIFT_Orientation& orient = DEFAULT_ORIENTATION,
                        const SWIFT_Translation& trans = DEFAULT_TRANSLATION,
                        SWIFT_Real scale = DEFAULT_SCALE );
#endif


