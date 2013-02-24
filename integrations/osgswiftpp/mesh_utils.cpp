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
// mesh_utils.cpp
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

char options[200];

///////////////////////////////////////////////////////////////////////////////
// Static functions
///////////////////////////////////////////////////////////////////////////////

// This code is from Numerical Recipes and was gotten from the RAPID system.
inline static void Rotate( SWIFT_Real a[3][3], int i, int j, int k, int l,
                      SWIFT_Real tau, SWIFT_Real s,
                      SWIFT_Real& g, SWIFT_Real& h )
{
    g = a[i][j]; h = a[k][l]; a[i][j] = g-s*(h+g*tau); a[k][l]=h+s*(g-h*tau);
}

inline static void Meigen( SWIFT_Real vout[3][3], SWIFT_Real dout[3],
                           SWIFT_Real a[3][3] )
{
    int n = 3;
    int j,iq,ip,i;
    SWIFT_Real tresh,theta,tau,t,sm,s,h,g,c;
    int nrot;
    SWIFT_Real b[3];
    SWIFT_Real z[3];
    SWIFT_Real v[3][3];
    SWIFT_Real d[3];

    v[0][0] = v[1][1] = v[2][2] = 1.0;
    v[0][1] = v[1][2] = v[2][0] = 0.0;
    v[0][2] = v[1][0] = v[2][1] = 0.0;
    for( ip = 0; ip < n; ip++ ) {
        b[ip] = a[ip][ip];
        d[ip] = a[ip][ip];
        z[ip] = 0.0;
    }

    nrot = 0;

    for( i = 0; i < 50; i++ ) {
        sm=0.0;
        for( ip = 0;ip < n; ip++ ) {
            for( iq = ip+1; iq < n; iq++ ) {
                sm += fabs(a[ip][iq]);
            }
        }
        if( sm == 0.0 ) {
            vout[0][0] = v[0][0];  vout[0][1] = v[0][1];  vout[0][2] = v[0][2];
            vout[1][0] = v[1][0];  vout[1][1] = v[1][1];  vout[1][2] = v[1][2];
            vout[2][0] = v[2][0];  vout[2][1] = v[2][1];  vout[2][2] = v[2][2];
            dout[0] = d[0];  dout[1] = d[1];  dout[2] = d[2];
            return;
        }

        if( i < 3 ) {
            tresh=(SWIFT_Real)0.2*sm/(n*n);
        } else {
            tresh=0.0;
        }

        for(ip=0; ip<n; ip++) {
            for(iq=ip+1; iq<n; iq++) {
                g = (SWIFT_Real)100.0*fabs(a[ip][iq]);
                if( i>3 && fabs(d[ip])+g==fabs(d[ip]) &&
                    fabs(d[iq])+g==fabs(d[iq])
                ) {
                    a[ip][iq]=0.0;
                } else if( fabs(a[ip][iq])>tresh ) {
                    h = d[iq]-d[ip];
                    if( fabs(h)+g == fabs(h) ) {
                        t=(a[ip][iq])/h;
                    } else {
                        theta=(SWIFT_Real)0.5*h/(a[ip][iq]);
                        t=(SWIFT_Real)(1.0/(fabs(theta)+sqrt(1.0+theta*theta)));
                        if (theta < 0.0) t = -t;
                    }
                    c=(SWIFT_Real)1.0/sqrt(1+t*t);
                    s=t*c;
                    tau=s/((SWIFT_Real)1.0+c);
                    h=t*a[ip][iq];
                    z[ip] -= h;
                    z[iq] += h;
                    d[ip] -= h;
                    d[iq] += h;
                    a[ip][iq]=0.0;
                    for( j = 0; j < ip; j++ ) {
                        Rotate( a, j, ip, j, iq, tau, s, g, h );
                    }
                    for( j = ip+1; j < iq; j++ ) {
                        Rotate( a, ip, j, j, iq, tau, s, g, h );
                    }
                    for( j = iq+1; j < n; j++ ) {
                        Rotate( a, ip, j, iq, j, tau, s, g, h );
                    }
                    for( j = 0; j < n; j++ ) {
                        Rotate( v, j, ip, j, iq, tau, s, g, h );
                    }
                    nrot++;
                }
            }
        }
        for( ip = 0 ; ip < n; ip++ ) {
            b[ip] += z[ip];
            d[ip] = b[ip];
            z[ip] = 0.0;
        }
    }

    cerr << "eigen: too many iterations in Jacobi transform." << endl;
}

