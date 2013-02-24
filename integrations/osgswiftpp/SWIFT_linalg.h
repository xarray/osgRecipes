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
// SWIFT_linalg.h
//
// Description:
//      Classes to manage linear algebra entities.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_LINALG_H_
#define _SWIFT_LINALG_H_

#include <iostream>
#include <math.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>  // We have calls to OpenGL directly from here for
                    // efficiency purposes

#include <SWIFT_config.h>
#include <SWIFT_common.h>

using namespace  std;

// Forward declarations
class SWIFT_Matrix33;
class SWIFT_Transformation;

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Triple
//
// Description:
//      A class to represent a triple.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Triple {
  public:
    inline SWIFT_Triple( ) {
#ifdef SWIFT_DEBUG
                            Set_Value( 0.0, 0.0, 0.0 );
#endif
                            }
    inline SWIFT_Triple( const SWIFT_Triple& t ) { *this = t; }
    inline SWIFT_Triple( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
                                                    { Set_Value( x, y, z ); }
    inline SWIFT_Triple( SWIFT_Real t[] ) { Set_Value( t ); }
    inline ~SWIFT_Triple( ) { }

    // Size of the data
    inline int Size( ) const { return 3; }

    // Get/set routines
    inline SWIFT_Real X( ) const { return val[0]; }
    inline SWIFT_Real Y( ) const { return val[1]; }
    inline SWIFT_Real Z( ) const { return val[2]; }
    inline SWIFT_Real Largest_Coord( );
    inline SWIFT_Real Smallest_Coord( );
    inline const SWIFT_Real* Value( ) const { return val; }
    inline void Get_Value( SWIFT_Real v[] ) const;
    inline void Get_Value( SWIFT_Real& x, SWIFT_Real& y, SWIFT_Real& z ) const;

    inline void Set_X( SWIFT_Real x ) { val[0] = x; }
    inline void Set_Y( SWIFT_Real y ) { val[1] = y; }
    inline void Set_Z( SWIFT_Real z ) { val[2] = z; }
    inline void Set_Value( const SWIFT_Real v[] );
    //inline void Set_Value( const float v[] );
    //inline void Set_Value( const double v[] );
    inline void Set_Value( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z );
    inline void Set_Spherical_Coordinates( const SWIFT_Triple& c, SWIFT_Real r,
                                           SWIFT_Real theta, SWIFT_Real phi );
    inline void Set_Spherical_Coordinates_This_Center(
                            SWIFT_Real r, SWIFT_Real theta, SWIFT_Real phi );
    inline void Set_Spherical_Coordinates_Zero_Center(
                            SWIFT_Real r, SWIFT_Real theta, SWIFT_Real phi );
    inline void Set_Largest_Coord( SWIFT_Real v );
    inline void Set_Smallest_Coord( SWIFT_Real v );

    // Distance functions
    inline SWIFT_Real Dist_Sq( const SWIFT_Triple& t ) const;
    // Distance to the line defined by the two points.
    inline SWIFT_Real Dist( const SWIFT_Triple& t ) const;
    inline SWIFT_Real Length_Sq( ) const;
    inline SWIFT_Real Length( ) const;

    // Min max functions
    // Take this triple and merge using min or max into the other triple(s)
    inline void Min( SWIFT_Triple& mn );
    inline void Max( SWIFT_Triple& mx );
    inline void Min_Max( SWIFT_Triple& mn, SWIFT_Triple& mx );

    // OpenGL efficiency functions
    inline void Send_VCoords_To_OpenGL( ) const;
    inline void Send_NCoords_To_OpenGL( ) const;
    // Operations on this object
    inline void Normalize( );
    inline void Zero( );
    inline void Identity( );
    inline void Negate( );
    inline void Add( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 );
    inline void Sub( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 );
    inline void Scale( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 );
    inline void Reflect_XY( );
    inline void Reflect_XZ( );
    inline void Reflect_YZ( );
    inline void Reflect( const SWIFT_Triple& v );
    inline void Transform( const SWIFT_Triple& x, const SWIFT_Triple& y,
                           const SWIFT_Triple& z );

    // Operators that change this object
    inline void operator+=( const SWIFT_Triple& t );
    inline void operator-=( const SWIFT_Triple& t );
    inline void operator*=( SWIFT_Real s );
    inline void operator/=( SWIFT_Real s );

    // Transformation by a row major matrix (right multiplication)
    // Point transformation (vector transformation is the same)
    inline void operator^=( const SWIFT_Matrix33& m );
    // Point transformation
    inline void operator^=( const SWIFT_Transformation& m );
    // Vector transformation
    inline void operator&=( const SWIFT_Transformation& m );

    // Operators that do not change this object
    inline SWIFT_Matrix33 To_Cross_Prod_Matrix( ) const;
    inline SWIFT_Triple Orthogonal( bool norm ) const;
    inline bool operator==( const SWIFT_Triple& t ) const;
    inline bool operator!=( const SWIFT_Triple& t ) const;
    inline bool operator<=( const SWIFT_Triple& t ) const;
    inline bool operator>=( const SWIFT_Triple& t ) const;

    friend SWIFT_Triple operator-( const SWIFT_Triple& t );
    friend SWIFT_Triple operator+( const SWIFT_Triple&, const SWIFT_Triple& );
    friend SWIFT_Triple operator-( const SWIFT_Triple&, const SWIFT_Triple& );
    friend SWIFT_Triple operator*( SWIFT_Real s, const SWIFT_Triple& t );
    friend SWIFT_Triple operator*( const SWIFT_Triple& t, SWIFT_Real s );
    friend SWIFT_Triple operator/( const SWIFT_Triple& t, SWIFT_Real s );
    friend SWIFT_Real operator*( const SWIFT_Triple&, const SWIFT_Triple& );
    friend SWIFT_Triple operator%( const SWIFT_Triple&, const SWIFT_Triple& );
    friend SWIFT_Triple operator*( const SWIFT_Triple&, const SWIFT_Matrix33& );
    friend SWIFT_Triple operator*( const SWIFT_Matrix33&, const SWIFT_Triple& );
    // m^T * t
    friend SWIFT_Triple operator%( const SWIFT_Matrix33&, const SWIFT_Triple& );

    friend class SWIFT_Transformation;
    // Point transformation
    friend SWIFT_Triple operator*( const SWIFT_Transformation&,
                                   const SWIFT_Triple& );
    // Vector transformation
    friend SWIFT_Triple operator&( const SWIFT_Transformation&,
                                   const SWIFT_Triple& );
    friend ostream& operator<<( ostream&, const SWIFT_Transformation& );
    friend ostream& operator<<( ostream&, const SWIFT_Triple& );

  private:
    SWIFT_Real val[3];
};


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Matrix33
//
// Description:
//      A class to represent a matrix.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Matrix33 {
  public:
    inline SWIFT_Matrix33( ) { }
    inline SWIFT_Matrix33( const SWIFT_Matrix33& m ) { *this = m; }
    inline SWIFT_Matrix33( SWIFT_Real v[3][3] ) { Set_Value( v ); }
    inline SWIFT_Matrix33( SWIFT_Real v[] ) { Set_Value( v ); }
    inline SWIFT_Matrix33( SWIFT_Real v0, SWIFT_Real v1, SWIFT_Real v2,
                           SWIFT_Real v3, SWIFT_Real v4, SWIFT_Real v5,
                           SWIFT_Real v6, SWIFT_Real v7, SWIFT_Real v8 )
    {
        val[0] = v0, val[1] = v1, val[2] = v2, val[3] = v3, val[4] = v4;
        val[5] = v5, val[6] = v6, val[7] = v7, val[8] = v8;
    }
    inline ~SWIFT_Matrix33( ) { }

    // Size of the data
    inline int Size( ) const { return 9; }

    // Get/set routines
    inline void Get_Value( SWIFT_Real v[] ) const;
    inline void Get_Value( SWIFT_Real v[3][3] ) const;
    inline void Set_Value( const SWIFT_Real v[] );
    inline void Set_Value( const SWIFT_Real v[3][3] );
    inline void Set_Value_Rows( const SWIFT_Triple& t0, const SWIFT_Triple& t1,
                                const SWIFT_Triple& t2 )
    {
        val[0] = t0.X(); val[3] = t1.X(); val[6] = t2.X();
        val[1] = t0.Y(); val[4] = t1.Y(); val[7] = t2.Y();
        val[2] = t0.Z(); val[5] = t1.Z(); val[8] = t2.Z();
    }
    inline void Set_Value_Cols( const SWIFT_Triple& t0, const SWIFT_Triple& t1,
                                const SWIFT_Triple& t2 )
    {
        val[0] = t0.X(); val[1] = t1.X(); val[2] = t2.X();
        val[3] = t0.Y(); val[4] = t1.Y(); val[5] = t2.Y();
        val[6] = t0.Z(); val[7] = t1.Z(); val[8] = t2.Z();
    }
    inline const SWIFT_Real* Value( ) const { return val; }

    inline void Set_Value( SWIFT_Real rotx, SWIFT_Real roty, SWIFT_Real rotz )
    {
        SWIFT_Real cos_a = cos( rotx );
        SWIFT_Real sin_a = sin( rotx );
        SWIFT_Matrix33 rx( 1.0, 0.0, 0.0, 0.0, cos_a, -sin_a,
                           0.0, sin_a, cos_a );
        cos_a = cos( roty );
        sin_a = sin( roty );
        SWIFT_Matrix33 ry( cos_a, 0.0, sin_a, 0.0, 1.0, 0.0,
                           -sin_a, 0.0, cos_a );
        cos_a = cos( rotz );
        sin_a = sin( rotz );
        SWIFT_Matrix33 rz( cos_a, -sin_a, 0.0, sin_a, cos_a, 0.0,
                     0.0, 0.0, 1.0 );
        *this = rz * (ry * rx);
    }

    // Operations that change this object
    inline void Zero( );
    inline void Identity( );
    //void Negate( );
    inline void Transpose( );
    //bool Invert( );

    // Operators that change this object
    inline void operator+=( const SWIFT_Matrix33& m );
    inline void operator-=( const SWIFT_Matrix33& m );
    inline void operator*=( SWIFT_Real s );
    inline void operator/=( SWIFT_Real s );

    // Operators that do not change this object
    //SWIFT_Real Determinant( ) const;

    friend void SWIFT_Triple::operator^=( const SWIFT_Matrix33& m );

    friend SWIFT_Matrix33 operator*( SWIFT_Real s,
                                     const SWIFT_Matrix33& m );
    friend SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m,
                                     SWIFT_Real s );
    friend SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m,
                                     SWIFT_Real s );

    friend SWIFT_Triple operator*( const SWIFT_Triple& t,
                                   const SWIFT_Matrix33& m );
    friend SWIFT_Triple operator*( const SWIFT_Matrix33& m,
                                   const SWIFT_Triple& t );
    // m^T * t
    friend SWIFT_Triple operator%( const SWIFT_Matrix33& m,
                                   const SWIFT_Triple& t );

    friend SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m );
    friend SWIFT_Matrix33 operator+( const SWIFT_Matrix33& m1,
                                     const SWIFT_Matrix33& m2 );
    friend SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m1,
                                     const SWIFT_Matrix33& m2 );
    friend SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m1,
                                     const SWIFT_Matrix33& m2 );
    // m1 * m2^T
    friend SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m1,
                                     const SWIFT_Matrix33& m2 );
    // m1^T * m2
    friend SWIFT_Matrix33 operator%( const SWIFT_Matrix33& m1,
                                     const SWIFT_Matrix33& m2 );
    friend ostream& operator<<( ostream& out, const SWIFT_Matrix33& m );

    friend class SWIFT_Transformation;
    friend void SWIFT_Triple::operator^=( const SWIFT_Transformation& m );
    friend void SWIFT_Triple::operator&=( const SWIFT_Transformation& m );
    friend SWIFT_Triple operator*( const SWIFT_Transformation& m,
                                   const SWIFT_Triple& t );
    friend SWIFT_Triple operator&( const SWIFT_Transformation& m,
                                   const SWIFT_Triple& t );
    friend ostream& operator<<( ostream& out, const SWIFT_Transformation& m );

  private:
    SWIFT_Real val[9];
};

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Transformation
//
// Description:
//      A class to represent a rigid transformation.
//////////////////////////////////////////////////////////////////////////////
class SWIFT_Transformation {
  public:
    inline SWIFT_Transformation( ) { }
    // Set as a row major 3x4 matrix where the fourth column is the translation
    inline SWIFT_Transformation( SWIFT_Real v[] ) { Set_Value( v ); }
    inline SWIFT_Transformation( const SWIFT_Matrix33& matrix
                                        ) : R( matrix ), T( 0.0, 0.0, 0.0 ) { }
    inline SWIFT_Transformation( const SWIFT_Matrix33& matrix,
                                 const SWIFT_Triple& t ) : R(matrix), T(t) { }
    // Set as a row major 3x4 matrix where the fourth column is the translation
    inline SWIFT_Transformation( SWIFT_Real v0, SWIFT_Real v1, SWIFT_Real v2,
                                 SWIFT_Real v3, SWIFT_Real v4, SWIFT_Real v5,
                                 SWIFT_Real v6, SWIFT_Real v7, SWIFT_Real v8,
                                 SWIFT_Real v9, SWIFT_Real v10, SWIFT_Real v11
            ) : R( v0, v1, v2, v4, v5, v6, v8, v9, v10 ), T( v3, v7, v11 ) { }
    inline ~SWIFT_Transformation( ) { }

