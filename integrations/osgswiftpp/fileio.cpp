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
// fileio.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include <SWIFT.h>
#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_array.h>
#include <SWIFT_mesh.h>
#include <SWIFT_fileio.h>

bool machine_is_big_endian;

union dbl8 {
  double d;
  unsigned char c[8];
};

union int4 {
  int i;
  unsigned char c[4];
};


///////////////////////////////////////////////////////////////////////////////
// SWIFT_Basic_File_Reader public functions
///////////////////////////////////////////////////////////////////////////////

bool SWIFT_Basic_File_Reader::Read( ifstream& fin, SWIFT_Real*& vs, int*& fs,
                                                   int& vn, int& fn, int*& fv )
{
    int i, j, k;
    char buffer[256];

    fin >> buffer;

    if( !strcmp( buffer, "TRI" ) ) {
        // Triangle file format
        fin >> vn >> fn;
        vs = new SWIFT_Real[vn*3];
        fs = new int[fn*3];
        fv = NULL;

        if (!vs || !fs) {
            cerr << "Error: Array allocation during file reading failed" << endl;
            return false;
        }

        // Read in the vertex coordinates
        for( j = 0; j < vn*3; j += 3 ) {
            fin >> vs[j] >> vs[j+1] >> vs[j+2];
        }

        // Read in the face indices
        for( i = 0, j = 0; i < fn; i++, j += 3 ) {
            fin >> fs[j] >> fs[j+1] >> fs[j+2];
        }
    } else if( !strcmp( buffer, "POLY" ) ) {
        // Polygon file format
        int fs_size;
        fin >> vn >> fn;
        fs_size = ((vn+fn-2)<<1)*3;
        vs = new SWIFT_Real[vn*3];
        fs = new int[fs_size];
        fv = new int[fn];

        if (!vs || !fs || !fv) {
            cerr << "Error: Array allocation during file reading failed" << endl;
            return false;
        }

        // Read in the vertex coordinates
        for( i = 0, j = 0; i < vn; i++, j += 3 ) {
            fin >> vs[j] >> vs[j+1] >> vs[j+2];
        }

        // Read in the face indices
        for( i = 0, j = 0; i < fn; i++ ) {
            fin >> fv[i];
            for( k = 0; k < fv[i]; k++ ) {
                if (j == fs_size) {
                    int l;
                    int *newfs = new int[fs_size*2];
                    if (!newfs) {
                        cerr << "Error: Array allocation during file reading failed" << endl;
                        return false;
                    }
                        
                    // Copy from the old array to the new array
                    for( l = 0; l < fs_size; l++ ) {
                        newfs[l] = fs[l];
                    }

                    fs_size *= 2;
                    delete fs;
                    fs = newfs;
                }
                fin >> fs[j++];
            }
        }
    } else {
        cerr << "Error: Do not recognize file type: \"" << buffer << "\".  "
             << "I was registered incorrectly" << endl;
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// SWIFT_Obj_File_Reader public functions
///////////////////////////////////////////////////////////////////////////////
//SWIFT_OBJ_File_Reader written by John Lucas
//johnl@vt.edu
//http://www.research.cs.vt.edu/3di/
bool SWIFT_Obj_File_Reader::Read( ifstream& fin, SWIFT_Real*& vs, int*& fs,
                                                   int& vn, int& fn, int*& fv )
{
    
	char mypeek;
	int readint;
	int faceverts; //total number of vertex indcies in the face array


	//pass thru once to get the number of verts and faces
	vn = 0; fn = 0; faceverts = 0;
	while (!fin.eof()){

		mypeek = fin.peek();
		if (mypeek == 'v'){
			fin.ignore(1,'\n');
			mypeek = fin.peek();
			if ((mypeek == ' ') || (mypeek == '\t'))//just v only
				vn++;

		} else if (mypeek == 'f'){
			fin.ignore(1,'\n');
			mypeek = fin.peek();
			if ((mypeek == ' ') || (mypeek == '\t')){//just f only
				fn++;
				//ok now count the vertices
				mypeek = fin.peek();
				while ((mypeek == ' ') || (mypeek == '\t')){
					fin >> readint;
					//ignoreVTVN(fin);
				
					//ignore VT and VN
					//possible VT
					mypeek = fin.peek();
					if (!((mypeek == ' ') || (mypeek == '\t') || (mypeek == '\n') || (mypeek == '\r'))){			
						fin.ignore(1,'\n');
						mypeek = fin.peek();
						if (mypeek != '/')
							fin >> readint;
						
						//possible VN
						mypeek = fin.peek();
						if (!((mypeek == ' ') || (mypeek == '\t') || (mypeek == '\n') || (mypeek == '\r'))){
							fin.ignore(1,'\n');
							fin >> readint;
						};//if possible VN
					};//if possible VT

					faceverts++;
					mypeek = fin.peek();
				};//while -counting face verts

			};//if its not a fn or ft line
		};
		fin.ignore(255,'\n');
	};
	

	//ok, allocate the arrays
	vs = new SWIFT_Real[vn*3];
	fs = new int[faceverts];
	fv = new int[fn];

	int vsidx = 0;
	int fsidx = 0;
	int fvidx = 0;
	int facevertidx = 0;
	
	fin.clear();
	fin.seekg(0);//go back to the beginning

	while (!fin.eof()){

		mypeek = fin.peek();
		if (mypeek == 'v'){
			fin.ignore(1,'\n');
			mypeek = fin.peek();
			if ((mypeek == ' ') || (mypeek == '\t')){//just v only
				fin >> vs[vsidx]; 
				fin >> vs[vsidx+1];
				fin >> vs[vsidx+2];
				vsidx += 3;
			};

		} else if (mypeek == 'f'){
			fin.ignore(1,'\n');
			mypeek = fin.peek();
			if ((mypeek == ' ') || (mypeek == '\t')){//just f only
				//ok now count the vertices
				mypeek = fin.peek();
				facevertidx = 0;
				while ((mypeek == ' ') || (mypeek == '\t')){
					fin >> fs[fsidx];
					fs[fsidx] -= 1;  //adjust since vertex indices in obj files start at 1
					//ignoreVTVN(fin);

					//ignore VT and VN
					//possible VT
					mypeek = fin.peek();
					if (!((mypeek == ' ') || (mypeek == '\t') || (mypeek == '\n') || (mypeek == '\r'))){			
						fin.ignore(1,'\n');
						mypeek = fin.peek();
						if (mypeek != '/')
							fin >> readint;
						
						//possible VN
						mypeek = fin.peek();
						if (!((mypeek == ' ') || (mypeek == '\t') || (mypeek == '\n') || (mypeek == '\r'))){
							fin.ignore(1,'\n');
							fin >> readint;
						};//if possible VN
					};//if possible VT

					fsidx++;
					facevertidx++;
					mypeek = fin.peek();
				};//while -counting face verts

				fv[fvidx] = facevertidx;
				fvidx++;
			};//if its not a fn or ft line
		};
		fin.ignore(255,'\n');
	};
    return true;
};


///////////////////////////////////////////////////////////////////////////////
// SWIFT_File_Read_Dispatcher public functions
///////////////////////////////////////////////////////////////////////////////

SWIFT_File_Read_Dispatcher::SWIFT_File_Read_Dispatcher( )
{
    magic_nums.Create( 10 );
    magic_nums.Set_Length( 0 );
    readers.Create( 10 );
    readers.Set_Length( 0 );
}

SWIFT_File_Read_Dispatcher::~SWIFT_File_Read_Dispatcher( )
{
    int i;
    for( i = 0; i < magic_nums.Length(); i++ ) {
        delete magic_nums[i];
    }
}

bool SWIFT_File_Read_Dispatcher::Register( const char* magic_number,
                                           SWIFT_File_Reader* file_reader )
{
    int i;
    const int mn_len = strlen( magic_number );

    // Check that the new magic_num is not a prefix of an existing one and
    // vice-versa
    for( i = 0; i < magic_nums.Length(); i++ ) {
        const int reg_mn_len = strlen( magic_nums[i] );
        if( reg_mn_len != mn_len ) {
            if( !strncmp( magic_nums[i], magic_number, mn_len ) ) {
                // The new magic number is a prefix of registered magic number
                cerr << "Error (Register_File_Reader): Given magic number: "
                     << "\"" << magic_number << "\"" << endl
                     << "      is a prefix of an already registered magic "
                     << "number: \"" << magic_nums[i] << "\"" << endl;
                return false;
            } else if( !strncmp( magic_nums[i], magic_number, reg_mn_len ) ) {
                // The registered magic number is a prefix of new magic number
                cerr << "Error (Register_File_Reader): Already registered magic"
                     << " number: \"" << magic_nums[i] << "\"" << endl
                     << "      is a prefix of the given magic number: "
                     << "\"" << magic_number << "\"" << endl;
                return false;
            }
        } else {
            if( !strcmp( magic_nums[i], magic_number ) ) {
                // Found it.  Already registered.  Replace it.
                cerr << "Warning (Register_File_Reader): Replacing already "
                     << "registered file reader for magic number: \""
                     << magic_number << "\"" << endl;
                readers[i] = file_reader;
                return true;
            }
        }
    }

    // Not found.  Register it.
    char* new_magic_num = new char[mn_len+1];
    strcpy( new_magic_num, magic_number );
    magic_nums.Add_Grow( new_magic_num, 10 );
    readers.Add_Grow( file_reader, 10 );

    return true;
}

bool SWIFT_File_Read_Dispatcher::Read( const char* filename, SWIFT_Real*& vs,
                                       int*& fs, int& vn, int& fn, int*& fv )
{
    int i, j;
    bool match;
    char buffer[256];
    SWIFT_Array<int> cur_match;

    // Try to open the file
    if( filename == NULL ) {
        cerr << "Error: Invalid filename given to read file" << endl;
        return false;
    }

#ifdef WIN32
    fin.open( filename, ios::in | ios::binary );
#else
    fin.open( filename, ios::in );
#endif

    if( !fin.rdbuf()->is_open( ) ) {
        cerr << "Error: file could not be opened for reading \""
             << filename << "\"" << endl;
        return false;
    }

    cur_match.Create( magic_nums.Length() );
    for( i = 0; i <  magic_nums.Length(); i++ ) {
        cur_match[i] = i;
    }

    // Read the magic number
    match = false;
    for( j = 0; !match && !cur_match.Empty(); j++ ) {
        fin >> buffer[j];
        for( i = 0; i < cur_match.Length(); i++ ) {
            if( buffer[j] != magic_nums[cur_match[i]][j] ) {
                cur_match[i] = cur_match.Last();
                cur_match.Decrement_Length();
                i--;
            } else {
                if( (unsigned int)j+1 == strlen(magic_nums[cur_match[i]]) ) {
                    // Found it
                    match = true;
                    break;
                }
            }
        }
    }

    if( !match ) {
        // Didn't find it
        cerr << "Error: file could not be read because no file reader can "
             << "handle the type: \"" << buffer << "\".  Consider writing "
             << "one (see documentation)." << endl;
        return false;
    }

    // Dispatch to the file reader.
    fin.seekg( 0, ios::beg );
    bool result = readers[cur_match[i]]->Read( fin, vs, fs, vn, fn, fv );
    fin.close();

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Static functions
///////////////////////////////////////////////////////////////////////////////
    
static void Swap( char* array, int len, int size )
{
    char temp_swap;
    int i;

    if( size == 4 ) {
        for( i = 0; i < len*size; i += size ) {
            temp_swap = array[i];
            array[i] = array[i+3];
            array[i+3] = temp_swap;
            temp_swap = array[i+1];
            array[i+1] = array[i+2];
            array[i+2] = temp_swap;
        }
    } else if( size == 8 ) {
        for( i = 0; i < len*size; i += size ) {
            temp_swap = array[i];
            array[i] = array[i+7];
            array[i+7] = temp_swap;
            temp_swap = array[i+1];
            array[i+1] = array[i+6];
            array[i+6] = temp_swap;
            temp_swap = array[i+2];
            array[i+2] = array[i+5];
            array[i+5] = temp_swap;
            temp_swap = array[i+3];
            array[i+3] = array[i+4];
            array[i+4] = temp_swap;
        }
    } else {
        cerr << "Error: Do not know how to swap with size = " << size << endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////

// This should be able to be called multiple times (at any time)
void SWIFT_File_IO_Initialize( )
{
    // Do endian determination
    union dbl8 x;

    x.d = 12345.6789;

    // Check for standard big-endian storage
    if( !strncmp((char*)x.c, "\x40\xc8\x1c\xd6\xe6\x31\xf8\xa1", 8) ) {
        machine_is_big_endian = true;
    } else {
        machine_is_big_endian = false;
    }
}

bool SWIFT_File_Type( const char* filename, SWIFT_FILE_TYPE& ft )
{
    char buffer[256];
    ifstream fin;

    // Try to open the file
    if( filename == NULL ) {
        cerr << "Error: Invalid filename given to read file" << endl;
        return false;
    }

#ifdef WIN32
    fin.open( filename, ios::in | ios::binary );
#else
    fin.open( filename, ios::in );
#endif

    if( !fin.rdbuf()->is_open( ) ) {
        cerr << "Error: file could not be opened for reading \""
             << filename << "\"" << endl;
        return false;
    }

    // Read the first character
    fin >> buffer[0];

    if( buffer[0] != '\0' ) {
        fin.putback( buffer[0] );
    }

    // Read the first word of the file to see what type it is
    fin >> buffer;

    if( !strcmp( buffer, "Convex_Hierarchy" ) ) {
        ft = FT_CHIER;
    } else if( !strcmp( buffer, "Convex_Decomposition" ) ) {
        ft = FT_DECOMP;
    } else {
        ft = FT_OTHER;
    }

    fin.close();
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////
bool SWIFT_File_Read_Decomposition(
                            const char* filename, SWIFT_Tri_Mesh*& mesh,
#ifdef SWIFT_DECOMP
                            bool create_hierarchy,
                            SWIFT_Array<int>& piece_ids,
                            SWIFT_Array< SWIFT_Array<int> >& mfs,
                            SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face> >& vfs,
#endif
                            const SWIFT_Orientation& orient,
                            const SWIFT_Translation& trans,
                            SWIFT_Real scale, SPLIT_TYPE split )
{
    char buffer[256];
    ifstream fin;

    // Try to open the file
    if( filename == NULL ) {
        cerr << "Error: Invalid filename given to read file" << endl;
        return false;
    }

#ifdef WIN32
    fin.open( filename, ios::in | ios::binary );
#else
    fin.open( filename, ios::in );
#endif

    if( !fin.rdbuf()->is_open( ) ) {
        cerr << "Error: file could not be opened for reading \""
             << filename << "\"" << endl;
        return false;
    }

    // Read the first character
    fin >> buffer[0];

    if( buffer[0] != '\0' ) {
        fin.putback( buffer[0] );
    }

    // Read the first word of the file to see what type it is
    fin >> buffer;

    if( strcmp( buffer, "Convex_Decomposition" ) ) {
        cerr << "Error: file " << filename
             << " is not a convex decomposition file" << endl;
        fin.close();
        return false;
    }

    // This is a convex decomposition read
    bool file_is_big_endian;
    bool real_is_float;
    bool swap;
    int i, j, k;
    int real_size;
    int nv, nf, nmv, nmf, np, mfsn;
    int num_vfaces;
    char* rvs;
    SWIFT_Real* vs;
    int* fs;
    int* len;
    SWIFT_Array<int> loc_piece_ids;
    SWIFT_Array< SWIFT_Array<int> > loc_mfs;
    SWIFT_Array< SWIFT_Array<SWIFT_Tri_Face> > loc_vfs;

    // Read endianness
    fin >> buffer;  // Read the word "binary"
    fin >> buffer;
    file_is_big_endian = !strcmp( buffer, "big_endian" );
    swap = machine_is_big_endian != file_is_big_endian;

    // Read real type
    fin >> buffer;  // Read the word "real"
    fin >> buffer;
    real_is_float = !strcmp( buffer, "float" );
    real_size = real_is_float ? sizeof(float) : sizeof(double);

    // Read in the number of vertices
    fin >> buffer;  // Read the word "vertices"
    fin >> nv;

    // Read in the number of faces
    fin >> buffer;  // Read the word "faces"
    fin >> nf;

    // Read in the number of vertex mappings
    fin >> buffer;  // Read the word "map_vids"
    fin >> nmv;

    // Read in the number of face mappings
    fin >> buffer;  // Read the word "map_fids"
    fin >> nmf;

    // Read in the number of pieces
    fin >> buffer;  // Read the word "pieces"
    fin >> np;

    // Read over the mandatory newline
    fin.getline( buffer, 255 );

    // Allocate vertices
    rvs = new char[nv*3*real_size];

    // Read vertices
    fin.read( rvs, nv*3*real_size );

    // Possibly swap
    if( swap ) {
        Swap( rvs, nv*3, real_size );
    }

    // Copy because SWIFT_Real may be different than what is in the file
    vs = new SWIFT_Real[nv*3];
    if( real_is_float ) {
        for( i = 0; i < nv*3; i++ ) {
            vs[i] = ((float*)rvs)[i];
        }
    } else {
        for( i = 0; i < nv*3; i++ ) {
            vs[i] = ((double*)rvs)[i];
        }
    }

    // Read in the original face vertex ids
    fs = new int[nf*3];
    fin.read( (char*)fs, nf*3*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, nf*3, sizeof(int) );
    }

    // Create the mesh
    mesh = new SWIFT_Tri_Mesh;

    bool result = mesh->Create( vs, fs, nv, nf, orient, trans, scale );
    delete rvs; delete vs;
    if( !result ) {
        delete fs;
        delete mesh; mesh = NULL;
        fin.close();
        return false;
    }

    mesh->Set_No_Duplicate_Vertices( nmv == 0 );
    if( !mesh->No_Duplicate_Vertices() ) {
        mesh->Map_Vertex_Ids().Create( nmv );
        fin.read( (char*)mesh->Map_Vertex_Ids().Data(), nmv*sizeof(int) );
        if( swap ) {
            Swap( (char*)mesh->Map_Vertex_Ids().Data(), nmv, sizeof(int) );
        }
    }

#ifdef SWIFT_ONLY_TRIS
    // Ignore this data
    fin.read( (char*)fs, nmf*sizeof(int) );
#else
    mesh->Set_Only_Triangles( nmf == 0 );
    if( !mesh->Only_Triangles() ) {
        mesh->Map_Face_Ids().Create( nmf );
        fin.read( (char*)mesh->Map_Face_Ids().Data(), nmf*sizeof(int) );
        if( swap ) {
            Swap( (char*)mesh->Map_Face_Ids().Data(), nmf, sizeof(int) );
        }
    }
#endif

    delete fs;

    // Read in the piece ids
    loc_piece_ids.Create( nf );
    fin.read( (char*)loc_piece_ids.Data(), nf*sizeof(int) );
    if( swap ) {
        Swap( (char*)loc_piece_ids.Data(), nf, sizeof(int) );
    }

    // Read in the piece lengths and create the 2D arrays
    loc_mfs.Create( np );
    loc_vfs.Create( np );

    len = new int[np];

    fin.read( (char*)len, np*sizeof(int) );
    if( swap ) {
        Swap( (char*)len, np, sizeof(int) );
    }
    mfsn = 0;
    for( i = 0; i < np; i++ ) {
        loc_mfs[i].Create( len[i] );
        mfsn += len[i];
    }

    fin.read( (char*)len, np*sizeof(int) );
    if( swap ) {
        Swap( (char*)len, np, sizeof(int) );
    }
    num_vfaces = 0;
    for( i = 0; i < np; i++ ) {
        num_vfaces += len[i];
        loc_vfs[i].Create( len[i] );
    }

    delete [] len;

    // Read in the original face ids
    len = new int[mfsn];
    fin.read( (char*)len, mfsn*sizeof(int) );
    if( swap ) {
        Swap( (char*)len, mfsn, sizeof(int) );
    }

    for( i = 0, k = 0; i < np; i++ ) {
        for( j = 0; j < loc_mfs[i].Length(); j++, k++ ) {
            loc_mfs[i][j] = len[k];
        }
    }

    delete [] len;

    // Read in the virtual face ids
    len = new int[num_vfaces*3];
    fin.read( (char*)len, num_vfaces*3*sizeof(int) );
    if( swap ) {
        Swap( (char*)len, num_vfaces*3, sizeof(int) );
    }

    for( i = 0, k = 0; i < np; i++ ) {
        for( j = 0; j < loc_vfs[i].Length(); j++, k += 3 ) {
            // Set the vertex pointers in the virtual faces
            loc_vfs[i][j].Edge1().Set_Origin( mesh->Vertices()( len[k] ) );
            loc_vfs[i][j].Edge2().Set_Origin( mesh->Vertices()( len[k+1] ) );
            loc_vfs[i][j].Edge3().Set_Origin( mesh->Vertices()( len[k+2] ) );

            // Compute all geometry
            loc_vfs[i][j].Edge1().Compute_Direction_Length();
            loc_vfs[i][j].Edge2().Compute_Direction_Length();
            loc_vfs[i][j].Edge3().Compute_Direction_Length();
            loc_vfs[i][j].Compute_Plane_From_Edges();
            loc_vfs[i][j].Edge1().Compute_Voronoi_Planes();
            loc_vfs[i][j].Edge2().Compute_Voronoi_Planes();
            loc_vfs[i][j].Edge3().Compute_Voronoi_Planes();
        }
    }

    delete [] len;

    fin.close();

    // Now create the hierarchy
#ifdef SWIFT_DECOMP
    if( create_hierarchy ) {
#endif
        SWIFT_Array<SWIFT_Tri_Face*> st_faces;
        SWIFT_Array<SWIFT_Tri_Edge*> st_twins;

        mesh->Create_BV_Hierarchy( split, loc_piece_ids, loc_mfs, loc_vfs,
                                   st_faces, st_twins );

        // Nullify the vfs
        for( i = 0; i < loc_vfs.Length(); i++ ) {
            loc_vfs[i].Nullify();
        }
#ifdef SWIFT_DECOMP
    } else {
        // Copy the lists
        piece_ids = loc_piece_ids;
        mfs = loc_mfs;
        vfs = loc_vfs;
        loc_piece_ids.Nullify();
        loc_mfs.Nullify();
        loc_vfs.Nullify();
    }
#endif

    return true;
}

bool SWIFT_File_Read_Hierarchy( const char* filename, SWIFT_Tri_Mesh*& mesh,
                                const SWIFT_Orientation& orient,
                                const SWIFT_Translation& trans,
                                SWIFT_Real scale )
{
    char buffer[256];
    ifstream fin;

    // Try to open the file
    if( filename == NULL ) {
        cerr << "Error: Invalid filename given to read file" << endl;
        return false;
    }

#ifdef WIN32
    fin.open( filename, ios::in | ios::binary );
#else
    fin.open( filename, ios::in );
#endif

    if( !fin.rdbuf()->is_open( ) ) {
        cerr << "Error: file could not be opened for reading \""
             << filename << "\"" << endl;
        return false;
    }

    // Read the first character
    fin >> buffer[0];

    if( buffer[0] != '\0' ) {
        fin.putback( buffer[0] );
    }

    // Read the first word of the file to see what type it is
    fin >> buffer;

    if( strcmp( buffer, "Convex_Hierarchy" ) ) {
        cerr << "Error: file " << filename
             << " is not a convex hierarchy file" << endl;
        fin.close();
        return false;
    }

    // This is a convex hierarchy read
    SWIFT_Triple t;
    bool file_is_big_endian;
    bool real_is_float;
    bool swap;
    int i, j, k;
    int real_size;
    int nv, nmv, nmf, nof, nohf, ntf, ncf, net, nn;
    char* cs;
    char* rvs;
    int* fs;

    // Set up the input transformation
    SWIFT_Triple T = SWIFT_Triple( trans[0], trans[1], trans[2] );
    SWIFT_Matrix33 R;

    R.Set_Value( orient );

    // Read endianness
    fin >> buffer;  // Read the word "binary"
    fin >> buffer;
    file_is_big_endian = !strcmp( buffer, "big_endian" );
    swap = machine_is_big_endian != file_is_big_endian;

    // Read real type
    fin >> buffer;  // Read the word "real"
    fin >> buffer;
    real_is_float = !strcmp( buffer, "float" );
    real_size = real_is_float ? sizeof(float) : sizeof(double);

    // Read in the number of vertices
    fin >> buffer;  // Read the word "vertices"
    fin >> nv;

    // Read in the number of map vids
    fin >> buffer;  // Read the word "map_vids"
    fin >> nmv;

    // Read in the number of map fids
    fin >> buffer;  // Read the word "map_fids"
    fin >> nmf;

    // Read in the total number of original faces
    fin >> buffer;  // Read the word "orig_faces"
    fin >> nof;

    // Read in the number of original hierarchy faces
    fin >> buffer;  // Read the word "orig_hier_faces"
    fin >> nohf;

    // Read in the total number of faces (not copied faces)
    fin >> buffer;  // Read the word "total_faces"
    fin >> ntf;

    // Read in the total number of copied (other) faces
    fin >> buffer;  // Read the word "copied_faces"
    fin >> ncf;

    // Read in the number of edge twins
    fin >> buffer;  // Read the word "edge_twins"
    fin >> net;

    // Read in the number of nodes
    fin >> buffer;  // Read the word "nodes"
    fin >> nn;

    // Read over the mandatory newline
    fin.getline( buffer, 255 );

    // Create the mesh
    // It is assumed that the mesh can simply be filled in
    mesh = new SWIFT_Tri_Mesh;

    mesh->Use_Big_Arrays();

    // Create internal mesh stuff
    mesh->Vertices().Create( nv );
    mesh->Faces().Create( ntf );
    mesh->Other_Faces().Create( ncf );
    mesh->Edge_Twins().Create( net );
    mesh->BVs().Create( nn );

    // Read the SWIFT_Tri_Mesh record
    cs = new char[sizeof(int)+7*real_size];
    fin.read( cs, sizeof(int)+7*real_size );

    if( swap ) {
        Swap( cs, 1, sizeof(int) );
        Swap( cs+sizeof(int), 7, real_size );
    }

    // Allocate vertices
    rvs = new char[nv*3*real_size];

    // Read vertices and swap if necessary
    fin.read( rvs, nv*3*real_size );
    if( swap ) {
        Swap( rvs, nv*3, real_size );
    }

    // Copy vertex coordinates because SWIFT_Real may be different than what is
    // in the file.  Transform them at the same time.
    mesh->Set_Height( *((int*)cs) );
    if( real_is_float ) {
        t.Set_Value( *((float*)(cs+sizeof(int))),
                     *((float*)(cs+sizeof(int)+real_size)),
                     *((float*)(cs+sizeof(int)+2*real_size)) );
        mesh->Set_Center_Of_Mass( t );
        t.Set_Value( *((float*)(cs+sizeof(int)+3*real_size)),
                     *((float*)(cs+sizeof(int)+4*real_size)),
                     *((float*)(cs+sizeof(int)+5*real_size)) );
        mesh->Set_Center( t );
        mesh->Set_Radius( *((float*)(cs+sizeof(int)+6*real_size)) );

        for( i = 0, j = 0; j < nv; i += 3, j++ ) {
            SWIFT_Triple coords(
                    ((float*)rvs)[i], ((float*)rvs)[i+1], ((float*)rvs)[i+2] );
            mesh->Vertices()[j].Set_Coords( scale * (R * coords) + T );
        }
    } else {
        t.Set_Value( *((double*)(cs+sizeof(int))),
                     *((double*)(cs+sizeof(int)+real_size)),
                     *((double*)(cs+sizeof(int)+2*real_size)) );
        mesh->Set_Center_Of_Mass( t );
        t.Set_Value( *((double*)(cs+sizeof(int)+3*real_size)),
                     *((double*)(cs+sizeof(int)+4*real_size)),
                     *((double*)(cs+sizeof(int)+5*real_size)) );
        mesh->Set_Center( t );
        mesh->Set_Radius( *((double*)(cs+sizeof(int)+6*real_size)) );

        for( i = 0, j = 0; j < nv; i += 3, j++ ) {
            SWIFT_Triple coords(
                ((double*)rvs)[i], ((double*)rvs)[i+1], ((double*)rvs)[i+2] );
            mesh->Vertices()[j].Set_Coords( scale * (R * coords) + T );
        }
    }

    delete cs; delete rvs;

    mesh->Set_No_Duplicate_Vertices( nmv == 0 );
    if( !mesh->No_Duplicate_Vertices() ) {
        mesh->Map_Vertex_Ids().Create( nmv );
        fin.read( (char*)mesh->Map_Vertex_Ids().Data(), nmv*sizeof(int) );
        if( swap ) {
            Swap( (char*)mesh->Map_Vertex_Ids().Data(), nmv, sizeof(int) );
        }
    }

#ifdef SWIFT_ONLY_TRIS
    // Ignore this data
    if( nmf != 0 ) {
        fs = new int[nmf];
        fin.read( (char*)fs, nmf*sizeof(int) );
        delete fs;
        cerr << "Warning: file " << filename
             << " was not created from only triangles" << endl;
        cerr << "         SWIFT++ has been compiled to expect only triangles."
             << endl;
        cerr << "         Reported feature ids will be wrong." << endl;
    }
#else
    mesh->Set_Only_Triangles( nmf == 0 );
    if( !mesh->Only_Triangles() ) {
        mesh->Map_Face_Ids().Create( nmf );
        fin.read( (char*)mesh->Map_Face_Ids().Data(), nmf*sizeof(int) );
        if( swap ) {
            Swap( (char*)mesh->Map_Face_Ids().Data(), nmf, sizeof(int) );
        }
    }
#endif

    // Handle the faces
    fs = new int[ntf*4];
    fin.read( (char*)fs, ntf*4*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, ntf*4, sizeof(int) );
    }

    for( i = 0, j = 0; i < ntf; i++, j += 4 ) {
        mesh->Faces()[i].Edge1().Set_Origin( mesh->Vertices()(fs[j]) );
        mesh->Faces()[i].Edge2().Set_Origin( mesh->Vertices()(fs[j+1]) );
        mesh->Faces()[i].Edge3().Set_Origin( mesh->Vertices()(fs[j+2]) );
        mesh->Faces()[i].Set_Bit_Field( fs[j+3] );
    }
    delete fs;

    // Handle the copied faces
    fs = new int[ncf];
    fin.read( (char*)fs, ncf*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, ncf, sizeof(int) );
    }

    for( i = 0; i < ncf; i++ ) {
        mesh->Other_Faces()[i] = mesh->Faces()(fs[i]);
    }
    delete fs;

    // Handle the original faces twins
    fs = new int[nof*3];
    fin.read( (char*)fs, nof*3*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, nof*3, sizeof(int) );
    }

    for( i = 0, j = 0; i < nof; i++, j += 3 ) {
        mesh->Faces()[i].Edge1().Set_Twin( fs[j] == -1 ? NULL :
                            mesh->Faces()[fs[j]>>2].EdgeP( fs[j]&0x3 ) );
        mesh->Faces()[i].Edge2().Set_Twin( fs[j+1] == -1 ? NULL :
                            mesh->Faces()[fs[j+1]>>2].EdgeP( fs[j+1]&0x3 ) );
        mesh->Faces()[i].Edge3().Set_Twin( fs[j+2] == -1 ? NULL :
                            mesh->Faces()[fs[j+2]>>2].EdgeP( fs[j+2]&0x3 ) );
        mesh->Faces()[i].Edge1().Origin()->Set_Adj_Edge(
                                                mesh->Faces()[i].Edge1P() );
        mesh->Faces()[i].Edge2().Origin()->Set_Adj_Edge(
                                                mesh->Faces()[i].Edge2P() );
        mesh->Faces()[i].Edge3().Origin()->Set_Adj_Edge(
                                                mesh->Faces()[i].Edge3P() );
    }
    delete fs;

    // Handle the twins for original faces that live in the hierarchy
    fs = new int[nohf*4];
    fin.read( (char*)fs, nohf*4*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, nohf*4, sizeof(int) );
    }

    for( i = 0, j = 0; i < nohf; i++, j += 4 ) {
        mesh->Faces()[fs[j]].Edge1().Set_Twin(
                            mesh->Faces()[fs[j+1]>>2].EdgeP( fs[j+1]&0x3 ) );
        mesh->Faces()[fs[j]].Edge2().Set_Twin(
                            mesh->Faces()[fs[j+2]>>2].EdgeP( fs[j+2]&0x3 ) );
        mesh->Faces()[fs[j]].Edge3().Set_Twin(
                            mesh->Faces()[fs[j+3]>>2].EdgeP( fs[j+3]&0x3 ) );
    }
    delete fs;

    // Handle edge twins
    fs = new int[net];
    fin.read( (char*)fs, net*sizeof(int) );
    if( swap ) {
        Swap( (char*)fs, net, sizeof(int) );
    }

    for( i = 0; i < net; i++ ) {
        mesh->Edge_Twins()[i] = mesh->Faces()[fs[i]>>2].EdgeP( fs[i]&0x3 );
    }
    delete fs;

    // Handle the BV records
    SWIFT_Array<int> twin_lens( mesh->Num_Faces() );
    for( i = 0; i < nn; i++ ) {
        cs = new char[9*sizeof(int)+4*real_size];
        fin.read( cs, 9*sizeof(int)+4*real_size );
        if( swap ) {
            Swap( cs, 8, sizeof(int) );
            Swap( cs+8*sizeof(int), 4, real_size );
            Swap( cs+8*sizeof(int)+4*real_size, 1, sizeof(int) );
        }

        // Set other faces
        j = *((int*)cs);
        mesh->BVs()[i].Other_Faces().Set_Max_Length( j );
        mesh->BVs()[i].Other_Faces().Set_Length( j );
        j = *((int*)(cs+sizeof(int)));
        mesh->BVs()[i].Other_Faces().Set_Data( mesh->Other_Faces().Data()+j );

        // Set faces
        j = *((int*)(cs+2*sizeof(int)));
        mesh->BVs()[i].Faces().Set_Max_Length( j );
        mesh->BVs()[i].Faces().Set_Length( j );
        j = *((int*)(cs+3*sizeof(int)));
        mesh->BVs()[i].Faces().Set_Data( mesh->Faces().Data()+j );

        // Set parent and children
        j = *((int*)(cs+4*sizeof(int)));
        mesh->BVs()[i].Set_Parent( j == -1 ? NULL : mesh->BVs()(j) );
        j = *((int*)(cs+5*sizeof(int)));
        if( j != -1 ) {
            mesh->BVs()[i].Children().Create( 2 );
            mesh->BVs()[i].Children()[0] = mesh->BVs()(j);
            j = *((int*)(cs+6*sizeof(int)));
            mesh->BVs()[i].Children()[1] = mesh->BVs()(j);
        }

        j = *((int*)(cs+7*sizeof(int)));
        mesh->BVs()[i].Set_Level( j );

        // Set the edge twin arrays
        for( k = 0; k < mesh->BVs()[i].Num_Faces(); k++ ) {
            twin_lens[mesh->Face_Id( mesh->BVs()[i].Faces()(k) )] =
                            j - mesh->BVs()[i].Faces()[k].Starting_Level() + 1;
        }

        // Also set the main mesh original faces twins arrays
        if( mesh->BVs()[i].Is_Leaf() ) {
            for( k = 0; k < mesh->BVs()[i].Num_Other_Faces(); k++ ) {
                twin_lens[mesh->Face_Id( mesh->BVs()[i].Other_Faces()[k] )] =
                    j - mesh->BVs()[i].Other_Faces()[k]->Starting_Level() + 1;
            }
        }

        if( real_is_float ) {
            t.Set_Value( *((float*)(cs+8*sizeof(int))),
                         *((float*)(cs+8*sizeof(int)+real_size)),
                         *((float*)(cs+8*sizeof(int)+2*real_size)) );
            mesh->BVs()[i].Set_Center_Of_Mass( t );
            mesh->BVs()[i].Set_Radius(
                                *((float*)(cs+7*sizeof(int)+3*real_size)) );
        } else {
            t.Set_Value( *((double*)(cs+8*sizeof(int))),
                         *((double*)(cs+8*sizeof(int)+real_size)),
                         *((double*)(cs+8*sizeof(int)+2*real_size)) );
            mesh->BVs()[i].Set_Center_Of_Mass( t );
            mesh->BVs()[i].Set_Radius(
                                *((double*)(cs+8*sizeof(int)+3*real_size)) );
        }

        j = *((int*)(cs+8*sizeof(int)+4*real_size));
        mesh->BVs()[i].Lookup_Table().Set_Type( j );

        j = mesh->BVs()[i].Lookup_Table().Size();
        fs = new int[j];
        fin.read( (char*)fs, j*sizeof(int) );
        if( swap ) {
            Swap( (char*)fs, j, sizeof(int) );
        }

        SWIFT_Array<SWIFT_Tri_Edge*> lut_edges( j );
        for( k = 0; k < j; k++ ) {
            lut_edges[k] = mesh->Faces()[fs[k]>>2].EdgeP( fs[k]&0x3 );
        }

        mesh->BVs()[i].Lookup_Table().Set_Table( lut_edges );

        lut_edges.Nullify();
        delete fs;
        delete cs;
    }

    // Run through the faces setting their twin array ptrs
    SWIFT_Tri_Edge** et_ptr = mesh->Edge_Twins().Data();
    for( i = 0; i < mesh->Num_Faces(); i++ ) {
        mesh->Faces()[i].Edge1().Set_Twin_Array( et_ptr );
        et_ptr += twin_lens[i];
        mesh->Faces()[i].Edge2().Set_Twin_Array( et_ptr );
        et_ptr += twin_lens[i];
        mesh->Faces()[i].Edge3().Set_Twin_Array( et_ptr );
        et_ptr += twin_lens[i];
    }

    mesh->Faces().Set_Length( nof );

    fin.close();

    return true;
}