///////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////

// This should be able to be called multiple times (at any time)
void Mesh_Utils_Initialize( )
{
    sprintf( options, "qhull Pp QJ i s Tcv" ); 
}

// This routine computes the spread of a closed convex polyhedron.
// The vertices, faces, and center of mass (centroids weighted by area)
// are given.  Resulting from eigen-vectors are returned.
// This algorithm was derived from the SIGGRAPH 96 paper on OBB's.
void Compute_Spread( SWIFT_Array<SWIFT_Tri_Vertex>& vs, int* fs, int fn,
                     bool have_com,
                     SWIFT_Triple& com, SWIFT_Triple& min_dir,
                     SWIFT_Triple& mid_dir, SWIFT_Triple& max_dir )
{
    int i;
    int mine, mide, maxe;
    SWIFT_Real area_x2;
    SWIFT_Real total_area;
    SWIFT_Real s[3];
    SWIFT_Real C[3][3];
    SWIFT_Real E[3][3];
    SWIFT_Triple centroid;
    SWIFT_Triple areav;

    // Create the covariance matrix
    C[0][0] = C[0][1] = C[0][2] = C[1][1] = C[1][2] = C[2][2] = 0.0;

    total_area = 0.0;
    if( have_com ) {
        for( i = 0; i < fn*3; ) {
            const SWIFT_Triple& vx = vs[fs[i++]].Coords();
            const SWIFT_Triple& vy = vs[fs[i++]].Coords();
            const SWIFT_Triple& vz = vs[fs[i++]].Coords();
            areav = (vx - vy) % (vx - vz);
            area_x2 = areav.Length();
            total_area += area_x2;
            centroid = vx + vy + vz;

            C[0][0] += area_x2 * (centroid.X() * centroid.X() +
                        vx.X() * vx.X() + vy.X() * vy.X() + vz.X() * vz.X());
            C[0][1] += area_x2 * (centroid.X() * centroid.Y() +
                        vx.X() * vx.Y() + vy.X() * vy.Y() + vz.X() * vz.Y());
            C[0][2] += area_x2 * (centroid.X() * centroid.Z() +
                        vx.X() * vx.Z() + vy.X() * vy.Z() + vz.X() * vz.Z());
            C[1][1] += area_x2 * (centroid.Y() * centroid.Y() +
                        vx.Y() * vx.Y() + vy.Y() * vy.Y() + vz.Y() * vz.Y());
            C[1][2] += area_x2 * (centroid.Y() * centroid.Z() +
                        vx.Y() * vx.Z() + vy.Y() * vy.Z() + vz.Y() * vz.Z());
            C[2][2] += area_x2 * (centroid.Z() * centroid.Z() +
                        vx.Z() * vx.Z() + vy.Z() * vy.Z() + vz.Z() * vz.Z());
        }
    } else {
        com.Set_Value( 0.0, 0.0, 0.0 );
        for( i = 0; i < fn*3; ) {
            const SWIFT_Triple& vx = vs[fs[i++]].Coords();
            const SWIFT_Triple& vy = vs[fs[i++]].Coords();
            const SWIFT_Triple& vz = vs[fs[i++]].Coords();
            areav = (vx - vy) % (vx - vz);
            area_x2 = areav.Length();
            total_area += area_x2;
            centroid = vx + vy + vz;

            com += area_x2 * (vx + vy + vz);
            C[0][0] += area_x2 * (centroid.X() * centroid.X() +
                        vx.X() * vx.X() + vy.X() * vy.X() + vz.X() * vz.X());
            C[0][1] += area_x2 * (centroid.X() * centroid.Y() +
                        vx.X() * vx.Y() + vy.X() * vy.Y() + vz.X() * vz.Y());
            C[0][2] += area_x2 * (centroid.X() * centroid.Z() +
                        vx.X() * vx.Z() + vy.X() * vy.Z() + vz.X() * vz.Z());
            C[1][1] += area_x2 * (centroid.Y() * centroid.Y() +
                        vx.Y() * vx.Y() + vy.Y() * vy.Y() + vz.Y() * vz.Y());
            C[1][2] += area_x2 * (centroid.Y() * centroid.Z() +
                        vx.Y() * vx.Z() + vy.Y() * vy.Z() + vz.Y() * vz.Z());
            C[2][2] += area_x2 * (centroid.Z() * centroid.Z() +
                        vx.Z() * vx.Z() + vy.Z() * vy.Z() + vz.Z() * vz.Z());
        }
        com /= 3.0 * total_area;
    }

    total_area *= 0.5;
    C[0][0] = C[0][0] / 24.0 - com.X() * com.X() * total_area;
    C[0][1] = C[0][1] / 24.0 - com.X() * com.Y() * total_area;
    C[1][0] = C[0][1];
    C[0][2] = C[0][2] / 24.0 - com.X() * com.Z() * total_area;
    C[2][0] = C[0][2];
    C[1][1] = C[1][1] / 24.0 - com.Y() * com.Y() * total_area;
    C[1][2] = C[1][2] / 24.0 - com.Y() * com.Z() * total_area;
    C[2][1] = C[1][2];
    C[2][2] = C[2][2] / 24.0 - com.Z() * com.Z() * total_area;

    // Do eigen-analysis to find the major/minor axes of the object
    Meigen( E, s, C );

    // Compare the eigen values and sort them
    if (s[0] > s[1]) { maxe = 0; mine = 1; }
    else { mine = 0; maxe = 1; }
    if (s[2] < s[mine]) { mide = mine; mine = 2; }
    else if (s[2] > s[maxe]) { mide = maxe; maxe = 2; }
    else { mide = 2; }

    min_dir = SWIFT_Triple( E[0][mine], E[1][mine], E[2][mine] );
    mid_dir = SWIFT_Triple( E[0][mide], E[1][mide], E[2][mide] );
    max_dir = SWIFT_Triple( E[0][maxe], E[1][maxe], E[2][maxe] );
    // Ensure a rotation matrix
    if( (max_dir % mid_dir) * min_dir < 0.0 ) {
        max_dir.Negate();
    }
}