    // Size of the data
    inline int Size( ) const { return 12; }

    // Get/set routines
    inline const SWIFT_Triple& Translation() const { return T; }
    inline const SWIFT_Matrix33& Rotation() const { return R; }

    inline void Set_Value( const SWIFT_Real v[] );
    inline void Set_Value( const SWIFT_Real R[], const SWIFT_Real T[] );
    inline void Set_Value( SWIFT_Matrix33 r, SWIFT_Triple t ) { R = r; T = t; }
    inline void Set_Rotation( SWIFT_Matrix33 r ) { R = r; }
    inline void Set_Rotation_Cols( SWIFT_Triple& t1, SWIFT_Triple& t2,
                        SWIFT_Triple& t3 ) { R.Set_Value_Cols( t1, t2, t3 ); }
    inline void Set_Translation( SWIFT_Triple t ) { T = t; }

  // Operations that change this object

    // m1 or m2 cannot be this.  transform from m1.CS to m2.CS
    inline void Transform_From_To( const SWIFT_Transformation& m1,
                                   const SWIFT_Transformation& m2 );
    // transform from this.CS to m.CS
    inline void Transform_To( const SWIFT_Transformation& m );
    // m then this
    inline void Pre_Compose( const SWIFT_Transformation& m );
    // m1 then m2
    inline void Compose( const SWIFT_Transformation& m1,
                         const SWIFT_Transformation& m2 );
    inline void Scale( const SWIFT_Triple& s );
    inline void Invert( const SWIFT_Transformation& m );
    inline void Identity( ) { R.Identity(); T.Identity(); }

