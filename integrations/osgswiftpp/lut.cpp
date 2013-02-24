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
// lut.cpp
//
//////////////////////////////////////////////////////////////////////////////


#include <math.h>

#include <SWIFT_config.h>
#include <SWIFT_common.h>
#include <SWIFT_linalg.h>
#include <SWIFT_mesh.h>
#include <SWIFT_lut.h>

//////////////////////////////////////////////////////////////////////////////
// Global lookup table definitions
//////////////////////////////////////////////////////////////////////////////

const SWIFT_Real LOOKUP_TABLE_RADIUS = 2.0;

static const int north[3] = {112, 480, 1984};
static const int south[3] = {113, 481, 1985};
static const int bases[3] = {0, 4, 12};
static const SWIFT_Real aincr[3] = { 0.125*PI, 0.0625*PI, 0.03125*PI };
static const SWIFT_Real tan_xpi_y[28] = {
    tan( 0.0625*PI ), tan( 0.1875*PI ), tan( 0.3125*PI ), tan( 0.4375*PI ),

    tan( 0.03125*PI ), tan( 0.09375*PI ), tan( 0.15625*PI ),
    tan( 0.21875*PI ), tan( 0.28125*PI ), tan( 0.34375*PI ),
    tan( 0.40625*PI ), tan( 0.46875*PI ),

    tan( 0.015625*PI ), tan( 0.046875*PI ), tan( 0.078125*PI ),
    tan( 0.109375*PI ), tan( 0.140625*PI ), tan( 0.171875*PI ),
    tan( 0.203125*PI ), tan( 0.234375*PI ), tan( 0.265625*PI ),
    tan( 0.296875*PI ), tan( 0.328125*PI ), tan( 0.359375*PI ),
    tan( 0.390625*PI ), tan( 0.421875*PI ), tan( 0.453125*PI ),
    tan( 0.484375*PI ) };
static const SWIFT_Real tan_sq_xpi_y[28] = {
  tan_xpi_y[0] * tan_xpi_y[0], tan_xpi_y[1] * tan_xpi_y[1],
  tan_xpi_y[2] * tan_xpi_y[2], tan_xpi_y[3] * tan_xpi_y[3],

  tan_xpi_y[4] * tan_xpi_y[4], tan_xpi_y[5] * tan_xpi_y[5],
  tan_xpi_y[6] * tan_xpi_y[6], tan_xpi_y[7] * tan_xpi_y[7],
  tan_xpi_y[8] * tan_xpi_y[8], tan_xpi_y[9] * tan_xpi_y[9],
  tan_xpi_y[10] * tan_xpi_y[10], tan_xpi_y[11] * tan_xpi_y[11],

  tan_xpi_y[12] * tan_xpi_y[12], tan_xpi_y[13] * tan_xpi_y[13],
  tan_xpi_y[14] * tan_xpi_y[14], tan_xpi_y[15] * tan_xpi_y[15],
  tan_xpi_y[16] * tan_xpi_y[16], tan_xpi_y[17] * tan_xpi_y[17],
  tan_xpi_y[18] * tan_xpi_y[18], tan_xpi_y[19] * tan_xpi_y[19],
  tan_xpi_y[20] * tan_xpi_y[20], tan_xpi_y[21] * tan_xpi_y[21],
  tan_xpi_y[22] * tan_xpi_y[22], tan_xpi_y[23] * tan_xpi_y[23],
  tan_xpi_y[24] * tan_xpi_y[24], tan_xpi_y[25] * tan_xpi_y[25],
  tan_xpi_y[26] * tan_xpi_y[26], tan_xpi_y[27] * tan_xpi_y[27] };

SWIFT_Triple** Compute_Sphere_Pts( )
{
    SWIFT_Triple** result = new SWIFT_Triple*[3];
    int i, j, idx;

    for( idx = 0; idx < 3; idx++ ) {
        SWIFT_Triple* sub_result = new SWIFT_Triple[south[idx]+1];
        // Sweep from north to south
        for( i = (4<<idx)-1; i >= 1-(4<<idx); i-- ) {
            const SWIFT_Real al = (SWIFT_Real)i * aincr[idx];
            const SWIFT_Real cos_al = cos( al );
            const SWIFT_Real sin_al = sin( al );
            for( j = 0; j < (16<<idx); j++ ) {
                const SWIFT_Real az = (SWIFT_Real)j * aincr[idx];
                sub_result[ (((4<<idx)-1-i)<<(4+idx))+j ].Set_Value(
                                cos_al * cos(az), cos_al * sin(az), sin_al );
            }
        }

        sub_result[north[idx]].Set_Value( 0.0, 0.0, 1.0 );
        sub_result[south[idx]].Set_Value( 0.0, 0.0, -1.0 );

        result[idx] = sub_result;
    }

    return result;
}

