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
// SWIFT_array.h
//
// Description:
//      Classes to manage an array.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_ARRAY_H_
#define _SWIFT_ARRAY_H_

#ifdef SWIFT_DEBUG
#include <iostream.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <SWIFT_config.h>
#include <SWIFT_common.h>

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Array
//
// Description:
//      Array class.
//////////////////////////////////////////////////////////////////////////////
template< class Type >
class SWIFT_Array {
  public:
    SWIFT_Array( ) { length = 0; max_length = 0; array = NULL; }
    SWIFT_Array( int l )
                    { length = 0; max_length = 0; array = NULL; Create( l ); }
    SWIFT_Array( const SWIFT_Array<Type>& l )
    {
        *this = l;
        if( max_length != 0 ) {
            array = new Type[max_length];
            memcpy( (void*)array, (void*)l.Data(), max_length*sizeof(Type) );
        } else {
            array = NULL;
        }
    }

    ~SWIFT_Array( ) { delete [] array; }


 // Copy functions
    void Copy( const SWIFT_Array<Type>& l )
    {
        if( array != NULL ) {
            Destroy();
        }
        length = l.Length();
        max_length = l.Max_Length();
        if( max_length != 0 ) {
            array = new Type[max_length];
            memcpy( (void*)array, (void*)l.Data(), max_length*sizeof(Type) );
        } else {
            array = NULL;
        }
    }
    void Copy_Length( const SWIFT_Array<Type>& l )
    {
        if( array != NULL ) {
            Destroy();
        }
        length = l.Length();
        max_length = l.Length();
        if( max_length != 0 ) {
            array = new Type[max_length];
            memcpy( (void*)array, (void*)l.Data(), max_length*sizeof(Type) );
        } else {
            array = NULL;
        }
    }

  // Content Query
    int Length( ) const { return length; }
    int Max_Length( ) const { return max_length; }
    Type& operator[]( int index )
    {
#ifdef SWIFT_DEBUG
        if( index < length && index >= 0 ) {
            return array[index];
        } else {
                cerr << "SWIFT_Array[] Error: indexing wrong into: "
                     << "index = " << index << " length = " << length << endl;
            return array[0];
        }
#else
        return array[index];
#endif
    }
    Type& Last( ) { return array[length-1]; }
    Type& Second_Last( ) { return array[length-2]; }
    Type* LastP( ) { return array+length-1; }
    Type* operator()( int index )
    {
#ifdef SWIFT_DEBUG
        if( index < length && index >= 0 ) {
            return array+index;
        } else {
                cerr << "SWIFT_Array() Error: indexing wrong into: "
                     << "index = " << index << " length = " << length << endl;
            return array;
        }
#else
        return array+index;
#endif
    }
    Type* Data( ) const { return array; }
    int Position( const Type* addr ) const { return (int)(addr-array); }
    bool Empty( ) const { return length == 0; }
    bool Exists( ) const { return array != NULL; }

  // Content modification
    void Set_Last( const Type t ) { array[length-1] = t; }
    void Add( const Type t ) 
    {
#ifdef SWIFT_DEBUG
        if( length == max_length ) {
            cerr << "SWIFT_Array Error: Adding onto already full array" << endl;
        } else {
#endif  
            array[length++] = t;
#ifdef SWIFT_DEBUG
        }
#endif      
    }
    void Add_Grow( const Type t, int amount ) 
    {
        if( length == max_length ) {
            Grow( amount );
        }
        Add( t );
    }
    void Set_Length( int len ) { length = len; }
    void Set_Max_Length( int len ) { max_length = len; }
    void Increment_Length( ) { length++; }
    void Decrement_Length( ) { length--; }
    void Nullify( ) { array = NULL; length = 0; max_length = 0; }

  // Creation/allocation and destroy
    void Create( int n )
    {
        if( max_length != 0 ) {
#ifdef SWIFT_DEBUG
                cerr << "Warning: creating SWIFT_Array when one already "
                     << "exists" << endl;
#endif
            Destroy();
        }
        if( n > 0 ) {
            array = new Type[n]; length = n; max_length = n;
        }
    }
    void Recreate( int n )
    {
        if( max_length != 0 ) {
            Destroy();
        }
        if( n > 0 ) {
            array = new Type[n]; length = n; max_length = n;
        }
    }
    void Destroy( )
    {
        delete [] array;
        array = NULL; length = 0; max_length = 0;
    }
    void Set_Data( Type* tp ) { array = tp; }

  // Growing
    void Grow( int amount )
    {
        Type* temp = array;
        array = new Type[max_length+amount];
        // Copy the old elements back
        memcpy( (void*)array, (void*)temp, length*sizeof(Type) );
        delete [] temp;
        max_length += amount;
    }
    void Grow_Double( )
    {
        if( length != 0 ) {
            Type* temp = array;
            array = new Type[max_length<<1];
            // Copy the old elements back
            memcpy( (void*)array, (void*)temp, length*sizeof(Type) );
            delete [] temp;
            max_length <<= 1;
        }
    }
    void Fit_Grow( int test, int amount  )
    {
        if( max_length-length < test ) {
            Grow( max(amount,test) );
        }
    }
    void Ensure_Length( int l  )
    {
        if( max_length < l ) {
            Recreate( l );
        }
    }

  private:
    int length;
    int max_length;
    Type* array;
};

#endif