    friend void SWIFT_Triple::operator^=( const SWIFT_Transformation& m );
    friend void SWIFT_Triple::operator&=( const SWIFT_Transformation& m );
    friend SWIFT_Triple operator*( const SWIFT_Transformation& m,
                                   const SWIFT_Triple& t );
    friend SWIFT_Triple operator&( const SWIFT_Transformation& m,
                                   const SWIFT_Triple& t );
    friend ostream& operator<<( ostream& out, const SWIFT_Transformation& m );

  private:
    SWIFT_Matrix33 R;
    SWIFT_Triple T;
};

//////////////////////////////////////////////////////////////////////////////
// Auxiliary operator functions
//////////////////////////////////////////////////////////////////////////////

inline SWIFT_Triple operator-( const SWIFT_Triple& t );
inline SWIFT_Triple operator+( const SWIFT_Triple& t1, const SWIFT_Triple& t2 );
inline SWIFT_Triple operator-( const SWIFT_Triple& t1, const SWIFT_Triple& t2 );
inline SWIFT_Triple operator*( SWIFT_Real s, const SWIFT_Triple& t );
inline SWIFT_Triple operator*( const SWIFT_Triple& t, SWIFT_Real s );
inline SWIFT_Triple operator/( const SWIFT_Triple& t, SWIFT_Real s );
// Dot product
inline SWIFT_Real operator*( const SWIFT_Triple& t1, const SWIFT_Triple& t2 );
// Cross product
inline SWIFT_Triple operator%( const SWIFT_Triple& t1, const SWIFT_Triple& t2 );
// Vector-Matrix multiplication
inline SWIFT_Triple operator*( const SWIFT_Triple& t, const SWIFT_Matrix33& m );
// Matrix-Vector multiplication
inline SWIFT_Triple operator*( const SWIFT_Matrix33& m, const SWIFT_Triple& t );
// Matrix transpose-Vector multiplication
inline SWIFT_Triple operator%( const SWIFT_Matrix33& m, const SWIFT_Triple& t );


inline SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m );
inline SWIFT_Matrix33 operator+( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 );
inline SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 );
inline SWIFT_Matrix33 operator*( SWIFT_Real s, const SWIFT_Matrix33& m );
inline SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m, SWIFT_Real s );
inline SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m, SWIFT_Real s );
// Matrix multiplication = m1 * m2
inline SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 );
// Matrix multiplication with transpose of second = m1 * m2^T
inline SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 );
// Matrix multiplication with transpose of first = m1^T * m2
inline SWIFT_Matrix33 operator%( const SWIFT_Matrix33& m1, 
                                 const SWIFT_Matrix33& m2 );

// Point transformation
inline SWIFT_Triple operator*( const SWIFT_Transformation& m,
                               const SWIFT_Triple& t );
// Vector transformation
inline SWIFT_Triple operator&( const SWIFT_Transformation& m,
                               const SWIFT_Triple& t );


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Triple member functions inline definitions
//////////////////////////////////////////////////////////////////////////////


inline SWIFT_Real SWIFT_Triple::Largest_Coord( )
{
    const SWIFT_Real val0 = myfabs(val[0]);
    const SWIFT_Real val1 = myfabs(val[1]);
    const SWIFT_Real val2 = myfabs(val[2]);
    if( val0 > val1 ) {
        if( val0 > val2 ) {
            return val[0];
        } else {
            return val[2];
        }
    } else {
        if( val1 > val2 ) {
            return val[1];
        } else {
            return val[2];
        }
    }
}

inline SWIFT_Real SWIFT_Triple::Smallest_Coord( )
{
    const SWIFT_Real val0 = myfabs(val[0]);
    const SWIFT_Real val1 = myfabs(val[1]);
    const SWIFT_Real val2 = myfabs(val[2]);
    if( val0 < val1 ) {
        if( val0 < val2 ) {
            return val[0];
        } else {
            return val[2];
        }
    } else {
        if( val1 < val2 ) {
            return val[1];
        } else {
            return val[2];
        }
    }
}

inline void SWIFT_Triple::Get_Value( SWIFT_Real v[] ) const
{ v[0] = val[0], v[1] = val[1], v[2] = val[2]; }

inline void SWIFT_Triple::Get_Value( SWIFT_Real& x, SWIFT_Real& y,
                                     SWIFT_Real& z ) const
{ x = val[0], y = val[1], z = val[2]; }


inline void SWIFT_Triple::Set_Value( const SWIFT_Real v[] )
{ val[0] = v[0], val[1] = v[1], val[2] = v[2]; }

inline void SWIFT_Triple::Set_Value( SWIFT_Real x, SWIFT_Real y, SWIFT_Real z )
{ val[0] = x, val[1] = y, val[2] = z; }

inline void SWIFT_Triple::Set_Spherical_Coordinates(
                                        const SWIFT_Triple& c, SWIFT_Real r,
                                        SWIFT_Real theta, SWIFT_Real phi )
{
    c.Get_Value( val );
    Set_Spherical_Coordinates_This_Center( r, theta, phi );
}