void Compute_Convex_Hull( coordT* vs, int vn, int*& fs, int& fn )
{
    int i;

    // qhull variables
    int exitcode;
    facetT *facet;
    vertexT *vertex;
    vertexT **vertexp;
    setT *vertices;

    qh_init_A( stdin, stdout, stderr, 0, NULL );
    if( (exitcode = setjmp (qh errexit)) ) exit(exitcode);
    qh_initflags( options );
    qh_init_B( vs, vn, 3, True );
    qh_qhull();
#ifdef SWIFT_DEBUG
    qh_check_output();
#endif

    fs = new int[((vn<<1) + 4)*3];
    fn = 0;
    FORALLfacets {
        setT *vertices = qh_facet3vertex(facet);
        FOREACHvertex_( vertices ) {
            fs[fn++] = qh_pointid(vertex->point);
        }
        // Swap the face vertex indices back
        i = fs[fn-1]; fs[fn-1] = fs[fn-2]; fs[fn-2] = i;
        qh_settempfree(&vertices);
    }
    //qh_freeqhull(qh_ALL);
    qh_freeqhull(!qh_ALL);

    int i1, i2;
    qh_memfreeshort(&i1, &i2);

    fn /= 3;
}

// Compute the convex hull of the list of vertices and returns a list of faces
// that index into those vertices.
void Compute_Convex_Hull( SWIFT_Array<SWIFT_Tri_Vertex*>& vs, int*& fs, int& fn)
{
    int i;
    //coordT *qhv = new coordT[vs.Length()*3];
    coordT *qhv = (coordT*)malloc( sizeof(coordT)*vs.Length()*3 );
    coordT *p = qhv;

    // Load the coordinates into the vertex array for qhull since SWIFT_Real
    // may not be the same type.
    for( i = 0; i < vs.Length(); i++ ) {
        *p++ = vs[i]->Coords().X();
        *p++ = vs[i]->Coords().Y();
        *p++ = vs[i]->Coords().Z();
    }

    Compute_Convex_Hull( qhv, vs.Length(), fs, fn );

    // Do not delete these here since they are deleted in the qh_init_B fcn
    //free( qhv );
    //delete qhv;
}

