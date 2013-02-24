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
// SWIFT_config.h
//
// Description:
//      Compile time configuration.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SWIFT_CONFIG_H_
#define _SWIFT_CONFIG_H_

#define SWIFTPP_VERSION1_1_02

///////////////////////////////////////////////////////////////////////////////
// SWIFT Compile Configuration
///////////////////////////////////////////////////////////////////////////////

// Set what type of floating point numbers to use in SWIFT.  If SWIFT_USE_FLOAT
// is defined then float's are used, otherwise doubles are used.  Doubles are
// recommended due to higher accuracy.
//#define SWIFT_USE_FLOAT

// If SWIFT_ALWAYS_LOOKUP_TABLE is defined, then a lookup table is always used
// to initialize the distance minimization.  Turn this on if there is no
// temporal coherence that can be taken advantage of by initializing with
// previous feature pairs.
//#define SWIFT_ALWAYS_LOOKUP_TABLE

// Turn one of these on for the resolution of the lookup table to use.  Even
// if SWIFT_ALWAYS_LOOKUP_TABLE is not defined, lookup tables are still created
// (used to initialize in some cases).  So, one of these must always be on.  The
// resolutions are 22.5 degrees, 11.25 degrees, and 5.625 degrees.  The storage
// costs are approximately 0.5 kB, 2 kB, and 8 kB respectively per convex piece
// in the bounding volume hierarchy.  There is not much of a performance
// difference, but in general the highest resolution provides the highest
// performance.
#define SWIFT_LUT_RESOLUTION_22_5
//#define SWIFT_LUT_RESOLUTION_11_25
//#define SWIFT_LUT_RESOLUTION_5_625

// Debugging option.  This may be useful if you are having a problem using the
// system and need to have some automatic checking done on imported geometry
// and certain function calls.  Error messages (if they occur) will be written
// to stderr.
//#define SWIFT_DEBUG

// If you plan on using non-closed polyhedra, then this should be defined.  It
// allows for better contact normal computation.  Note that you will encounter
// undefined behavior if the interior of any of the models contacts any other
// model.  See the user manual for more details.
//#define SWIFT_ALLOW_BOUNDARY

///////////////////////////////////////////////////////////////////////////////
// Optimizations
// For more details on the following optimization settings, see the user manual.
///////////////////////////////////////////////////////////////////////////////

// Define this if the models that are fed into the preprocessor are composed
// solely of triangles.  Furthermore, this allows for reporting of edge ids.
// This alleviates some of the work otherwise involved when an edge is reported.
//#define SWIFT_ONLY_TRIS

#ifdef SWIFT_ONLY_TRIS

// Define this if you want edge ids reported instead of edges reported as the
// two endpoint vertex ids.  Refer to the system documentation to see what the
// edge ids represent.
//# define SWIFT_REPORT_EDGE_IDS

#endif

// Define this to turn on the piece caching optimization.  It is useful for
// intersection, tolerance, and distance queries when there is some temporal
// coherence.
//#define SWIFT_PIECE_CACHING

// Define this to turn on the priority directed search optimization.  It is
// useful for distance queries even when there is no temporal coherence.
//#define SWIFT_PRIORITY_DIRECTION

// Define this to turn on the generalized front tracking optimization.  It is
// useful for all types of queries when there is some temporal coherence.
//#define SWIFT_FRONT_TRACKING

// These are settings that the decomposer app relies on.  Do not change!
#ifdef SWIFT_DECOMP
# undef SWIFT_USE_FLOAT
#endif

///////////////////////////////////////////////////////////////////////////////
// Disable Warnings
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Error Checking
///////////////////////////////////////////////////////////////////////////////


#if !defined(LUT_RESOLUTION_22_5) && !defined(LUT_RESOLUTION_11_25) && !defined(LUT_RESOLUTION_5_625)
# define LUT_RESOLUTION_22_5
#endif


#endif