inline void SWIFT_Triple::Set_Spherical_Coordinates_This_Center(
                            SWIFT_Real r, SWIFT_Real theta, SWIFT_Real phi )
{
    val[0] += r * cos( theta ) * cos( phi );
    val[1] += r * sin( theta ) * cos( phi );
    val[2] += r * sin( phi );
}

inline void SWIFT_Triple::Set_Spherical_Coordinates_Zero_Center(
                            SWIFT_Real r, SWIFT_Real theta, SWIFT_Real phi )
{
    val[0] = r * cos( theta ) * cos( phi );
    val[1] = r * sin( theta ) * cos( phi );
    val[2] = r * sin( phi );
}

inline void SWIFT_Triple::Set_Largest_Coord( SWIFT_Real v )
{
    const SWIFT_Real val0 = myfabs(val[0]);
    const SWIFT_Real val1 = myfabs(val[1]);
    const SWIFT_Real val2 = myfabs(val[2]);
    if( val0 > val1 ) {
        if( val0 > val2 ) {
            val[0] = v;
        } else {
            val[2] = v;
        }
    } else {
        if( val1 > val2 ) {
            val[1] = v;
        } else {
            val[2] = v;
        }
    }
}

inline void SWIFT_Triple::Set_Smallest_Coord( SWIFT_Real v )
{
    const SWIFT_Real val0 = myfabs(val[0]);
    const SWIFT_Real val1 = myfabs(val[1]);
    const SWIFT_Real val2 = myfabs(val[2]);
    if( val0 < val1 ) {
        if( val0 < val2 ) {
            val[0] = v;
        } else {
            val[2] = v;
        }
    } else {
        if( val1 < val2 ) {
            val[1] = v;
        } else {
            val[2] = v;
        }
    }
}

inline SWIFT_Real SWIFT_Triple::Dist_Sq( const SWIFT_Triple& t ) const
{
    SWIFT_Real x, y, z;
    t.Get_Value( x, y, z );
    x -= val[0];
    y -= val[1];
    z -= val[2];
    return x * x + y * y + z * z;
}

inline SWIFT_Real SWIFT_Triple::Dist( const SWIFT_Triple& t ) const
{
    SWIFT_Real x, y, z;
    t.Get_Value( x, y, z );
    x -= val[0];
    y -= val[1];
    z -= val[2];
    return sqrt( x * x + y * y + z * z );
}

inline SWIFT_Real SWIFT_Triple::Length_Sq( ) const
{ return val[0] * val[0] + val[1] * val[1] + val[2] * val[2]; }

inline SWIFT_Real SWIFT_Triple::Length( ) const
{ return sqrt( val[0] * val[0] + val[1] * val[1] + val[2] * val[2] ); }

# ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
# endif

# ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
# endif

inline void SWIFT_Triple::Min( SWIFT_Triple& mn )
{
    mn.Set_X( min( mn.X(), val[0] ) );
    mn.Set_Y( min( mn.Y(), val[1] ) );
    mn.Set_Z( min( mn.Z(), val[2] ) );
}

inline void SWIFT_Triple::Max( SWIFT_Triple& mx )
{
    mx.Set_X( max( mx.X(), val[0] ) );
    mx.Set_Y( max( mx.Y(), val[1] ) );
    mx.Set_Z( max( mx.Z(), val[2] ) );
}

inline void SWIFT_Triple::Min_Max( SWIFT_Triple& mn, SWIFT_Triple& mx )
{ Min( mn ); Max( mx ); }

inline void SWIFT_Triple::Send_VCoords_To_OpenGL( ) const
#ifdef SWIFT_USE_FLOAT
{ glVertex3f( val[0], val[1], val[2] ); }
#else
{ glVertex3d( val[0], val[1], val[2] ); }
#endif

inline void SWIFT_Triple::Send_NCoords_To_OpenGL( ) const
#ifdef SWIFT_USE_FLOAT
{ glNormal3f( val[0], val[1], val[2] ); }
#else
{ glNormal3d( val[0], val[1], val[2] ); }
#endif
inline void SWIFT_Triple::Normalize( )
{ *this /= this->Length(); }

inline void SWIFT_Triple::Zero( )
{ val[0] = val[1] = val[2] = 0.0; }

inline void SWIFT_Triple::Identity( )
{ Zero(); }

inline void SWIFT_Triple::Negate( )
{ val[0] = -val[0]; val[1] = -val[1]; val[2] = -val[2]; }

inline void SWIFT_Triple::Add( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 )
{ val[0] += v1; val[1] += v2; val[2] += v3; }

inline void SWIFT_Triple::Sub( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 )
{ val[0] -= v1; val[1] -= v2; val[2] -= v3; }

inline void SWIFT_Triple::Scale( SWIFT_Real v1, SWIFT_Real v2, SWIFT_Real v3 )
{ val[0] *= v1; val[1] *= v2; val[2] *= v3; }


inline void SWIFT_Triple::Reflect_XY( ) { val[2] = -val[2]; }

inline void SWIFT_Triple::Reflect_XZ( ) { val[1] = -val[1]; }

inline void SWIFT_Triple::Reflect_YZ( ) { val[0] = -val[0]; }

inline void SWIFT_Triple::Reflect( const SWIFT_Triple& v )
{ (*this) = (*this) - 2.0 * (v * (*this)) * v; }

// Transforms this set of coordinates from the given frame to the world frame.
inline void SWIFT_Triple::Transform( const SWIFT_Triple& x,
                                     const SWIFT_Triple& y,
                                     const SWIFT_Triple& z )
{ *this = val[0] * x + val[1] * y + val[2] * z; }

inline void SWIFT_Triple::operator+=( const SWIFT_Triple& t )
{ val[0] += t.X(); val[1] += t.Y(); val[2] += t.Z(); }

inline void SWIFT_Triple::operator-=( const SWIFT_Triple& t )
{ val[0] -= t.X(); val[1] -= t.Y(); val[2] -= t.Z(); }

inline void SWIFT_Triple::operator*=( SWIFT_Real s )
{ val[0] *= s; val[1] *= s; val[2] *= s; }

inline void SWIFT_Triple::operator/=( SWIFT_Real s )
{ const SWIFT_Real _s = 1.0/s; val[0] *= _s; val[1] *= _s; val[2] *= _s; }

inline void SWIFT_Triple::operator^=( const SWIFT_Matrix33& m )
{
    SWIFT_Real temp0 = val[0]*m.val[0] + val[1]*m.val[1] + val[2]*m.val[2];
    SWIFT_Real temp1 = val[0]*m.val[3] + val[1]*m.val[4] + val[2]*m.val[5];
    val[2] = val[0]*m.val[6] + val[1]*m.val[7] + val[2]*m.val[8];
    val[0] = temp0;
    val[1] = temp1;
}

