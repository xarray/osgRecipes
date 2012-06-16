/*
 TUIO C++ Library - part of the reacTIVision project
 http://reactivision.sourceforge.net/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#ifndef INCLUDED_LIBEXPORT_H
#define INCLUDED_LIBEXPORT_H

#ifdef WIN32

	#pragma warning(disable: 4251) // disable annoying template exporting warnings
	#pragma warning(disable: 4275) // disable warning caused by not exported OSC classes

	#ifdef LIB_EXPORT
		#define LIBDECL __declspec(dllexport)
	#else
		#ifdef LIB_IMPORT
			#define LIBDECL __declspec(dllimport)
		#else
			#define LIBDECL
		#endif
	#endif

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock.h>

#else

	#define LIBDECL

#endif

#endif