SWIFT_Triple** sphere_pts = Compute_Sphere_Pts();

//////////////////////////////////////////////////////////////////////////////
// SWIFT_Object public functions
//////////////////////////////////////////////////////////////////////////////

void SWIFT_Lookup_Table::Create( SWIFT_BV* p )
{
    // Compute the lookup table.  Find object points that are nearest to points
    // on the sphere of a radius a factor bigger than the object radius.
    int i;
    SWIFT_Real lut_radius = LOOKUP_TABLE_RADIUS * p->Radius();

    type = LUT_22_5;

    lut.Create( south[(int)type]+1 );

    for( i = 0; i < north[(int)type]; i++ ) {
        lut[i] = p->Close_Edge(
            (lut_radius * sphere_pts[(int)type][i]) + p->Center_Of_Mass() );
    }

    lut[north[(int)type]] = p->Close_Edge(
                  SWIFT_Triple( 0.0, 0.0, lut_radius ) + p->Center_Of_Mass() );
    lut[south[(int)type]] = p->Close_Edge(
                  SWIFT_Triple( 0.0, 0.0, -lut_radius ) + p->Center_Of_Mass() );
}

#ifdef SWIFT_DEBUG
void SWIFT_Lookup_Table::Dump( )
{
    int i;
    cerr << "Dumping LUT of type " << (int)type << endl;
    for( i = 0; i < lut.Length(); i++ ) {
        cerr << (void*)lut[i] << " ";
    }
    cerr << endl << endl;
}
#endif

SWIFT_Tri_Edge* SWIFT_Lookup_Table::Lookup_Internal( const SWIFT_Triple& dir )
{
    const SWIFT_Real xy_sq = dir.X() * dir.X() + dir.Y() * dir.Y();
    const SWIFT_Real z_sq = dir.Z() * dir.Z();
    const int idx = (int)type;
    const int tan_len_shift = 4 + idx;
    const int tan_len = 4 << idx;
    const SWIFT_Real* tan_sq_xpi_y_base = tan_sq_xpi_y+bases[idx];
    const SWIFT_Real* tan_xpi_y_base = tan_xpi_y+bases[idx];
    int i = (2 << idx) - 1;
    int j = i;
    int stride = 1 << idx;

    while( stride != 0 ) {
        if( z_sq < tan_sq_xpi_y_base[i] * xy_sq ) {
            i -= stride;
        } else {
            i += stride;
        }
        if( fabs(dir.Y()) < tan_xpi_y_base[j] * fabs(dir.X()) ) {
            j -= stride;
        } else {
            j += stride;
        }
        stride >>= 1;
    }

    if( z_sq > tan_sq_xpi_y_base[i] * xy_sq ) {
        i++;
    }

    if( i == tan_len - 1 && z_sq > tan_sq_xpi_y_base[i] * xy_sq ) {
        return lut[(dir.Z() > 0.0 ? north[idx] : south[idx])];
    } else {
        i = (dir.Z() > 0.0 ? (tan_len - i - 1) : (tan_len + i - 1));
        i <<= tan_len_shift;
    }

    if( fabs(dir.Y()) > tan_xpi_y_base[j] * fabs(dir.X()) ) {
        j++;
    }

    if( j == tan_len - 1 && fabs(dir.Y()) > tan_xpi_y_base[j] * fabs(dir.X())
    ) {
        return lut[(dir.Y() > 0.0 ? i+tan_len : i+(3 << (tan_len_shift-2)) )];
    } else if( j == 0 ) {
        return lut[(dir.X() > 0.0 ? i : i+(tan_len<<1))];
    } else {
        return lut[(dir.Y() > 0.0 ?
                   (dir.X() > 0.0 ? i+j : i+(tan_len<<1)-j) :
                   (dir.X() > 0.0 ? i+(tan_len<<2)-j : i+(tan_len<<1)+j) )];
    }
}