inline void SWIFT_Triple::operator^=( const SWIFT_Transformation& m )
{
    SWIFT_Real temp0 = val[0]*m.R.val[0] + val[1]*m.R.val[1] +
                       val[2]*m.R.val[2] + m.T.val[0];
    SWIFT_Real temp1 = val[0]*m.R.val[3] + val[1]*m.R.val[4] +
                       val[2]*m.R.val[5] + m.T.val[1];
    val[2] = val[0]*m.R.val[6] + val[1]*m.R.val[7] + val[2]*m.R.val[8] +
                                                     m.T.val[2];
    val[0] = temp0;
    val[1] = temp1;
}

inline void SWIFT_Triple::operator&=( const SWIFT_Transformation& m )
{
    SWIFT_Real temp0 = val[0]*m.R.val[0] + val[1]*m.R.val[1] +
                       val[2]*m.R.val[2];
    SWIFT_Real temp1 = val[0]*m.R.val[3] + val[1]*m.R.val[4] +
                       val[2]*m.R.val[5];
    val[2] = val[0]*m.R.val[6] + val[1]*m.R.val[7] + val[2]*m.R.val[8];
    val[0] = temp0;
    val[1] = temp1;
}

inline bool SWIFT_Triple::operator==( const SWIFT_Triple& t ) const
{ return val[0] == t.X() && val[1] == t.Y() && val[2] == t.Z(); }

inline bool SWIFT_Triple::operator!=( const SWIFT_Triple& t ) const
{ return val[0] != t.X() || val[1] != t.Y() || val[2] != t.Z(); }

inline bool SWIFT_Triple::operator<=( const SWIFT_Triple& t ) const
{ return val[0] <= t.X() && val[1] <= t.Y() && val[2] <= t.Z(); }

inline bool SWIFT_Triple::operator>=( const SWIFT_Triple& t ) const
{ return val[0] >= t.X() && val[1] >= t.Y() && val[2] >= t.Z(); }