void Compute_Convex_Hull( SWIFT_Array<SWIFT_Tri_Vertex>& vs, int*& fs, int& fn )
{
    int i;
    //coordT *qhv = new coordT[vs.Length()*3];
    coordT *qhv = (coordT*)malloc( sizeof(coordT)*vs.Length()*3 );
    coordT *p = qhv;

    // Load the coordinates into the vertex array for qhull since SWIFT_Real
    // may not be the same type.
    for( i = 0; i < vs.Length(); i++ ) {
        *p++ = vs[i].Coords().X();
        *p++ = vs[i].Coords().Y();
        *p++ = vs[i].Coords().Z();
    }

    Compute_Convex_Hull( qhv, vs.Length(), fs, fn );

    // Do not delete these here since they are deleted in the qh_init_B fcn
    //free( qhv );
    //delete qhv;
}

void Compute_Convex_Hull( SWIFT_Array<SWIFT_Triple>& vs,
                          SWIFT_Array<SWIFT_Tri_Vertex>& cs, int*& fs, int& fn )
{
    int i;
    //coordT *qhv = new coordT[vs.Length()*3];
    coordT *qhv = (coordT*)malloc( sizeof(coordT)*vs.Length()*3 );
    coordT *p = qhv;

    // Load the coordinates into the vertex array for qhull since SWIFT_Real
    // may not be the same type.
    for( i = 0; i < vs.Length(); i++ ) {
        *p++ = vs[i].X();
        *p++ = vs[i].Y();
        *p++ = vs[i].Z();
        cs[i].Set_Coords( vs[i] );
    }

    Compute_Convex_Hull( qhv, vs.Length(), fs, fn );

    // Do not delete these here since they are deleted in the qh_init_B fcn
    //free( qhv );
    //delete qhv;
}

void Compute_Min_And_Max_Spread( SWIFT_Tri_Mesh* m, SWIFT_BV* p,
                                 SWIFT_Real& min_spread,
                                 SWIFT_Real& max_spread )
{
    int i, j;
    int fn = p->Num_All_Faces();
    int* fs = new int[fn*3];
    for( i = 0, j = 0; i < p->Num_Faces(); i++ ) {
        fs[j++] = m->Vertex_Id( p->Faces()[i].Edge1().Origin() );
        fs[j++] = m->Vertex_Id( p->Faces()[i].Edge2().Origin() );
        fs[j++] = m->Vertex_Id( p->Faces()[i].Edge3().Origin() );
    }
    for( i = 0; i < p->Num_Other_Faces(); i++ ) {
        fs[j++] = m->Vertex_Id( p->Other_Faces()[i]->Edge1().Origin() );
        fs[j++] = m->Vertex_Id( p->Other_Faces()[i]->Edge2().Origin() );
        fs[j++] = m->Vertex_Id( p->Other_Faces()[i]->Edge3().Origin() );
    }

    SWIFT_Triple min_dir, mid_dir, max_dir;
    SWIFT_Tri_Edge* starte;
    Compute_Spread( m->Vertices(), fs, fn, true, p->Center_Of_Mass(),
                    min_dir, mid_dir, max_dir );
    starte = p->Other_Faces().Empty() ? p->Faces()[0].Edge1P()
                                      : p->Other_Faces()[0]->Edge1P();
    min_spread = p->Extremal_Vertex( min_dir, p->Level(), starte );
    max_spread = p->Extremal_Vertex( max_dir, p->Level(), starte );
    min_spread -= p->Extremal_Vertex( -min_dir, p->Level(), starte );
    max_spread -= p->Extremal_Vertex( -max_dir, p->Level(), starte );

    delete fs;
}