inline SWIFT_Triple SWIFT_Triple::Orthogonal( bool norm ) const
{
    SWIFT_Triple result( *this );

    // Try to project this triple to make it as orthogonal as possible
    result.Set_Largest_Coord( 0.0 );
#ifdef SWIFT_USE_FLOAT
    if( result.Length_Sq() < EPSILON7 ) {
#else
    if( result.Length_Sq() < EPSILON10 ) {
#endif
        // new triple is too small
        result = (*this) + SWIFT_Triple( 1.0, 1.0, 1.0 );
        result.Set_Largest_Coord( 0.0 );
    }

    // new triple is now not equal to this triple in direction and neither
    // triple is zero.
    result = result % (*this);

    if( norm ) {
        result.Normalize( );
    }

    return result;
}

inline SWIFT_Matrix33 SWIFT_Triple::To_Cross_Prod_Matrix( ) const
{
    return SWIFT_Matrix33( 0.0, -val[2], val[1], val[2], 0.0, -val[0],
                           -val[1], val[0], 0.0 );
}


//////////////////////////////////////////////////////////////////////////////
// SWIFT_Matrix33 member functions inline definitions
//////////////////////////////////////////////////////////////////////////////
inline void SWIFT_Matrix33::Get_Value( SWIFT_Real v[] ) const
{
    v[0] = val[0], v[1] = val[1], v[2] = val[2];
    v[3] = val[3], v[4] = val[4], v[5] = val[5];
    v[6] = val[6], v[7] = val[7], v[8] = val[8];
}

inline void SWIFT_Matrix33::Get_Value( SWIFT_Real v[3][3] ) const
{
    v[0][0] = val[0], v[0][1] = val[1], v[0][2] = val[2];
    v[1][0] = val[3], v[1][1] = val[4], v[1][2] = val[5];
    v[2][0] = val[6], v[2][1] = val[7], v[2][2] = val[8];
}

inline void SWIFT_Matrix33::Set_Value( const SWIFT_Real v[] )
{
    val[0] = v[0], val[1] = v[1], val[2] = v[2];
    val[3] = v[3], val[4] = v[4], val[5] = v[5];
    val[6] = v[6], val[7] = v[7], val[8] = v[8];
}

inline void SWIFT_Matrix33::Set_Value( const SWIFT_Real v[3][3] )
{
    val[0] = v[0][0], val[1] = v[0][1], val[2] = v[0][2];
    val[3] = v[1][0], val[4] = v[1][1], val[5] = v[1][2];
    val[6] = v[2][0], val[7] = v[2][1], val[8] = v[2][2];
}

inline void SWIFT_Matrix33::Zero( )
{
    val[0] = 0.0; val[1] = 0.0; val[2] = 0.0;
    val[3] = 0.0; val[4] = 0.0; val[5] = 0.0;
    val[6] = 0.0; val[7] = 0.0; val[8] = 0.0;
}

inline void SWIFT_Matrix33::Identity( )
{
    val[0] = 1.0; val[1] = 0.0; val[2] = 0.0;
    val[3] = 0.0; val[4] = 1.0; val[5] = 0.0;
    val[6] = 0.0; val[7] = 0.0; val[8] = 1.0;
}

void SWIFT_Matrix33::Transpose( )
{
    SWIFT_Real temp;

    temp = val[1];
    val[1] = val[3];
    val[3] = temp;

    temp = val[2];
    val[2] = val[6];
    val[6] = temp;

    temp = val[5];
    val[5] = val[7];
    val[7] = temp;
}   

void SWIFT_Matrix33::operator+=( const SWIFT_Matrix33& m )
{
    SWIFT_Real v[9];
    m.Get_Value( v );
    val[0] += v[0]; val[1] += v[1]; val[2] += v[2];
    val[3] += v[3]; val[4] += v[4]; val[5] += v[5];
    val[6] += v[6]; val[7] += v[7]; val[8] += v[8];
}

void SWIFT_Matrix33::operator-=( const SWIFT_Matrix33& m )
{
    SWIFT_Real v[9];
    m.Get_Value( v );
    val[0] -= v[0]; val[1] -= v[1]; val[2] -= v[2];
    val[3] -= v[3]; val[4] -= v[4]; val[5] -= v[5];
    val[6] -= v[6]; val[7] -= v[7]; val[8] -= v[8];
}

void SWIFT_Matrix33::operator*=( SWIFT_Real s )
{
    val[0] *= s; val[1] *= s; val[2] *= s; val[3] *= s;
    val[4] *= s; val[5] *= s; val[6] *= s; val[7] *= s;
    val[8] *= s;
}

void SWIFT_Matrix33::operator/=( SWIFT_Real s )
{
    const SWIFT_Real _s = 1.0/s;
    val[0] *= _s; val[1] *= _s; val[2] *= _s; val[3] *= _s; val[4] *= _s;
    val[5] *= _s; val[6] *= _s; val[7] *= _s; val[8] *= _s;
}

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Transformation member functions inline definitions
//////////////////////////////////////////////////////////////////////////////

// Transform from LCS1 with m1 = LCS1->WCS to LCS2 where m2 = LCS2->WCS.
inline void SWIFT_Transformation::Transform_From_To(
            const SWIFT_Transformation& m1, const SWIFT_Transformation& m2 )
{
    R.val[0] = (m1.T.val[0] - m2.T.val[0]);
    R.val[1] = (m1.T.val[1] - m2.T.val[1]);
    R.val[2] = (m1.T.val[2] - m2.T.val[2]);
    T.val[0] = R.val[0]*m2.R.val[0] + R.val[1] * m2.R.val[3] +
                                      R.val[2] * m2.R.val[6];
    T.val[1] = R.val[0]*m2.R.val[1] + R.val[1] * m2.R.val[4] +
                                      R.val[2] * m2.R.val[7];
    T.val[2] = R.val[0]*m2.R.val[2] + R.val[1] * m2.R.val[5] +
                                      R.val[2] * m2.R.val[8];

    R.val[0] = m1.R.val[0]*m2.R.val[0] + m1.R.val[3]*m2.R.val[3] +
                                         m1.R.val[6]*m2.R.val[6];
    R.val[1] = m1.R.val[1]*m2.R.val[0] + m1.R.val[4]*m2.R.val[3] +
                                         m1.R.val[7]*m2.R.val[6];
    R.val[2] = m1.R.val[2]*m2.R.val[0] + m1.R.val[5]*m2.R.val[3] +
                                         m1.R.val[8]*m2.R.val[6];

    R.val[3] = m1.R.val[0]*m2.R.val[1] + m1.R.val[3]*m2.R.val[4] +
                                         m1.R.val[6]*m2.R.val[7];
    R.val[4] = m1.R.val[1]*m2.R.val[1] + m1.R.val[4]*m2.R.val[4] +
                                         m1.R.val[7]*m2.R.val[7];
    R.val[5] = m1.R.val[2]*m2.R.val[1] + m1.R.val[5]*m2.R.val[4] +
                                         m1.R.val[8]*m2.R.val[7];

    R.val[6] = m1.R.val[0]*m2.R.val[2] + m1.R.val[3]*m2.R.val[5] +
                                         m1.R.val[6]*m2.R.val[8];
    R.val[7] = m1.R.val[1]*m2.R.val[2] + m1.R.val[4]*m2.R.val[5] +
                                         m1.R.val[7]*m2.R.val[8];
    R.val[8] = m1.R.val[2]*m2.R.val[2] + m1.R.val[5]*m2.R.val[5] +
                                         m1.R.val[8]*m2.R.val[8];
}

// Transform from LCS1 with this = LCS1->WCS to LCS2 where m = LCS2->WCS.
inline void SWIFT_Transformation::Transform_To( const SWIFT_Transformation& m )
{
    const SWIFT_Triple t = T - m.T;
    T.val[0] = t.val[0]*m.R.val[0] + t.val[1] * m.R.val[3] +
                                     t.val[2] * m.R.val[6];
    T.val[1] = t.val[0]*m.R.val[1] + t.val[1] * m.R.val[4] +
                                     t.val[2] * m.R.val[7];
    T.val[2] = t.val[0]*m.R.val[2] + t.val[1] * m.R.val[5] +
                                     t.val[2] * m.R.val[8];
    const SWIFT_Matrix33 m33( R );
    R.val[0] = m33.val[0]*m.R.val[0] + m33.val[3]*m.R.val[3] +
                                       m33.val[6]*m.R.val[6];
    R.val[1] = m33.val[1]*m.R.val[0] + m33.val[4]*m.R.val[3] +
                                       m33.val[7]*m.R.val[6];
    R.val[2] = m33.val[2]*m.R.val[0] + m33.val[5]*m.R.val[3] +
                                       m33.val[8]*m.R.val[6];

    R.val[3] = m33.val[0]*m.R.val[1] + m33.val[3]*m.R.val[4] +
                                       m33.val[6]*m.R.val[7];
    R.val[4] = m33.val[1]*m.R.val[1] + m33.val[4]*m.R.val[4] +
                                       m33.val[7]*m.R.val[7];
    R.val[5] = m33.val[2]*m.R.val[1] + m33.val[5]*m.R.val[4] +
                                       m33.val[8]*m.R.val[7];

    R.val[6] = m33.val[0]*m.R.val[2] + m33.val[3]*m.R.val[5] +
                                       m33.val[6]*m.R.val[8];
    R.val[7] = m33.val[1]*m.R.val[2] + m33.val[4]*m.R.val[5] +
                                       m33.val[7]*m.R.val[8];
    R.val[8] = m33.val[2]*m.R.val[2] + m33.val[5]*m.R.val[5] +
                                       m33.val[8]*m.R.val[8];
}

// Transform from CS1 to CS3 with m = CS1->CS2 and this = CS2->CS3.
inline void SWIFT_Transformation::Pre_Compose( const SWIFT_Transformation& m )
{
    T += R * m.T;
    R = R * m.R;
}

// Transform from CS1 to CS3 with m1 = CS1->CS2 and m2 = CS2->CS3.
inline void SWIFT_Transformation::Compose( const SWIFT_Transformation& m1,
                                           const SWIFT_Transformation& m2 )
{
    R = m2.R * m1.R;
    T = m2.R * m1.T + m2.T;
}

// Transform from CS1 to CS3 with this = CS1->CS2 and s = CS2->CS3.
inline void SWIFT_Transformation::Scale( const SWIFT_Triple& s )
{
    R.val[0] *= s.X(); R.val[1] *= s.X(); R.val[2] *= s.X();
    R.val[3] *= s.Y(); R.val[4] *= s.Y(); R.val[5] *= s.Y();
    R.val[6] *= s.Z(); R.val[7] *= s.Z(); R.val[8] *= s.Z();
    T.val[0] *= s.X(); T.val[1] *= s.Y(); T.val[2] *= s.Z();
}

inline void SWIFT_Transformation::Invert( const SWIFT_Transformation& m )
{
    R.val[0] = m.R.val[0];
    R.val[1] = m.R.val[3];
    R.val[2] = m.R.val[6];
    R.val[3] = m.R.val[1];
    R.val[4] = m.R.val[4];
    R.val[5] = m.R.val[7];
    R.val[6] = m.R.val[2];
    R.val[7] = m.R.val[5];
    R.val[8] = m.R.val[8];
    T.val[0] = -(m.R.val[0]*m.T.val[0] + m.R.val[3]*m.T.val[1] +
                                         m.R.val[6]*m.T.val[2]);
    T.val[1] = -(m.R.val[1]*m.T.val[0] + m.R.val[4]*m.T.val[1] +
                                         m.R.val[7]*m.T.val[2]);
    T.val[2] = -(m.R.val[2]*m.T.val[0] + m.R.val[5]*m.T.val[1] +
                                         m.R.val[8]*m.T.val[2]);
}

inline void SWIFT_Transformation::Set_Value( const SWIFT_Real v[] )
{
    R.val[0] = v[0], R.val[1] = v[1], R.val[2] = v[2];
    R.val[3] = v[4], R.val[4] = v[5], R.val[5] = v[6];
    R.val[6] = v[8], R.val[7] = v[9], R.val[8] = v[10];
    T.val[0] = v[3];
    T.val[1] = v[7];
    T.val[2] = v[11];
}

inline void SWIFT_Transformation::Set_Value( const SWIFT_Real r[],
                                             const SWIFT_Real t[] )
{
    R.val[0] = r[0], R.val[1] = r[1], R.val[2] = r[2];
    R.val[3] = r[3], R.val[4] = r[4], R.val[5] = r[5];
    R.val[6] = r[6], R.val[7] = r[7], R.val[8] = r[8];
    T.val[0] = t[0];
    T.val[1] = t[1];
    T.val[2] = t[2];
}


//////////////////////////////////////////////////////////////////////////////
// Auxiliary operator functions inline definitions
//////////////////////////////////////////////////////////////////////////////

// Triple negation
inline SWIFT_Triple operator-( const SWIFT_Triple& t )
{ return SWIFT_Triple( -t.val[0], -t.val[1], -t.val[2] ); }

// Triple addition
inline SWIFT_Triple operator+( const SWIFT_Triple& t1, const SWIFT_Triple& t2 )
{ return SWIFT_Triple( t1.val[0] + t2.val[0], t1.val[1] + t2.val[1],
                       t1.val[2] + t2.val[2] ); }

// Triple subtraction
inline SWIFT_Triple operator-( const SWIFT_Triple& t1, const SWIFT_Triple& t2 )
{ return SWIFT_Triple( t1.val[0] - t2.val[0], t1.val[1] - t2.val[1],
                       t1.val[2] - t2.val[2] ); }

// Scalar multiplication
inline SWIFT_Triple operator*( SWIFT_Real s, const SWIFT_Triple& t )
{ return SWIFT_Triple( s * t.val[0], s * t.val[1], s * t.val[2] ); }

// Scalar multiplication
inline SWIFT_Triple operator*( const SWIFT_Triple& t, SWIFT_Real s )
{ return SWIFT_Triple( s * t.val[0], s * t.val[1], s * t.val[2] ); }

// Scalar division
inline SWIFT_Triple operator/( const SWIFT_Triple& t, SWIFT_Real s )
{ const SWIFT_Real _s = 1.0/s;
  return SWIFT_Triple( t.val[0] * _s, t.val[1] * _s, t.val[2] * _s ); }

// Dot product
inline SWIFT_Real operator*( const SWIFT_Triple& t1, const SWIFT_Triple& t2 )
{ return t1.val[0]*t2.val[0] + t1.val[1]*t2.val[1] + t1.val[2]*t2.val[2]; }

// Cross product
inline SWIFT_Triple operator%( const SWIFT_Triple& t1, const SWIFT_Triple& t2 )
{
    return SWIFT_Triple( t1.val[1] * t2.val[2] - t1.val[2] * t2.val[1],
                         t1.val[2] * t2.val[0] - t1.val[0] * t2.val[2],
                         t1.val[0] * t2.val[1] - t1.val[1] * t2.val[0] );
}


// Matrix negation
inline SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m )
{
    return SWIFT_Matrix33( -m.val[0], -m.val[1], -m.val[2],
                           -m.val[3], -m.val[4], -m.val[5],
                           -m.val[6], -m.val[7], -m.val[8] );
}