void Compute_Spread( SWIFT_Array<SWIFT_Tri_Vertex>& vs, SWIFT_Triple& center,
                     bool compute_spreads,
                     SWIFT_Triple& min_dir, SWIFT_Real& min_spread,
                     SWIFT_Triple& mid_dir, SWIFT_Real& mid_spread,
                     SWIFT_Triple& max_dir, SWIFT_Real& max_spread )
{
    int fn;
    int* fs;

    if( vs.Length() == 3 ) {
        fs = new int[6];
        fs[0] = 0; fs[1] = 1; fs[2] = 2;
        fs[3] = 0; fs[4] = 2; fs[5] = 1;
        fn = 2;
    } else {
        Compute_Convex_Hull( vs, fs, fn );
    }

    Compute_Spread( vs, fs, fn, false, center, min_dir, mid_dir, max_dir );

    if( compute_spreads ) {
        // Now we have compute the spreads.  This can be done by inserting
        // convex hull vertices into an OBB or by doing hill climbing on the
        // convex hull.  We will go for the first option since building the
        // connectivity of the convex hull is too expensive.

        int i;
        SWIFT_Real min_min_spread = SWIFT_INFINITY;
        SWIFT_Real min_mid_spread = SWIFT_INFINITY;
        SWIFT_Real min_max_spread = SWIFT_INFINITY;
        min_spread = -SWIFT_INFINITY;
        mid_spread = -SWIFT_INFINITY;
        max_spread = -SWIFT_INFINITY;

        for( i = 0; i < fn*3; ) {
            const SWIFT_Triple& v1 = vs[fs[i++]].Coords();
            const SWIFT_Triple& v2 = vs[fs[i++]].Coords();
            const SWIFT_Triple& v3 = vs[fs[i++]].Coords();

            const SWIFT_Real min_dot1 = min_dir * v1;
            const SWIFT_Real min_dot2 = min_dir * v2;
            const SWIFT_Real min_dot3 = min_dir * v3;

            const SWIFT_Real mid_dot1 = mid_dir * v1;
            const SWIFT_Real mid_dot2 = mid_dir * v2;
            const SWIFT_Real mid_dot3 = mid_dir * v3;

            const SWIFT_Real max_dot1 = max_dir * v1;
            const SWIFT_Real max_dot2 = max_dir * v2;
            const SWIFT_Real max_dot3 = max_dir * v3;

            Min_And_Max( min_min_spread, min_spread, min_dot1 );
            Min_And_Max( min_min_spread, min_spread, min_dot2 );
            Min_And_Max( min_min_spread, min_spread, min_dot3 );

            Min_And_Max( min_mid_spread, mid_spread, mid_dot1 );
            Min_And_Max( min_mid_spread, mid_spread, mid_dot2 );
            Min_And_Max( min_mid_spread, mid_spread, mid_dot3 );

            Min_And_Max( min_max_spread, max_spread, max_dot1 );
            Min_And_Max( min_max_spread, max_spread, max_dot2 );
            Min_And_Max( min_max_spread, max_spread, max_dot3 );
        }

        // Recompute the center
        center = 0.5 * (SWIFT_Triple( max_spread, mid_spread, min_spread ) +
             SWIFT_Triple( min_max_spread, min_mid_spread, min_min_spread ));

        min_spread -= min_min_spread;
        mid_spread -= min_mid_spread;
        max_spread -= min_max_spread;
    }

    delete fs;
}


void Quicksort( SWIFT_Array<SWIFT_Real>& vals, SWIFT_Array<SWIFT_BV*>& leaves,
                int p, int r )
{
    if( p < r ) {
        // Compute a random element to use as the pivot
        int rn = (int) ((SWIFT_Real)(r-p+1) * drand48()) + p;
        int i = p-1;
        int j = r+1;
        SWIFT_Real x = vals[rn];
        SWIFT_Real tr;
        SWIFT_BV* tp = leaves[rn];

        // Swap the random element into the first position
        vals[rn] = vals[p];
        vals[p] = x;
        leaves[rn] = leaves[p];
        leaves[p] = tp;

        while( true ) {
            j--;
            while( vals[j] > x ) {
                j--;
            }
            i++;
            while( vals[i] < x ) {
                i++;
            }
            if( i < j ) {
                tr = vals[i];
                vals[i] = vals[j];
                vals[j] = tr;
                tp = leaves[i];
                leaves[i] = leaves[j];
                leaves[j] = tp;
            } else {
                break;
            }
        }

        Quicksort( vals, leaves, p, j );
        Quicksort( vals, leaves, j+1, r );
    }
}