// Matrix addition
inline SWIFT_Matrix33 operator+( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 )
{
    return SWIFT_Matrix33( m1.val[0] + m2.val[0],
                           m1.val[1] + m2.val[1],
                           m1.val[2] + m2.val[2],
                           m1.val[3] + m2.val[3],
                           m1.val[4] + m2.val[4],
                           m1.val[5] + m2.val[5],
                           m1.val[6] + m2.val[6],
                           m1.val[7] + m2.val[7],
                           m1.val[8] + m2.val[8] );
}

// Matrix addition
inline SWIFT_Matrix33 operator-( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 )
{
    return SWIFT_Matrix33( m1.val[0] - m2.val[0],
                           m1.val[1] - m2.val[1],
                           m1.val[2] - m2.val[2],
                           m1.val[3] - m2.val[3],
                           m1.val[4] - m2.val[4],
                           m1.val[5] - m2.val[5],
                           m1.val[6] - m2.val[6],
                           m1.val[7] - m2.val[7],
                           m1.val[8] - m2.val[8] );
}

// Scalar multiplication
inline SWIFT_Matrix33 operator*( SWIFT_Real s, const SWIFT_Matrix33& m )
{
    return SWIFT_Matrix33( s * m.val[0], s * m.val[1], s * m.val[2],
                           s * m.val[3], s * m.val[4], s * m.val[5],
                           s * m.val[6], s * m.val[7], s * m.val[8] );
}

// Scalar multiplication
inline SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m, SWIFT_Real s )
{
    return SWIFT_Matrix33( s * m.val[0], s * m.val[1], s * m.val[2],
                           s * m.val[3], s * m.val[4], s * m.val[5],
                           s * m.val[6], s * m.val[7], s * m.val[8] );
}

// Scalar division
inline SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m, SWIFT_Real s )
{
    s = 1.0/s;
    return SWIFT_Matrix33( s * m.val[0], s * m.val[1], s * m.val[2],
                           s * m.val[3], s * m.val[4], s * m.val[5],
                           s * m.val[6], s * m.val[7], s * m.val[8] );
}

// Matrix vector right multiply
inline SWIFT_Triple operator*( const SWIFT_Matrix33& m, const SWIFT_Triple& t )
{
    return SWIFT_Triple(
                   t.val[0]*m.val[0] + t.val[1]*m.val[1] + t.val[2]*m.val[2],
                   t.val[0]*m.val[3] + t.val[1]*m.val[4] + t.val[2]*m.val[5],
                   t.val[0]*m.val[6] + t.val[1]*m.val[7] + t.val[2]*m.val[8] );
}

// Matrix transpose-Vector multiplication
inline SWIFT_Triple operator%( const SWIFT_Matrix33& m, const SWIFT_Triple& t )
{
    return SWIFT_Triple(
                   t.val[0]*m.val[0] + t.val[1]*m.val[3] + t.val[2]*m.val[6],
                   t.val[0]*m.val[1] + t.val[1]*m.val[4] + t.val[2]*m.val[7],
                   t.val[0]*m.val[2] + t.val[1]*m.val[5] + t.val[2]*m.val[8] );
}

// Matrix multiplication = m1 * m2
inline SWIFT_Matrix33 operator*( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 )
{
    return SWIFT_Matrix33(
            m1.val[0]*m2.val[0] + m1.val[1]*m2.val[3] + m1.val[2]*m2.val[6],
            m1.val[0]*m2.val[1] + m1.val[1]*m2.val[4] + m1.val[2]*m2.val[7],
            m1.val[0]*m2.val[2] + m1.val[1]*m2.val[5] + m1.val[2]*m2.val[8],
            m1.val[3]*m2.val[0] + m1.val[4]*m2.val[3] + m1.val[5]*m2.val[6],
            m1.val[3]*m2.val[1] + m1.val[4]*m2.val[4] + m1.val[5]*m2.val[7],
            m1.val[3]*m2.val[2] + m1.val[4]*m2.val[5] + m1.val[5]*m2.val[8],
            m1.val[6]*m2.val[0] + m1.val[7]*m2.val[3] + m1.val[8]*m2.val[6],
            m1.val[6]*m2.val[1] + m1.val[7]*m2.val[4] + m1.val[8]*m2.val[7],
            m1.val[6]*m2.val[2] + m1.val[7]*m2.val[5] + m1.val[8]*m2.val[8] );
}

// Matrix multiplication with transpose of second = m1 * m2^T
inline SWIFT_Matrix33 operator/( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 )
{
    return SWIFT_Matrix33(
            m1.val[0]*m2.val[0] + m1.val[1]*m2.val[1] + m1.val[2]*m2.val[2],
            m1.val[0]*m2.val[3] + m1.val[1]*m2.val[4] + m1.val[2]*m2.val[5],
            m1.val[0]*m2.val[6] + m1.val[1]*m2.val[7] + m1.val[2]*m2.val[8],
            m1.val[3]*m2.val[0] + m1.val[4]*m2.val[1] + m1.val[5]*m2.val[2],
            m1.val[3]*m2.val[3] + m1.val[4]*m2.val[4] + m1.val[5]*m2.val[5],
            m1.val[3]*m2.val[6] + m1.val[4]*m2.val[7] + m1.val[5]*m2.val[8],
            m1.val[6]*m2.val[0] + m1.val[7]*m2.val[1] + m1.val[8]*m2.val[2],
            m1.val[6]*m2.val[3] + m1.val[7]*m2.val[4] + m1.val[8]*m2.val[5],
            m1.val[6]*m2.val[6] + m1.val[7]*m2.val[7] + m1.val[8]*m2.val[8] );
}

// Matrix multiplication with transpose of first = m1^T * m2
inline SWIFT_Matrix33 operator%( const SWIFT_Matrix33& m1,
                                 const SWIFT_Matrix33& m2 )
{
    return SWIFT_Matrix33(
            m1.val[0]*m2.val[0] + m1.val[3]*m2.val[3] + m1.val[6]*m2.val[6],
            m1.val[0]*m2.val[1] + m1.val[3]*m2.val[4] + m1.val[6]*m2.val[7],
            m1.val[0]*m2.val[2] + m1.val[3]*m2.val[5] + m1.val[6]*m2.val[8],
            m1.val[1]*m2.val[0] + m1.val[4]*m2.val[3] + m1.val[7]*m2.val[6],
            m1.val[1]*m2.val[1] + m1.val[4]*m2.val[4] + m1.val[7]*m2.val[7],
            m1.val[1]*m2.val[2] + m1.val[4]*m2.val[5] + m1.val[7]*m2.val[8],
            m1.val[2]*m2.val[0] + m1.val[5]*m2.val[3] + m1.val[8]*m2.val[6],
            m1.val[2]*m2.val[1] + m1.val[5]*m2.val[4] + m1.val[8]*m2.val[7],
            m1.val[2]*m2.val[2] + m1.val[5]*m2.val[5] + m1.val[8]*m2.val[8] );
}

// Matrix point right multiply
inline SWIFT_Triple operator*( const SWIFT_Transformation& m,
                               const SWIFT_Triple& t )
{
    return SWIFT_Triple(
        t.val[0]*m.R.val[0] + t.val[1]*m.R.val[1] + t.val[2]*m.R.val[2] +
                                                    m.T.val[0],
        t.val[0]*m.R.val[3] + t.val[1]*m.R.val[4] + t.val[2]*m.R.val[5] +
                                                    m.T.val[1],
        t.val[0]*m.R.val[6] + t.val[1]*m.R.val[7] + t.val[2]*m.R.val[8] +
                                                    m.T.val[2] );
}

// Matrix vector right multiply
inline SWIFT_Triple operator&( const SWIFT_Transformation& m,
                               const SWIFT_Triple& t )
{
    return SWIFT_Triple(
        t.val[0]*m.R.val[0] + t.val[1]*m.R.val[1] + t.val[2]*m.R.val[2],
        t.val[0]*m.R.val[3] + t.val[1]*m.R.val[4] + t.val[2]*m.R.val[5],
        t.val[0]*m.R.val[6] + t.val[1]*m.R.val[7] + t.val[2]*m.R.val[8] );
}

inline ostream& operator<<( ostream& out, const SWIFT_Triple& t )
{
    return out << "[" << t.val[0] << "," << t.val[1] << "," << t.val[2] << "]";
}


inline ostream& operator<<( ostream& out, const SWIFT_Matrix33& m )
{
    return out << "[" << m.val[0] << "," << m.val[1] << "," << m.val[2] << "|"
               << endl << m.val[3] << "," << m.val[4] << "," << m.val[5] << "|"
               << endl << m.val[6] << "," << m.val[7] << "," << m.val[8] << "]";
}

inline ostream& operator<<( ostream& out, const SWIFT_Transformation& m )
{
    return out << "R = ["
               << m.R.val[0] << "," << m.R.val[1] << "," << m.R.val[2]
               << "     " << endl
               << m.R.val[3] << "," << m.R.val[4] << "," << m.R.val[5]
               << "     " << endl
               << m.R.val[6] << "," << m.R.val[7] << "," << m.R.val[8]
               << "]" << endl
               << "T = [" << m.T.val[0] << ","
                          << m.T.val[1] << ","
                          << m.T.val[2] << "]";
}
#endif


